
#include <Wire.h>

const byte MY_ADDRESS = 2;
const byte MASTER_ADDRESS = 10;
const byte LED = 13;

String text = "";
boolean canSend = false;


void receiveEvent(int bytes) {
  text = "";
  while (Wire.available()) {
    char c = Wire.read(); // receive a byte as character
    text += c;
  }
  Serial.print("Printing: ");
  Serial.println(text);
  manageLed(text);

}

void requestEvent() {
  if (canSend){
    for(int i=0; i<(text.length()); i++){
      Wire.write(text[i]);
    }
    canSend = false;  
  }
  else{
     Wire.write("ERROR: 2");
  }      
}

void manageLed(String cmd) {
  if(cmd == "CAN SEND"){
    canSend = true;
    digitalWrite(LED,HIGH);
  }
  else if(cmd == "OFF")
    digitalWrite(LED,LOW);
  else
    Serial.println("Unknown command: " + cmd);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Slave here");
 
  // Start the I2C Bus as Slave
  Wire.begin(MY_ADDRESS);

  TWAR = (MY_ADDRESS << 1) | 1;  // enable broadcasts to be received

   // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent); // register event
  // LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
}

void loop() {
  delay(10);
}
