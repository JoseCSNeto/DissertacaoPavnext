// include the library
#include <RadioLib.h>
#include "nucleo_g431kb_RS485.h"

// SX1262 has the following connections:
#define CS PA4
#define DIO1 PB0
#define RESET PA0
#define BUSY PB4

#define MOSI PA7
#define MISO PA6
#define SCLK PA5

SX1262 radio = new Module(CS, DIO1, RESET, BUSY);

#define TX PA10
#define RX PA8

// or using RadioShield
// https://github.com/jgromes/RadioShield
//SX1262 radio = RadioShield.ModuleA;

// save transmission state between loops
int transmissionState = ERR_NONE;
// flag to indicate that a packet was sent
volatile bool transmittedFlag;

// disable interrupt when it's not needed
volatile bool enableInterrupt;
void setFlag(void);
int counter = 0;
void enableTX();

void setup()
{
  delay(3000);
  Serial22.begin(9600);
  SPI.setMISO(MISO);
  SPI.setMOSI(MOSI);
  SPI.setSCLK(SCLK);
  SPI.setSSEL(CS);
  SPI.begin(1);
  pinMode(MAX485Control, OUTPUT);
  RS485Serial.begin(4800);
  delay(100);
  digitalWrite(MAX485Control, RS485Transmit); // Enable RS485 Transmit
  RS485Serial.write("hello",6);
  digitalWrite(MAX485Control, RS485Receive); // Disable RS485 Transmit
  delay(5000);

  // initialize SX1262 with default settings
  Serial22.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(868.0, 125.0, 9, 7, SX126X_SYNC_WORD_PRIVATE, 22, 8, 0, true);
  if (state == ERR_NONE)
  {
    Serial22.println(F("success!"));
  }
  else
  {
    Serial22.print(F("failed, code "));
    Serial22.println(state);
    while (true)
      ;
  }
  radio.setRfSwitchPins(RX, TX);
  // pinMode(TX, OUTPUT);
  // pinMode(RX, OUTPUT);
  // enableTX();
  //pinMode(DIO1, INPUT);
 // attachInterrupt(DIO1, setFlag, RISING);

  // set the function that will be called
  // when packet transmission is finished
  radio.setDio1Action(setFlag);
  //radio.explicitHeader();
  // radio.setCRC(2);
  // start transmitting the first packet
  Serial22.print(F("[SX1262] Sending first packet ... "));

  // you can transmit C-string or Arduino string up to
  // 256 characters long

  // you can also transmit byte array up to 256 bytes long

  // byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
  //                   0x89, 0xAB, 0xCD, 0xEF};
  // state = radio.startTransmit(byteArr, 8);
}

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
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

void loop()
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
      Serial22.println(F("transmission finished!"));

      // NOTE: when using interrupt-driven transmit method,
      //       it is not possible to automatically measure
      //       transmission data rate using getDataRate()
    }
    else
    {
      Serial22.print(F("failed, code "));
      Serial22.println(transmissionState);
    }

    // wait a second before transmitting again
    delay(1000);

    // send another one
    Serial22.print(F("[SX1262] Sending another packet ... "));

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
  }
}
void enableTX(void)
{
  digitalWrite(RX, LOW);
  digitalWrite(TX, HIGH);
  delay(100);
}