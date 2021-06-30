#ifndef powermacros_h
#define powermacros_h

/**
* @brief Voltage of ADC #0 (Power generator V2)
*
*/
double voltage_V2 = 0;
/**
* @brief Voltage of ADC #1 (Power generator V1)
*
*/
double voltage_V1 = 0;
/**
* @brief Voltage of ADC #2 (Power generator I2)
*
*/
double current_I2 = 0;
/**
* @brief Voltage of ADC #3 (Power generator I1)
*
*/
double current_I1 = 0;
/**
* @brief Aux variable for ADC readings
*
*/
double Voltage;
/**
* @brief Calculated instantaneous power of generator #1
*
*/
double power_I = 0;
/**
* @brief Calculated average power of generator #1
*
*/
double avgPower_I = 0;
/**
* @brief Calculated energy of generator #1
*
*/
double energy_I = 0;
/**
* @brief Calculated instantaneous power of generator #2
*
*/
double power_II = 0;
/**
* @brief Calculated average power of generator #2
*
*/
double avgPower_II = 0;
/**
* @brief Calculated energy of generator #2
*
*/
double energy_II = 0;



#endif
