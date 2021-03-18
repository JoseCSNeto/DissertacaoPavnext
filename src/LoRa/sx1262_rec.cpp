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

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

void setFlag(void);

void setup() {
  delay(3000);
  Serial22.begin(9600);
  SPI.setMISO(MISO);
  SPI.setMOSI(MOSI);
  SPI.setSCLK(SCLK);
  SPI.setSSEL(CS);
  SPI.begin(1);

  // initialize SX1262 with default settings
  Serial22.print(F("[SX1262] Initializing ... "));
    int state = radio.begin(868.0, 125.0, 9, 7, SX126X_SYNC_WORD_PRIVATE, 22, 8, 0, true);
  if (state == ERR_NONE) {
    Serial22.println(F("success!"));
  } else {
    Serial22.print(F("failed, code "));
    Serial22.println(state);
    while (true);
  }
  radio.setRfSwitchPins(RX, TX);
  //pinMode(DIO1, INPUT);
  //attachInterrupt(DIO1, setFlag, RISING);

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);

  // start listening for LoRa packets
  Serial22.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == ERR_NONE) {
    Serial22.println(F("success!"));
  } else {
    Serial22.print(F("failed, code "));
    Serial22.println(state);
    while (true);
  }
}

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
}

void loop() {
  // check if the flag is set
  if(receivedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    String str;
    int state = radio.readData(str);

    // you can also read received data as byte array
    
     // byte byteArr[8];
     // int state = radio.readData(byteArr, 8);
    

    if (state == ERR_NONE) {
      // packet was successfully received
      Serial22.println(F("[SX1262] Received packet!"));

      // print data of the packet
      Serial22.print(F("[SX1262] Data:\t\t"));
      /*
      for (int i = 0; i< sizeof(byteArr);i++)
        Serial22.print (byteArr[i]);
      */
      Serial22.println(str);


      // print RSSI (Received Signal Strength Indicator)
      Serial22.print(F("[SX1262] RSSI:\t\t"));
      Serial22.print(radio.getRSSI());
      Serial22.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial22.print(F("[SX1262] SNR:\t\t"));
      Serial22.print(radio.getSNR());
      Serial22.println(F(" dB"));

    } else if (state == ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial22.println(F("CRC error!"));

    } else {
      // some other error occurred
      Serial22.print(F("failed, code "));
      Serial22.println(state);

    }

    // put module back to listen mode
    radio.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }

}