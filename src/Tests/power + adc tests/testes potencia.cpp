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

int analogPin = A3; // potentiometer wiper (middle terminal) connected to analog pin 3
// outside leads to ground and +5V
double voltage_V2 = 0; // variable to store the value read
double voltage_V1 = 0;
double current_I2 = 0;
double current_I1 = 0;
double Voltage;

double power_I = 0;
double energy_I = 0;
double power_II = 0;
double energy_II = 0;

volatile unsigned long timer1_millis;
volatile unsigned long FirstWheelStart;
volatile unsigned long timer1stWheel;
volatile unsigned long timeStart;

int count = 0;
void init_millis(unsigned long f_cpu);
unsigned long millis2();
char str[40];
ISR(TIMER1_COMPA_vect)
{
	timer1_millis++;
}

void init_millis(unsigned long f_cpu)
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

unsigned long millis2()
{
	unsigned long millis_return;

	// Ensure this cannot be disrupted
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		millis_return = timer1_millis;
	}
	return millis_return;
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

void setup()
{
	adc_init();
	FirstWheelStart = millis2();
	timeStart = millis2();
	uart_init(BAUD_CALC(4800)); // 8n1 transmission is set as default
	init_millis(F_CPU);         //frequency the atmega328p is running at
	sei();                      // enable interrupts
}

void loop()
{
	if (millis2() - FirstWheelStart >= 5)
	{
		Voltage = adc_read(0); // read the input pin
		voltage_V2 = (Voltage / 1024.0) * 5;
		//uart_putstr("voltage_V2: "); // debug value
		// uart_putfloat(Voltage);      // debug value

		Voltage = adc_read(1); // read the input pin
		voltage_V1 = (Voltage / 1024.0) * 5;
		// uart_putstr("voltage_V1: "); // debug value
		//  uart_putfloat(Voltage);      // debug value

		Voltage = adc_read(2); // read the input pin
		current_I2 = (Voltage / 1024.0) * 5;
		//  uart_putstr("current_I2: "); // debug value
		// uart_putfloat(Voltage);      // debug value

		Voltage = adc_read(3); // read the input pin
		current_I1 = (Voltage / 1024.0) * 5;
		//  uart_putstr("current_I1: "); // debug value
		//  uart_putfloat(Voltage);      // debug value

		power_I += (double)(voltage_V1 * current_I1);
		power_II += (double)(voltage_V2 * current_I2);

		FirstWheelStart = millis2(); //reset the timer
		count++;

		if (count == 10)
		{
			count = 0;
			timer1stWheel = millis2() - timeStart;
			energy_I = (double)power_I * timer1stWheel / 1000;
			energy_II = (double)power_II * timer1stWheel / 1000;

			uart_putstr("\n\n\ntimer1stWheel: ");
			uart_putint(timer1stWheel);

			uart_putstr("\npower_I: ");
			uart_putint(power_I);
			uart_putstr("\npower_II: ");
			uart_putint(power_II);
			uart_putstr("\nenergy_I: ");
			uart_putint(energy_I);
			uart_putstr("\nenergy_II: ");
			uart_putint(energy_II);
			power_I = 0;
			energy_I = 0;
			power_II = 0;
			energy_II = 0;
			timer1stWheel = 0;

			_delay_ms(1000);
			FirstWheelStart = millis2();
			timeStart = millis2();
		}
	}
}