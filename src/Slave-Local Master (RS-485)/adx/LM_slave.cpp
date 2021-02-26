/* PAVNEXT NR

Local Master  v1 

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
#include "usart.h"
#include "CRC16.h"

#define SLAVE_DATA_SIZE				9
#define SLAVE_DATA_SIZE_CRC    		SLAVE_DATA_SIZE + 2
#define DEFAULT_MESSAGES_SIZE		2 // C1, P1, etc.
#define ALERT_MESSAGES_SIZE			3

#define MASTER_DATA_SIZE			10
#define MASTER_DATA_SIZE_CRC		MASTER_DATA_SIZE + 2

#define MASTER_ACCEPTING_COMMUNICATION 'M'
#define ACCEPTING_COMMUNICATION 'O'
#define CAR_DETECTED            'C'
#define WAITING_FOR_PACKET      'P'
#define HIGH_VALUE              'H' //for example, RH or temperature
#define NORMAL_VALUE            'N' //for example, RH or temperature
#define RLTV_HUMDT              'R'

#define MASTER_ID				'9'
#define LOCAL_MASTER_ID         '0'
#define SLAVE_ID_1              '1'
#define SLAVE_ID_2              '2'
#define SLAVE_ID_3              '3'
#define SLAVE_ID_4              '4'

uint16_t CRC_calculated;
uint16_t CRC_packet;

uint8_t peek;
char byteReceived;
char byteSend;
char packet_received [SLAVE_DATA_SIZE_CRC];
char data_packet [SLAVE_DATA_SIZE];
char default_message [DEFAULT_MESSAGES_SIZE];
char alert_message   [ALERT_MESSAGES_SIZE];

char Master_data_packet [MASTER_DATA_SIZE_CRC];

//int flagReady = 0; //flag that tells the slave if the master is accepting communication or not
int car_detected = 0; //flag that tells the master if the slave detected a car
int waiting_car = 0; //flag that tells the master if it is waiting for a car
int waiting_packet = 0; //flag that tells the master if it is waiting for a packet
int open_communciation = 0; //flag that tells the slaves can communicate
int waiting_packet_slave_id = 0;
int Wheel = 0;
uint8_t high_relative_humdt = 0;

//timers
volatile unsigned long timer1_millis;
unsigned long slave_x_received;

//aux
uint8_t n = 0; //used in for loops
uint8_t bytes = 0;

//functions
void init_millis(unsigned long f_cpu);
unsigned long millis2 ();

int main(void)
{
	Wire.begin();
	uart_init(BAUD_CALC(4800)); // 8n1 transmission is set as default
	init_millis(F_CPU); //frequency the atmega328p is running at
	sei(); // enable interrupts, library wouldn"t work without this
	waiting_car = 1;

	for (int i = 0; i<500; i++)
		_delay_ms(1);

	//uart_putc(MASTER_ACCEPTING_COMMUNICATION);
	//uart_putc(MASTER_ID);
	uart_putc(ACCEPTING_COMMUNICATION);
	uart_putc(SLAVE_ID_1);
	waiting_packet_slave_id = SLAVE_ID_1;
	
	while(1)
	{	
		if (waiting_car)
		{
			bytes = uart_AvailableBytes();
			if (bytes > 1)
			{
				peek = uart_getc();

				if(peek == CAR_DETECTED)
				{
					if(uart_peek() == SLAVE_ID_1)
					{
						uart_putc(WAITING_FOR_PACKET);
						uart_putc(SLAVE_ID_1);
						waiting_packet = 1;
						waiting_car = 0;
						waiting_packet_slave_id = SLAVE_ID_1;
						uart_getc();
					}

					else if(uart_peek() == SLAVE_ID_2)
					{
						uart_putc(WAITING_FOR_PACKET);
						uart_putc(SLAVE_ID_2);
						waiting_packet = 1;
						waiting_car = 0;
						waiting_packet_slave_id = SLAVE_ID_2;
						uart_getc();
					}   
					
					else if(uart_peek() == SLAVE_ID_3)
					{
						uart_putc(WAITING_FOR_PACKET);
						uart_putc(SLAVE_ID_3);
						waiting_packet = 1;
						waiting_car = 0;
						waiting_packet_slave_id = SLAVE_ID_3;
						uart_getc();
					}

					else if(uart_peek() == SLAVE_ID_4)
					{
						uart_putc(WAITING_FOR_PACKET);
						uart_putc(SLAVE_ID_4);
						waiting_packet = 1;
						waiting_car = 0;
						waiting_packet_slave_id = SLAVE_ID_4;
						uart_getc();
					}
										                 
					else
					{
						uart_getc();
					}					
				}
			}
		}
		
		else if (waiting_packet)
		{
			bytes = uart_AvailableBytes();
			
			if (bytes == SLAVE_DATA_SIZE_CRC)
			{
				if(uart_peek() == waiting_packet_slave_id)
				{
					uart_gets(packet_received, SLAVE_DATA_SIZE_CRC + 1);

					int ID_packet_received = (int) packet_received[0];

					if (ID_packet_received != waiting_packet_slave_id)
					{
						uart_puts("Packet from wrong Slave! Discarding the data...");
						waiting_packet = 0;
						waiting_car = 1;
						uart_putc(ACCEPTING_COMMUNICATION);
						uart_putc(SLAVE_ID_1);
					}

					else if (ID_packet_received == waiting_packet_slave_id)
					{
						for(n = 0; n < SLAVE_DATA_SIZE; n++)
						{
							data_packet[n] = packet_received[n];   // Extract the DATA from the PACKET
						}

						CRC_calculated = calc_crc(data_packet,SLAVE_DATA_SIZE); //Calculate CRC
						CRC_packet = packet_received[SLAVE_DATA_SIZE + 1] | (packet_received[SLAVE_DATA_SIZE] << 8); //MSB | LSB
						
						if(CRC_packet != CRC_calculated)
						    uart_puts("CRC errado!\n");

						slave_x_received = millis2();
						
						if (ID_packet_received == SLAVE_ID_1)
						{
							while((millis2() - slave_x_received < 20));
							uart_putc(ACCEPTING_COMMUNICATION);
							uart_putc(SLAVE_ID_2);

							waiting_packet = 0;
							waiting_car = 1;
							waiting_packet_slave_id = SLAVE_ID_2;	
						}
						
						else if (ID_packet_received == SLAVE_ID_2)
						{
							/*
							while((millis2() - slave_x_received < 20));	
							uart_putc(ACCEPTING_COMMUNICATION);
							uart_putc(SLAVE_ID_3);

							waiting_packet = 0;
							waiting_car = 1;
							waiting_packet_slave_id = SLAVE_ID_3;
							*/
							waiting_packet = 0;
							waiting_car = 1;
							uart_putc(ACCEPTING_COMMUNICATION);
							uart_putc(SLAVE_ID_1);		
						}

						else if (ID_packet_received == SLAVE_ID_3) 
						{
							while((millis2() - slave_x_received < 20));
							uart_putc(ACCEPTING_COMMUNICATION);
							uart_putc(SLAVE_ID_4);

							waiting_packet = 0;
							waiting_car = 1;
							waiting_packet_slave_id = SLAVE_ID_4;						
						}
						
						else if (ID_packet_received == SLAVE_ID_4)
						{
							while((millis2() - slave_x_received < 20));

							waiting_packet = 0;
							waiting_car = 1;
							uart_putc(ACCEPTING_COMMUNICATION);
							uart_putc(SLAVE_ID_1);	
						}
					}
					//bytes = 0;
				}
				else
				{
					uart_getc();
				}
			}
		}
		/*
		if(!waiting_packet)
		{
			bytes = uart_AvailableBytes();
			
			if (bytes == ALERT_MESSAGES_SIZE && uart_peek() == HIGH_VALUE)
			{
				uart_gets(alert_message, ALERT_MESSAGES_SIZE + 1);
				
				if (alert_message[0] == HIGH_VALUE && alert_message[1] == RLTV_HUMDT && alert_message[2] == SLAVE_ID_1) //ASCII 'C' - Slave X telling a car passed
				{
					high_relative_humdt = 1;
				}
			}
			
			else if (bytes == ALERT_MESSAGES_SIZE && uart_peek() == NORMAL_VALUE)
			{
				uart_gets(alert_message, ALERT_MESSAGES_SIZE + 1);
				
				if (alert_message[0] == NORMAL_VALUE && alert_message[1] == RLTV_HUMDT && alert_message[2] == SLAVE_ID_1) //ASCII 'C' - Slave X telling a car passed
				{
					high_relative_humdt = 0;
				}
			}
		}*/
	}//while1
}//main

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

unsigned long millis2 ()
{
	unsigned long millis_return;
	
	// Ensure this cannot be disrupted
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		millis_return = timer1_millis;
	}
	return millis_return;
}