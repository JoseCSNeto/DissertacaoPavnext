/**
 * @file nucleo_g431kb_FDCAN.h
 * @author José Neto (up201603912@fe.up.pt)
 * @brief Local Master FDCAN Macros
 * @version 1.0
 * @date 2021-03-18
 * 
 * 
 */

// #include <CRC16.h>
// #include <SimpleCan.h>
// #include "nucleo_g431kb_RS485.h"
// #include "communicationMacros.h"
//
// #include "LM_macros.h"
#include "LM_FDCAN.h"

// pass in optional shutdown and terminator pins that disable transceiver and add 120ohm resistor respectively
SimpleCan can1(-1, -1);
SimpleCan::RxHandler can1RxHandler(64, handleCanMessage);

FDCAN_TxHeaderTypeDef TxHeader;
//uint8_t TxData[8];



uint8_t canPacket[16];
uint8_t canSize = 0;
uint32_t canIdentifier = 0;
bool canReceived = false;

void FDCANInit()
{
    Serial2.println(can1.init(CanSpeed::Mbit1) == HAL_OK
                        ? "CAN: initialized."
                        : "CAN: error when initializing.");

    // FDCAN_FilterTypeDef sFilterConfig;

    // // Configure Rx filter
    // sFilterConfig.IdType = FDCAN_STANDARD_ID;
    // sFilterConfig.FilterIndex = 0;
    // sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    // sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    // sFilterConfig.FilterID1 = 0x321;
    // sFilterConfig.FilterID2 = 0x7FF;

    // can1.configFilter(&sFilterConfig);
    // can1.configGlobalFilter(FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    can1.activateNotification(&can1RxHandler);

    Serial2.println(can1.start() == HAL_OK
                        ? "CAN: started."
                        : "CAN: error when starting.");
}

int dlcToLength(uint32_t dlc)
{
    int length = dlc >> 16;
    // if (length >= 13)
    // {
    //     return 32 + (13 - length) * 16;
    // }
    // else if (length == 12)
    // {
    //     return 24;
    // }
    // else if (length >= 9)
    // {
    //     return 12 + (9 - length) * 4;
    // }
    return length;
}

void handleCanMessage(FDCAN_RxHeaderTypeDef rxHeader, uint8_t *rxData)
 {
     int packetLength = dlcToLength(rxHeader.DataLength);
     canSize = packetLength;
     canIdentifier = rxHeader.Identifier;
     for (int byte_index = 0; byte_index < packetLength; byte_index++)
    {
        // Serial2.print(" byte[");
        // Serial2.print(byte_index);
        // Serial2.print("]=");
        //Serial2.print((char)rxData[byte_index]);
       // Serial2.print(" ");
        canPacket[byte_index] = rxData[byte_index];
    }
    canReceived = true;

//     

//     Serial2.print("Received packet, id=0x");
 //    Serial2.print(rxHeader.Identifier, HEX);
//     Serial2.print(", length=");
//     Serial2.print(packetLength);
//     Serial2.print(" ");
//     for (int byte_index = 0; byte_index < packetLength; byte_index++)
//     {
//         // Serial2.print(" byte[");
//         // Serial2.print(byte_index);
//         // Serial2.print("]=");
//         Serial2.print((char)rxData[byte_index]);
//         Serial2.print(" ");
//         canPacket[byte_index] = rxData[byte_index];
//     }

//     Serial2.println();
//     if (!masterAccepting)
//     {
//         if (rxHeader.Identifier == masterID && packetLength == DEFAULT_MESSAGES_SIZE)
//         {
//             Serial2.println("?");
//             if (rxData[0] == MASTER_ACCEPTING_COMMUNICATION && rxData[1] == MY_LM_ID)
//             {
//                 Serial2.println("??");
//                 //Serial2.println("Telling the Slave x the Local Master is accepting communication");

//                 LMWaitingPacket = false;
//                 waitingCar = true;
//                 masterAccepting = true;
//                 nextSlave = nextSlave + 1;
//                 // if (slaveID < NUMBER_OF_SLAVES - 48)
//                 // {
//                 //     nextSlave = nextSlave + 1;
//                 // }
//                 // else
//                 // {
//                 //     nextSlave = 1;
//                 // }
//                 //     delay(1000);
//                 ACCEPTING_COMMUNICATION_SV_X[1] = 1 + '0';
//                 sendPacket((byte *)"hello", 6); // OX-> local master accepting communication from Slave_X
//                 //digitalToggle(LED_BUILTIN);
//                 //digitalWrite(LED_BUILTIN, LOW);
//                 //delay(500);

//             }
//         }
//     }

//     else if (masterWaitingPacket)
//     {
//         if (packetLength == MASTER_MESSAGES_SIZE)
//         {
//             if (rxHeader.Identifier == masterID)
//             {
//                 if (rxData[0] == MASTER_ASKING_PACKET && rxData[1] == MY_LM_ID) // && rxData[2] == waitingPacketLMID)
//                 {
//                     masterWaitingPacket = true;
//                     //waitingPacketLMID = rxData[1];
//                     waitingPacketSlaveID = rxData[2] - 48;

//                     Serial2.print("Packet from slave ");
//                     Serial2.print(waitingPacketSlaveID);
//                     Serial2.print(" to be sent to the Master: [");
//                     FDCANSend(slavesPackets[waitingPacketSlaveID], sizeof(slavesPackets[waitingPacketSlaveID]));
//                     for (int n = 0; n < MASTER_DATA_SIZE_CRC; n++)
//                     {
//                         Serial2.print((int)slavesPackets[waitingPacketSlaveID][n]);
//                         Serial2.print(" ");
//                     }
//                     Serial2.println("]");
//                     if (waitingPacketSlaveID + 48 == NUMBER_OF_SLAVES)
//                     {
//                         masterWaitingPacket = false;
//                         masterAccepting = false;
//                     }
//                 }

//                 //MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[1] = waitingPacketLMID;
//                 //MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[2] = SLAVE_ID_1;
//                 //FDCANSend(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y, sizeof(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y));
//             }
//         }
//     }

//     // digitalToggle(LED_BUILTIN);
}

void FDCANSend(uint8_t *TxData, uint32_t dataLength)
{

    static uint8_t press_count = 0;

    press_count++;

    TxHeader.Identifier = 0x1;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = dataLength << 16;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_FD_CAN;
    TxHeader.FDFormat = FDCAN_FD_CAN; // FDCAN_CLASSIC_CAN
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    // Serial2.print("CAN: sending message ");
    // Serial2.println(can1.addMessageToTxFifoQ(&TxHeader, TxData) == HAL_OK ? "was ok." : "failed.");
    if (can1.addMessageToTxFifoQ(&TxHeader, TxData) == HAL_OK)
    {
    }
    else
    {
    }
}
