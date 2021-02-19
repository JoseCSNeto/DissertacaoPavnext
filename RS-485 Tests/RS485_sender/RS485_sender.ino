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

#define F_CPU 16000000UL
char* string = "Olá, sou o Master\n";

int main(void)
{
	Wire.begin();
	uart_init(BAUD_CALC(4800)); // 8n1 transmission is set as default

	sei(); // enable interrupts, library wouldn't work without this
	
	uart_putstr(string); // write const string to usart buffer // C++ restriction, in C its the same as uart_putstr()

	while(1)
	{
		_delay_ms(1000);
		uart_putstr(string);
	}
}
