#include "CRC16.h"
#define CRC_SIZE 4

uint16_t calc_crc(char *msg,int n)
{
  uint16_t x = 0;

  while(n--)
  {
    x = crc_xmodem_update(x, (uint16_t)*msg++);
  }

  return(x);
}

uint16_t crc_xmodem_update (uint16_t crc, uint8_t data)
{
  int i;

  crc = crc ^ ((uint16_t)data << 8);
  for (i=0; i<8; i++)
  {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021; //(polynomial = 0x1021)
    else
      crc <<= 1;
  }
  return crc;
}

void convert_crc_to_string (uint16_t crc, char crc_string[CRC_SIZE + 1])
{
  sprintf(crc_string, "%X", crc);         // passa-se CRC para char

  int crc_string_length = strlen(crc_string);
  if (crc_string_length < CRC_SIZE)
  {
    char aux[CRC_SIZE + 1];

    if(crc_string_length == 1)
    {
      aux[0] = '0';
      aux[1] = '0';
      aux[2] = '0';
      aux[3] = crc_string[0];
      aux[4] = '\0';
    }

    else if(crc_string_length == 2)
    {
      aux[0] = '0';
      aux[1] = '0';
      aux[2] = crc_string[0];
      aux[3] = crc_string[1];
      aux[4] = '\0';
    }    
    else
    {
      aux[0] = '0';
      aux[1] = crc_string[0];
      aux[2] = crc_string[1];
      aux[3] = crc_string[2];
      aux[4] = '\0';      
    }
    strcpy(crc_string,aux);         
  }
}
