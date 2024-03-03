/****************************************************************** 
  Project: DIP 3s BMS ESP32 Firmware V1
  Requirements:
    1. Read battery voltage
    2. Convert battery voltage ADC value to actual voltage value
    3. Post actual voltage to ThingSpeak
    4. TBC...

  WARNING: Remove sensitive info before pushing to Github
 ******************************************************************/

#include <WiFi.h>
#include "ThingSpeak.h"

//Pin Definitions
#define cell1Pin 32  //Cell 1 voltage pin
#define cell2Pin 33  //Cell 2 voltage pin
#define cell3Pin 35  //Cell 3 voltage pin

//WiFi Vars
const char* ssid = "ssid";         // ***Remove before pushing***
const char* pwd = "pwd";  //***Remove before pushing***
WiFiClient client;

//Cell Monitoring Vars
float cell1Voltage;
float cell2Voltage;
float cell3Voltage;

void setup() {
  Serial.begin(115200);  //Init serial

  ConnectWifi(); //This is a blocking function!!!

  ThingSpeak.begin(client);  //Initialize ThingSpeak

  pinMode(cell1Pin, INPUT);
  pinMode(cell2Pin, INPUT);
  pinMode(cell3Pin, INPUT);
}

void loop() {
  ConnectWifi(); //ConnectWifi called again in case Wifi abruptly disconnects 

  //Read cell voltages
  cell1Voltage = ReadCellVoltage(cell1Pin);
  cell2Voltage = 3.93; //Testing
  cell2Voltage = 3.56; //Testing
  //cell2Voltage = ReadCellVoltage(cell2Pin);
  //cell3Voltage = ReadCellVoltage(cell3Pin);

  //Publish cell voltages to ThingSpeak
  Serial.println(cell1Voltage);
  bool cell1UploadStatus = ThingSpeakWriteFloat(cell1Voltage,1);
  bool cell2UploadStatus = ThingSpeakWriteFloat(cell2Voltage,2);
  bool cell3UploadStatus = ThingSpeakWriteFloat(cell3Voltage,3);

  //Debugging
  if(!cell1UploadStatus) {
    Serial.println("Failed to Publish Cell 1 Voltage to ThingSpeak");
  }
  if(!cell2UploadStatus) {
    Serial.println("\nFailed to Publish Cell 2 Voltage to ThingSpeak");
  }
  if(!cell3UploadStatus) {
    Serial.println("\nFailed to Publish Cell 3 Voltage to ThingSpeak");
  }
    
  if(cell1UploadStatus && cell2UploadStatus && cell3UploadStatus) {
    Serial.println("\nSuccessfully Published Cell Voltages to ThingSpeak");
  }
}

void ConnectWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to Wifi");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pwd);
      Serial.print(".");
      delay(5000);  //Removing this delay will cause the ESP32 to crash while connecting to a wifi network
    }
    Serial.println("\nConnected to Wifi");
  }
}

float ReadCellVoltage(int cellVoltageSensePin) {
  //Conversion Vars
  const float conversionFactor = 4.2 / 4096;  //Li ion max cell voltage = 4.2 and ESP32 ADC = 12 bits so 2^12 = 4096
  const float conversionOffset = -0.1;        //ADC conversion offset

  int cellVoltageRaw = analogRead(cellVoltageSensePin);
  float cellVoltageActual = (cellVoltageRaw * conversionFactor) + conversionOffset;
  
  //Debugging
  Serial.println();
  Serial.print("Cell Voltage Raw: ");
  Serial.print(cellVoltageRaw);
  Serial.print(" Cell Voltage Actual: ");
  Serial.print(cellVoltageActual);
  Serial.println();

  return cellVoltageActual;
}

bool ThingSpeakWriteFloat(float data, int field) {
  //ThingSpeak Vars
  unsigned long myChannelId = Channel_ID;             //***Remove before pushing***
  const char* myWriteAPIKey = "";  //***Remove before pushing***
  const char* myReadAPIKey = "";   //***Remove before pushing***
  int writeStatus = false;

  //ThingSpeak.writeField(channel_id, field_num, data, write_api_key)
  int writeStatusCode = ThingSpeak.writeField(myChannelId, field, data, myWriteAPIKey);

  //ThingSpeak will return HTTP status code 200 if successful
  if (writeStatusCode == 200) {
    //Serial.println("Channel update successful.");
    writeStatus = true;
  } else {
    Serial.println("Problem updating ThingSpeak channel. HTTP error code " + String(writeStatusCode));
    writeStatus = false;
  }

  //If the data packets are sent out in under 15s intervals, HTTP status code 401 will be returned
  //Some packets will still be sent but this is not recommended
  //So set min delay to atleast 15s
  delay(20000);

  if(writeStatus) {
    return true;    
  } else {
    return false;
  }
}