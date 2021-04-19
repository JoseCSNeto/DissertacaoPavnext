/**
 * @file slave1_adx.cpp
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Slave Communication with the Local-Master
 * @version 1.2
 * @date 2021-03-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <Arduino.h>
#include <Wire.h>
#include "communicationMacros.h"
#include "sensorsMacros.h"
#include <Adafruit_HTS221.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>
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

/**
 * @brief Slave ID. Only need to change this value in order to be used by different boards
 * 
 */
#define MY_SLAVE_ID '1'
/**
 * @brief  Local Master ID. Only need to change this value in order to be used by different boards
 * 
 */
#define MY_LM_ID '1'
//Sensors
/**
 * @brief  Class that stores state and functions for interacting with
 *            the HTS221 I2C Digital Potentiometer (from the original documentation)
 * 
 */
Adafruit_HTS221 hts;
/**
 * @brief  Sensor event (36 bytes). struct sensor_event_s is used to provide a single sensor event in a common format. (from the original documentation)
 * 
 */
sensors_event_t temp, humidity;

/**
 * @brief ADXL345 struct
 * 
 */
ADXL345 adxl = ADXL345(); // USE FOR I2C COMMUNICATION
/**
 * @brief Accelerometer readings
 * 
 */
int x, y, z;
/**
 * @brief Calibration factor for z-axis readings
 * 
 */
double factor = 0.30625;
/**
 * @brief Accelerometer interrupt trigger
 * 
 */
bool carDetectedTrigger = false;
/**
 * @brief Used when clearing the accelerometer's interrupt by reading its source
 * 
 */
byte ADXInterruptSource;

/**
 * @brief I2C address of the ADX345
 * 
 */
const byte ADX345_ID = 0x53;
/**
 * @brief I2C address of the MCP3221 A0
 * 
 */
const byte BIELA1_ADDR = 0x48;
/**
 * @brief I2C address of the MCP3221 A5
 * 
 */
const byte BIELA2_ADDR = 0x4D;
/**
 * @brief MCP3221 class
 * 
 * @return MCP3221 
 */
MCP3221 mcp3221_biela1(BIELA1_ADDR), mcp3221_biela2(BIELA2_ADDR);
/**
 * @brief Stores the calculated "biela" voltage
 * 
 */
double Biela1Voltage, Biela2Voltage;
/**
 * @brief "Biela"'s reference voltage
 * 
 */
#define VREF 4999

//COMMUNICATION

//Variables
/**
 * @brief Next byte in RS485's line. Doesn't actually retrieve it from the there
 * 
 */
uint8_t peek;
/**
 * @brief Data packet to be sent to the Local-Master
 * 
 */
char dataPacket[DATA_SIZE_CRC];
/**
 * @brief Calculated CRC for the data packet
 * 
 */
uint16_t calculatedCRC;
/**
 * @brief Available RS485 bytes
 * 
 */
uint8_t bytes = 0;

/**
 * @brief Message sent when "biela1" isn't detected when initializing it
 * 
 */
char failedBiela1[] = "BIELA1_FAILED\n";
/**
 * @brief Message sent when "biela2" isn't detected when initializing it
 * 
 */
char failedBiela2[] = "BIELA2_FAILED\n";
/**
 * @brief Message sent when the temperature sensor isn't detected when initializing it
 * 
 */
char failedHTS[] = "HTS_FAILED\n";
/**
 * @brief Message sent when the accelerometer isn't detected when initializing it
 * 
 */
char failedADX[] = "ADX_FAILED\n";

//FLAGS
/**
 * @brief Flag that tells the slave if the master is accepting communication
 * 
 */
bool LocalMasterAccepting = false;
/**
 * @brief Flag that tells the slave if the master is expecting a packet
 * 
 */
bool canSendPacket = false;
/**
 * @brief Counter for the number of readings before sending a packet (when count = 10 the packet is data is ready to be sent)
 * 
 */
int count = 0;

/**
 * @brief Flag that signals if the slave has completed the 10 readings
 * 
 */
bool sensorsReadingsReady = true;
/**
 * @brief Flag that signals if the relative humidity is too high
 * 
 */
bool highRelativeHumidity = false;
/**
 * @brief Flag that signals if the temperature is too high
 * 
 */
bool highTemperature = false;
/**
 * @brief Detects which car wheel/half triggered the slave 
 * 
 */
int Wheel = 0;
/**
 * @brief How many cars have been detected
 * 
 */
int carCount = 0;

// Power System
/**
 * @brief Voltage of ADC #0 (Power System V2)
 * 
 */
double voltage_V2 = 0;
/**
 * @brief Voltage of ADC #1 (Power System V1)
 * 
 */
double voltage_V1 = 0;
/**
 * @brief Voltage of ADC #2 (Power System I2)
 * 
 */
double current_I2 = 0;
/**
 * @brief Voltage of ADC #3 (Power System I1)
 * 
 */
double current_I1 = 0;
/**
 * @brief Aux variable for ADC readings
 * 
 */
double Voltage;
/**
 * @brief Calculated power of system #1
 * 
 */
double power_I = 0;
/**
 * @brief Calculated energy of system #1
 * 
 */
double energy_I = 0;
/**
 * @brief Calculated power of system #2
 * 
 */
double power_II = 0;
/**
 * @brief Calculated energy of system #2
 * 
 */
double energy_II = 0;
/**
 * @brief Maximum value of voltage detected in "biela1"
 * 
 */
double maxVoltageBiela1 = 0;
/**
 * @brief Maximum value of voltage detected in "biela2"
 * 
 */
double maxVoltageBiela2 = 0;
/**
 * @brief Measured Z-axis acceleration
 * 
 */
float measuredZAccel = 0;
/**
 * @brief Maxiumum Z-axis acceleration
 * 
 */
float maxZAccel = 0;

//timers
/**
 * @brief Counts the elapsed microsonds since the wheel's detection. Used in the power system calculations
 * 
 */
unsigned long timeStart;
/**
 * @brief Elapsed microsonds since the last reading after the wheel detection (readings every 5 ms)
 * 
 */
unsigned long wheelStart[2];
/**
 * @brief Counts the elapsed microsonds since the wheel detection. Used in the power system calculations
 * 
 */
unsigned long wheelTimer[2];
/**
 * @brief Number of seconds to read the temp/rh values
 * 
 */
#define HTSTimer 60
/**
 * @brief Counts the elapsed microsonds since the last temperature/RH readings. Currently reading every minute
 * 
 */
unsigned long HTSReadings;

//DEFAULT MESSAGES
/**
 * @brief Pre-defined message for 'HRX' -> notifies the RH is too high
 * 
 */
char HIGH_RLTV_HUMDT[ALERT_MESSAGES_SIZE] = {HIGH_VALUE, RLTV_HUMDT, MY_SLAVE_ID};
/**
 * @brief Pre-defined message for 'NRX' -> notifies the RH is back to normal
 * 
 */
char NORMAL_RLTV_HUMDT[ALERT_MESSAGES_SIZE] = {NORMAL_VALUE, RLTV_HUMDT, MY_SLAVE_ID};
/**
 * @brief Pre-defined message for 'HTX' -> notifies the temperature is too high
 * 
 */
char HIGH_TEMP[ALERT_MESSAGES_SIZE] = {HIGH_VALUE, TEMPERATURE, MY_SLAVE_ID};
/**
 * @brief Pre-defined message for 'NTX' -> notifies the temperature is back to normal
 * 
 */
char NORMAL_TEMP[ALERT_MESSAGES_SIZE] = {NORMAL_VALUE, TEMPERATURE, MY_SLAVE_ID};

//new

float maxZ = 0;
int readTimeout = 0;
bool readAccel = false;
int countMaxZ = 0;
#define countMax 50

//functions headers
void readSensors(int Wheel);
void ADXInit(void);
uint16_t ADCRead(uint8_t ch);
void ADCInit();

/**
 * @brief Initialization of sensors, communication, timers and interrupts
 * 
 * @return int 
 */
int main(void)
{
    Wire.begin();
    ADCInit();
    uart_init(BAUD_CALC(4800)); // 8n1 transmission is set as default
    sei();                      // enable interrupts
    //uart_putstr("hello from slave");
    //for (int i = 0; i<1000;i++)
    //_delay_ms(1);

    /*mcp3221_biela1.setVref(VREF);
	mcp3221_biela2.setVref(VREF);
	
	if(mcp3221_biela1.ping())
		uart_putstr(failedBiela1);
		
	if(mcp3221_biela2.ping())
		uart_putstr(failedBiela2);
   

	if (!hts.begin_I2C()) {
		uart_putstr(failedHTS);
	}*/

    ADXInit();
    //HTSReadings = micros();

    while (1)
    {
        /*
        if (micros() - HTSReadings > HTSTimer * 1000)
        {
            HTSReadings = micros();
            hts.getEvent(&humidity, &temp);

            if (humidity.relative_humidity > MAX_RLTV_HUMD)
            {
                highRelativeHumidity = true;
                if (LocalMasterAccepting)
                    uart_putstrl(HIGH_RLTV_HUMDT, ALERT_MESSAGES_SIZE);
            }
            if (temp.temperature > MAX_TEMP)
            {
                highTemperature = true;
                if (LocalMasterAccepting)
                    uart_putstrl(HIGH_TEMP, ALERT_MESSAGES_SIZE);
            }

            if (humidity.relative_humidity < MAX_RLTV_HUMD && highRelativeHumidity)
            {
                highRelativeHumidity = false;
                if (LocalMasterAccepting)
                    uart_putstrl(NORMAL_RLTV_HUMDT, ALERT_MESSAGES_SIZE);
            }
            if (temp.temperature < MAX_TEMP && highTemperature)
            {
                highTemperature = false;
                if (LocalMasterAccepting)
                    uart_putstrl(NORMAL_TEMP, ALERT_MESSAGES_SIZE);
            }
        }
        */

        if (!LocalMasterAccepting || !canSendPacket)
        {
            bytes = uart_AvailableBytes();
            if (bytes > 1)
            {
                peek = uart_getc();
                if (peek == LOCAL_MASTER_ACCEPTING_COMMUNICATION)
                {
                    peek = uart_getc();
                    if (peek == MY_SLAVE_ID)
                    {
                        LocalMasterAccepting = true;
                        adxl.ActivityINT(1);
                        EIMSK |= (1 << INT1); // Turns on INT1
                        EIFR = (1 << INTF1);  // Clear interrupt
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

        if (carDetectedTrigger)
        {
            carDetectedTrigger = false;
            ADXInterruptSource = adxl.getInterruptSource();

            if (LocalMasterAccepting)
            {
                uart_putc(CAR_DETECTED);
                uart_putc(MY_SLAVE_ID);
                LocalMasterAccepting = false;

                sensorsReadingsReady = false;

                adxl.ActivityINT(0);
                EIFR = (1 << INTF1); // Clear interrupt
                EIMSK = (0 << INT1); // Turns off INT1
                wheelStart[Wheel] = micros();
                Wheel++;
                timeStart = micros();
                readTimeout = 1600;
                readAccel = true;
            }
        }

        if (!sensorsReadingsReady)
        {
            readSensors(Wheel);
        }

        if (canSendPacket && sensorsReadingsReady)
        {
            //uart_putc(START_PACKET);
            // dataPacket = [SLAVE_ID][DEST_ID][DATA][CRC:MSB|LSB]
            maxVoltageBiela1 = (float)maxVoltageBiela1 * 0.05;
            maxVoltageBiela2 = (float)maxVoltageBiela2 * 0.05;
            maxZAccel = maxZAccel * 10;
            measuredZAccel = measuredZAccel * 10;

            //	car_weight_biela1 = pow(maxVoltageBiela1,3)*405816.33 - pow(maxVoltageBiela1,2)*3294162.89 + maxVoltageBiela1*8927838.36 - 8077416.56;
            //car_weight_biela2 = pow(maxVoltageBiela2,3)*3337.84 - pow(maxVoltageBiela2,2)*19838.71 + maxVoltageBiela2*44620.52 - 33344.6;
            if (power_I > 255)
                power_I = 255;
            if (energy_I > 255)
                energy_I = 255;
            if (power_II > 255)
                power_II = 255;
            if (energy_II > 255)
                energy_II = 255;

            dataPacket[0] = MY_SLAVE_ID;
            dataPacket[1] = MY_LM_ID;
            dataPacket[2] = (int)maxZAccel;
            dataPacket[3] = 43;
            dataPacket[4] = (int)measuredZAccel;
            dataPacket[5] = (int)power_I;
            dataPacket[6] = (int)energy_I;
            dataPacket[7] = (int)power_II;
            dataPacket[8] = (int)energy_II;

            maxZAccel = 0;
            maxVoltageBiela1 = 0;
            maxVoltageBiela2 = 0;
            power_I = 0;
            energy_I = 0;
            power_II = 0;
            energy_II = 0;

            calculatedCRC = calc_crc(dataPacket, DATA_SIZE); //Calculate CRC

            dataPacket[DATA_SIZE] = (calculatedCRC >> 8);     //MSB
            dataPacket[DATA_SIZE + 1] = calculatedCRC & 0xff; //LSB -> last byte -> DATA_SIZE + 1 = last position

            uart_putstrl(dataPacket, sizeof(dataPacket)); // dataPacket = [SLAVE_ID][DEST_ID][DATA][CRC:MSB|LSB]

            canSendPacket = false;
            LocalMasterAccepting = false;
        }
        if (Wheel == 1 && (micros() - wheelStart[0] >= 5000))
        {
            Wheel = 0;
            carCount--;
        }
    } //while1
} //main
/**
 * @brief Reads the values from different sensors in order to build a packet to send to the LM
 * 
 * @param Wheel Which wheel/half of the car was detected
 */
void readSensors(int Wheel)
{

    if (micros() - wheelStart[Wheel - 1] >= readTimeout)
    {
        wheelStart[Wheel - 1] = micros(); //reset the timer
        //hts.getEvent(&humidity, &temp);
        if (readAccel)
            adxl.readAccel(&x, &y, &z); // Read the accelerometer values and store them in variables declared above x,y,z
        else
            z = 0;
        measuredZAccel = z * factor;

        if (measuredZAccel > maxZ)
        {
            maxZ = measuredZAccel;
            countMaxZ = 0;
            readAccel = true;
        }

        else
        {
            countMaxZ++;
            if (countMaxZ >= 10)
            {
                countMaxZ = 0;
                readTimeout = 1000;
                readAccel = false;
            }
        }

        //Biela1Voltage = mcp3221_biela1.getVoltage();
        //Biela2Voltage = mcp3221_biela2.getVoltage();

        Voltage = ADCRead(0);
        voltage_V2 = (Voltage / 1024) * 5; //*50 com divisor //multiplicar por 10 devido a divisor resistivo

        Voltage = ADCRead(1);
        voltage_V1 = (Voltage / 1024) * 5; //*50 com divisor //multiplicar por 10 devido a divisor resistivo

        Voltage = ADCRead(2);
        current_I2 = (Voltage / 1024) * 5;

        Voltage = ADCRead(3);
        current_I1 = (Voltage / 1024) * 5;

        if (!count)
        {
            maxVoltageBiela1 = Biela1Voltage;
            maxVoltageBiela2 = Biela2Voltage;
            maxZAccel = measuredZAccel;
        }

        maxVoltageBiela1 = (Biela1Voltage > maxVoltageBiela1) ? Biela1Voltage : maxVoltageBiela1;
        maxVoltageBiela2 = (Biela2Voltage > maxVoltageBiela2) ? Biela2Voltage : maxVoltageBiela2;
        maxZAccel = (measuredZAccel > maxZAccel) ? measuredZAccel : maxZAccel;

        power_I += (double)(voltage_V1 * current_I1);
        power_II += (double)(voltage_V2 * current_I2);

        count++;

        if (count == countMax)
        {
            sensorsReadingsReady = true;
            count = 0;
            wheelTimer[Wheel - 1] = micros() - timeStart;
            energy_I = (double)power_I * wheelTimer[Wheel - 1] / 10000;
            energy_II = (double)power_II * wheelTimer[Wheel - 1] / 10000;
            if (Wheel == 2)
                Wheel = 0;
            carCount++;
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