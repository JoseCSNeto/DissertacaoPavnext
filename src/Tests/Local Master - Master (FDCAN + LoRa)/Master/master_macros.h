#ifndef master_macros_h
#define master_macros_h


#include <CRC16.h>
#include <SimpleCan.h>
#include "communicationMacros.h"
#include "master_FDCAN.h"
#include "nucleo_g431kb_LoRa.h"


#define LOCAL_MASTER_ID 1

// uint16_t CRC_calculated;
// uint16_t CRC_packet;

// char peek;
// byte byteReceived;
// byte byteSend;
// byte packet_received[DATA_SIZE_CRC];
// byte data_packet[DATA_SIZE];
// byte default_message[DEFAULT_MESSAGES_SIZE];
// byte alert_message[ALERT_MESSAGES_SIZE];

//DEFAULT MESSAGES
extern uint8_t MASTER_ACCEPTING_COMMUNICATION_LM_X[DEFAULT_MESSAGES_SIZE];// = {MASTER_ACCEPTING_COMMUNICATION, (byte)'x'};                        // M9-> accepting communication from LM
// byte MASTER_ASKING_FOR_PACKET_SLAVE_1[MASTER_MESSAGES_SIZE] = {MASTER_ASKING_PACKET, LOCAL_MASTER_ID, SLAVE_ID_1}; // Q91-> waiting for packet from slave_X
// byte MASTER_ASKING_FOR_PACKET_SLAVE_2[MASTER_MESSAGES_SIZE] = {MASTER_ASKING_PACKET, LOCAL_MASTER_ID, '2'};        // Q92-> waiting for packet from slave_X
// byte MASTER_ASKING_FOR_PACKET_SLAVE_3[MASTER_MESSAGES_SIZE] = {MASTER_ASKING_PACKET, LOCAL_MASTER_ID, '3'};        // Q93-> waiting for packet from slave_X
// byte MASTER_ASKING_FOR_PACKET_SLAVE_4[MASTER_MESSAGES_SIZE] = {MASTER_ASKING_PACKET, LOCAL_MASTER_ID, '4'};        // Q94-> waiting for packet from slave_X
extern byte MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[MASTER_MESSAGES_SIZE];// = {MASTER_ASKING_PACKET, (byte)'x', (byte)'y'}; // Q9X-> waiting for packet from lm_x slave_y

//FLAGS
extern bool waitingPacket; //flag that tells the master if it is waiting for a packet

extern uint32_t waitingPacketLMID;
extern uint32_t waitingPacketSlaveID;
extern uint32_t packetReceivedLMID;

/**
 * @brief Retrieved data from the packet received (excludes the CRC)
 * 
 */
extern char dataFromPacket[MASTER_DATA_SIZE_CRC];

/**
 * @brief CRC retrieved from the packet received.
 * 
 */
extern uint16_t packetCRC;

/**
 * @brief Calculated CRC from the packet received. When compared to 'packetCRC' validates or excluded the packet
 * 
 */
extern uint16_t calculatedCRC;

/**
 * @brief Defines who is the next LM to send the 'accepting_communication' macro
 * 
 */
extern uint8_t nextLM;
/**
 * @brief Defines what is next slave packet to be sent by the lm
 * 
 */
extern uint8_t nextSlave;

//timers
#define NUMBER_OF_LM '1'

/**
 * @brief Array of LM's packets to be sent later from the Master to the gateway
 * 
 */
extern uint8_t LMPackets[NUMBER_OF_LM - 48][NUMBER_OF_SLAVES - 48][MASTER_DATA_SIZE_CRC];

/**
 * @brief Aux variables, used in 'for' loops
 * 
 */
extern uint8_t n;// = 0;
extern uint8_t k;// = 0;
extern uint8_t j;// = 0;


void MasterWaitingComm(int LMID);
void MasterStorePacket(int LMID, int slaveID);
void MasterAskingPacket(int slaveID);
extern bool canReceived;
extern uint32_t canIdentifier;
extern uint8_t canSize;
extern uint8_t canPacket[16];

#endif