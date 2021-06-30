#ifndef flagsmacros_h
#define flagsmacros_h

/**
* @brief Flag that tells all the slaves if the master is accepting communication
*
*/
bool LocalMasterAccepting = false;
/**
* @brief Flag that tells the specific slave X if the master is accepting communication
*
*/
bool LocalMasterAcceptingFromMe = false;
/**
* @brief Flag that tells the slave if the master is expecting a packet
*
*/
bool canSendPacket = false;
/**
* @brief Counter for the number of readings before sending a packet (when count = 10 the packet is data is ready to be sent)
*
*/
int count = 0;

/**
* @brief Flag that signals if the slave has completed the 10 readings
*
*/
bool sensorsReadingsReady = true;
/**
* @brief Flag that signals if the relative humidity is too high
*
*/
bool highRelativeHumidity = false;
/**
* @brief Flag that signals if the temperature is too high
*
*/
bool highTemperature = false;
/**
* @brief Detects which car wheel/half triggered the slave
*
*/
int Wheel = 0;
/**
* @brief How many cars have been detected
*
*/
int carCount = 0;
/**
 * @brief Minimum threshold of voltage reading to be considered as being '0' (the ADC doesn't read 0 often)
 * 
 */
#define vLimit 4

/**
 * @brief Number of sucessive readings below threshold to consider acquisition finished
 * 
 */
#define vNullCounterMax 6
/**
 * @brief Counts the number of sucessive readings of 'Voltage_1' below threshold
 * 
 */
int V1NullCounter = 0;
/**
 * @brief Counts the number of sucessive readings of 'Voltage_1' below threshold
 * 
 */
int V2NullCounter = 0;
/**
 * @brief Flag to enable sensor acquisiton after trigger event
 * 
 */
bool canRead = true;

/**
 * @brief Maximum number of power acquisitions (usually stops before reaching this value)
 * 
 */
#define countMax 500
/**
 * @brief Flag that tells the slave if the acquisition is finished 
 * 
 */
int acquisitionFinished = 0;
/**
 * @brief Flag that tells the slave if the reading is finished (different from acquisiton, because reading means two trigger events)
 * 
 */
bool readFinished = false;
/**
 * @brief Flag that tells the slave if it has warned the LM of a vehicle detection
 * 
 */
bool notifiedCarDetection = false;


bool firstGenFinish = false;
bool secondGenFinish = false;

#endif