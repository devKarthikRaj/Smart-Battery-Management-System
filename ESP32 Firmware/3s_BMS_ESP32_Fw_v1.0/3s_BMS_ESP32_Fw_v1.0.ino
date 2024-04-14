//Constant Definitions
#define ambientTempThreshold 15  //Fan will turn on if temp exceeds 30 deg C
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6m-UOY92p" // Access code for Blynk
#define BLYNK_TEMPLATE_NAME "BMS" // Access code for Blynk
#define BLYNK_AUTH_TOKEN "M9BpsXbl3JkOrzozAKsXzokN0JTHX8Of"// Access code for Blynk
#define V1 1
#define V2 2
#define V3 3
#define V4 4

// Blynk Auth Token
char auth[] = BLYNK_AUTH_TOKEN;

//Pin Definitions
const int cell1Pin      =  33;  //Cell 1 voltage pin
const int cell2Pin      =  32;  //Cell 2 voltage pin
const int cell3Pin      =  35;  //Cell 3 voltage pin
const int tempSensePin  =  34;  //Temperature sensor pin
const int fanConPin     =  23;  //Fan control pin
const int rPin          =  25;  //RGB red pin
const int gPin          =  26;  //RGB green pin
const int bPin          =  27;  //RGB blue pin

//Add Libraries
#include <WiFi.h>
#include <ThingSpeak.h>
#include <BlynkSimpleEsp32.h>

//WiFi Vars
const char* ssid = "dlink-193C";    // ***Remove before pushing***
const char* pwd = "mczep88481";     //***Remove before pushing***
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
  Serial.begin(115200); //Init serial

  ConnectWifi(); //This is a blocking function!!!

  ThingSpeak.begin(client); //Initialize ThingSpeak
  Blynk.begin(auth, ssid, pwd); // Initialize Blynk

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

  Blynk.run(); // Run Blynk loop

  //Read stepped down added cell voltages and calculate the actual added cell voltages
  float Vc1 = CellVoltageSense(cell1Pin) - 0.67343; //Offset for res tolerance
  float Vc2 = CellVoltageSense(cell2Pin) + 0.45296; //Offset for res tolerance
  float Vc3 = CellVoltageSense(cell3Pin) + 1.67958; //Offset for res tolerance

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
  
  if (Vc1 > 4.1) { // Good battery
    if (cell1EndChargeTimerFlag) {
      cell1EndChargeTime = millis();
      DriveRgb(0, 255, 0);  // Blink green 1 time
      cell1EndChargeTimerFlag = false;
    }
    // Calculate SOH only when battery is at full charge
    float cell1SOH = calculateSOH(Vc1);
  } else { // Bad battery
    if (lastUpdatedVc1 == Vc1 && !timerStarted) {
      unsigned long currentTime = millis();
      if (currentTime - cell1EndChargeTime >= 180000) {  // Check if 3 minutes has passed
        //timerStarted = true;  // Start the timer
        DriveRgb(0, 255, 0);  // Flash green
        delay(50);  // Adjust delay time as needed
        DriveRgb(0, 0, 0);  // Turn off LED
        delay(50);  // Adjust delay time as needed
      }
    }
  }
  lastUpdatedVc1 = Vc1;  // Update last updated Vc1 value
  
  if (Vc2 > 4.1) { // Good battery
    if (cell2EndChargeTimerFlag) {
      cell2EndChargeTime = millis();
      DriveRgb(255, 255, 0);  // Blink yellow 1 time
      cell2EndChargeTimerFlag = false;
    }
    // Calculate SOH only when battery is at full charge
    float cell2SOH = calculateSOH(Vc2);
  } else { // Bad battery
    if (lastUpdatedVc2 == Vc2 && !timerStarted) {
      unsigned long currentTime = millis();
      if (currentTime - cell2EndChargeTime >= 180000) {  // Check if 3 minutes has passed
        //timerStarted = true;  // Start the timer
        DriveRgb(255, 255, 0);  // Flash yellow
        delay(50);  // Adjust delay time as needed
        DriveRgb(0, 0, 0);  // Turn off LED
        delay(50);  // Adjust delay time as needed
      }
    }
  }
  lastUpdatedVc2 = Vc2;  // Update last updated Vc2 value


  if (Vc3 > 4.1) { // Good battery
    if (cell3EndChargeTimerFlag) {
      cell3EndChargeTime = millis();
      DriveRgb(255, 0, 0);  // Blink red 1 time
      cell3EndChargeTimerFlag = false;
    }
    // Calculate SOH only when battery is at full charge
    float cell3SOH = calculateSOH(Vc3);
  } else { // Bad battery
    if (lastUpdatedVc3 == Vc3 && !timerStarted) {
      unsigned long currentTime = millis();
      if (currentTime - cell3EndChargeTime >= 180000) {  // Check if 3 minutes has passed
        //timerStarted = true;  // Start the timer
        DriveRgb(255, 0, 0);  // Flash red
        delay(100);  // Adjust delay time as needed
        DriveRgb(0, 0, 0);  // Turn off LED
        delay(100);  // Adjust delay time as needed
      }
    }
  }
  lastUpdatedVc3 = Vc3;  // Update last updated Vc2 value

  combinedSOH = ((cell1SOH+cell2SOH+cell3SOH) / 300) * 100;
  
  //Read Ambient Temp + Fan Control
  float ambientTemp = TempSense(34);
  
  if (ambientTemp > ambientTempThreshold) {
    digitalWrite(fanConPin, HIGH);
  }

  //Publish all cell monitoring data to Blynk
  PublishBlynkData(Vc1,Vc2,Vc3,ambientTemp);
  
  //Publish all cell monitoring data to ThingSpeak
  ThingSpeakWrite8Floats(Vc1, Vc2, Vc3, cell1SOC, cell2SOC, cell3SOC, combinedSOH, ambientTemp);

  //Fast blink RGB orange to indicate looping
  DriveRgb(247, 152, 29);
  delay(100);
  DriveRgb(247, 152, 29);
  delay(100);
  DriveRgb(0, 0, 0);
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
  const float conversionFactor = 3.3 / 4096;  //Li ion max cell voltage = 4.2 and ESP32 ADC = 12 bits so 2^12 = 4096
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

  return Vb;
}

void PublishBlynkData(float data1, float data2, float data3, float data4) {
  // Send data to Blynk
  Blynk.virtualWrite(V1, data1);
  Blynk.virtualWrite(V2, data2);
  Blynk.virtualWrite(V3, data3);
  Blynk.virtualWrite(V4, data4);
  delay (1000);
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

  //Debugging
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
  const float conversionFactor = 3.3 / 4096;  //System max voltage = 3.3V as ESP ADC runs on 3.3V and ESP32 ADC = 12 bits so 2^12 = 4096
  const float conversionOffset = 0.1;           //ADC conversion offset

  //Conversion
  int tempRaw = analogRead(tempSensePin);
  float temp = (((tempRaw * conversionFactor) + conversionOffset) - 0.5) * 100;

  return temp;
}

float calculateSOC(float voltage) {
  //Define voltage range and corresponding SOC values
  float minVoltage = 3.2;  //Minimum bat voltage
  float maxVoltage = 4.2;  //Maximum bat voltage
  float minSOC = 0.0;      //Minimum SOC when bat is empty
  float maxSOC = 100.0;    //Maximum SOC when bat is on full charged

  //Calculate SOC with linear interpolation formula
  float soc = (((voltage - minVoltage) / (maxVoltage - minVoltage)) * 100

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
  soh = max(0.0f, min(100.0f, soh)); // It returns the larger of the 2 values, this is used to keep it within the range

  return soh;
}

void DriveRgb(int redvalue, int greenvalue, int bluevalue) {
  analogWrite(rPin, redvalue);
  analogWrite(gPin, greenvalue);
  analogWrite(bPin, bluevalue);
}