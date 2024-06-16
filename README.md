# NTU EE3180 Design Innovation Project 3S Battery Management System (BMS) AY2023-24
> [!IMPORTANT]
> This was a group project. The parts of the project I worked on are in this repo.

## About
With the advent of electric vehicles, charging has become an integral part of our daily lives. This has resulted in a sudden increase in the demand for battery chargers, specifically lithium-ion battery chargers. These lithium-ion batteries are very needy. They have to be charged at a specific voltage and current to remain healthy and function reliably. When charging multiple lithium-ion cells in a battery pack, they have to be charged and discharged in a balanced manner. Over-charging or over-discharging a particular cell in a battery pack could lead to dire consequences i.e the battery going BOOM!!! Now we certainly do not want that. This is where our battery management system (BMS) comes in. 

This BMS we created balance charges 3 lithium-ion cells, basically a glorified lithium-ion battery charger. Why glorified? Well, most chargers / passive BMS on the market right just charge batteries. There is no monitoring capability whatsoever. Due to this, we cannot leave our batteries unattended while charging them. What if I want to do a grocery run while I charge my batteries? What if I want to go for an evening run while I charge my batteries? I can't do any of those things. This is where our BMS comes in! Our BMS lets you monitor the your batteries while they are charging from anywhere in the world through a webpage and a mobile app available on both iOS and Android devices. 

## Contents
The parts of the project I worked on are the BMS PCB designed in Altium Designer, ESP32 firmware written in C++ through the Arduino IDE and an enclosure for the PCB designed in Fusion 360 and 3D printed with TPU.

## BMS PCB
