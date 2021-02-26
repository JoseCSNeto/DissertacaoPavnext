#include <SoftwareSerial.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <Wire.h>


#define SSerialRX        9  //Serial Receive pin  RO
#define SSerialTX        10  //Serial Transmit pin DI

#define SSerialTxControl 8  //RS485 Direction control DE
#define RS485Transmit    HIGH
#define RS485Receive     LOW

      

SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX
byte byteReceived;

void setup()
{
  
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Receiver");  

  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver
  RS485Serial.begin(4800);
}

void loop()
{

  int bytes = RS485Serial.available();  

   /* if (bytes==1)
    {
      byteReceived = RS485Serial.read();
      
        Serial.print(byteReceived);
        Serial.print(" ");       
    }*/
     if (bytes)
    {
      for (int i = 0; i < bytes; i++)
        {
          byteReceived = RS485Serial.read();
          Serial.print((char)byteReceived);
          Serial.print(" ");
        }
    }
     
}
