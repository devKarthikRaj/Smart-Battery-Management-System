# NTU EE3180 Design Innovation Project 3S Battery Management System (BMS) AY2023-24
> [!IMPORTANT]
> This was a group project. The parts of the project I worked on are in this repo.

<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/Completed%20BMS%20System.png"></div><br>

## About
With the advent of electric vehicles, charging has become an integral part of our daily lives. This has resulted in a sudden increase in the demand for battery chargers, specifically lithium-ion battery chargers. These lithium-ion batteries are very needy. They have to be charged at a specific voltage and current to remain healthy and function reliably. When charging multiple lithium-ion cells in a battery pack, they have to be charged and discharged in a balanced manner. Over-charging or over-discharging a particular cell in a battery pack could lead to dire consequences i.e the battery going BOOM!!! Now we certainly do not want that. This is where our battery management system (BMS) comes in. 

This BMS we created balance charges 3 lithium-ion cells, basically a glorified lithium-ion battery charger. Why glorified? Well, most chargers / passive BMS on the market right just charge batteries. There is no monitoring capability whatsoever. Due to this, we cannot leave our batteries unattended while charging them. What if I want to do a grocery run while I charge my batteries? What if I want to go for an evening run while I charge my batteries? I can't do any of those things. This is where our BMS comes in! Our BMS lets you monitor the your batteries while they are charging from anywhere in the world through a webpage and a mobile app available on both iOS and Android devices. 

## Contents
The parts of the project I worked on are the BMS PCB designed in Altium Designer, ESP32 firmware written in C++ through the Arduino IDE and an enclosure for the PCB designed in Fusion 360 and 3D printed with TPU.

## BMS PCB
<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/3S%20BMS%20Schematic%20Main.png" alt="Rover v1 final take 2"></div><br><br>
This is the main schematic. The top half of the schematic is the constant voltage constant current CV/CC circuit implemented using an adjustable LM317 voltage regulator. The bottom half of the schematic is the ESP32 microcontroller board to sensor/actuator interface circuit - This circuit connects the onboard temperature sensor, RGB LED and mini fan to the the microcontroller. This circuit also connects various parts of the charging circuit to the ESP32 to allow the microcontroller to non-invasively monitor the charging process.

&nbsp;

<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/3S%20BMS%20Schematic%20Balance%20Indicator.png" alt="Rover v1 final take 2"></div><br><br>
This circuit is contained inside the green circuit blocks in the main schematic. This particular circuit is in charge of cutting off the charging process when the max desired cell voltage has been reached. This max desired cell voltage can be set by adjusting the potentiometer. Once the max desired cell voltage is reached, the indicator LED will light up giving a visual indicator to the user that the charging process is complete.

&nbsp;

<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/3S%20BMS%20PCB%20v2%20Layout.png"></div><br><br>
This is the layout of the BMS PCB. Copper pours (polygons) have been used for power nets to keep this PCB as cool as a cucumber! <br>

&nbsp;

<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/3S%20BMS%20PCB%20v2%203d%20View.png"></div><br><br>
Here's a 3D render of the PCB. Note that the ESP32 microcontroller board is not shown in the render.

### BMS PCB Features
●	Up to 3 lithium-ion batteries can be charged simultaneously with this PCB. <br>
●	DC Jack for input voltage from a wall plug. <br>
●	ATOF series blade fuse for overcurrent protection is easily replaceable by the user. <br>
●	3 Potentiometers enable the user to adjust the maximum charge voltage of each battery separately thereby allowing the user to charge various types of lithium-ion cells to different voltages as and when required. <br>
●	3 Single colour indicator LEDs to alert the user when each battery has been fully charged. <br>
●	RGB LED to indicate SoH and charging status. <br>
●	Temperature sensor to sense ambient temperature. <br>
●	Interface for a small ventilator fan to keep the PCB at desirable temperatures (~25°C-30°C). <br>
●	Onboard microcontroller with WiFi capabilities to monitor all parameters of the BMS. <br>
●	A detailed breakdown of all parameters (3 cell voltages, SoC, SoH and ambient temperature) and trend charts available through a web interface (ThingSpeak). <br>

<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/ThingSpeak%20Dashboard.png"></div><br><br>

&nbsp;

●	3 Cell voltages and ambient temperature also available through a mobile application on both iOS and Android devices. <br>

## BMS Enclosure CAD Model
<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/3S%20BMS%20PCB%20Enclosure%20CAD%20Model%20(1).png"></div>
<div align="center"><img src="https://github.com/devKarthikRaj/Smart-Battery-Management-System/blob/master/Media/3S%20BMS%20PCB%20Enclosure%20CAD%20Model%20(2).png"></div><br><br>
Here is a 3D render of the BMS Enclosure. This model was 3D printed in TPU. The mini fan that was supposed to keep things cool is not shown in this render.

# Conclusion
Many aspects of this project could be improved. A more modern PMIC could have been used as compared to the LM317 - a veteran, tried and tested PMIC in my opinion. An ESP32 micrcontroller circuit could have directly been implemented on the BMS PCB itself instead of using a pluggable module (development board). SoC could have been implemented using columb counting using a current sensor like the INA219. The cut off voltage could have been dynamically adjusted by the ESP32 based on the SoH of the battery. However this was just a prototype. This project was designed and realized in the span of 13 weeks.   