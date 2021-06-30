/**
 * @file master_FDCAN.h
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Local Master FDCAN Macros
 * @version 1.0
 * @date 2021-03-18
 * 
 * 
 */

#ifndef master_FDCAN_h
#define master_FDCAN_h

#include <CRC16.h>
#include <SimpleCan.h>
#include "communicationMacros.h"
#include "master_macros.h"


void handleCanMessage(FDCAN_RxHeaderTypeDef rxHeader, uint8_t *rxData);
void FDCANInit(void);
void FDCANSend(uint8_t *TxData, uint32_t dataLength);
int  dlcToLength(uint32_t dlc);


#endif
