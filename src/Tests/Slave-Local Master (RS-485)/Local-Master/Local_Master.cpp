/**
 * @file main.cpp
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Local Master Communication with multiple Slaves
 * @version 2.1
 * @date 2021-03-08
 * 
 * 
 */

#include <SoftwareSerial.h>
#include <CRC16.h>
#include <SPI.h>
#include <Wire.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "nucleo_g431kb_RS485.h"
#include "communicationMacros.h"

//DEFAULT MESSAGES
/**
 * @brief Pre-defined message for 'OX' -> notifies the slave #X that it can communicate
 * 
 */
byte ACCEPTING_COMMUNICATION_SV_X[DEFAULT_MESSAGES_SIZE] = {LOCAL_MASTER_ACCEPTING_COMMUNICATION, (byte)'x'};
/**
 * @brief Pre-defined message for 'PX' -> notifies the slave #X that a packet from it is expected
 * 
 */
byte WAITING_PACKET_SV_X[DEFAULT_MESSAGES_SIZE] = {WAITING_FOR_PACKET, (byte)'x'};                            

//packet arrays initialization
/**
 * @brief Packet received
 * 
 */
byte packetReceived[DATA_SIZE_CRC];
/**
 * @brief Retrieved data from the packet received (excludes the CRC)
 * 
 */
char dataFromPacket[DATA_SIZE];
/**
 * @brief Used in messages such as 'CX' (car detected by slave #X)
 * 
 */
byte defaultMessage[DEFAULT_MESSAGES_SIZE];
/**
 * @brief Used in messages such as high/low temperature alerts sent by slaves
 * 
 */
byte alertMessage[ALERT_MESSAGES_SIZE];

/**
 * @brief CRC retrieved from the packet received.
 * 
 */
uint16_t packetCRC;

/**
 * @brief Calculated CRC from the packet received. When compared to 'packetCRC' validates or excluded the packet
 * 
 */
uint16_t calculatedCRC;


//flags
/**
 * @brief Flag that tells the local-master if it is waiting for a car
 * 
 */
bool waitingCar = false;   
/**
 * @brief Flag that tells the local-master if it is waiting for a packet
 * 
 */
bool waitingPacket = false;
/**
 * @brief Defines who is the next slave to send the 'accepting_communication' marco
 * 
 */
uint8_t nextSlave = 0;
/**
 * @brief Which slaveID is expected to be received in the next packet
 * 
 */
uint8_t waitingPacketSlaveID = 0;
/**
 * @brief Slave ID from the received packet. When compared to 'waitingPacketSlaveID' validates or excludes the packet
 * 
 */
uint8_t packetReceivedID = 0;

/**
 * @brief Detects which car wheel/half triggered the slave 
 * 
 */
uint8_t Wheel = 0;

/**
 * @brief Available RS485 bytes
 * 
 */
uint8_t bytes = 0; 

//timers
/**
 * @brief Used for waiting for 20 ms to send 'WAITING_PACKET_SV_X'
 * 
 */
unsigned long slaveTimer; 

/**
 * @brief Aux variable, used in 'for' loops
 * 
 */
uint8_t n = 0; 

//functions headers
void LMWaitingCar(int slaveASCII);
void sendPacket(byte *buf, int len);

void setup()
{
    Serial2.begin(4800);

    pinMode(MAX485Control, OUTPUT);
    RS485Serial.begin(4800);
    delay(5);
    LMWaitingCar(nextSlave);
}
void loop()
{
    if (waitingCar)
    {
        bytes = RS485Serial.available();
        if (bytes == DEFAULT_MESSAGES_SIZE)
        {
            for (n = 0; n < DEFAULT_MESSAGES_SIZE; n++)
            {
                defaultMessage[n] = RS485Serial.read();
            }
            if (defaultMessage[0] == CAR_DETECTED) //ASCII 'C' - Slave X telling a car passed
            {
                if (defaultMessage[1] >= SLAVE_ID_1 && defaultMessage[1] <= NUMBER_OF_SLAVES)
                {
                    Serial2.println("Tell the Slave it can send a packet");
                    WAITING_PACKET_SV_X[1] = defaultMessage[1];                       //Updating the Slave_ID from the packet received
                    sendPacket(WAITING_PACKET_SV_X, sizeof(WAITING_PACKET_SV_X) + 1); // PX-> master waiting for a packet from Slave_X
                    waitingPacket = true;
                    waitingCar = false;
                    waitingPacketSlaveID = (int)defaultMessage[1];
                }
            }
        }
    }

    else if (waitingPacket)
    {
        bytes = RS485Serial.available();
        //Serial2.print("A espera de packet: ");
        //Serial2.println(bytes);
        //delay(2);
        if (bytes == DATA_SIZE_CRC)
        {
            for (n = 0; n < DATA_SIZE_CRC; n++)
                packetReceived[n] = RS485Serial.read();

            Serial2.print("Packet received: [");
            Serial2.print(packetReceived[0]);
            Serial2.print(" ");
            Serial2.print(packetReceived[1]);
            Serial2.print(" ");
            Serial2.print(packetReceived[2]);
            Serial2.print(" ");
            Serial2.print(packetReceived[3]);
            Serial2.print(" ");
            Serial2.print(packetReceived[4]);
            Serial2.print(" ");
            Serial2.print(packetReceived[5]);
            Serial2.print(" ");
            Serial2.print(packetReceived[6]);
            Serial2.print(" ");
            Serial2.print(packetReceived[7]);
            Serial2.print(" ");
            Serial2.print(packetReceived[8]);
            Serial2.println("]");

            packetReceivedID = (int)packetReceived[0];

            if (packetReceivedID != waitingPacketSlaveID)
            {
                Serial2.println("Packet from wrong Slave! Discarding data...");
                LMWaitingCar(packetReceivedID);
            }

            else if (packetReceivedID == waitingPacketSlaveID)
            {
                for (n = 0; n < DATA_SIZE; n++)
                {
                    dataFromPacket[n] = packetReceived[n]; // Extract the DATA from the PACKET
                                                           //Serial2.print((int)dataFromPacket[n]);
                }

                calculatedCRC = calc_crc(dataFromPacket, DATA_SIZE); //Calculate CRC
                //Serial2.print("\nCRC calculado: ");
                //Serial2.println(calculatedCRC,HEX);

                packetCRC = packetReceived[DATA_SIZE + 1] | (packetReceived[DATA_SIZE] << 8); //MSB | LSB
                //Serial2.print("CRC packet: ");
                //Serial2.println(packetCRC,HEX);

                if (calculatedCRC == packetCRC)
                {
                    Serial2.println("CRC correct");

                } // Checks if the packet's CRC is the same as the calculated CRC

                Serial2.print("SLAVE No: ");
                Serial2.print((char)packetReceived[0]);
                Serial2.print(" sent a packet to dest: ");
                Serial2.println((char)packetReceived[1]);
                //slaveTimer = millis();
                while ((millis() - slaveTimer < 20));

                LMWaitingCar(waitingPacketSlaveID);
            }
        }
    }

    /*
  if(!waitingCar || !waiting_packet)
  {
    int bytes = RS485Serial.available();  
        delay(100);

    if (bytes == ALERT_MESSAGES_SIZE)  
    {
      for(n = 0; n < ALERT_MESSAGES_SIZE; n++)
      {
        alertMessage[n] = RS485Serial.read();
      }  
      if (alertMessage[0] == HIGH_VALUE && alertMessage[1] == RLTV_HUMDT &&  alertMessage[2] == SLAVE_ID_1) //ASCII 'C' - Slave X telling a car passed
      {
        Serial2.println("Relative humidity on slave 1 is too high!");    
      }
      else if (alertMessage[0] == NORMAL_VALUE && alertMessage[1] == RLTV_HUMDT &&  alertMessage[2] == SLAVE_ID_1) //ASCII 'C' - Slave X telling a car passed
      {
        Serial2.println("Relative humidity on slave 1 back to normal!");    
      }      
    }
  }*/
}
/**
 * @brief Notify the slave that the Local-Master is accepting communication
 * 
 * @param slaveASCII The ASCII code of the previous slave
 */
void LMWaitingCar(int slaveASCII) // slaveASCII is the ASCII code of the slave #
{
    Serial2.println("Telling the Slave #slaveASCII the Master is accepting communication");

    waitingPacket = false;
    waitingCar = true;

    if (slaveASCII < NUMBER_OF_SLAVES)
    { //mudar quando houver mais slaves
        nextSlave = nextSlave + 1;
    }
    else
    {
        nextSlave = 1;
    }

    ACCEPTING_COMMUNICATION_SV_X[1] = nextSlave + '0';
    sendPacket(ACCEPTING_COMMUNICATION_SV_X, sizeof(ACCEPTING_COMMUNICATION_SV_X) + 1); // OX-> local master accepting communication from Slave_X
}
/**
 * @brief Enables RS485 transmissition, sends the packet and disables RS485 transmissition
 * 
 * @param buf byte array to send
 * @param len length of the data array
 */
void sendPacket(byte *buf, int len)
{
    digitalWrite(MAX485Control, RS485Transmit); // Enable RS485 Transmit
    RS485Serial.write(buf, len);
    digitalWrite(MAX485Control, RS485Receive); // Disable RS485 Transmit
}