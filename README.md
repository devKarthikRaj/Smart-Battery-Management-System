# NTU EE3180 Design Innovation Project 3S Battery Management System (BMS) AY2023-24
> [!IMPORTANT]
> This was a group project. The parts of the project I worked on are in this repo.

## About
With the advent of electric vehicles, charging has become an integral part of our daily lives. This has resulted in a sudden increase in the demand for battery chargers, specifically lithium-ion battery chargers. These lithium-ion batteries are very needy. They have to be charged at a specific voltage and current to remain healthy and function reliably. When charging multiple lithium-ion cells in a battery pack, they have to be charged and discharged in a balanced manner. Over-charging or over-discharging a particular cell in a battery pack could lead to dire consequences i.e the battery going BOOM!!! Now we certainly do not want that. This is where our battery management system (BMS) comes in. 

This BMS we created balance charges 3 lithium-ion cells, basically a glorified lithium-ion battery charger. Why glorified? Well, most chargers / passive BMS on the market right just charge batteries. There is no monitoring capability whatsoever. Due to this, we cannot leave our batteries unattended while charging them. What if I want to do a grocery run while I charge my batteries? What if I want to go for an evening run while I charge my batteries? I can't do any of those things. This is where our BMS comes in! Our BMS lets you monitor the your batteries while they are charging from anywhere in the world through a webpage and a mobile app available on both iOS and Android devices. 

## Contents
The parts of the project I worked on are the BMS PCB designed in Altium Designer, ESP32 firmware written in C++ through the Arduino IDE and an enclosure for the PCB designed in Fusion 360 and 3D printed with TPU.

## BMS PCB
<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/3S%20BMS%20Schematic%20Main.png" alt="Rover v1 final take 2"></div><br>
This is the main schematic. The top half of the schematic is the constant voltage constant current CV/CC circuit implemented using an adjustable LM317 voltage regulator. The bottom half of the schematic is the ESP32 microcontroller board to sensor/actuator interface circuit - This circuit connects the onboard temperature sensor, RGB LED and mini fan to the the microcontroller. This circuit also connects various parts of the charging circuit to the ESP32 to allow the microcontroller to non-invasively monitor the charging process.

<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/3S%20BMS%20Schematic%20Balance%20Indicator.png" alt="Rover v1 final take 2"></div><br>
This circuit is contained inside the green circuit blocks in the main schematic. This particular circuit is in charge of cutting off the charging process when the max desired cell voltage has been reached. This max desired cell voltage can be set by adjusting the potentiometer. Once the max desired cell voltage is reached, the indicator LED will light up giving a visual indicator to the user that the charging process is complete.

### BMS PCB Features
●	Up to 3 lithium-ion batteries can be charged simultaneously with this PCB.
●	DC Jack for input voltage from a wall plug.
●	ATOF series blade fuse for overcurrent protection is easily replaceable by the user.
●	3 Potentiometers enable the user to adjust the maximum charge voltage of each battery separately thereby allowing the user to charge various types of lithium-ion cells to different voltages as and when required.
●	3 Single colour indicator LEDs to alert the user when each battery has been fully charged.
●	RGB LED to indicate SoH and charging status.
●	Temperature sensor to sense ambient temperature.
●	Interface for a small ventilator fan to keep the PCB at desirable temperatures. (~25°C-30°C)
●	Onboard microcontroller with WiFi capabilities to monitor all parameters of the BMS.
●	A detailed breakdown of all parameters (3 cell voltages, SoC, SoH and ambient temperature) and trend charts available through a web interface. (ThinkSpeak)
●	3 Cell voltages and ambient temperature also available through a mobile application on both iOS and Android devices

