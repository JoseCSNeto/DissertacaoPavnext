/**
 * @file main.h
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Local Master RS485 Macros
 * @version 2.1
 * @date 2021-03-08
 * 
 * 
 */
#ifndef main_h
#define main_h

#include <SoftwareSerial.h>
#include <CRC16.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

//RS485 related
/**
 * @brief nucleo_g431kb's pin to connect to MAX485's Receive (RO)
 * 
 */
#define MAX485RX PB4
/**
 * @brief nucleo_g431kb's pin to connect to MAX485's Transmit (DI)
 * 
 */
#define MAX485TX PB5
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
 * @brief nucleo_g431kb's Serial1 RX pin
 * 
 */
#define Serial1RX PA3
/**
 * @brief nucleo_g431kb's Serial1 TX pin
 * 
 */
#define Serial1TX PA2
/**
 * @brief Defines 'Serial1' as nucleo_g431kb's USART1. Used for debug (Serial.print)
 * 
 * @return HardwareSerial 
 */
HardwareSerial Serial1(Serial1RX, Serial1TX); // RX, TX
/**
 * @brief Defines 'RS485Serial' as nucleo_g431kb's SoftwareSerial
 * 
 * @return SoftwareSerial 
 */
SoftwareSerial RS485Serial(MAX485RX, MAX485TX); // RX, TX

#endif
