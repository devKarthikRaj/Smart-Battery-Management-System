/****************************************************************** 
  Project: DIP 3s BMS ESP32 Firmware V1
  Requirements:
    1. Read battery voltage                                      (Done)
    2. Convert battery voltage ADC value to actual voltage value (Done)
    3. Read temp sensor                                          (Done)
    4. Activate the fan if temp exceeds a preset threshold       (Done)
    5. Calculate SoC                                             (Done)
    6. Calculate SoH                                             (Done)
    7. Drive the RGB LED based on combined SoH for all 3 cells   (Done)
    8. Post all data to ThingSpeak                               (Done)
    9. Post all data to Blynk                                    (Done)

  WARNING: Remove sensitive info before pushing to Github
 ******************************************************************/

#include <WiFi.h>
#include "ThingSpeak.h"

//Pin Definitions
const int cell1Pin      =  32;  //Cell 1 voltage pin
const int cell2Pin      =  33;  //Cell 2 voltage pin
const int cell3Pin      =  35;  //Cell 3 voltage pin
const int tempSensePin  =  34;  //Temperature sensor pin
const int fanConPin     =  23;  //Fan control pin
const int rPin          =  25;    //RGB red pin
const int gPin          =  26;    //RGB green pin
const int bPin          =  27;    //RGB blue pin

//Const Definitions
#define ambientTempThreshold 40  //Fan will turn on if temp exceeds 40 deg C

//WiFi Vars
const char* ssid = "";    // ***Remove before pushing***
const char* pwd = "";     //***Remove before pushing***
WiFiClient client;

void setup() {
  Serial.begin(115200);  //Init serial

  ConnectWifi();  //This is a blocking function!!!

  ThingSpeak.begin(client);  //Initialize ThingSpeak

  pinMode(cell1Pin, INPUT);
  pinMode(cell2Pin, INPUT);
  pinMode(cell3Pin, INPUT);
  pinMode(tempSensePin, INPUT);
  pinMode(fanConPin, OUTPUT);
  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
}

void loop() {
  ConnectWifi();  //ConnectWifi called again in case Wifi abruptly disconnects

  //Read cell voltages
  float cell1Voltage = CellVoltageSense(cell1Pin);
  float cell2Voltage = CellVoltageSense(cell2Pin);
  float cell3Voltage = CellVoltageSense(cell3Pin);

  float cell1SOC = calculateSOC(cell1Voltage);
  float cell2SOC = calculateSOC(cell1Voltage);
  float cell3SOC = calculateSOC(cell1Voltage);

  //Set SOH to -1 to indicate that SOH cannot be calculated as battery is not fully charged
  float cell1SOH = -1;
  float cell2SOH = -1;
  float cell3SOH = -1;
  float combinedSOH = -1;
  if (cell1Voltage > 4.1 && cell1Voltage < 4.2) {
    //Calculate SOH only when battery is at full charge
    float cell1SOH = calculateSOH(cell1Voltage);
    float cell2SOH = calculateSOH(cell2Voltage);
    float cell3SOH = calculateSOH(cell3Voltage);
    float combinedSOH = ((cell1SOH+cell2SOH+cell3SOH) / 300) * 100;
  }

  //Read Ambient Temp + Fan Control
  float ambientTemp = TempSense(34);
  if (ambientTemp > ambientTempThreshold) {
    digitalWrite(fanConPin, HIGH);
  }

  //Publish all cell monitoring data to ThingSpeak
  ThingSpeakWrite8Floats(cell1Voltage, cell2Voltage, cell3Voltage, cell1SOC, cell2SOC, cell3SOC, combinedSOH, ambientTemp);

  //Drive RGB LED based on SOH value
  DriveRGB(rPin, gPin, bPin, cell1SOH, cell2SOH, cell3SOH);
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

float CellVoltageSense(int cellVoltageSensePin) {
  //Conversion Vars
  const float conversionFactor = 4.2 / 4096;  //Li ion max cell voltage = 4.2 and ESP32 ADC = 12 bits so 2^12 = 4096
  const float conversionOffset = -0.1;        //ADC conversion offset

  //Conversion
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

bool ThingSpeakWrite8Floats(float data1, float data2, float data3, float data4, float data5, float data6, float data7, float data8) {
  //ThingSpeak Vars
  unsigned long channelId = Channel_ID;     //***Remove before pushing***
  const char* writeAPIKey = "";             //***Remove before pushing***
  int writeStatus = false;

  ThingSpeak.setField(1, data1);
  ThingSpeak.setField(2, data2);
  ThingSpeak.setField(3, data3);
  ThingSpeak.setField(4, data4);
  ThingSpeak.setField(5, data5);
  ThingSpeak.setField(6, data6);
  ThingSpeak.setField(7, data7);
  ThingSpeak.setField(8, data8);
  int writeStatusCode = ThingSpeak.writeFields(channelId, writeAPIKey);

  //ThingSpeak will return HTTP status code 200 if successful
  if (writeStatusCode == 200) {
    Serial.println("ThingSpeak Channel update successful.");
    return true;
  } else {
    Serial.println("Problem updating ThingSpeak channel... HTTP error code " + String(writeStatusCode));
    return false;
  }

  //If the data packets are sent out in under 15s intervals, HTTP status code 401 will be returned
  //Some packets will still be sent but this is not recommended
  //So set min delay to atleast 15s
  delay(20000);
}

float TempSense(int tempSensePin) {
  //Conversion Vars
  const float conversionFactor = 3.3 / 4096;  //Li ion max cell voltage = 4.2 and ESP32 ADC = 12 bits so 2^12 = 4096
  const float conversionOffset = 0;           //ADC conversion offset

  //Conversion
  int tempRaw = analogRead(tempSensePin);
  float temp = (((tempRaw * conversionFactor) + conversionOffset) - 0.5) * 100;

  return temp;
}

float calculateSOC(float voltage) {
  //Define voltage range and corresponding SOC values
  float minVoltage = 2.5;  //Minimum bat voltage
  float maxVoltage = 4.2;  //Maximum bat voltage
  float minSOC = 0.0;      //Minimum SOC when bat is empty
  float maxSOC = 100.0;    //Maximum SOC when bat is on full charged

  //Calculate SOC with linear interpolation formula
  float soc = (((voltage - minVoltage) / (maxVoltage - minVoltage)) * (maxSOC - minSOC)) + minSOC;

  // Ensure SOC stays within valid range
  soc = max(minSOC, min(maxSOC, soc));

  return soc;
}

float calculateSOH(float voltage) {
  // Initial fully charged voltage (for example)
  float initialVoltage = 4.2;

  // Calculate SOH as a percentage of initial voltage
  float soh = (voltage / initialVoltage) * 100.0;

  // Ensure SOH stays within valid range
  soh = max(0.0f, min(100.0f, soh));

  return soh;
}

void DriveRGB(int rPin, int gPin, int bPin, float cell1SOH, float cell2SOH, float cell3SOH) {
  if (cell1SOH == -1  cell2SOH == -1  cell3SOH == -1) {
    digitalWrite(rPin, LOW);
    digitalWrite(gPin, LOW);
    digitalWrite(bPin, LOW);
    return; // Exit function
  }

  // Determine the overall health of the 3 cells based on SoH
  int healthyCells = 0;
  if (cell1SOH > 90) healthyCells++;
  if (cell2SOH > 90) healthyCells++;
  if (cell3SOH > 90) healthyCells++;


  // Drive the RGB LED based on the health of the 3 cells
  if (healthyCells == 3) { // All 3 cells are healthy
    digitalWrite(rPin, LOW);
    digitalWrite(gPin, HIGH); // Green
    digitalWrite(bPin, LOW);
  } else if (healthyCells == 2) { // 2 out of the 3 cells are healthy
    digitalWrite(rPin, HIGH); // Orange
    analogWrite(gPin, 128); 
    digitalWrite(bPin, LOW);
  } else if (healthyCells == 1) { // 1 out of the 3 cells is healthy
    digitalWrite(rPin, HIGH); // Yellow
    digitalWrite(gPin, HIGH); 
    digitalWrite(bPin, LOW);
  } else { // 0 out of 3 cells is healthy
    digitalWrite(rPin, HIGH); // Red
    digitalWrite(gPin, LOW);
    digitalWrite(bPin, LOW);
  }
}
