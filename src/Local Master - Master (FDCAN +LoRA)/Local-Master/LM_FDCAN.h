/**
 * @file LM_FDCAN.h
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Local Master FDCAN
 * @version 1.5
 * @date 2021-03-25
 * 
 * 
 */

#ifndef LM_FDCAN_h
#define LM_FDCAN_h

#include <CRC16.h>
#include <SimpleCan.h>
#include "communicationMacros.h"
#include "LM_macros.h"

void handleCanMessage(FDCAN_RxHeaderTypeDef rxHeader, uint8_t *rxData);
void FDCANInit(void);
void FDCANSend(uint8_t *TxData, uint32_t dataLength);
int dlcToLength(uint32_t dlc);
/**
 * @brief Retrieved data from the packet received (excludes the CRC)
 * 
 */
extern char dataFromPacket[DATA_SIZE];




#endif
