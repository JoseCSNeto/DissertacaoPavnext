#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

//#include <RadioLib.h>
#include <SimpleCan.h>
#include <SimpleFOC.h>

#include "nucleo_g431kb_RS485.h"

static void handleCanMessage(FDCAN_RxHeaderTypeDef rxHeader, uint8_t *rxData);
static void init_CAN(void);
static void Button_Down(void);


// pass in optional shutdown and terminator pins that disable transceiver and add 120ohm resistor respectively
SimpleCan can1(-1, -1);
SimpleCan::RxHandler can1RxHandler(64, handleCanMessage);

FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];

void setup()
{
	Serial2.begin(9600);
	delay(3000);
	Serial2.println("hello123");

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	attachInterrupt(PB0, Button_Down, RISING);
    delay(100);
    init_CAN();
}

void loop()
{
    // delay(1000);
    // Button_Down();

}

static void init_CAN()
{
	Serial2.println(can1.init(CanSpeed::Mbit1) == HAL_OK
					   ? "CAN: initialized."
					   : "CAN: error when initializing.");

	 //DCAN_FilterTypeDef sFilterConfig;

	// Configure Rx filter
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
	if (length >= 13)
	{
		return 32 + (13 - length) * 16;
	}
	else if (length == 12)
	{
		return 24;
	}
	else if (length >= 9)
	{
		return 12 + (9 - length) * 4;
	}
	return length;
}

static void handleCanMessage(FDCAN_RxHeaderTypeDef rxHeader, uint8_t *rxData)
{
    //Serial2.println("Recebi algo");

	int byte_length = dlcToLength(rxHeader.DataLength);

	Serial2.print("Received packet, id=0x");
	Serial2.print(rxHeader.Identifier, HEX);
	Serial2.print(", length=");
	Serial2.print(byte_length);
	for (int byte_index = 0; byte_index < byte_length; byte_index++)
	{
		Serial2.print(" byte[");
		Serial2.print(byte_index);
		Serial2.print("]=");
		Serial2.print(rxData[byte_index]);
		Serial2.print(" ");
	}
	Serial2.println();

	digitalToggle(LED_BUILTIN);
}

static void Button_Down()
{

	static uint8_t press_count = 0;

	press_count++;

	TxHeader.Identifier = 0x321;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.DataLength = FDCAN_DLC_BYTES_2;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_FD_CAN;// FDCAN_CLASSIC_CAN
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	TxData[0] = press_count;
	TxData[1] = 0xAD;
	TxData[2] = press_count;
	TxData[3] = 0xAD;
	TxData[4] = press_count;
	TxData[5] = 0xAD;
	TxData[6] = press_count;
	TxData[7] = press_count;


	Serial2.print("CAN: sending message ");
	Serial2.println(can1.addMessageToTxFifoQ(&TxHeader, TxData) == HAL_OK ? "was ok." : "failed.");
}