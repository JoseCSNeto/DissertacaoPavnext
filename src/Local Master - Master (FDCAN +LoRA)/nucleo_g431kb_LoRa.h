/**
 * @file nucleo_g431kb_LoRa.h
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Master LoRa Macros
 * @version 1.0
 * @date 2021-03-23
 * 
 * 
 */

#ifndef LoRa_h
#define LoRa_h

#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include "communicationMacros.h"
//#include "master_macros.h"

// SX1262 has the following connections:
#define CS PA4
#define DIO1 PB0
#define RESET PA0
#define BUSY PB4

#define MOSI PA7
#define MISO PA6
#define SCLK PA5

extern SX1262 radio;// = new Module(CS, DIO1, RESET, BUSY);

#define TX PA10
#define RX PA8

// save transmission state between loops
extern int transmissionState;
// flag to indicate that a packet was sent
extern volatile bool transmittedFlag;
// disable interrupt when it's not needed
extern volatile bool enableInterrupt;

void setFlag(void);
void initLoRa();

#endif
