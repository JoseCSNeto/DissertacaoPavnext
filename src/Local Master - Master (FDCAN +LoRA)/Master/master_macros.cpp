#include "master_macros.h"

bool waitingPacket = false;
uint32_t waitingPacketLMID = 1;
uint32_t waitingPacketSlaveID = 0;
uint32_t packetReceivedLMID = 0;

uint8_t nextLM = 0;
uint8_t nextSlave = 0;

uint8_t MASTER_ACCEPTING_COMMUNICATION_LM_X[DEFAULT_MESSAGES_SIZE] = {MASTER_ACCEPTING_COMMUNICATION, (byte)'x'}; // M9-> accepting communication from LM
byte MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[MASTER_MESSAGES_SIZE] = {MASTER_ASKING_PACKET, (byte)'x', (byte)'y'};  // Q9X-> waiting for packet from lm_x slave_y

uint8_t n = 0;
uint8_t k = 0;
uint8_t j = 0;
/**
 * @brief Retrieved data from the packet received (excludes the CRC)
 * 
 */
char dataFromPacket[MASTER_DATA_SIZE_CRC];
uint16_t packetCRC = 0;
uint16_t calculatedCRC = 0;

uint8_t LMPackets[NUMBER_OF_LM - 48][NUMBER_OF_SLAVES - 48][MASTER_DATA_SIZE_CRC];
void MasterWaitingComm(int LMID)
{
  Serial2.println("Telling the LM x the Master is accepting communication");

  waitingPacket = true;
  if (LMID < NUMBER_OF_LM - 48)
  {
    nextLM = nextLM + 1;
  }
  else
  {
    nextLM = 1;
  }
  MASTER_ACCEPTING_COMMUNICATION_LM_X[1] = nextLM + '0';
  FDCANSend(MASTER_ACCEPTING_COMMUNICATION_LM_X, sizeof(MASTER_ACCEPTING_COMMUNICATION_LM_X));
}

void MasterStorePacket(int LMID, int slaveID)
{
  Serial2.print("LMID: ");
  Serial2.println(LMID);
  Serial2.print("slaveID: ");
  Serial2.println(slaveID);

  LMPackets[LMID - 1][slaveID - 1][0] = MASTER_ID - '0';
  LMPackets[LMID - 1][slaveID - 1][1] = LMID;
  LMPackets[LMID - 1][slaveID - 1][2] = slaveID;
  for (n = 3; n <= MASTER_DATA_SIZE; n++)
  {
    LMPackets[LMID - 1][slaveID - 1][n] = dataFromPacket[n];
  }

  packetCRC = calc_crc((char *)LMPackets[LMID - 1][slaveID - 1], MASTER_DATA_SIZE); //Calculate CRC

  LMPackets[LMID - 1][slaveID - 1][MASTER_DATA_SIZE] = (packetCRC >> 8);     //MSB
  LMPackets[LMID - 1][slaveID - 1][MASTER_DATA_SIZE + 1] = packetCRC & 0xff; //LSB -> last byte -> DATA_SIZE + 1 = last position

  if (slaveID == NUMBER_OF_SLAVES - 48)
  {
    //packetsReady = true;
    for (k = 0; k < NUMBER_OF_LM - 48; k++)
    {
      for (j = 0; j < NUMBER_OF_SLAVES - 48; j++)
      {
        Serial2.print("Packet from master ");
        Serial2.print(MASTER_ID);
        Serial2.print(" and LM ");
        Serial2.print(k);
        Serial2.print(" and Slave ");
        Serial2.print(j);
        Serial2.print(" to be sent to the gateaway: [");
        for (n = 0; n < MASTER_DATA_SIZE_CRC; n++)
        {
          Serial2.print((int)LMPackets[k][j][n]);
          Serial2.print(" ");
        }
        Serial2.println("]");
        transmissionState = radio.startTransmit(LMPackets[k][j], sizeof(LMPackets[k][j]));
        enableInterrupt = true;
        while (!transmittedFlag)
        {
        }
        if (transmittedFlag)
        {
          enableInterrupt = false;
          transmittedFlag = false;

          if (transmissionState == ERR_NONE)
          {
            Serial2.println(F("transmission finished!"));
          }

          else
          {
            Serial2.print(F("failed, code "));
            Serial2.println(transmissionState);
          }
        }
      }
    }
  }
}

void MasterAskingPacket(int slaveID)
{
  Serial2.println("Telling the LM x to send the next packet");
  if (slaveID < NUMBER_OF_SLAVES - 48)
  {
    nextSlave = nextSlave + 1;
    waitingPacketLMID = canPacket[1] - '0';
    waitingPacketSlaveID = nextSlave;

    MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[1] = waitingPacketLMID + '0';
    MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[2] = nextSlave + '0';
    FDCANSend(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y, sizeof(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y));
  }
  else
  {
    nextSlave = 0;
    MasterWaitingComm(nextLM);
  }
}