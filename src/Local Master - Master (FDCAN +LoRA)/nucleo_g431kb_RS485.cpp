#include "nucleo_g431kb_RS485.h"
#include <SoftwareSerial.h>

/**
 * @brief Defines 'RS485Serial' as nucleo_g431kb's SoftwareSerial
 * 
 * @return SoftwareSerial 
 */
SoftwareSerial RS485Serial(MAX485RX, MAX485TX); // RX, TX