/**
 * @file nucleo_g431kb_RS485.h
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Local Master RS485 Macros
 * @version 2.1
 * @date 2021-03-08
 * 
 * 
 */
#ifndef nucleo_g431kb_RS485_h
#define nucleo_g431kb_RS485_h

#include <SoftwareSerial.h>

//RS485 related
/**
 * @brief nucleo_g431kb's pin to connect to MAX485's Receive (RO)
 * 
 */
#define MAX485RX PB7
/**
 * @brief nucleo_g431kb's pin to connect to MAX485's Transmit (DI)
 * 
 */
#define MAX485TX PB6
/**
 * @brief nucleo_g431kb's pin to connect to MAX485's Direction control (DE)
 * 
 */
#define MAX485Control PA1

/**
 * @brief Used when setting the Direction control (DE) pin high.
 * 
 */
#define RS485Transmit HIGH
/**
 * @brief Used when setting the Direction control (DE) pin low.
 * 
 */
#define RS485Receive LOW

//Serial related
/**
 * @brief nucleo_g431kb's Serial2 RX pin
 * 
 */
#define Serial2RX PA3
/**
 * @brief nucleo_g431kb's Serial2 TX pin
 * 
 */
#define Serial2TX PA2
/**
 * @brief Defines 'Serial1' as nucleo_g431kb's USART2. Used for debug (Serial.print)
 * 
 * @return HardwareSerial 
 */
//HardwareSerial Serial2(Serial2RX, Serial2TX); // RX, TX

extern SoftwareSerial RS485Serial;
#endif
