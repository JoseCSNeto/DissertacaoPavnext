
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
void receiveEvent(int howMany);
void send();
void send_message();
const byte interruptPin = 2;
byte x = 0;
int can_send = 0;
void setup()
{
	// Los master pueden obviar este ID, pero al querer recibir datos, tendremos que ponerlo
	Wire.begin(4); // Se une al bus i2C con la ID #0
	Wire.onReceive(receiveEvent); // Función a ejecutar al recibir datos
  	pinMode(interruptPin, INPUT_PULLUP);
  	attachInterrupt(digitalPinToInterrupt(interruptPin), send, RISING);
	  }
void loop()
{
	  /*Wire.beginTransmission(1); // Send to Slave #1
	  Wire.write("x is ");       // Send five bytes
	  Wire.write(x);             // Send one byte
	  Wire.endTransmission();    // Stop sending
	  x++;
	  delay(500);*/
	  send_message();
}
// Esta función se ejecutará al recibir datos, lo cual provocará que se salga del loop principal.
void receiveEvent(int howMany)
{
	while(1 < Wire.available()) // hacemos loop por todos los bytes salvo el último
	{
		char c = Wire.read();    // recibe un byte como carácter
		Serial.print(c);         // imprime el carácter
	}
	int x = Wire.read();       // recibe el último byte como número
	Serial.println(x);         // imprime el número
}

void send(){
	can_send = 1;
}

void send_message(){
	if (can_send == 1){
		Wire.beginTransmission(1); // Send to Slave #1
	  Wire.write("slave 2");       // Send five bytes
	  Wire.endTransmission();    // Stop sending
	  can_send = 0;
	}
}