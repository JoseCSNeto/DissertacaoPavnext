/**
* @file slave_X.cpp
* @author José Neto (up201603912@fe.up.pt)
* @brief Slave Communication with the Local-Master
* @version 3.2
* @date 2021-06-12
*
* @copyright Copyright (c) 2021
*
*/

/**
* @brief Slave ID. Only need to change this value in order to be used by different slaves
*
*/
#define MY_SLAVE_ID '2'
/**
* @brief  Local Master ID. Only need to change this value in order to be used in different installations
*
*/
#define MY_LM_ID '1'

#include <Arduino.h>
#include <Wire.h>
#include "communicationMacros.h"
#include "sensorsMacros.h"
#include "powerMacros.h"
#include "flagsMacros.h"
#include "timerMacros.h"
#include <Adafruit_HTS221.h>
#include <Adafruit_Sensor.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <math.h>
#include "usart.h"
#include "CRC16.h"
#include "MCP3221.h"
#include "SparkFun_ADXL345.h"

//functions headers
void readSensors(int Wheel);
void ADXInit(void);
uint16_t ADCRead(uint8_t ch);
void ADCInit();

char helloFromSlave[] = {'S', 'l', 'a', 'v', 'e','#',MY_SLAVE_ID,'\n','\0'};

bool firstFinish = false;;
bool secondFinish = false;

void setup()
{
	delay(4000);
	
    Wire.begin();
    ADCInit();
    uart_init(BAUD_CALC(9600)); // 8n1 transmission is set as default
    sei();                      // enable interrupts
    uart_putstr(helloFromSlave);	
    ADXInit();
	

#ifdef measureCRVoltage
    mcp3221_biela1.setVref(VREF);
    mcp3221_biela2.setVref(VREF);

    if (mcp3221_biela1.ping())
        uart_putstr(failedBiela1);

    if (mcp3221_biela2.ping())
        uart_putstr(failedBiela2);
#endif

#ifdef HUM_RH_MEASURE
    if (!hts.begin_I2C())
    {
        uart_putstr(failedHTS);
    }
#endif
}

void loop()
{
    if (!LocalMasterAccepting || !canSendPacket || !LocalMasterAcceptingFromMe)
    {
        bytes = uart_AvailableBytes();
        if (bytes > 1)
        {
            peek = uart_getc();
            if (peek == LOCAL_MASTER_ACCEPTING_COMMUNICATION)
            {
                peek = uart_getc();
                if (peek == LOCAL_MASTER_ACCEPTING_COMMUNICATION_FROM_ALL_SLAVES)
                {
                    uart_puts("BA");

                    LocalMasterAccepting = true;
                    adxl.ActivityINT(1);
                    EIMSK |= (1 << INT1); // Turns on INT1
                    EIFR = (1 << INTF1);  // Clear interrupt
                }
                else if (peek == MY_SLAVE_ID && LocalMasterAccepting)
                {
                    uart_puts("B2");
                    LocalMasterAcceptingFromMe = true;
                }
            }
            else if (peek == CAN_SEND_PACKET)
            {
                peek = uart_getc();
                if (peek == MY_SLAVE_ID)
                {
                    canSendPacket = true;
                }
            }
        }
    }

    if (readFinished && LocalMasterAcceptingFromMe && !notifiedCarDetection)
    {
        uart_putc(CAR_DETECTED);
        uart_putc(MY_SLAVE_ID);
        readFinished = false;
        notifiedCarDetection = true;
    }

    if (micros() - firstReadEndMicros > 1000000 && firstReadFinish && reEnableInterruptMicros == 0)
    {
        reEnableInterruptMicros = micros();
        adxl.ActivityINT(1);
        adxl.getInterruptSource();

        EIMSK |= (1 << INT1); // Turns on INT1
        EIFR = (1 << INTF1);  // Clear interrupt
    }

    if (carDetectedTrigger)
    {
        carDetectedTrigger = false;
        ADXInterruptSource = adxl.getInterruptSource();

        if (LocalMasterAccepting && !acquisitionFinished)
        {
            uart_puts("VB");
            acquisitionFinished++;
            sensorsReadingsReady = false;

            adxl.ActivityINT(0);
            EIFR = (1 << INTF1); // Clear interrupt
            EIMSK = (0 << INT1); // Turns off INT1
            Wheel++;
            timeSinceLastRead = micros();
            canRead = true;

            if (firstReadFinish)
            {

                firstGenFinish = false;
                secondGenFinish = false;

                secondReadStart = true;
                secondDetectionMicros = micros();
                intervalBetweenDetections = micros() - firstDetectionMicros;
                speed = (WHEELBASE/100)  / (intervalBetweenDetections / 1000000);
            }
            else
            {
                firstDetectionMicros = micros();
                firstReadStart = true;
            }
        }
    }

    if (!sensorsReadingsReady)
    {
        readSensors(Wheel);
    }

    if (canSendPacket && LocalMasterAcceptingFromMe && sensorsReadingsReady)
    {
        //dataPacket = [SLAVE_ID][DEST_ID][DATA][CRC:MSB|LSB]

        maxXAccel = maxXAccel * 5;
        maxYAccel = maxYAccel * 5;
        maxZAccel = maxZAccel * 5;
        speed = speed * 100;

#ifdef measureCRVoltage
        maxVoltageCR1 = (float)maxVoltageCR1 * 0.05;
        maxVoltageCR2 = (float)maxVoltageCR2 * 0.05;
        car_weight_biela1 = pow(maxVoltageCR1, 3) * 405816.33 - pow(maxVoltageCR1, 2) * 3294162.89 + maxVoltageCR1 * 8927838.36 - 8077416.56;
        car_weight_biela2 = pow(maxVoltageCR2, 3) * 3337.84 - pow(maxVoltageCR2, 2) * 19838.71 + maxVoltageCR2 * 44620.52 - 33344.6;
#endif

        if (maxXAccel > 255)
            maxXAccel = 255;
        if (maxYAccel > 255)
            maxYAccel = 255;
        if (maxZAccel > 255)
            maxZAccel = 255;

        if (speed > 255)
            speed = 255;

        if (power_I > 255)
            power_I = 255;
        if (energy_I > 255)
            energy_I = 255;
        if (power_II > 255)
            power_II = 255;
        if (energy_II > 255)
            energy_II = 255;

#ifdef HUM_RH_MEASURE
        if (!carCount || (carCount % 2 == 0))
        { //measure temperature and rh every 10 cars
            hts.getEvent(&humidity, &temp);
            temperature = (int)temp.temperature + 100; //added 100 because of negative values
            rh = (int)humidity.relative_humidity * 2;
        }
#else

        temperature = 50;
        rh = 88;
#endif

        carCount++;

        dataPacket[0] = MY_SLAVE_ID;
        dataPacket[1] = MY_LM_ID;
        dataPacket[2] = (int)temperature;
        dataPacket[3] = (int)rh;
        dataPacket[4] = (int)maxXAccel;
        dataPacket[5] = (int)maxYAccel;
        dataPacket[6] = (int)maxZAccel;
        dataPacket[7] = (int)speed;
        dataPacket[8] = (int)avgPower_I;
        dataPacket[9] = (int)energy_I;
        dataPacket[10] = (int)avgPower_II;
        dataPacket[11] = (int)energy_II;

        count = 0;
        acquisitionFinished = 0;
        maxXAccel = 0;
        maxYAccel = 0;
        maxZAccel = 0;
        speed = 0;
        maxVoltageCR1 = 0;
        maxVoltageCR2 = 0;
        power_I = 0;
        avgPower_I = 0;
        energy_I = 0;
        power_II = 0;
        avgPower_II = 0;
        energy_II = 0;

        calculatedCRC = calc_crc(dataPacket, DATA_SIZE); //Calculate CRC

        dataPacket[DATA_SIZE] = (calculatedCRC >> 8);     //MSB
        dataPacket[DATA_SIZE + 1] = calculatedCRC & 0xff; //LSB -> last byte -> DATA_SIZE + 1 = last position

        uart_putstrl(dataPacket, sizeof(dataPacket)); // dataPacket = [SLAVE_ID][DEST_ID][DATA][CRC:MSB|LSB]

        canSendPacket = false;
        LocalMasterAccepting = false;
        LocalMasterAcceptingFromMe = false;
    }
} //loop
/**
* @brief Reads the values from different sensors in order to build a packet to send to the LM					
*
* @param Wheel Which wheel/half of the car was detected
*/
void readSensors(int Wheel)
{
    if ((micros() - timeSinceLastRead) >= readTimeout && canRead && (firstReadStart || secondReadStart))
    {
        timeSinceLastRead = micros();
        betweenReadingsTimer = micros() - timeSinceLastRead;

        adxl.readAccel(&x, &y, &z); // Read the accelerometer values and store them in variables declared above x,y,z

        measuredXAccel = x * factor;
        measuredYAccel = y * factor;
        measuredZAccel = z * factor;

#ifdef measureCRVoltage
//CR1Voltage = mcp3221_biela1.getVoltage();
//CR2Voltage = mcp3221_biela2.getVoltage();
#endif

        voltage_V2 = ADCRead(0);
        //voltage_V2 = (Voltage / 1024) * 50; //*50 com divisor //multiplicar por 10 devido a divisor resistivo

        voltage_V1 = ADCRead(1);
        //voltage_V1 = (Voltage / 1024) * 50; //*50 com divisor //multiplicar por 10 devido a divisor resistivo

        current_I2 = ADCRead(2);
       // current_I2 = (6.1597 * ((Voltage / 1024.0) * 5) - 15.495);

        current_I1 = ADCRead(3);
        //current_I1 = (6.1597 * ((Voltage / 1024.0) * 5) - 15.495);

        if (!count)
        {
#ifdef measureCRVoltage
            maxVoltageCR1 = CR1Voltage;
            maxVoltageCR2 = CR2Voltage;
#endif

            maxXAccel = measuredXAccel;
            maxYAccel = measuredYAccel;
            maxZAccel = measuredZAccel;
        }

        if (voltage_V1 <= vLimit && count > 30)
        {
            V1NullCounter++;

            if (V1NullCounter == vNullCounterMax)
            {
                if (!firstReadFinish)
                {
                    acquisitionFinished = 0;
                    canRead = false;
                    firstReadEndMicros = micros();
                    carDetectedTrigger = false;
                        firstReadFinish = true;
                }
                if (secondReadStart)
                {
                        secondReadFinish = true;
                }
            }
        }

        else
        {
            V1NullCounter = 0;
        }

        if (voltage_V2 <= vLimit && count > 30)
        {
            V2NullCounter++;

            if (V2NullCounter == vNullCounterMax)
            {
                if (!firstReadFinish)
                {
                    acquisitionFinished = 0;
                    canRead = false;
                    firstReadEndMicros = micros();
                    carDetectedTrigger = false;
                        firstReadFinish = true;
                }
                if (secondReadStart)
                {
                        secondReadFinish = true;
                }
            }
        }

        else
        {
            V2NullCounter = 0;
        }

#ifdef measureCRVoltage
        maxVoltageCR1 = (CR1Voltage > maxVoltageCR1) ? CR1Voltage : maxVoltageCR1;
        maxVoltageCR2 = (CR2Voltage > maxVoltageCR2) ? CR2Voltage : maxVoltageCR2;
#endif

        maxXAccel = (measuredXAccel > maxXAccel) ? measuredXAccel : maxXAccel;
        maxYAccel = (measuredYAccel > maxYAccel) ? measuredYAccel : maxYAccel;
        maxZAccel = (measuredZAccel > maxZAccel) ? measuredZAccel : maxZAccel;

        power_I = (double)(voltage_V1/ 1024.0) * 50 * (6.1597*((current_I1/ 1024.0)*5) - 15.495);
        power_II = (double)(voltage_V2/ 1024.0) * 50 * (6.1597*((current_I2/ 1024.0)*5) - 15.495);

        avgPower_I += power_I;
        avgPower_I += power_II;

        energy_I += power_I * betweenReadingsTimer * 0.000001;
        energy_II += power_II * betweenReadingsTimer * 0.000001;

        count++;

        if (count == countMax || secondReadFinish)
        {

            readFinished = true;
            notifiedCarDetection = false;
            sensorsReadingsReady = true;

            count = 0;
            acquisitionFinished = 0;

            firstGenFinish = false;
            secondGenFinish = false;

            firstReadStart = false;
            firstReadFinish = false;
            secondReadStart = false;
            secondReadFinish = false;

            intervalBetweenDetections = 0;
            reEnableInterruptMicros = 0;

            avgPower_I = (avgPower_I / count) * 10;
            avgPower_II = (avgPower_II / count) * 10;

            energy_I = energy_I * 10;
            energy_II = energy_II * 10;

            if (Wheel >= 2)
                Wheel = 0;
        }
    }
}
/**
		* @brief Interrupt generated by the accelerometer - Car detected
		*
		*/
ISR(INT1_vect)
{
    carDetectedTrigger = true;
}
/**
		* @brief Accelerometer ADX345 initialization
		*
		*/
void ADXInit()
{

    Wire.beginTransmission(ADX345_ID);
    byte error = Wire.endTransmission();
    if (error)
    {
        uart_putstr(failedADX);
    }

    else if (!error)
    {
        adxl.powerOn();                                   // Power on the ADXL345
        adxl.setRangeSetting(16);                         // Give the range settings
        adxl.setActivityXYZ(0, 0, 1);                     // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
        adxl.setActivityThreshold(40);                    // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
        adxl.setImportantInterruptMapping(1, 1, 1, 1, 1); // Sets "adxl.setEveryInterruptMapping(single tap, double tap, free fall, activity, inactivity);"
        adxl.InactivityINT(0);
        adxl.ActivityINT(0);
        adxl.FreeFallINT(0);
        adxl.doubleTapINT(0);
        adxl.singleTapINT(0);
        adxl.setRate(1600);

        EIFR = (1 << INTF1);  //clear interrupt
        DDRD &= ~(1 << DDD3); // Clear the PD3 pin
        // PD3 (PCINT1 pin) is now an input
        PORTD |= (1 << PORTD3); // turn On the Pull-up
        // PD3 is now an input with pull-up enabled
        EICRA |= (1 << ISC10) | (1 << ISC11);
        // set INT1 to trigger on RISING logic change
        //EIMSK |= (1 << INT1); // Turns on INT1
        //pinMode(interruptPin, INPUT_PULLUP);
        byte interrupts = adxl.getInterruptSource();
    }
}

/**
		* @brief ADC Initialization (based on http://maxembedded.com/2011/06/the-adc-of-the-avr)
		*
		*/
void ADCInit()
{
    // AREF = AVcc
    ADMUX = (1 << REFS0);

    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}
/**
		* @brief Reads the ADC channel 'ch' (based on https://embedds.com/adc-on-atmega328-part-1)
		*
		* @param ch Which channel to read (0-7)
		* @return uint16_t ADC value in mV
		*/
uint16_t ADCRead(uint8_t ch)
{
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    if (ch < 0 || ch > 7)
    {             //ADC0 - ADC7 is available
        return 1; //pin number is out of range
    }

    /* ADMUX's first few bits are
		* the binary representations of the numbers of the pins so we can
		* just 'OR' the pin's number with ADMUX to select that pin.
		* We first zero the four bits by setting ADMUX equal to its higher
		* four bits. */
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);

    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1 << ADSC);

    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while (ADCSRA & (1 << ADSC))
        ;

    return (ADC);
}