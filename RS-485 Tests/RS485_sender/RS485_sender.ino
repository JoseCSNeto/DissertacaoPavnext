#include <SoftwareSerial.h>

#define SSerialRX        9  //Serial Receive pin  RO
#define SSerialTX        10  //Serial Transmit pin DI

#define SSerialTxControl 8  //RS485 Direction control DE
#define RS485Transmit    HIGH
#define RS485Receive     LOW

SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

/*-----( Declare Variables )-----*/
int byteReceived;
int byteSend;

void setup()   /****** SETUP: RUNS ONCE ******/
{
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(9600);
  
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver   

  // Start the software serial port, to another device
  RS485Serial.begin(9600);   // set the data rate 

}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  if (Serial.available())
  {
    byteReceived = Serial.read();

    digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit   
    RS485Serial.write(byteReceived);          // Send byte to Remote Arduino
    delay(10);
    digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit       
  }

  if (RS485Serial.available())  //Look for data from other Arduino
   {
    byteReceived = RS485Serial.read();    // Read received byte
    Serial.write((char)byteReceived);        // Show on Serial Monitor
   }

}


