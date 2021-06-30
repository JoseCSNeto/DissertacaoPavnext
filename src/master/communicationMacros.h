#ifndef commmacros_h
#define commmacros_h
//Communication macros

/**
 * @brief Char 'M'
 * 
 */
#define MASTER_ACCEPTING_COMMUNICATION 'M'
/**
 * @brief Char 'O'
 * 
 */
#define MASTER_ASKING_PACKET 'Q'

#define LOCAL_MASTER_ACCEPTING_COMMUNICATION 'O'
/**
 * @brief Char 'L'
 * 
 */
#define LM_READY_TO_SEND_PACKET 'L'
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
#define MASTER_DATA_SIZE DATA_SIZE + 1
/**
 * @brief Data + CRC size from LM's packet
 * 
 */
#define MASTER_DATA_SIZE_CRC MASTER_DATA_SIZE + 2
/**
 * @brief Size of messages such as 'OX', 'CX' or 'PX' 
 * 
 */
#define DEFAULT_MESSAGES_SIZE 2
/**
 * @brief Size of messages such as high/low temperature alerts sent by slaves
 * 
 */
#define ALERT_MESSAGES_SIZE 3
#define MASTER_MESSAGES_SIZE 3


#endif
