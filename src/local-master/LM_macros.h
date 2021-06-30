#ifndef LM_macros_h
#define LM_macros_h

#include <CRC16.h>
#include <SimpleCan.h>
#include "nucleo_g431kb_RS485.h"
#include "communicationMacros.h"


/**
 * @brief  Local Master ID. Only need to change this value in order to be used by different boards
 * 
 */
#define MY_LM_ID '1'

extern bool LMWaitingPacket;
/**
 * @brief Flag that tells the local-master if the master is accepting packets from it
 * 
 */
extern bool masterWaitingPacket;
/**
 * @brief Flag that tells the local-master if the master is accepting communication from it
 * 
 */
extern bool masterAccepting;

//extern int waitingPacketLMID;
extern int waitingPacketSlaveID;
extern int packetReceivedLMID;

/**
 * @brief Array of slave's packets to be sent later from the LM to the Master
 * 
 */
extern uint8_t slavesPackets[NUMBER_OF_SLAVES - 48][MASTER_DATA_SIZE_CRC];

extern uint32_t masterID; //master fdcan ID

//DEFAULT MESSAGES

/**
 * @brief Pre-defined message for 'OX' -> notifies the slave #X that it can communicate
 * 
 */
extern byte ACCEPTING_COMMUNICATION_SV_X[DEFAULT_MESSAGES_SIZE];
/**
 * @brief Pre-defined message for 'PX' -> notifies the slave #X that a packet from it is expected
 * 
 */
extern byte WAITING_PACKET_SV_X[DEFAULT_MESSAGES_SIZE];
/**
 * @brief Pre-defined message for 'LX' -> notifies the Master that the LM x can send a packet
 * 
 */
extern byte LM_X_READY_TO_SEND_PACKET[DEFAULT_MESSAGES_SIZE];// = {LM_READY_TO_SEND_PACKET, (byte)'x'};

extern byte MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[MASTER_MESSAGES_SIZE];// = {MASTER_ASKING_PACKET, (byte)'x', (byte)'y'}; // Q9X-> waiting for packet from lm_x slave_y


//packet arrays initialization
/**
 * @brief Packet received
 * 
 */
extern byte packetReceived[DATA_SIZE_CRC];
/**
 * @brief Retrieved data from the packet received (excludes the CRC)
 * 
 */
extern char dataFromPacket[DATA_SIZE];

/**
 * @brief Used in messages such as 'CX' (car detected by slave #X)
 * 
 */
extern byte defaultMessage[DEFAULT_MESSAGES_SIZE];
/**
 * @brief Used in messages such as high/low temperature alerts sent by slaves
 * 
 */
extern byte alertMessage[ALERT_MESSAGES_SIZE];



//flags


/**
 * @brief Flag that tells the local-master if it is waiting for a car
 * 
 */
extern bool waitingCar;
/**
 * @brief Flag that tells the local-master if it is waiting for a packet
 * 
 */
extern bool waitingPacket;
/**
 * @brief Defines who is the next slave to send the 'accepting_communication' macro
 * 
 */
extern uint8_t nextSlave;
/**
 * @brief Slave ID from the received packet. When compared to 'waitingPacketSlaveID' validates or excludes the packet
 * 
 */
extern uint8_t packetReceivedID;
/**
 * @brief Packets are ready to be sent to the master
 * 
 */
extern bool packetsReady;

/**
 * @brief Detects which car wheel/half triggered the slave 
 * 
 */
extern uint8_t Wheel;

/**
 * @brief Available RS485 bytes
 * 
 */
extern uint8_t bytes;

//timers
/**
 * @brief Used for waiting for 20 ms to send 'WAITING_PACKET_SV_X'
 * 
 */
extern unsigned long slaveTimer;

/**
 * @brief Aux variables, used in 'for' loops
 * 
 */
extern uint8_t n;
extern uint8_t j;

extern bool canReceived;
extern uint32_t canIdentifier;
extern uint8_t canSize;
extern uint8_t canPacket[16];

extern void sendPacket(byte *buf, int len);
#endif
