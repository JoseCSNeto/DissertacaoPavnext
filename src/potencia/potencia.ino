int analogPin = A3; // potentiometer wiper (middle terminal) connected to analog pin 3
                    // outside leads to ground and +5V
int voltage_V2 = 0; // variable to store the value read
int voltage_V1 = 0;
int current_I2 = 0;
int current_I1 = 0;
double Voltage;

double power_I = 0;
double energy_I = 0;
double power_II = 0;
double energy_II = 0;

volatile unsigned long FirstWheelStart;
volatile unsigned long timer1stWheel;
volatile unsigned long timeStart;

int count = 0;

void setup()
{
    Serial.begin(9600); //  setup serial
    FirstWheelStart = millis();
    timeStart = millis();
}

void loop()
{
    if (millis() - FirstWheelStart >= 5)
    {
        Voltage = analogRead(A0); // read the input pin
        voltage_V2 = (Voltage / 1024.0) * 4.8;
        //Serial.print("voltage_V2: "); // debug value
        // Serial.println(Voltage);      // debug value

        Voltage = analogRead(A1); // read the input pin
        voltage_V1 = (Voltage / 1024.0) * 4.8;
        // Serial.print("voltage_V1: "); // debug value
        //  Serial.println(Voltage);      // debug value

        Voltage = analogRead(A2); // read the input pin
        current_I2 = (Voltage / 1024.0) * 4.8;
        //  Serial.print("current_I2: "); // debug value
        // Serial.println(Voltage);      // debug value

        Voltage = analogRead(A3); // read the input pin
        current_I1 = (Voltage / 1024.0) * 4.8;
        //  Serial.print("current_I1: "); // debug value
        //  Serial.println(Voltage);      // debug value

        power_I += (double)(voltage_V1 * current_I1);
        power_II += (double)(voltage_V2 * current_I2);

        FirstWheelStart = millis(); //reset the timer
        count++;

        if (count == 10)
        {
            count = 0;
            timer1stWheel = millis() - timeStart;
            energy_I = (double)power_I * timer1stWheel / 1000;
            energy_II = (double)power_II * timer1stWheel / 1000;
            Serial.print("timer1stWheel: ");
            Serial.println(timer1stWheel);

            Serial.print("power_I: ");
            Serial.println(power_I);
            Serial.print("power_II: ");
            Serial.println(power_II);
            Serial.print("energy_I: ");
            Serial.println(energy_I);
            Serial.print("energy_II: ");
            Serial.println(energy_II);
            power_I = 0;
            energy_I = 0;
            power_II = 0;
            energy_II = 0;
            timer1stWheel = 0;

            delay(1000);
            FirstWheelStart = millis();
            timeStart = millis();
        }
    }
}