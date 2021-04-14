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

	ADMUX &= (0xF0);
	ADMUX |= (ch & 0x0F); //set channel, limit channel selection to lower 4 bits


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
	uart_init(BAUD_CALC(4800)); // 8n1 transmission is set as default
	sei();                      // enable interrupts
}

void loop()
{

	Voltage = adc_read(0); // read the input pin
	voltage_V2 = (double)(Voltage / 1024.0) * 5;
	uart_putstr("\nvoltage_V2: "); // debug value
	uart_fputfloat(voltage_V2,2);      // debug value

	Voltage = adc_read(1); // read the input pin
	voltage_V1 = (double)(Voltage / 1024.0) * 5;
	uart_putstr("\nvoltage_V1: "); // debug value
	uart_fputfloat(voltage_V1,2);      // debug value

	Voltage = adc_read(2); // read the input pin
	current_I2 = (double)(Voltage / 1024.0) * 5;
	uart_putstr("\ncurrent_I2: "); // debug value
	uart_fputfloat(current_I2,2);      // debug value

	Voltage = adc_read(3); // read the input pin
	current_I1 = (double)(Voltage / 1024.0) * 5;
	uart_putstr("\ncurrent_I1: "); // debug value
	uart_fputfloat(current_I1,2);      // debug value
	
	_delay_ms(1000);

}