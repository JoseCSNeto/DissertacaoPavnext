/**
 * @file Local_Master.cpp
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Local Master Communication with multiple Slaves
 * @version 3.0
 * @date 2021-03-25
 * 
 * 
 */

//#include <SoftwareSerial.h>
#include <CRC16.h>
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <SimpleCan.h>
#include <SimpleFOC.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
//#include "nucleo_g431kb_RS485.h"
#include "LM_FDCAN.h"
#include "communicationMacros.h"
#include "LM_macros.h"

//functions headers
void LMStorePacket(int slaveID);
void LMWaitingCar(int slaveASCII);

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
uint8_t LMHello[7] = {108, 111, 99, 97, 108, 32, 77}; //local M
uint32_t masterID = 9; //master fdcan ID

void setup()
{
    delay(4000);

    pinMode(MAX485Control, OUTPUT);
    RS485Serial.begin(4800);
    delay(2000);
    Serial2.begin(9600);
    Serial2.println("Local Master");

    FDCANInit();
    FDCANSend(LMHello, sizeof(LMHello));
    //LMWaitingCar(nextSlave);
}
void loop()
{
    if (canReceived)
    {
        Serial2.print("Received packet, ID: ");
        Serial2.println(canIdentifier);
        for (int byte_index = 0; byte_index < canSize; byte_index++)
        {
            // Serial2.print(" byte[");
            // Serial2.print(byte_index);
            // Serial2.print("]=");
            Serial2.print((char)canPacket[byte_index]);
            Serial2.print(" ");
            //canPacket[byte_index] = canPacket[byte_index];
        }
        Serial2.println();
        if (!masterAccepting)
        {
            if (canIdentifier == masterID && canSize == DEFAULT_MESSAGES_SIZE)
            {
                if (canPacket[0] == MASTER_ACCEPTING_COMMUNICATION && canPacket[1] == MY_LM_ID)
                {
                    //Serial2.println("Telling the Slave x the Local Master is accepting communication");

                    LMWaitingPacket = false;
                    waitingCar = true;
                    masterAccepting = true;
                    LMWaitingCar(nextSlave);
                }
            }
        }

        else if (masterWaitingPacket)
        {
            if (canSize == MASTER_MESSAGES_SIZE)
            {
                if (canIdentifier == masterID)
                {
                    if (canPacket[0] == MASTER_ASKING_PACKET && canPacket[1] == MY_LM_ID) // && canPacket[2] == waitingPacketLMID)
                    {
                        masterWaitingPacket = true;
                        //waitingPacketLMID = canPacket[1];
                        waitingPacketSlaveID = canPacket[2] - '0';

                        Serial2.print("Packet from slave ");
                        Serial2.print(waitingPacketSlaveID);
                        Serial2.print(" to be sent to the Master: [");
                        FDCANSend(slavesPackets[waitingPacketSlaveID - 1], sizeof(slavesPackets[waitingPacketSlaveID - 1]));
                        for (int n = 0; n < MASTER_DATA_SIZE_CRC; n++)
                        {
                            Serial2.print((int)slavesPackets[waitingPacketSlaveID - 1][n]);
                            Serial2.print(" ");
                        }
                        Serial2.println("]");
                        if (waitingPacketSlaveID + '0' == NUMBER_OF_SLAVES)
                        {
                            masterWaitingPacket = false;
                            masterAccepting = false;
                           // LMWaitingCar(waitingPacketSlaveID - 1);
                        }
                    }

                    //MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[1] = waitingPacketLMID;
                    //MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[2] = SLAVE_ID_1;
                    //FDCANSend(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y, sizeof(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y));
                }
            }
        }
        canSize = 0;
        canReceived = false;
    }

    if (waitingCar)
    {
        bytes = RS485Serial.available();
        if (bytes > 1)
        {
            Serial2.print("bytes: ");
            Serial2.println(bytes);

            for (n = 0; n < DEFAULT_MESSAGES_SIZE; n++)
            {
                defaultMessage[n] = RS485Serial.read();
                Serial2.print("defaultMessage[n]: ");
                Serial2.println(defaultMessage[n]);
            }
            if (defaultMessage[0] == CAR_DETECTED) //ASCII 'C' - Slave X telling a car passed
            {
                Serial2.print("Car detected: ");

                if (defaultMessage[1] >= SLAVE_ID_1 && defaultMessage[1] <= NUMBER_OF_SLAVES)
                {
                    Serial2.println(defaultMessage[1]);

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
            for (n = 0; n < DATA_SIZE; n++)
            {
                Serial2.print(packetReceived[n]);
                Serial2.print(" ");
            }
            Serial2.println("]");

            packetReceivedID = (int)packetReceived[0];

            if (packetReceivedID != waitingPacketSlaveID)
            {
                Serial2.println("Packet from wrong Slave! Discarding data...");
                LMWaitingCar(packetReceivedID);
            }
            if (packetReceived[1] != MY_LM_ID)
            {
                Serial2.println("Packet sent to the wrong LM! Discarding data...");
                LMWaitingCar(packetReceivedID);
            }

            if (packetReceivedID == waitingPacketSlaveID && packetReceived[1] == MY_LM_ID)
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

                // Checks if the packet's CRC is the same as the calculated CRC
                if (calculatedCRC == packetCRC)
                {
                    LMStorePacket(packetReceivedID - 48); //ASCII code to correspondent number. ex. ('1') 49 - 48  -> 1
                    Serial2.println("CRC correct");
                }
                Serial2.print("SLAVE No: ");
                Serial2.print((char)packetReceived[0]);
                Serial2.print(" sent a packet to LM: ");
                Serial2.println((char)packetReceived[1]);

                slaveTimer = millis();
                while ((millis() - slaveTimer < 100))
                    ;
                // if (waitingPacketSlaveID < NUMBER_OF_SLAVES - '0')
                //     LMWaitingCar(waitingPacketSlaveID);
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
 * @brief 
 * 
 * @param ID 
 */
void LMStorePacket(int slaveID)
{
    slavesPackets[slaveID - 1][0] = MASTER_ID;
    slavesPackets[slaveID - 1][1] = MY_LM_ID;
    slavesPackets[slaveID - 1][2] = packetReceivedID;
    for (n = 3; n <= DATA_SIZE; n++)
    {
        slavesPackets[slaveID - 1][n] = dataFromPacket[n - 1];
    }

    packetCRC = calc_crc((char *)slavesPackets[slaveID - 1], MASTER_DATA_SIZE); //Calculate CRC

    slavesPackets[slaveID - 1][MASTER_DATA_SIZE] = (packetCRC >> 8);     //MSB
    slavesPackets[slaveID - 1][MASTER_DATA_SIZE + 1] = packetCRC & 0xff; //LSB -> last byte -> DATA_SIZE + 1 = last position

    if (slaveID == NUMBER_OF_SLAVES - 48)
    {
        packetsReady = true;
        if (masterAccepting)
        {
            Serial2.println("Telling the master that slave packets are ready to be sent");
            LM_X_READY_TO_SEND_PACKET[1] = MY_LM_ID;
            FDCANSend(LM_X_READY_TO_SEND_PACKET, sizeof(LM_X_READY_TO_SEND_PACKET));
            packetsReady = false;
            masterWaitingPacket = true;
        }
    }
    else{
        LMWaitingCar(slaveID);
    }


}
/**
 * @brief Notify the slave that the Local-Master is accepting communication
 * 
 * @param slaveID The previous slave ID
 */
void LMWaitingCar(int slaveID)
{
    Serial2.print("Telling the Slave x ");
    Serial2.print(slaveID);
    Serial2.println(" Local Master is accepting communication");

    waitingPacket = false;
    waitingCar = true;

    if (slaveID < NUMBER_OF_SLAVES - 48)
    {
        nextSlave = nextSlave + 1;
    }
    else
    {
        nextSlave = 1;
    }

    ACCEPTING_COMMUNICATION_SV_X[1] = nextSlave + '0';
    
    Serial2.print("ACCEPTING_COMMUNICATION_SV_X ");
    Serial2.println(ACCEPTING_COMMUNICATION_SV_X[1]);
    sendPacket(ACCEPTING_COMMUNICATION_SV_X, sizeof(ACCEPTING_COMMUNICATION_SV_X) + 1); // OX-> local master accepting communication from Slave_X
}
