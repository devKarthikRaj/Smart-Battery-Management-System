/****************************************************************** 
  Project: DIP 3s BMS ESP32 Firmware V1
  Requirements:
    1. Read battery voltage
    2. Convert battery voltage ADC value to actual voltage value
    3. Post actual voltage to ThingSpeak
    4. TBC...
 ******************************************************************/

#include <WiFi.h>
#include "ThingSpeak.h"

//ESP32 WiFi Vars
const char* ssid = "WiFi Name";           // ***Remove before pushing*** 
const char* password = "WiFi Password";   //***Remove before pushing***
WiFiClient  client;

//ThingSpeak Vars
unsigned long myChannelId = Channel ID;     //***Remove before pushing***
const char *myWriteAPIKey = "Write API Key";//***Remove before pushing***
const char *myReadAPIKey = "Read API Key";  //***Remove before pushing***

// Data Var
float randomData;

void setup() {
  Serial.begin(115200);  //Initialize serial
  
  WiFi.mode(WIFI_STA);   //Set WiFi mode to station mode - ESP32 connects to an access point
  
  ThingSpeak.begin(client);  //Initialize ThingSpeak
}

void loop() {
    // Connect/reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      Serial.print(".");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        Serial.print(".");
        delay(5000); //Removing this delay will cause the ESP32 to crash while connecting to a wifi network
      } 
      Serial.println("\nConnected.");
    }

    //Enter a random value into randomData
    randomData = 36.6;
    
    //--------------------------------------------Write--------------------------------------------------
    /*Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
      pieces of information in a channel.  Here, we write to field 1 */
    int writeStatusCode = ThingSpeak.writeField(myChannelId, 1, randomData, myWriteAPIKey);

    //ThingSpeak will return HTTP status code 200 if successful
    if(writeStatusCode == 200){
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(writeStatusCode));
    }

    //If the data packets are sent out in under 15s intervals, HTTP status code 401 will be returned
    //Some packets will still be sent but this is not recommended
    //So...Min delay is 15s
    delay(20000);

    //---------------------------------------------Read--------------------------------------------------
    //Read data from ThingSpeak
    float readData = ThingSpeak.readFloatField(myChannelId, 1, myReadAPIKey);
    int readStatusCode = ThingSpeak.getLastReadStatus();
    //ThingSpeak will return HTTP status code 200 if successful
    if(readStatusCode == 200){
      Serial.println(String(readData));
    } else {
      Serial.println("Problem reading from channel. HTTP error code " + String(readStatusCode));
    }

    delay(20000);
}