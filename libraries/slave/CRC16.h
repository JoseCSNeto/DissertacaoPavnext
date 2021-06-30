#ifndef CRC16_H
#define CRC16_H
#define CRC_SIZE 4


#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

uint16_t calc_crc(char *msg,int n);
uint16_t crc_xmodem_update (uint16_t crc, uint8_t data);
void convert_crc_to_string (uint16_t crc, char crc_string[CRC_SIZE + 1]);

#endif
