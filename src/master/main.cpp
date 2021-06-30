/**
 * @file Master.cpp
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Master Communication with Local-Masters
 * @version 2.1
 * @date 2021-05-25
 * 
 * 
 */

#include <CRC16.h>
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <SimpleCan.h>
#include <SimpleFOC.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "master_FDCAN.h"
#include "communicationMacros.h"
#include "master_macros.h"
#include "nucleo_g431kb_LoRa.h"

uint8_t masterHello[6] = {109, 97, 115, 116, 101, 114}; //master
/**
 * @brief Initialization routines
 * 
 */
void setup()
{
  delay(3000);
  Serial2.begin(9600);
  Serial2.println("Master");

  initLoRa();
  FDCANInit();
  FDCANSend(masterHello, sizeof(masterHello));
  // Serial2.print(F("[SX1262] Sending first packet ... "));
  // transmissionState = radio.startTransmit("Hello World!!");
  // while (!transmittedFlag)
  // {
  // }
  // if (transmittedFlag)
  // {
  //   enableInterrupt = false;
  //   transmittedFlag = false;

  //   if (transmissionState == ERR_NONE)
  //   {
  //     Serial2.println(F("transmission finished!"));
  //   }

  //   else
  //   {
  //     Serial2.print(F("failed, code "));
  //     Serial2.println(transmissionState);
  //   }
  // }
  // if (transmittedFlag)
  // {
  //   enableInterrupt = false;
  //   transmittedFlag = false;

  //   if (transmissionState == ERR_NONE)
  //   {
  //     Serial2.println(F("transmission finished!"));
  //   }

  //   else
  //   {
  //     Serial2.print(F("failed, code "));
  //     Serial2.println(transmissionState);
  //   }
  //  // transmissionState = radio.startTransmit(LMPackets[k][j], sizeof(LMPackets[k][j]));
  //   enableInterrupt = true;
  // }
  delay(1000);
  MasterWaitingComm(nextLM);
}

// volatile bool transmittedFlag = false;
// volatile bool enableInterrupt = true;
/**
 * @brief Code running indefinitely
 * 
 */
void loop()
{
  if (canReceived)
  {
    Serial2.print("Received packet, ID: ");
    Serial2.println(canIdentifier);
    Serial2.print("Size: ");
    Serial2.println(canSize);
    for (int byte_index = 0; byte_index < canSize; byte_index++)
    {
      // Serial2.print(" byte[");
      // Serial2.print(byte_index);
      // Serial2.print("]=");
      Serial2.print(canPacket[byte_index]);
      Serial2.print(" ");
      //canPacket[byte_index] = canPacket[byte_index];
    }
    Serial2.println();
    if (waitingPacket)
    {
      if (canSize == DEFAULT_MESSAGES_SIZE)
      {
        if (canIdentifier == waitingPacketLMID)
        {
          if (canPacket[0] == LM_READY_TO_SEND_PACKET && canPacket[1] == waitingPacketLMID + '0')
          {
            //waitingPacket = 1;
            // waitingPacketLMID = canPacket[1] - '0';
            // waitingPacketSlaveID = SLAVE_ID_1;

            // MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[1] = waitingPacketLMID + '0';
            // MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[2] = SLAVE_ID_1;
            // FDCANSend(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y, sizeof(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y));

            MasterAskingPacket(nextSlave);
          }
        }
      }
      else if (canSize == MASTER_DATA_SIZE_CRC)
      {
        if (canIdentifier == waitingPacketLMID)
        {
          if (canPacket[0] == MASTER_ID && canPacket[1] == (waitingPacketLMID + '0') && canPacket[2] == (waitingPacketSlaveID + '0'))
          {
            for (int byte_index = 0; byte_index < canSize; byte_index++)
            {
              dataFromPacket[byte_index] = canPacket[byte_index];
            }
            calculatedCRC = calc_crc(dataFromPacket, MASTER_DATA_SIZE); //Calculate CRC
            //Serial2.print("\nCRC calculado: ");
            //Serial2.println(calculatedCRC,HEX);

            packetCRC = canPacket[MASTER_DATA_SIZE + 1] | (canPacket[MASTER_DATA_SIZE] << 8); //MSB | LSB
            if (calculatedCRC == packetCRC)
            {
              Serial2.println("CRC correct");
              MasterStorePacket(canPacket[1] - '0', canPacket[2] - '0'); //ASCII code to correspondent number. ex. ('1') 49 - 48  -> 1
            }
            MasterAskingPacket(nextSlave);
          }
        }
      }
    }
    canSize = 0;
    canReceived = false;
  }
}
