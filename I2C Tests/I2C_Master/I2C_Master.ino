
#include <Wire.h>

const byte MY_ADDRESS = 10;
const byte BROADCAST = 0;
const byte LOCALMASTER1_ADDRESS = 1;
const byte LOCALMASTER2_ADDRESS = 2;
const byte LED = 13;

String text;
byte nextLocalMaster = 1;
int localmasterResponse = 0;

void getText(){
  if(Serial.available()) // check if we send a message
  {
    while(Serial.available())
    {
      char c = Serial.read(); // read the next character.
      text += c;
      delay(10);
    } 
  }
}

void sendText() {
  Wire.beginTransmission(0); // transmit to device 
  for(int i=0; i<(text.length()); i++){
    Wire.write(text[i]);
  }             
  Wire.endTransmission();    // stop transmitting
  requestText();
}

void requestText() {
  /*
  switch (nextLocalMaster)
  {
  case 1:
    if(Wire.requestFrom(1, 8) == 0)
      localmasterResponse = -1; //Error from request
    
    else{
      localmasterResponse = 2; //if crc correct, assign response as valid - ToDo
      Serial.print("Received: ");
      for (byte i = 0; i < 8; i++){
        char c = Wire.read();       // receive a byte as character
        Serial.print(c);         // print the character
      }
      Serial.print("\n");
    }
    break;
  
  case 2:
    if(Wire.requestFrom(1, 8) == 0)
      localmasterResponse = -1; //Error from request
    
    else{
      localmasterResponse = 2; //if crc correct, assign response as valid - ToDo
      Serial.print("Received: ");
      for (byte i = 0; i < 8; i++){
        char c = Wire.read();       // receive a byte as character
        Serial.print(c);         // print the character
      }
      Serial.print("\n");
    }
    break;
  }*/
    if(Wire.requestFrom(1, 8) == 0){
      Serial.println("Erro no request 1");
      localmasterResponse = -1; //Error from request
    }
    
    else{
      localmasterResponse = 1; //if crc correct, assign response as valid - ToDo
      Serial.print("Received: ");
      for (byte i = 0; i < 8; i++){
        char c = Wire.read();       // receive a byte as character
        Serial.print(c);         // print the character
      }
      Serial.print("\n");
    }

    if(Wire.requestFrom(2, 8) == 0){
      Serial.println("Erro no request 2");
      localmasterResponse = -1; //Error from request
    }
    
    else{
      localmasterResponse = 2; //if crc correct, assign response as valid - ToDo
      Serial.print("Received: ");
      for (byte i = 0; i < 8; i++){
        char c = Wire.read();       // receive a byte as character
        Serial.print(c);         // print the character
      }
      Serial.print("\n");
    }
}


void receiveString(int bytes) {
  text = "";
  while (Wire.available()) {
    char c = Wire.read(); // receive a byte as character
    text += c;
  }
  Serial.print("Printing: ");
  Serial.println(text);

  sendText();
}

void setup() {
  Serial.begin(9600);
  Serial.println("Master here");
 
  // Start the I2C Bus as Master
  Wire.begin();
  Wire.onReceive(receiveString);

}

void loop() {
  text = "";
  while(text.length() == 0) { // Until we send text in the serial monitor
    getText();
    }
  if(text.length() > 0) {
     Serial.println("Send: " + text);
     sendText();
  }
  delay(500);
}
