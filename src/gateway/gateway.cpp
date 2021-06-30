/**
 * @file gateway.cpp
 * @author José Neto (up201603912@fe.up.pt)
 * @brief gateway software
 * @version 1.3
 * @date 2021-06-03
 * 
 * @copyright Copyright (c) 2021
 */
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>
#include <CRC16.h>

#define CS 10
#define DIO1 42
#define RESET 1
#define BUSY 41

#define MOSI 35
#define MISO 37
#define SCLK 36

SX1262 radio = new Module(CS, DIO1, RESET, BUSY);

#define TX 40
#define RX 39

// save transmission state between loops
int transmissionState = ERR_NONE;
// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;
void setFlag(void);

#define DATA_SIZE 13
#define MASTER_DATA_SIZE DATA_SIZE + 1
#define MASTER_DATA_SIZE_CRC DATA_SIZE + 2

uint16_t calculatedCRC;
uint16_t packetCRC;

const char *ssid = "UPTEC";
const char *password = "UPTECNET";

//Your Domain name with URL path or IP address with path
const char *serverName = "http://tese-317713.ew.r.appspot.com/measurements/multiple";

const int capacity = JSON_OBJECT_SIZE(10);
StaticJsonDocument<capacity> doc;

/**
 * @brief Initialization routines
 * 
 */
void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  SPI.begin(SCLK, MISO, MOSI, CS);

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(868.0, 125.0, 9, 7, SX126X_SYNC_WORD_PRIVATE, 22, 8, 0, true);
  if (state == ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
  radio.setRfSwitchPins(RX, TX);

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);

  // start listening for LoRa packets
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
}

/**
 * @brief  This function is called when a complete packet is transmitted by the module
 * 
 */
void setFlag(void)
{
  // check if the interrupt is enabled
  if (!enableInterrupt)
  {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
}
/**
 * @brief Code running indefinitely
 * 
 */
void loop()
{
  // check if the flag is set
  if (receivedFlag)
  {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    // String str;
    // int state = radio.readData(str);

    // you can also read received data as byte array

    byte byteArr[MASTER_DATA_SIZE_CRC];
    char dataPacket[MASTER_DATA_SIZE_CRC];
    int state = radio.readData(byteArr, MASTER_DATA_SIZE_CRC);

    if (state == ERR_NONE)
    {
      // packet was successfully received
      Serial.println(F("[SX1262] Received packet!"));

      // print data of the packet
      Serial.print(F("[SX1262] Data:\t\t"));

      for (int i = 0; i < sizeof(byteArr); i++)
      {
        Serial.print((char)byteArr[i]);
      }
      Serial.println("");

      for (int i = 0; i < sizeof(byteArr); i++)
      {
        Serial.print(byteArr[i]);
        Serial.print(" ");
        dataPacket[i] = byteArr[i];
      }
      Serial.println("");

      //Serial.println(str);

      calculatedCRC = calc_crc(dataPacket, DATA_SIZE); //Calculate CRC
      Serial.print("\nCRC calculado: ");
      Serial.println(calculatedCRC,HEX);

      packetCRC = dataPacket[DATA_SIZE + 1] | (dataPacket[DATA_SIZE] << 8); //MSB | LSB
      Serial.print("packetCRC: ");        
      Serial.println(packetCRC,HEX);
      if (calculatedCRC == packetCRC)
      {
        Serial.println("CRC correct");
        //Check WiFi connection status
        if (WiFi.status() == WL_CONNECTED)
        {
          HTTPClient http;

          // Your Domain name with URL path or IP address with path
          http.begin(serverName);

          // If you need an HTTP request with a content type: application/json, use the following:
          http.addHeader("Content-Type", "application/json");
          doc["temperature"] = dataPacket[3] - 100; //added 100 when sampling because of negative values
          doc["humidity"] = dataPacket[4] / 2;
          doc["xaxis"] = dataPacket[5] / 5;
          doc["yaxis"] = dataPacket[6] / 5;
          doc["zaxis"] = dataPacket[7] / 5; 
          doc["speed"] = (dataPacket[8] / 100) * 3.6;
          doc["power_g1"] = dataPacket[9] / 10;
          doc["energy_g1"] = dataPacket[10] / 10;
          doc["power_g2"] = dataPacket[11] / 10;
          doc["energy_g2"] = dataPacket[12] / 10;

          String requestBody;

          serializeJson(doc, requestBody);
          Serial.println("in posting");
          Serial.println(requestBody);

          int httpResponseCode = http.POST(requestBody);

          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);

          // Free resources
          http.end();
        }
        else
        {
          Serial.println("WiFi Disconnected");
        }
      }
      else
              Serial.println("CRC incorrect");

      // // print RSSI (Received Signal Strength Indicator)
      // Serial.print(F("[SX1262] RSSI:\t\t"));
      // Serial.print(radio.getRSSI());
      // Serial.println(F(" dBm"));

      // // print SNR (Signal-to-Noise Ratio)
      // Serial.print(F("[SX1262] SNR:\t\t"));
      // Serial.print(radio.getSNR());
      // Serial.println(F(" dB"));
    }
    else if (state == ERR_CRC_MISMATCH)
    {
      // packet was received, but is malformed
      Serial.println(F("CRC error!"));
    }
    else
    {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);
    }

    // put module back to listen mode
    radio.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }
}
