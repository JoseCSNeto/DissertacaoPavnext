#ifndef commmacros_h
#define commmacros_h
//Communication macros

/**
 * @brief Char 'M'
 * 
 */
#define MASTER_ACCEPTING_COMMUNICATION 'M'
/**
 * @brief Char 'A'
 * 
 */
#define LOCAL_MASTER_ACCEPTING_COMMUNICATION 'A'
/**
 * @brief Char 'X'
 * 
 */
#define LOCAL_MASTER_ACCEPTING_COMMUNICATION_FROM_ALL_SLAVES 'X'
/**
 * @brief Char 'C'
 * 
 */
#define CAR_DETECTED 'C'
/**
 * @brief Char 'P'
 * 
 */
#define WAITING_FOR_PACKET 'P'
/**
 * @brief Char 'P'
 * 
 */
#define CAN_SEND_PACKET 'P'
/**
 * @brief Char 'H' used for example in Relative humidity or Temperature
 * 
 */
#define HIGH_VALUE 'H'
/**
 * @brief Char 'N' used for example in Relative humidity or Temperature
 * 
 */
#define NORMAL_VALUE 'N'
/**
 * @brief Char 'R' of Relative humidity
 * 
 */
#define RLTV_HUMDT 'R'
/**
 * @brief Char 'T' of Temperature
 * 
 */
#define TEMPERATURE 'T'
/**
 * @brief Char '9'
 * 
 */
#define MASTER_ID '9'
/**
 * @brief Char '1'
 * 
 */
#define SLAVE_ID_1 '1'
/**
 * @brief Number of slaves in the system. Helps automatizing the algorithm
 * 
 */
#define NUMBER_OF_SLAVES '2'

//messages sizes macros
/**
 * @brief Size of the data from slave's packet
 * 
 */
#define DATA_SIZE 12
/**
 * @brief Data + CRC size from slave's packet
 * 
 */
#define DATA_SIZE_CRC DATA_SIZE + 2
/*
 * @brief Size of the data from LM's packet
 * 
 */
#define MASTER_DATA_SIZE 10
/**
 * @brief Data + CRC size from LM's packet
 * 
 */
#define MASTER_DATA_SIZE_CRC MASTER_DATA_SIZE + 2
/**
 * @brief Size of messages such as 'OX', 'CX' (car detected by slave #X) or 'PX' 
 * 
 */
#define DEFAULT_MESSAGES_SIZE 2
/**
 * @brief Size of messages such as high/low temperature alerts sent by slaves
 * 
 */
#define ALERT_MESSAGES_SIZE 3

//Variables
/**
* @brief Next byte in RS485's line. Doesn't actually retrieve it from the there
*
*/
uint8_t peek;
/**
* @brief Data packet to be sent to the Local-Master
*
*/
char dataPacket[DATA_SIZE_CRC];
/**
* @brief Calculated CRC for the data packet
*
*/
uint16_t calculatedCRC;
/**
* @brief Available RS485 bytes
*
*/
uint8_t bytes = 0;

//DEFAULT MESSAGES
/**
* @brief Pre-defined message for 'HRX' -> notifies the RH is too high
*
*/
char HIGH_RLTV_HUMDT[ALERT_MESSAGES_SIZE] = {HIGH_VALUE, RLTV_HUMDT, MY_SLAVE_ID};
/**
* @brief Pre-defined message for 'NRX' -> notifies the RH is back to normal
*
*/
char NORMAL_RLTV_HUMDT[ALERT_MESSAGES_SIZE] = {NORMAL_VALUE, RLTV_HUMDT, MY_SLAVE_ID};
/**
* @brief Pre-defined message for 'HTX' -> notifies the temperature is too high
*
*/
char HIGH_TEMP[ALERT_MESSAGES_SIZE] = {HIGH_VALUE, TEMPERATURE, MY_SLAVE_ID};
/**
* @brief Pre-defined message for 'NTX' -> notifies the temperature is back to normal
*
*/
char NORMAL_TEMP[ALERT_MESSAGES_SIZE] = {NORMAL_VALUE, TEMPERATURE, MY_SLAVE_ID};

#endif
