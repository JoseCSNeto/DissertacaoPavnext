#include "LM_macros.h"

bool LMWaitingPacket = false;
bool masterWaitingPacket = false;
bool masterAccepting = false;

//int waitingPacketLMID = 0;
int waitingPacketSlaveID = 0;
int packetReceivedLMID = 0;

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
byte MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[MASTER_MESSAGES_SIZE] = {MASTER_ASKING_PACKET, (byte)'x', (byte)'y'}; // Q9X-> waiting for packet from lm_x slave_y
byte LM_X_READY_TO_SEND_PACKET[DEFAULT_MESSAGES_SIZE] = {LM_READY_TO_SEND_PACKET, (byte)'x'};
uint8_t slavesPackets[NUMBER_OF_SLAVES - 48][MASTER_DATA_SIZE_CRC];


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
 * @brief Defines who is the next slave to send the 'accepting_communication' macro
 * 
 */
uint8_t nextSlave = 0;
/**
 * @brief Slave ID from the received packet. When compared to 'waitingPacketSlaveID' validates or excludes the packet
 * 
 */
uint8_t packetReceivedID = 0;
/**
 * @brief Packets are ready to be sent to the master
 * 
 */
bool packetsReady = false;

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
 * @brief Aux variables, used in 'for' loops
 * 
 */
uint8_t n = 0;
uint8_t j = 0;


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