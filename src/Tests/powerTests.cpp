#include <Arduino.h>
#include <Wire.h>
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

int16_t voltage_V2 = 0;
//int16_t voltage_V1 = 0;
int16_t current_I2 = 0;
//int16_t current_I1 = 0;
double Voltage;

#define countMax 260
int16_t voltage_V2_array[countMax] = {0};
//int16_t voltage_V1_array[countMax] = {0};
int16_t current_I2_array[countMax] = {0};
//int16_t current_I1_array[countMax] = {0};
//int16_t zMeasures[countMax] = {0};

//double power_I = 0;
//double energy_I = 0;
double power_II = 0;
double energy_II = 0;

unsigned long timeSinceLastRead;
unsigned long acquisitonDuration;
unsigned long timeStart;
float measuredZAccel = 0;
float maxZ = 0;
int readTimeout = 0;
int stop = countMax;
unsigned long timer = 0;
int x, y, z;

#define vLimit 4
#define V2NullMax 6
int V1Null = 0;
int V2Null = 0;

bool isV1Null = false;
bool isV2Null = false;

bool canRead = true;

volatile int carsDetected = 0;
int count3 = 0;
volatile int newCarCount = 0;
long stopReadingAccel = 0;

int firstReadCount = 0;
bool firstReadStart = false;
bool firstReadFinish = false;
bool secondReadStart = false;
bool secondReadFinish = false;

unsigned long firstReadEndMicros = 0;
unsigned long firstReadDuration = 0;
unsigned long firstDetectionMicros = 0;


unsigned long reEnableInterruptInterval = 0;
unsigned long reEnableInterruptMicros = 0;

unsigned long secondDetectionMicros = 0;
unsigned long secondReadDuration = 0;

unsigned long intervalBetweenDetections = 0;

double aux = 0;

/**
* @brief Calibration factor for z-axis readings
*
*/
double factor = 0.30625;
int count = 0;
int count2 = 0;
int countMaxZ = 0;
bool readAccel = false;
void ADXInit(void);
uint16_t adc_read(uint8_t ch);
void adc_init();
ADXL345 adxl = ADXL345(); // USE FOR I2C COMMUNICATION
const byte ADX345_ID = 0x53;
char failedADX[] = "ADX_FAILED\n";
bool carDetectedTrigger = false;
byte ADXInterruptSource;

void setup()
{
	Wire.begin();
	adc_init();

	uart_init(BAUD_CALC(9600)); // 8n1 transmission is set as default
	sei();						// enable interrupts
	uart_putstr("hello!!!\n");
	ADXInit();
	//uart_putint(adxl.getRate());
	adxl.ActivityINT(1);
	EIMSK |= (1 << INT1); // Turns on INT1
	EIFR = (1 << INTF1);  // Clear interrupt
}

void loop()
{
	if (micros() - firstReadEndMicros > 300000 && firstReadFinish && reEnableInterruptMicros == 0)
	{
		reEnableInterruptMicros = micros();
		reEnableInterruptInterval = micros() - firstReadEndMicros;
		adxl.ActivityINT(1);
		adxl.getInterruptSource();
		
		EIMSK |= (1 << INT1); // Turns on INT1
		EIFR = (1 << INTF1);  // Clear interrupt
		
		//carDetectedTrigger = true;
	}
	if (carDetectedTrigger)
	{
		if (count2 == 0)
		{
			//uart_putstr("car detected\n");
			carsDetected++;
			count2++;
			
			adxl.ActivityINT(0);
			adxl.getInterruptSource();
			EIFR = (1 << INTF1); // Clear interrupt
			EIMSK = (0 << INT1); // Turns off INT1

			timeStart = micros();
			timeSinceLastRead = micros();
			readTimeout = 1600;
			readAccel = true;
			stop = countMax;
			canRead = true;
			newCarCount = 0;
			stopReadingAccel = 0;

			if (firstReadFinish)
			{
				secondReadStart = true;
				secondDetectionMicros = micros();
				intervalBetweenDetections = micros() - firstDetectionMicros;
			}
			else
			{
				firstDetectionMicros = micros();
				firstReadStart = true;
			}
		}

		if ((micros() - timeSinceLastRead) >= 1000 && canRead && (firstReadStart || secondReadStart))
		{
			timer = micros() - timeSinceLastRead;
			timeSinceLastRead = micros();
			adxl.readAccel(&x, &y, &z);

			//zMeasures[count] = z;  //measuredZAccel;



			Voltage = adc_read(1); // read the input pin
			voltage_V2 = Voltage;  //(Voltage / 1024.0) * 5;


			Voltage = adc_read(3); // read the input pin
			current_I2 = Voltage;//(Voltage / 1024.0) * 5;

			voltage_V2_array[count] = voltage_V2;
			current_I2_array[count] = current_I2;

			//power_I += (double)(voltage_V1 * current_I1);
			//energy_I += (double)(voltage_V1 * current_I1) * timer * 0.000001;
			
			aux = (double)(voltage_V2/ 1024.0) * 50 * (6.1597*((current_I2/ 1024.0)*5) - 15.495);
			
			power_II += aux;
			energy_II += aux * timer * 0.000001;

			count++;

			if (voltage_V2 <= vLimit && count > 30)
			{
				isV2Null = true;
				V2Null++;

				if (V2Null == V2NullMax)
				{
					if (!firstReadFinish)
					{
						canRead = false;
						firstReadFinish = true;
						firstReadCount = count;
						firstReadEndMicros = micros();
						firstReadDuration = micros() - firstDetectionMicros;
						carDetectedTrigger = false;
						count2 = 0;				
					}
					if (secondReadStart)
					{
						secondReadFinish = true;
						stop = count;
						secondReadDuration = micros() - secondDetectionMicros;
					}
				}
			}
			
			else
			{
				isV2Null = false;
				V2Null = 0;
			}

			if (count == countMax || secondReadFinish)
			{
                if (count==countMax)
                    secondReadDuration = micros() - secondDetectionMicros;
					
				acquisitonDuration = micros() - firstDetectionMicros;
				power_II = power_II/count;
				uart_putstr("\n\ncount: ");
				uart_putulong(count);

				uart_putstr("\n\nstop: ");
				uart_putulong(stop);

				uart_putstr("\n\nacquisitonDuration: ");
				uart_putulong(acquisitonDuration);

				uart_putstr("\n\nfirstReadCount: ");
				uart_putulong(firstReadCount);

				uart_putstr("\n\nfirstReadDuration: ");
				uart_putulong(firstReadDuration);

				uart_putstr("\n\nreEnableInterruptInterval: ");
				uart_putulong(reEnableInterruptInterval);

				uart_putstr("\n\nsecondReadDuration: ");
				uart_putulong(secondReadDuration);

				uart_putstr("\n\nintervalBetweenDetections: ");
				uart_putulong(intervalBetweenDetections);

				uart_putstr("\n\ncarsDetected: ");
				uart_putint(carsDetected);

				// uart_putstr("\n\nstopReadingAccel: ");
				// uart_putulong(stopReadingAccel);

				for (int i = 0; i < count; i++)
				{
					uart_putstr("\n\nmeasure #: ");
					uart_putint(i);

					uart_putstr("\nvoltage_V2: ");
					uart_fputfloat(voltage_V2_array[i], 2);
					
					uart_putstr("\ncurrent_I2: ");
					uart_fputfloat(current_I2_array[i], 2);

					uart_putstr("\nz_Measures: ");
					uart_fputfloat(0, 2);
					_delay_ms(50);
				}
				//uart_putstr("\n\npower_I: ");
				//uart_fputfloat(power_I,2);
				uart_putstr("\npower_II: ");
				uart_fputfloat(power_II,2);
				//uart_putstr("\nenergy_I: ");
				//uart_fputfloat(energy_I,2);
				uart_putstr("\nenergy_II: ");
				uart_fputfloat(energy_II,2);
				//power_I = 0;
				//energy_I = 0;
				power_II = 0;
				energy_II = 0;
				
				acquisitonDuration = 0;

				adxl.ActivityINT(1);
				adxl.getInterruptSource();

				EIMSK |= (1 << INT1); // Turns on INT1
				EIFR = (1 << INTF1);  // Clear interrupt
				//delay(50);
				carDetectedTrigger = false;
				count2 = 0;
				count = 0;
				carsDetected = 0;
				//uart_putstr("\nWaiting...");

				firstReadStart = false;
				firstReadFinish = false;
				secondReadStart = false;
				secondReadFinish = false;
				
				firstReadCount = 0;
				reEnableInterruptInterval = 0;
				firstReadDuration = 0;
				intervalBetweenDetections = 0;
			}
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
		adxl.powerOn();
		adxl.setRangeSetting(16);						  // Give the range settings
		adxl.setActivityXYZ(0, 0, 1);					  // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
		adxl.setActivityThreshold(40);					  // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
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

//http://maxembedded.com/2011/06/the-adc-of-the-avr/
void adc_init()
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
uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	if (ch < 0 || ch > 7)
	{			  //ADC0 - ADC7 is available
		return 1; //pin number is out of range
	}

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