#include <Wire.h>

//const byte ledPin = 13;
const byte interruptPin = 2;
volatile byte state = LOW;
char received[8];
volatile int i = 0;
void setup()
{
  // Los master pueden obviar este ID, pero al querer recibir datos, tendremos que ponerlo
  Wire.begin(1); // Se une al bus i2C con la ID #0
  Wire.onReceive(receiveEvent); // Función a ejecutar al recibir datos
  Serial.begin(9600);

  //pinMode(ledPin, OUTPUT);
}

void loop()
{
}
// Esta función se ejecutará al recibir datos, lo cual provocará que se salga del loop principal.
void receiveEvent(int howMany)
{

  while(Wire.available()) // hacemos loop por todos los bytes salvo el último
  {
    char c = Wire.read();    // recibe un byte como carácter
    //Serial.print(c);         // imprime el carácter
    received[i] = c;
    i++;
  }
  received[7] = '\0';
  
  if (!strcmp(received, "slave 2")){
     Serial.print("certo 2: ");
     Serial.println(received);

  }
  else if (!strcmp(received, "slave 3")){
     Serial.print("certo 3: ");
     Serial.println(received);
  }
  else{
     Serial.print("errado: ");
     Serial.println(received);  
  }

  //blink();
}

/*
void blink() {
    if (state == HIGH) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
  state = !state;
}
*/