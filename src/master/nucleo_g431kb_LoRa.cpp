/**
 * @file nucleo_g431kb_LoRa.cpp
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Master LoRa Macros
 * @version 1.0
 * @date 2021-03-23
 * 
 * 
 */

#ifndef nucleo_g431kb_LoRa_h
#define nucleo_g431kb_LoRa_h

#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include "nucleo_g431kb_LoRa.h"

// SX1262 has the following connections:
#define CS PA4
#define CS2 PB5
#define DIO1 PB0
#define RESET PA0
#define BUSY PB4

#define MOSI PA7
#define MISO PA6
#define SCLK PA5

SX1262 radio = new Module(CS2, DIO1, RESET, BUSY);

#define TX PA10
#define RX PA8

// save transmission state between loops
int transmissionState = ERR_NONE;
// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;
// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

/**
 * @brief  This function is called when a complete packet is transmitted by the module
 * 
 */
void setFlag(void)
{
	// check if the interrupt is enabled
	if (!enableInterrupt)
	{
		return;
	}

	// we sent a packet, set the flag
	transmittedFlag = true;
}
/**
 * @brief Initialization of the LoRa module
 * 
 */
void initLoRa()
{
	SPI.setMISO(MISO);
	SPI.setMOSI(MOSI);
	SPI.setSCLK(SCLK);
	SPI.setSSEL(CS);
	SPI.begin(1);

	Serial2.print(F("[SX1262] Initializing ... "));
	int state = radio.begin(868.0, 125.0, 9, 7, SX126X_SYNC_WORD_PRIVATE, 22, 8, 0, true);
	radio.setCRC(2);
	if (state == ERR_NONE)
	{
		Serial2.println(F("success!"));
	}

	else
	{
		Serial2.print(F("failed, code "));
		Serial2.println(state);
		while (true)
			;
	}

	radio.setRfSwitchPins(RX, TX);
	radio.setDio1Action(setFlag);
}

#endif
