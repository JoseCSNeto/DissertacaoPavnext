#include <SoftwareSerial.h>

#define SSerialRX        9  //Serial Receive pin  RO
#define SSerialTX        10  //Serial Transmit pin DI

#define SSerialTxControl 8  //RS485 Direction control DE
#define RS485Transmit    HIGH
#define RS485Receive     LOW

SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

/*-----( Declare Variables )-----*/
byte byteReceived;
byte byteSend;
int counter = 0;
char received [5];

void setup()   /****** SETUP: RUNS ONCE ******/
{
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(9600);
  Serial.println("SerialRemote");  // Can be ignored
  
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver

  // Start the software serial port, to another device
  RS485Serial.begin(4800);   // set the data rate 
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  //Copy input data to output  
  if (RS485Serial.available())
  {
    byteReceived = RS485Serial.read();   // Read the byte 
    Serial.println((char)byteReceived); 
    received[counter] = (char)byteReceived;
    counter++;
  }
  if (counter == 5){
        Serial.println("contei 5");  

    digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit
    delay(10);
    int len = strlen(received);
    byte arr[5];
    int i;
    
    //converting string to BYTE[]
    string2ByteArray(received, arr);


    for (int i = 0; i < 5; i++){
      RS485Serial.write(arr[i]);          // Send byte to Remote Arduino
      delay(10);
      Serial.println((char)arr[i]); 

    }  
    counter = 0; 
    digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit   
  }

}

void string2ByteArray(char* input, byte* output)
{
    int loop;
    int i;
    
    i = 0;
    
    while(i<5)
    {
        output[i] = input[i];
        i++;
        loop++;
    }
}
