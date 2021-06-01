/*
 * Driver for Microchip Technology Inc. 23LC (23LCV) SPI SRAM chips for
 * AVR, SAM3X (Due), and SAM M0+ (SAMD, SAML, SAMC) microcontrollers
 * 
 * Copyright (c) 2017, Justin Mattair (justin@mattair.net)
 * 
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#include <Arduino.h>
#include <SPI.h>
#include <SRAM_23LC.h>
#include <RadioLib.h>

#define CS PA4
#define CS2 PB5
#define DIO1 PB0
#define RESET PA0
#define BUSY PB4

#define MOSI PA7
#define MISO PA6
#define SCLK PA5

// SPI bus can be SPI, SPI1 (if present), etc.
#define SPI_PERIPHERAL SPI
//#define CHIP_SELECT_PIN		27

/* Device can be:
 * 128KB: SRAM_23LCV1024, SRAM_23LC1024, SRAM_23A1024
 * 64KB: SRAM_23LCV512, SRAM_23LC512, SRAM_23A512
 * 32KB: SRAM_23A256, SRAM_23K256
 * 8KB: SRAM_23A640, SRAM_23K640
 */
SRAM_23LC SRAM(&SPI_PERIPHERAL, CS, SRAM_23K256);

// Additional SRAM chips
// SRAM_23LC SRAM1(&SPI_PERIPHERAL1, CHIP_SELECT_PIN1, SRAM_23LC512);

#define START_ADDRESS 250

//uint8_t buffer[BUFFER_SIZE];
//#define BUFFER_SIZE  320

//char buffer[] = "The MattairTech MT-D21E is a development board for the 32-pin Microchip / Atmel SAMx21E ARM Cortex M0+ microcontrollers. Choose between the D21E, L21E, or C21E. Arduino compatible core files for all 3 chips is provided.";
char buffer[] = "Teste lora + memoria";

#define BUFFER_SIZE (sizeof(buffer) / sizeof(uint8_t))

SX1262 radio = new Module(CS2, DIO1, RESET, BUSY);

#define TX PA10
#define RX PA8
int aux = 0;
// save transmission state between loops
int transmissionState = ERR_NONE;
// flag to indicate that a packet was sent
volatile bool transmittedFlag;

// disable interrupt when it's not needed
volatile bool enableInterrupt;
void setFlag(void);
int counter = 0;
void enableTX();

void setup(void)
{
  /* Without parameters, begin() uses the default speed for this
   * library (12MHz for samd, 14MHz for sam, and 4MHz for avr).
   * Note that SPI transaction support is required.
   */
  delay(2000);
  SPI.setMISO(MISO);
  SPI.setMOSI(MOSI);
  SPI.setSCLK(SCLK);
  SPI.setSSEL(CS);
  SPI.begin(1);
  SRAM.begin(2000000);
  //SRAM.begin(8000000UL);      // or specify speed

  Serial2.begin(9600);

  // Print buffer to serial monitor
  Serial2.print("Write Block: ");
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    Serial2.write(buffer[i]);
  }
  Serial2.println();

  // Write block
  if (!SRAM.writeBlock(START_ADDRESS, BUFFER_SIZE, buffer))
  {
    Serial2.println("Write Block Failure");
  }

  // Clear buffer
  memset(&buffer[0], 0, BUFFER_SIZE);

  // Read Byte, print to serial monitor
  Serial2.print("Read Byte:  ");
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    buffer[i] = SRAM.readByte(START_ADDRESS + i);
    Serial2.write(buffer[i]);
  }
  Serial2.println();

  // Write Byte, print to serial monitor
  Serial2.print("Write Byte:  ");
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    Serial2.write(buffer[i]);
    SRAM.writeByte(START_ADDRESS + i, buffer[i]);
  }
  Serial2.println();

  // Clear buffer
  memset(&buffer[0], 0, BUFFER_SIZE);

  // Read block
  Serial2.print("Read Block:  ");
  if (!SRAM.readBlock(START_ADDRESS, BUFFER_SIZE, buffer))
  {
    Serial2.println("Read Block Failure");
  }

  // Print buffer to serial monitor
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    Serial2.write(buffer[i]);
  }
  Serial2.println();

  delay(1000);
  // initialize SX1262 with default settings
  Serial2.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(868.0, 125.0, 9, 7, SX126X_SYNC_WORD_PRIVATE, 22, 8, 0, true);
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
  // set the function that will be called
  // when packet transmission is finished
  radio.setDio1Action(setFlag);

  // start transmitting the first packet
  Serial2.print(F("[SX1262] Sending first packet ... "));
}
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

void loop(void)
{

  if (counter == 0)
  {
    counter = 1;
    enableInterrupt = true;
    transmittedFlag = false;
    transmissionState = radio.startTransmit("Hello World!");
    // transmittedFlag = true;
  }

  // check if the previous transmission finished
  if (transmittedFlag)
  {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    transmittedFlag = false;

    if (transmissionState == ERR_NONE)
    {
      // packet was successfully sent
      Serial2.println(F("transmission finished!"));

      // NOTE: when using interrupt-driven transmit method,
      //       it is not possible to automatically measure
      //       transmission data rate using getDataRate()
    }
    else
    {
      Serial2.print(F("failed, code "));
      Serial2.println(transmissionState);
    }

    // wait a second before transmitting again
    delay(1000);
    // Print buffer to serial monitor
    //char buffer[BUFFER_SIZE] = aux + '0';
        for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
      buffer[i] = aux + '0';
    }
    //buffer[BUFFER_SIZE - 1] = aux + '0';
    Serial2.print("Write Block: ");
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
      Serial2.write(buffer[i]);
    }
    Serial2.println();

    // Write block
    if (!SRAM.writeBlock(START_ADDRESS, BUFFER_SIZE, buffer))
    {
      Serial2.println("Write Block Failure");
    }

    // Clear buffer
    memset(&buffer[0], 0, BUFFER_SIZE);

    // Read Byte, print to serial monitor
    Serial2.print("Read Byte:  ");
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
      buffer[i] = SRAM.readByte(START_ADDRESS + i);
      Serial2.write(buffer[i]);
    }
    Serial2.println();

    // Write Byte, print to serial monitor
    Serial2.print("Write Byte:  ");
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
      Serial2.write(buffer[i]);
      SRAM.writeByte(START_ADDRESS + i, buffer[i]);
    }
    Serial2.println();

    // Clear buffer
    memset(&buffer[0], 0, BUFFER_SIZE);

    // Read block
    Serial2.print("Read Block:  ");
    if (!SRAM.readBlock(START_ADDRESS, BUFFER_SIZE, buffer))
    {
      Serial2.println("Read Block Failure");
    }

    // Print buffer to serial monitor
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
      Serial2.write(buffer[i]);
    }
    Serial2.println();
    // send another one
    Serial2.print(F("[SX1262] Sending another packet ... "));

    // you can transmit C-string or Arduino string up to
    // 256 characters long
    transmissionState = radio.startTransmit("Hello World!");

    // you can also transmit byte array up to 256 bytes long
    /*
      byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
      int state = radio.startTransmit(byteArr, 8);
    */

    // we're ready to send more packets,
    // enable interrupt service routine
    enableInterrupt = true;
    aux++;
  }
}