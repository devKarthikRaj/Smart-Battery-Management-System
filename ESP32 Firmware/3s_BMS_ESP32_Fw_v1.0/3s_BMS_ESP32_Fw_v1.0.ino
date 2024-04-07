/****************************************************************** 
  Project: DIP 3s BMS ESP32 Firmware V1
  Requirements:
    1. Read battery voltage                                      (Done)
    2. Convert battery voltage ADC value to actual voltage value (Done)
    3. Read temp sensor                                          (Done)
    4. Activate the fan if temp exceeds a preset threshold       (Done)
    5. Calculate SoC                                             (Done)
    6. Calculate SoH                                             (Done)
    7. Drive the RGB LED based on combined SoH for all 3 cells   (TBC...)
    8. Post data to ThingSpeak                                   (Done)
    9. Post data to Mobile app                                   (Done)

    Notes:
    time calculation using millis = (current time - initial time)

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
const int rPin          =  25;  //RGB red pin
const int gPin          =  26;  //RGB green pin
const int bPin          =  27;  //RGB blue pin

//Const Definitions
#define ambientTempThreshold 30  //Fan will turn on if temp exceeds 30 deg C

//WiFi Vars
const char* ssid = "";    // ***Remove before pushing***
const char* pwd = "";     //***Remove before pushing***
WiFiClient client;

//Charge Stats Vars
unsigned long cell1EndChargeTime;
unsigned long cell2EndChargeTime;
unsigned long cell3EndChargeTime;
bool cell1EndChargeTimerFlag = false;
bool cell2EndChargeTimerFlag = false;
bool cell3EndChargeTimerFlag = false;
float lastUpdatedVc1;
float lastUpdatedVc2;
float lastUpdatedVc3;

void setup() {
  //Serial.begin(115200); //Init serial

  ConnectWifi(); //This is a blocking function!!!

  ThingSpeak.begin(client); //Initialize ThingSpeak

  //IO Definitions
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

  //Read stepped down added cell voltages and calculate the actual added cell voltages
  float Vc1 = CellVoltageSense(cell1Pin);
  float Vc2 = CellVoltageSense(cell2Pin);
  float Vc3 = CellVoltageSense(cell3Pin);

  //Calculate individual cell voltages
  Vc1 = Vc1 - Vc2;
  Vc2 = Vc2 - Vc3;
  //Vc3 is the actual voltage of the 3rd cell

  //Calculate SOC
  float cell1SOC = calculateSOC(Vc1);
  float cell2SOC = calculateSOC(Vc2);
  float cell3SOC = calculateSOC(Vc3);

  
  //Set SOH to -1 to indicate that SOH cannot be calculated as battery is not fully charged
  float cell1SOH = -1;
  float cell2SOH = -1;
  float cell3SOH = -1;
  float combinedSOH = -1;
  
  if (Vc1 > 4.1) { //Good battery
    if(cell1EndChargeTimerFlag) {
      cell1EndChargeTime = millis();
      //long blink color 1 time
    }
    //Calculate SOH only when battery is at full charge
    float cell1SOH = calculateSOH(Vc1);
  } else { //Bad battery
    if(lastUpdatedVc1 == Vc1 && /*logic for millis*/) {
      //rapid blink cell 1 colour 3 times
    }
  }

  if (Vc2 > 4.1) { //Good battery
    if(cell2EndChargeTimerFlag) {
      cell2EndChargeTime = millis();
    }
    //Calculate SOH only when battery is at full charge
    float cell2SOH = calculateSOH(Vc2);
  } else { //Bad battery

  }

  if (Vc3 > 4.1) { //Good battery
    if(cell3EndChargeTimerFlag) {
      cell3EndChargeTime = millis();
    }
    //Calculate SOH only when battery is at full charge
    float cell3SOH = calculateSOH(Vc3);
  } else { //Bad battery

  }
  float combinedSOH = ((cell1SOH+cell2SOH+cell3SOH) / 300) * 100;
  //Drive RGB LED based on SOH value
  DriveRgbSoh(rPin, gPin, bPin, cell1SOH, cell2SOH, cell3SOH);
  
  //Read Ambient Temp + Fan Control
  float ambientTemp = TempSense(34);

  
  if (ambientTemp > ambientTempThreshold) {
    digitalWrite(fanConPin, HIGH);
  }
  
  //Publish all cell monitoring data to ThingSpeak
  ThingSpeakWrite8Floats(Vc1, Vc2, Vc3, cell1SOC, cell2SOC, cell3SOC, combinedSOH, ambientTemp);
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

//Reads the raw stepped down added cell voltage and converts this raw reading back to the actual added cell voltage
float CellVoltageSense(int cellVoltageSensePin) {
  //Conversion Vars
  const float conversionFactor = 4.2 / 4096;  //Li ion max cell voltage = 4.2 and ESP32 ADC = 12 bits so 2^12 = 4096
  const float conversionOffset = -0.1;        //ADC conversion offset
  const int Rb = 10;
  const int Rs = 1;

  int cellVoltageRaw = analogRead(cellVoltageSensePin);
  
  //Conversion
  float Vs = (cellVoltageRaw * conversionFactor) + conversionOffset; //Calculate raw voltage from ADC value
  float Vb = ((Rs+Rb)/Rs) * Vs; //Calculate actual added cell voltage (Vb = Vbig)

  //Debugging
  //Serial.println();
  //Serial.print("Cell Voltage Raw: ");
  //Serial.print(cellVoltageRaw);
  //Serial.print(" Cell Voltage Actual: ");
  ///Serial.print(Vb);
  //Serial.println();

  return cellVoltageRaw;
}

bool ThingSpeakWrite8Floats(float data1, float data2, float data3, float data4, float data5, float data6, float data7, float data8) {
  //ThingSpeak Vars
  unsigned long channelId = 2454172;            //***Remove before pushing***
  const char* writeAPIKey = "4POHYZ95Y3YP52OG"; //***Remove before pushing***
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

  
  /*//ThingSpeak will return HTTP status code 200 if successful
  if (writeStatusCode == 200) {
    Serial.println("ThingSpeak Channel update successful.");
    return true;
  } else {
    Serial.println("Problem updating ThingSpeak channel... HTTP error code " + String(writeStatusCode));
    return false;
  }*/

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

void DriveRgb(int rPin, int gPin, int bPin, int color) {
}