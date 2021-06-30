#ifndef timermacros_h
#define timermacros_h

/**
 * @brief In order to prevent false triggers, they have to be separated by a certain time interval
 * 
 */
unsigned long timeSinceLastRead;
/**
 * @brief Flag to signal that the first triggering event has started a reading
 * 
 */
bool firstReadStart = false;
/**
 * @brief Flag to signal that the first reading has ended
 * 
 */
bool firstReadFinish = false;
/**
 * @brief Flag to signal that the second triggering event has started a reading
 * 
 */
bool secondReadStart = false;
/**
 * @brief Flag to signal that the second reading has ended
 * 
 */
bool secondReadFinish = false;
/**
 * @brief Microseconds that elapsed since the power-up until the finishing of the first read
 * 
 */
unsigned long firstReadEndMicros = 0;
/**
 * @brief Microseconds that elapsed since the power-up until the first trigger event
 * 
 */
unsigned long firstDetectionMicros = 0;
/**
 * @brief Microseconds that elapsed since the power-up until the second trigger event
 * 
 */
unsigned long secondDetectionMicros = 0;

/**
 * @brief Microseconds that elapsed since the power-up until re-enabling the trigger event
 * 
 */
unsigned long reEnableInterruptMicros = 0;
/**
 * @brief Counts the microseconds that elapsed between trigger events (used to calculate the speed of vehicles)
 * 
 */
unsigned long intervalBetweenDetections = 0;
/**
 * @brief Counts the microseconds that elapsed between readings (used to calculate the energy generated)
 * 
 */
unsigned long betweenReadingsTimer = 0;
/**
 * @brief Time interval between trigger events to prevent false detections
 * 
 */
#define readTimeout 1000

#endif