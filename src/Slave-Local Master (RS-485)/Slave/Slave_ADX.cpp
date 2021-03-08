/**
 * @file slave1_adx.cpp
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Slave Communication with the Local-Master
 * @version 1.1
 * @date 2021-03-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <Arduino.h>
#include <Wire.h>
#include "communicationMacros.h"
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
double factor = 0.0383072;
/**
 * @brief Accelerometer interrupt trigger
 * 
 */
bool trigger = false;
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
 * @brief Flag that signals if the Relative Humidity is too high
 * 
 */
bool highRelativeHumidity = false;
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
 * @brief Variable used in the milliSec function
 * 
 */
volatile unsigned long timer1_millis;
/**
 * @brief Counts the elapsed milliseconds since the car detection. Used in the power system calculations
 * 
 */
unsigned long timeStart;
/**
 * @brief Elapsed milliseconds since the last reading after the first wheel detection (readings every 5 ms)
 * 
 */
unsigned long FirstWheelStart;
/**
 * @brief Counts the elapsed milliseconds since the first wheel detection. Used in the power system calculations
 * 
 */
unsigned long timer1stWheel;
/**
 * @brief Elapsed milliseconds since the last reading after the second wheel detection (readings every 5 ms)
 * 
 */
unsigned long ScndWheelStart;
/**
 * @brief Counts the elapsed milliseconds since the second wheel detection. Used in the power system calculations
 * 
 */
unsigned long timer2ndWheel;
/**
 * @brief Counts the elapsed milliseconds since the last temperature/RH readings. Currently reading every 5 sec
 * 
 */
unsigned long HTS_readings;

//DEFAULT MESSAGES
/**
 * @brief Pre-defined message for 'CX' -> notifies the LM that a car was detected
 * 
 */
char CAR_DETECTED_SV1[DEFAULT_MESSAGES_SIZE] = {CAR_DETECTED, MY_SLAVE_ID};            // C1-> car detected in slave1
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

//functions headers
void initMilliS(unsigned long f_cpu);
unsigned long milliSec();
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
    initMilliS(F_CPU);         //frequency the atmega328p is running at
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
    //HTS_readings = milliSec();

    while (1)
    {

        /*	if(milliSec()-HTS_readings > 5000)
		{
			HTS_readings = milliSec();
			hts.getEvent(&humidity, &temp);

			if(humidity.relative_humidity > 80)
			{
				highRelativeHumidity = true;
				if(LocalMasterAccepting)
				uart_putstrl(HIGH_RLTV_HUMDT,ALERT_MESSAGES_SIZE);
			}

			else if(humidity.relative_humidity < 80 && highRelativeHumidity)
			{
				highRelativeHumidity = false;
				if(LocalMasterAccepting)
				uart_putstrl(NORMAL_RLTV_HUMDT,ALERT_MESSAGES_SIZE);
			}
		}*/

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

        if (trigger)
        {
            trigger = false;
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
                if (Wheel == 0)
                {
                    Wheel = 1;
                    FirstWheelStart = milliSec();
                    carCount++;
                }

                else if (Wheel == 1)
                {
                    Wheel = 2;
                    ScndWheelStart = milliSec();
                }

                else if (Wheel == 2)
                {
                    Wheel = 1;
                    FirstWheelStart = milliSec();
                }
                timeStart = milliSec();
            }
        }

        if (!sensorsReadingsReady)
        {
            if (Wheel == 1)
            {
                if (milliSec() - FirstWheelStart >= 5)
                {
                    //hts.getEvent(&humidity, &temp);
                    adxl.readAccel(&x, &y, &z); // Read the accelerometer values and store them in variables declared above x,y,z
                    measuredZAccel = z * factor;

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

                    FirstWheelStart = milliSec(); //reset the timer
                    count++;

                    if (count == 10)
                    {
                        sensorsReadingsReady = true;
                        count = 0;
                        timer1stWheel = milliSec() - timeStart;
                        energy_I = (double)power_I * timer1stWheel / 1000;
                        energy_II = (double)power_II * timer1stWheel / 1000;
                    }
                }
            }

            else if (Wheel == 2)
            {
                if (milliSec() - ScndWheelStart >= 5)
                {
                    //hts.getEvent(&humidity, &temp);
                    adxl.readAccel(&x, &y, &z); // Read the accelerometer values and store them in variables declared above x,y,z
                    measuredZAccel = z * factor;

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

                    ScndWheelStart = milliSec(); //reset the timer
                    count++;

                    if (count == 10)
                    {
                        sensorsReadingsReady = true;
                        count = 0;
                        timer2ndWheel = milliSec() - timeStart;
                        energy_I = (double)power_I * timer2ndWheel / 1000;
                        energy_II = (double)power_II * timer2ndWheel / 1000;
                    }
                }
            }
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
            dataPacket[1] = LOCAL_MASTER_ID;
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
        if (Wheel == 1 && (milliSec() - FirstWheelStart >= 5000))
        {
            Wheel = 0;
            carCount--;
        }
    } //while1
} //main
/**
 * @brief Interrupt generated by the accelerometer - Car detected
 * 
 */
ISR(INT1_vect)
{
    trigger = true;
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
        adxl.setRangeSetting(2);                          // Give the range settings
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
 * @brief Timer interrupt
 * 
 */
ISR(TIMER1_COMPA_vect)
{
    timer1_millis++;
}
/**
 * @brief Initializes the function that counts the elapsed seconds
 * 
 * @param f_cpu CPU's clock frequency
 */
void initMilliS(unsigned long f_cpu)
{
    unsigned long ctc_match_overflow;

    ctc_match_overflow = ((f_cpu / 1000) / 8); //when timer1 is this value, 1ms has passed

    // (Set timer to clear when matching ctc_match_overflow) | (Set clock divisor to 8)
    TCCR1B |= (1 << WGM12) | (1 << CS11);

    // high byte first, then low byte
    OCR1AH = (ctc_match_overflow >> 8);
    OCR1AL = ctc_match_overflow;

    // Enable the compare match interrupt
    TIMSK1 |= (1 << OCIE1A);

    //REMEMBER TO ENABLE GLOBAL INTERRUPTS AFTER THIS WITH sei(); !!!
}
/**
 * @brief Counts the elapsed seconds
 * 
 * @return unsigned long elapsed seconds
 */
unsigned long milliSec()
{
    unsigned long millis_return;

    // Ensure this cannot be disrupted
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        millis_return = timer1_millis;
    }
    return millis_return;
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
// read adc value
//https://hekilledmywire.wordpress.com/2011/03/16/using-the-adc-tutorial-part-5/
//https://www.xanthium.in/atmega328p-10bit-sar-adc-usart-serial-transmit-to-pc-data-acquisition-tutorial
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
