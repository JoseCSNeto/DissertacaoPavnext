#include <Wire.h>

//const byte ledPin = 13;
const byte interruptPin = 2;
volatile byte state = LOW;

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

  while(1 < Wire.available()) // hacemos loop por todos los bytes salvo el último
  {
    char c = Wire.read();    // recibe un byte como carácter
    Serial.print(c);         // imprime el carácter
  }
  int x = Wire.read();       // recibe el último byte como número
  Serial.println(x);         // imprime el número
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