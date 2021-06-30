#include <Arduino.h>
#include <Wire.h>
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


double voltage_V2 = 0; // variable to store the value read
double voltage_V1 = 0;
double current_I2 = 0;
double current_I1 = 0;
double Voltage;

#define countMax 50
double voltage_V2_array[countMax] = {0}; // variable to store the value read
double voltage_V1_array[countMax] = {0};
double current_I2_array[countMax] = {0};
double current_I1_array[countMax] = {0};
double zMeasures[countMax] = {0};


double power_I = 0;
double energy_I = 0;
double power_II = 0;
double energy_II = 0;

unsigned long FirstWheelStart;
unsigned long timer1stWheel;
unsigned long timeStart;
float measuredZAccel = 0;
float maxZ = 0;
int readTimeout = 0;

int x, y, z;
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

	uart_init(BAUD_CALC(4800)); // 8n1 transmission is set as default
	sei();                      // enable interrupts
	uart_putstr("hello\n");
	ADXInit();
	//uart_putint(adxl.getRate());
	adxl.ActivityINT(1);
	EIMSK |= (1 << INT1); // Turns on INT1
	EIFR = (1 << INTF1);  // Clear interrupt
}

void loop()
{
	if (carDetectedTrigger)
	{
		if (count2 == 0){
			//uart_putstr("car detected\n");
			count2++;
			adxl.getInterruptSource();
			adxl.ActivityINT(0);
			EIFR = (1 << INTF1); // Clear interrupt
			EIMSK = (0 << INT1); // Turns off INT1
			
			//FirstWheelStart = millis2();
			//timeStart = millis2();
			timeStart = micros();
			FirstWheelStart = micros();
			readTimeout = 1600;
			readAccel = true;
		}

		if ((micros() - FirstWheelStart) >= readTimeout){
			
			FirstWheelStart = micros();
			if(readAccel)
				adxl.readAccel(&x, &y, &z); // Read the accelerometer values and store them in variables declared above x,y,z
			else 
				z = 0;
			measuredZAccel = z * factor;
			
			if(measuredZAccel > maxZ){
				maxZ = measuredZAccel;
				countMaxZ = 0;
				readAccel = true;
			}
			
			else{
				countMaxZ++;
				if (countMaxZ >= 10){
					countMaxZ = 0;
					readTimeout = 1000;
					readAccel = false;
				}
			}
			Voltage = adc_read(0); // read the input pin
			voltage_V2 = (Voltage / 1024.0) * 5;
			//uart_putstr("voltage_V2: "); // debug value
			//uart_putfloat(Voltage);      // debug value

			Voltage = adc_read(1); // read the input pin
			voltage_V1 = (Voltage / 1024.0) * 5;
			// uart_putstr("voltage_V1: "); // debug value
			// uart_putfloat(Voltage);      // debug value

			//y = 6.1597x - 15.495

			Voltage = adc_read(2); // read the input pin
			current_I2 = (Voltage / 1024.0) * 5;
			//uart_putstr("current_I2: "); // debug value
			//uart_putfloat(Voltage);      // debug value

			Voltage = adc_read(3); // read the input pin
			current_I1 = (Voltage / 1024.0) * 5;
			//uart_putstr("current_I1: "); // debug value
			//uart_putfloat(Voltage);      // debug value
			
			zMeasures[count] = measuredZAccel;
			voltage_V2_array[count] = voltage_V2;
			voltage_V1_array[count] = voltage_V1;
			current_I2_array[count] = current_I2;
			current_I1_array[count] = current_I1;
			power_I += (double)(voltage_V1 * current_I1);
			power_II += (double)(voltage_V2 * current_I2);

			count++;

			if (count == countMax)
			{
				count = 0;
				unsigned long micro = micros();

				timer1stWheel = micro - timeStart;
				energy_I = (double)power_I * timer1stWheel / 1000;
				energy_II = (double)power_II * timer1stWheel / 1000;
				uart_putstr("\n\nmicros(): ");
				uart_putulong(micro);
				
				uart_putstr("\n\ntimeStart: ");
				uart_putulong(timeStart);

				uart_putstr("\n\n\ntimer1stWheel: ");
				uart_putulong(timer1stWheel);
				
				for (int i = 0; i<countMax; i++){
					uart_putstr("\n\nmeasure #: ");
					uart_putint(i);
					uart_putstr("\nvoltage_V2: ");
					uart_fputfloat(voltage_V2_array[i],2);
					uart_putstr("\nvoltage_V1: ");
					uart_fputfloat(voltage_V1_array[i],2);
					uart_putstr("\ncurrent_I2: ");
					uart_fputfloat(current_I2_array[i],2);
					uart_putstr("\ncurrent_I1: ");
					uart_fputfloat(current_I1_array[i],2);
					
					uart_putstr("\nz_Measures: ");
					uart_fputfloat(zMeasures[i],2);
					_delay_ms(50);
				}
				uart_putstr("\n\npower_I: ");
				uart_fputfloat(power_I,2);
				uart_putstr("\npower_II: ");
				uart_fputfloat(power_II,2);
				uart_putstr("\nenergy_I: ");
				uart_fputfloat(energy_I,2);
				uart_putstr("\nenergy_II: ");
				uart_fputfloat(energy_II,2);
				power_I = 0;
				energy_I = 0;
				power_II = 0;
				energy_II = 0;
				timer1stWheel = 0;

				adxl.ActivityINT(1);
				adxl.getInterruptSource();

				EIMSK |= (1 << INT1); // Turns on INT1
				EIFR = (1 << INTF1);  // Clear interrupt
				//delay(50);
				carDetectedTrigger = false;
				count2 = 0;
				//uart_putstr("\nWaiting...");
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
		adxl.setRangeSetting(16);                          // Give the range settings
		adxl.setActivityXYZ(0, 0, 1);                     // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
		adxl.setActivityThreshold(40);                    // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
		adxl.setImportantInterruptMapping(1, 1, 1, 1, 1); // Sets "adxl.setEveryInterruptMapping(single tap, double tap, free fall, activity, inactivity);"
		adxl.InactivityINT(0);
		adxl.ActivityINT(0);
		adxl.FreeFallINT(0);
		adxl.doubleTapINT(0);
		adxl.singleTapINT(0);
		//adxl.setRate(400);
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
	{             //ADC0 - ADC7 is available
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