#include "master_FDCAN.h"
#include "master_macros.h"

// pass in optional shutdown and terminator pins that disable transceiver and add 120ohm resistor respectively
SimpleCan can1(-1, -1);
SimpleCan::RxHandler can1RxHandler(64, handleCanMessage);

FDCAN_TxHeaderTypeDef TxHeader;
//uint8_t TxData[8];

// bool waitingPacket = false; //flag that tells the master if it is waiting for a packet
// uint32_t waitingPacketLMID = 0;
// uint32_t waitingPacketSlaveID = 0;
// uint32_t packetReceivedLMID = 0;

uint8_t canPacket[16];
uint8_t canSize = 0;
uint32_t canIdentifier = 0;
bool canReceived = false;

void FDCANInit(void)
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
	// 	return 32 + (13 - length) * 16;
	// }
	// else if (length == 12)
	// {
	// 	return 24;
	// }
	// else if (length >= 9)
	// {
	// 	return 12 + (9 - length) * 4;
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


	// int packetLength = dlcToLength(rxHeader.DataLength);

	// Serial2.print("Received packet, id=0x");
	//  Serial2.print(rxHeader.Identifier, HEX);
	// Serial2.print(", length=");
	// Serial2.print(packetLength);
	// Serial2.print(" ");
	// for (int byte_index = 0; byte_index < packetLength; byte_index++)
	// {
	// 	//Serial2.print(" byte[");
	// 	//Serial2.print(byte_index);
	// 	//Serial2.print("]=");
	// 	Serial2.print((char)rxData[byte_index]);
	// 	Serial2.print(" ");
	// }
	// Serial2.println();
	// if (waitingPacket)
	// {
	// 	if (packetLength == DEFAULT_MESSAGES_SIZE)
	// 	{
	// 		if (rxHeader.Identifier + '0'== waitingPacketLMID)
	// 		{
	// 			if (rxData[0] == LM_READY_TO_SEND_PACKET && rxData[1] == waitingPacketLMID + '0')
	// 			{
	// 				//waitingPacket = 1;
	// 				waitingPacketLMID = rxData[1];
	// 				waitingPacketSlaveID = SLAVE_ID_1;

	// 				MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[1] = waitingPacketLMID;
	// 				MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y[2] = SLAVE_ID_1;
	// 				FDCANSend(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y, sizeof(MASTER_ASKING_FOR_PACKET_LM_X_SLAVE_Y));
	// 			}
	// 		}
	// 	}
	// 	else if (packetLength == DATA_SIZE_CRC)
	// 	{
	// 		if (rxHeader.Identifier + '0' == waitingPacketLMID)
	// 		{
	// 			if (rxData[0] == waitingPacketLMID && rxData[1] == waitingPacketSlaveID)
	// 			{
	// 				for (int byte_index = 0; byte_index < packetLength; byte_index++)
	// 				{
	// 					dataFromPacket[byte_index] = rxData[byte_index];
	// 				}
	// 				calculatedCRC = calc_crc(dataFromPacket, DATA_SIZE); //Calculate CRC
	// 				//Serial2.print("\nCRC calculado: ");
	// 				//Serial2.println(calculatedCRC,HEX);

	// 				packetCRC = rxData[DATA_SIZE + 1] | (rxData[DATA_SIZE] << 8); //MSB | LSB
	// 				if (calculatedCRC == packetCRC)
	// 				{
	// 					MasterStorePacket(packetReceivedLMID, waitingPacketSlaveID); //ASCII code to correspondent number. ex. ('1') 49 - 48  -> 1
	// 					Serial2.println("CRC correct");
	// 				}
	// 			}
	// 		}
	// 	}
	// }
	// // digitalToggle(LED_BUILTIN);
}

void FDCANSend(uint8_t *TxData, uint32_t dataLength)
{

	static uint8_t press_count = 0;

	press_count++;

	TxHeader.Identifier = 0x9; 
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
