/* PAVNEXT NR

Slave #1 v1

By: José Neto */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_HTS221.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <math.h>
#include "usart.h"
#include "CRC16.h"
#include "MCP3221.h"
#include "SparkFun_ADXL345.h"

//Sensors
Adafruit_HTS221 hts;
sensors_event_t temp;
sensors_event_t humidity;

/*const int LIS3DH_ADDR = 0x18;
volatile sensors_event_t event;
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
sensors_event_t accel_z;*/

ADXL345 adxl = ADXL345(); // USE FOR I2C COMMUNICATION
int x, y, z;
double factor = 0.0383072;
bool trigger = 0;
byte interrupt_source;

const byte ADX345_ID = 0x53;   // I2C address of the ADX345
const byte BIELA1_ADDR = 0x48; // I2C address of the MCP3221 A0
const byte BIELA2_ADDR = 0x4D; // I2C address of the MCP3221 A5

MCP3221 mcp3221_biela1(BIELA1_ADDR);
MCP3221 mcp3221_biela2(BIELA2_ADDR);

double Biela1_voltage;
double Biela2_voltage;

#define DATA_SIZE 9
#define DATA_SIZE_CRC DATA_SIZE + 2
#define DEFAULT_MESSAGES_SIZE 2
#define ALERT_MESSAGES_SIZE 3

//COMMUNICATION

//Variables
uint8_t peek;
char charReceived;
char default_message[DEFAULT_MESSAGES_SIZE];
char Data_packet[DATA_SIZE_CRC];
uint16_t crc_out;
uint8_t bytes = 0;

//FLAGS
#define MY_SLAVE_ID '1'
#define MASTER_ID '9'
#define LOCAL_MASTER_ID '0'

#define MY_SLAVE_ID_ASCII 1
#define MASTER_ID_ASCII 9
#define LOCAL_MASTER_ID_ASCII 0

#define START_PACKET '!'

#define MASTER_ACCEPTING_COMMUNICATION 'M'
#define LOCAL_MASTER_ACCEPTING_COMMUNICATION 'O'
#define CAR_DETECTED 'C'
#define CAN_SEND_PACKET 'P'
#define HIGH_VALUE 'H'   //for example, RH or temperature
#define NORMAL_VALUE 'N' //for example, RH or temperature
#define RLTV_HUMDT 'R'

#define CRC_SIZE 4

#define VREF 4999

char temp_string[20];
char humidity_string[20];
char z_accel_string[20];

char failed_biela1[] = "BIELA1_FAILED\n";
char failed_biela2[] = "BIELA2_FAILED\n";
char failed_hts[] = "HTS_FAILED\n";
char failed_lis[] = "LIS_FAILED\n";
char failed_ADX[] = "ADX_FAILED\n";

//FLAGS
int Local_Master_accepting = 0; //flag that tells the slave if the master is accepting communication or not
int send_packet = 0;

int possible = 0;
int count = 0;                      // counter for the number of packets sent when a car is detected
bool sensors_readings_ready = true; //flag that signals if the slave has completed the 0.5s of readings
bool high_relative_humdt = false;
volatile int Wheel = 0;
volatile bool CAR_ARRIVED = LOW;
volatile int car_count = 0;

// sensors

double ADCValue_V1 = 0;
double ADCValue_V2 = 0;
double ADCValue_I1 = 0;
double ADCValue_I2 = 0;

double voltage_V2 = 0; // variable to store the value read
double voltage_V1 = 0;
double current_I2 = 0;
double current_I1 = 0;
double Voltage;

double power_I = 0;
double energy_I = 0;
double power_II = 0;
double energy_II = 0;

double max_biela1_voltage = 0;
double max_biela2_voltage = 0;

float measured_z_accel = 0;
float max_z_accel = 0;
float max_temp = 0;
float max_humdt = 0;
float max_current = 0;
float max_voltage = 0;



//timers
volatile unsigned long timer1_millis;
volatile unsigned long timeStart;
volatile unsigned long FirstWheelStart;
volatile unsigned long timer1stWheel;
volatile unsigned long ScndWheelStart;
volatile unsigned long timer2ndWheel;
volatile unsigned long HTS_readings;

//DEFAULT MESSAGES
char CAR_DETECTED_SV1[DEFAULT_MESSAGES_SIZE] = {CAR_DETECTED, MY_SLAVE_ID};            // C1-> car detected in slave1
char HIGH_RLTV_HUMDT[ALERT_MESSAGES_SIZE] = {HIGH_VALUE, RLTV_HUMDT, MY_SLAVE_ID};     // HR1
char NORMAL_RLTV_HUMDT[ALERT_MESSAGES_SIZE] = {NORMAL_VALUE, RLTV_HUMDT, MY_SLAVE_ID}; // NR1

//functions
void init_millis(unsigned long f_cpu);
unsigned long millis2();
void init_adx(void);
uint16_t adc_read(uint8_t ch);
void adc_init();

int main(void)
{
    Wire.begin();
    adc_init();
    uart_init(BAUD_CALC(4800)); // 8n1 transmission is set as default
    init_millis(F_CPU);         //frequency the atmega328p is running at
    sei();                      // enable interrupts
    //uart_putstr("hello from slave");
    //for (int i = 0; i<1000;i++)
    //_delay_ms(1);

    /*mcp3221_biela1.setVref(4999);
	mcp3221_biela2.setVref(4999);
	
	if(mcp3221_biela1.ping())
		uart_putstr(failed_biela1);
		
	if(mcp3221_biela2.ping())
		uart_putstr(failed_biela2);
   

	if (!hts.begin_I2C()) {
		uart_putstr(failed_hts);
	}*/

    init_adx();
    //HTS_readings = millis2();

    while (1)
    {

        /*	if(millis2()-HTS_readings > 5000)
		{
			HTS_readings = millis2();
			hts.getEvent(&humidity, &temp);

			if(humidity.relative_humidity > 80)
			{
				high_relative_humdt = true;
				if(Local_Master_accepting)
				uart_putstrl(HIGH_RLTV_HUMDT,ALERT_MESSAGES_SIZE);
			}

			else if(humidity.relative_humidity < 80 && high_relative_humdt)
			{
				high_relative_humdt = false;
				if(Local_Master_accepting)
				uart_putstrl(NORMAL_RLTV_HUMDT,ALERT_MESSAGES_SIZE);
			}
		}*/

        if (!Local_Master_accepting || !send_packet)
        {
            bytes = uart_AvailableBytes();
            if (bytes > 1)
            {
                peek = uart_getc();
                if (peek == LOCAL_MASTER_ACCEPTING_COMMUNICATION)
                {
                    peek = uart_getc();
                    if (peek == MY_SLAVE_ID)
                    {
                        Local_Master_accepting = 1;
                        adxl.ActivityINT(1);
                        EIMSK |= (1 << INT1); // Turns on INT1
                        EIFR = (1 << INTF1);  // Clear interrupt
                    }
                }
                else if (peek == CAN_SEND_PACKET)
                {
                    peek = uart_getc();
                    if (peek == MY_SLAVE_ID)
                    {
                        send_packet = 1;
                    }
                }
            }
        }

        if (trigger)
        {
            trigger = 0;
            interrupt_source = adxl.getInterruptSource();

            if (Local_Master_accepting)
            {
                uart_putc(CAR_DETECTED);
                uart_putc(MY_SLAVE_ID);
                Local_Master_accepting = 0;


                sensors_readings_ready = false;

                adxl.ActivityINT(0);
                EIFR = (1 << INTF1); // Clear interrupt
                EIMSK = (0 << INT1); // Turns off INT1
                if (Wheel == 0)
                {
                    Wheel = 1;
                    FirstWheelStart = millis2();
                    car_count++;
                }

                else if (Wheel == 1)
                {
                    Wheel = 2;
                    ScndWheelStart = millis2();
                }

                else if (Wheel == 2)
                {
                    Wheel = 1;
                    FirstWheelStart = millis2();
                }
                timeStart = millis2();
            }
        }

        if (!sensors_readings_ready)
        {
            if (Wheel == 1)
            {
                if (millis2() - FirstWheelStart >= 5)
                {
                    //hts.getEvent(&humidity, &temp);
                    adxl.readAccel(&x, &y, &z); // Read the accelerometer values and store them in variables declared above x,y,z
                    measured_z_accel = z * factor;

                    //Biela1_voltage = mcp3221_biela1.getVoltage();
                    //Biela2_voltage = mcp3221_biela2.getVoltage();

                    Voltage = adc_read(0);                    
                    voltage_V2 = (Voltage / 1024) * 5; //*50 com divisor //multiplicar por 10 devido a divisor resistivo

                    Voltage = adc_read(1); 
                    voltage_V1 = (Voltage / 1024) * 5; //*50 com divisor //multiplicar por 10 devido a divisor resistivo

                    Voltage = adc_read(2);
                    current_I2 = (Voltage / 1024) * 5;
                    
                    Voltage = adc_read(3);
                    current_I1 = (Voltage / 1024) * 5;

                    //current_I2 = (double)(((ADCValue_I2 / 1024) * 5) * 6.1597) - 15.495; //ajustar valor de tensão para correspondente corrente
                    //current_I1 = (double)(((ADCValue_I1 / 1024) * 5) * 6.1597) - 15.495;

                    if (!count)
                    {
                        max_biela1_voltage = Biela1_voltage;
                        max_biela2_voltage = Biela2_voltage;
                        max_z_accel = measured_z_accel;
                        //max_current = voltage_V1;
                        //max_voltage = voltage_V1;
                    }

                    max_biela1_voltage = (Biela1_voltage > max_biela1_voltage) ? Biela1_voltage : max_biela1_voltage;
                    max_biela2_voltage = (Biela2_voltage > max_biela2_voltage) ? Biela2_voltage : max_biela2_voltage;
                    max_z_accel = (measured_z_accel > max_z_accel) ? measured_z_accel : max_z_accel;
                    //	max_current = (voltage_V1 > max_current) ? voltage_V1 : max_current;
                    //	max_voltage = (voltage_V1 > max_voltage) ? voltage_V1 : max_voltage;

                    power_I += (double)(voltage_V1 * current_I1);
                    power_II += (double)(voltage_V2 * current_I2);

                    FirstWheelStart = millis2(); //reset the timer
                    count++;

                    if (count == 10)
                    {
                        sensors_readings_ready = true;
                        count = 0;
                        timer1stWheel = millis2() - timeStart;
                        energy_I = (double)power_I * timer1stWheel / 1000;
                        energy_II = (double)power_II * timer1stWheel / 1000;
                    }
                }
            }

            else if (Wheel == 2)
            {
                if (millis2() - ScndWheelStart >= 5)
                {
                    //hts.getEvent(&humidity, &temp);
                    adxl.readAccel(&x, &y, &z); // Read the accelerometer values and store them in variables declared above x,y,z
                    measured_z_accel = z * factor;

                    //Biela1_voltage = mcp3221_biela1.getVoltage();
                    //Biela2_voltage = mcp3221_biela2.getVoltage();

                    Voltage = adc_read(0);                    
                    voltage_V2 = (Voltage / 1024) * 5; //*50 com divisor //multiplicar por 10 devido a divisor resistivo

                    Voltage = adc_read(1); 
                    voltage_V1 = (Voltage / 1024) * 5; //*50 com divisor //multiplicar por 10 devido a divisor resistivo

                    Voltage = adc_read(2);
                    current_I2 = (Voltage / 1024) * 5;
                    
                    Voltage = adc_read(3);
                    current_I1 = (Voltage / 1024) * 5;
                    
                    //current_I2 = (double)(((ADCValue_I2 / 1024) * 5) * 6.1597) - 15.495; //ajustar valor de tensão para correspondente corrente
                    //current_I1 = (double)(((ADCValue_I1 / 1024) * 5) * 6.1597) - 15.495;

                    if (!count)
                    {
                        max_biela1_voltage = Biela1_voltage;
                        max_biela2_voltage = Biela2_voltage;
                        max_z_accel = measured_z_accel;
                        //max_current = voltage_V1;
                        //max_voltage = voltage_V1;
                    }

                    max_biela1_voltage = (Biela1_voltage > max_biela1_voltage) ? Biela1_voltage : max_biela1_voltage;
                    max_biela2_voltage = (Biela2_voltage > max_biela2_voltage) ? Biela2_voltage : max_biela2_voltage;
                    max_z_accel = (measured_z_accel > max_z_accel) ? measured_z_accel : max_z_accel;
                    //	max_current = (voltage_V1 > max_current) ? voltage_V1 : max_current;
                    //	max_voltage = (voltage_V1 > max_voltage) ? voltage_V1 : max_voltage;

                    power_I += (double)(voltage_V1 * current_I1);
                    power_II += (double)(voltage_V2 * current_I2);

                    ScndWheelStart = millis2(); //reset the timer
                    count++;

                    if (count == 10)
                    {
                        sensors_readings_ready = true;
                        count = 0;
                        timer2ndWheel = millis2() - timeStart;
                        energy_I = (double)power_I * timer2ndWheel / 1000;
                        energy_II = (double)power_II * timer2ndWheel / 1000;
                    }
                }
            }
        }

        if (send_packet && sensors_readings_ready)
        {
            //uart_putc(START_PACKET);
            // Data_packet = [SLAVE_ID][DEST_ID][DATA][CRC:MSB|LSB]
            max_biela1_voltage = (float)max_biela1_voltage * 0.05;
            max_biela2_voltage = (float)max_biela2_voltage * 0.05;
            max_z_accel = max_z_accel * 10;
            measured_z_accel = measured_z_accel * 10;

            //	car_weight_biela1 = pow(max_biela1_voltage,3)*405816.33 - pow(max_biela1_voltage,2)*3294162.89 + max_biela1_voltage*8927838.36 - 8077416.56;
            //car_weight_biela2 = pow(max_biela2_voltage,3)*3337.84 - pow(max_biela2_voltage,2)*19838.71 + max_biela2_voltage*44620.52 - 33344.6;
            if (power_I > 255)
                power_I = 255;
            if (energy_I > 255)
                energy_I = 255;
            if (power_II > 255)
                power_II = 255;
            if (energy_II > 255)
                energy_II = 255;

            Data_packet[0] = MY_SLAVE_ID;
            Data_packet[1] = LOCAL_MASTER_ID;
            Data_packet[2] = (int)max_z_accel;
            Data_packet[3] = 43;
            Data_packet[4] = (int)measured_z_accel;
            Data_packet[5] = (int)power_I;
            Data_packet[6] = (int)energy_I;
            Data_packet[7] = (int)power_II;
            Data_packet[8] = (int)energy_II;

            max_z_accel = 0;
            max_biela1_voltage = 0;
            max_biela2_voltage = 0;
            power_I = 0;
            energy_I = 0;
            power_II = 0;
            energy_II = 0;

            crc_out = calc_crc(Data_packet, DATA_SIZE); //Calculate CRC

            Data_packet[DATA_SIZE] = (crc_out >> 8);     //MSB
            Data_packet[DATA_SIZE + 1] = crc_out & 0xff; //LSB -> last byte -> DATA_SIZE + 1 = last position

            uart_putstrl(Data_packet, sizeof(Data_packet)); // Data_packet = [SLAVE_ID][DEST_ID][DATA][CRC:MSB|LSB]

            send_packet = 0;
            Local_Master_accepting = 0;
        }
        if (Wheel == 1 && (millis2() - FirstWheelStart >= 5000))
        {
            Wheel = 0;
            car_count--;
        }
    } //while1
} //main

ISR(INT1_vect)
{
    trigger = 1;
}

void init_adx()
{

    Wire.beginTransmission(ADX345_ID);
    byte error = Wire.endTransmission();
    if (error)
    {
        uart_putstr(failed_ADX);
    }

    else if (!error)
    {
        adxl.powerOn();                                   // Power on the ADXL345
        adxl.setRangeSetting(2);                          // Give the range settings
        adxl.setActivityXYZ(0, 0, 1);                     // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
        adxl.setActivityThreshold(40);                    // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
        adxl.setImportantInterruptMapping(1, 1, 1, 1, 1); // Sets "adxl.setEveryInterruptMapping(single tap, double tap, free fall, activity, inactivity);"
        adxl.InactivityINT(0);
        adxl.ActivityINT(0);
        adxl.FreeFallINT(0);
        adxl.doubleTapINT(0);
        adxl.singleTapINT(0);

        EIFR = (1 << INTF1);  //clear interrupt
        DDRD &= ~(1 << DDD3); // Clear the PD3 pin
        // PD3 (PCINT1 pin) is now an input
        PORTD |= (1 << PORTD3); // turn On the Pull-up
        // PD3 is now an input with pull-up enabled
        EICRA |= (1 << ISC10) | (1 << ISC11);
        // set INT1 to trigger on RISING logic change
        //EIMSK |= (1 << INT1); // Turns on INT1
        //pinMode(interruptPin, INPUT_PULLUP);
        byte interrupts = adxl.getInterruptSource();
    }
}

ISR(TIMER1_COMPA_vect)
{
    timer1_millis++;
}

void init_millis(unsigned long f_cpu)
{
    unsigned long ctc_match_overflow;

    ctc_match_overflow = ((f_cpu / 1000) / 8); //when timer1 is this value, 1ms has passed

    // (Set timer to clear when matching ctc_match_overflow) | (Set clock divisor to 8)
    TCCR1B |= (1 << WGM12) | (1 << CS11);

    // high byte first, then low byte
    OCR1AH = (ctc_match_overflow >> 8);
    OCR1AL = ctc_match_overflow;

    // Enable the compare match interrupt
    TIMSK1 |= (1 << OCIE1A);

    //REMEMBER TO ENABLE GLOBAL INTERRUPTS AFTER THIS WITH sei(); !!!
}

unsigned long millis2()
{
    unsigned long millis_return;

    // Ensure this cannot be disrupted
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        millis_return = timer1_millis;
    }
    return millis_return;
}
//http://maxembedded.com/2011/06/the-adc-of-the-avr/
void adc_init()
{
    // AREF = AVcc
    ADMUX = (1 << REFS0);

    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

// read adc value
//https://hekilledmywire.wordpress.com/2011/03/16/using-the-adc-tutorial-part-5/
//https://www.xanthium.in/atmega328p-10bit-sar-adc-usart-serial-transmit-to-pc-data-acquisition-tutorial
uint16_t adc_read(uint8_t ch)
{
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    if (ch < 0 || ch > 7)
    {             //ADC0 - ADC7 is available
        return 1; //pin number is out of range
    }

    /* ADMUX's first few bits are
	 * the binary representations of the numbers of the pins so we can
	 * just 'OR' the pin's number with ADMUX to select that pin.
	 * We first zero the four bits by setting ADMUX equal to its higher
	 * four bits. */
	ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);

    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1 << ADSC);

    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while (ADCSRA & (1 << ADSC))
        ;

    return (ADC);
}
