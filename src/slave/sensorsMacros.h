#ifndef sensormacros_h
#define sensormacros_h

#include <Adafruit_HTS221.h>
#include <Adafruit_Sensor.h>
#include "MCP3221.h"
#include "SparkFun_ADXL345.h"

//#define measureCRVoltage 
#define HUM_RH_MEASURE 

/**
 * @brief Maximum value of relative humidity
 * 
 */
#define MAX_RLTV_HUMD 80
/**
 * @brief Maximum value of temperature
 * 
 */
#define MAX_TEMP 70

/**
* @brief  Class that stores state and functions for interacting with
*            the HTS221 I2C Digital Potentiometer (from the original documentation)
*
*/
Adafruit_HTS221 hts;
/**
* @brief  Sensor event (36 bytes). struct sensor_event_s is used to provide a single sensor event in a common format. (from the original documentation)
*
*/
sensors_event_t temp, humidity;
/**
* @brief temperature and rh int type
*
*/
int temperature, rh;
/**
* @brief ADXL345 struct
*
*/
ADXL345 adxl = ADXL345(); // USE FOR I2C COMMUNICATION
/**
* @brief Accelerometer readings
*
*/
int x, y, z;
/**
* @brief Calibration factor for z-axis readings
*
*/
double factor = 0.30625;
/**
* @brief Accelerometer interrupt trigger
*
*/
bool carDetectedTrigger = false;
/**
* @brief Used when clearing the accelerometer's interrupt by reading its source
*
*/
byte ADXInterruptSource;

/**
* @brief I2C address of the ADX345
*
*/
const byte ADX345_ID = 0x53;
/**
* @brief I2C address of the MCP3221 A0
*
*/
const byte CR1_ADDR = 0x48;
/**
* @brief I2C address of the MCP3221 A5
*
*/
const byte CR2_ADDR = 0x4D;
/**
* @brief MCP3221 class
*
* @return MCP3221
*/
MCP3221 mcp3221_cr1(CR1_ADDR), mcp3221_cr2(CR2_ADDR);
/**
* @brief Stores the calculated "connecting rod" voltage
*
*/
double CR1Voltage, CR2Voltage;

/**
* @brief CR's reference voltage
*
*/
#define VREF 4999

/**
* @brief Message sent when "CR1" isn't detected when initializing it
*
*/
char failedCR1[] = "BIELA1_FAILED\n";
/**
* @brief Message sent when "CR2" isn't detected when initializing it
*
*/
char failedCR2[] = "BIELA2_FAILED\n";
/**
* @brief Message sent when the temperature sensor isn't detected when initializing it
*
*/
char failedHTS[] = "HTS_FAILED\n";
/**
* @brief Message sent when the accelerometer isn't detected when initializing it
*
*/
char failedADX[] = "ADX_FAILED\n";

/**
* @brief Maximum value of voltage detected in "CR1"
*
*/
double maxVoltageCR1 = 0;
/**
* @brief Maximum value of voltage detected in "CR2"
*
*/
double maxVoltageCR2 = 0;
/**
* @brief Measured X-axis acceleration
*
*/
float measuredXAccel = 0;
/**
* @brief Measured Y-axis acceleration
*
*/
float measuredYAccel = 0;
/**
* @brief Measured Z-axis acceleration
*
*/
float measuredZAccel = 0;
/**
* @brief Maximum X-axis acceleration
*
*/
float maxXAccel = 0;
/**
* @brief Maximum Y-axis acceleration
*
*/
float maxYAccel = 0;
/**
* @brief Maximum Z-axis acceleration
*
*/
float maxZAccel = 0;
/**
* @brief Vehicle's speed
*
*/
int speed;
/**
 * @brief Average distance between rotational centers of vehicle's wheels in cm
 * 
 */
#define WHEELBASE 254

#endif
