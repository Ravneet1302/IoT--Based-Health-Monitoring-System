1.IoT-Based Health Monitoring System

This project is an IoT-based Health Monitoring System developed using ESP8266 (NodeMCU) and Blynk. It monitors vital health parameters such as body temperature, heart rate, and blood oxygen level (SpO₂) 
in real time and displays them on a mobile dashboard.
Aim is To design and implement an IoT-based Health Monitoring System for real-time monitoring of body temperature, heart rate, and SpO₂ with alert generation and remote access.

2. Features
- Real-time monitoring of health parameters  
- Wireless data transmission using Wi-Fi  
- Mobile dashboard using Blynk  
- Alert notifications for abnormal values  
- Easy-to-use and low-cost system  

3. Components Used
- ESP8266 NodeMCU  
- MAX30100 / MAX30102 Sensor  
- DS18B20 Temperature Sensor  
- 4.7kΩ Resistor  
- Breadboard & Jumper Wires  

 4.Circuit Connections

4.1. MAX30100 / MAX30102
- VIN → 3.3V  
- GND → GND  
- SDA → D2  
- SCL → D1  

4.2. DS18B20
- Red → 3.3V  
- Black → GND  
- Yellow → D4  
- 4.7kΩ resistor between D4 and 3.3V  

5. Working Principle
The system uses sensors to measure health parameters. The ESP8266 processes the data and sends it via Wi-Fi to the Blynk cloud. The data is displayed on a mobile dashboard, and alerts are generated when values cross predefined limits.

6.Output
The output is displayed on the Blynk mobile application in the form of gauges showing:
- Temperature  
- Heart Rate  
- SpO₂  

How to Run the Project

1.Install Arduino IDE  
2.Install required libraries:
   - Blynk  
   - MAX3010x  
   - DallasTemperature  
   - OneWire
      
3.Upload the code to NodeMCU

4.Connect hardware as per circuit diagram  

5.Open Blynk app and monitor data

<img width="996" height="463" alt="Screenshot 2026-04-20 135637" src="https://github.com/user-attachments/assets/698e1c90-d770-4b20-94ac-f01dd5146bba" />
<br>

<img width="1178" height="565" alt="Screenshot 2026-04-20 141230" src="https://github.com/user-attachments/assets/1dbc16c5-1ab7-4d57-8196-400b15167daa" />
<br>
<img width="1134" height="317" alt="Screenshot 2026-04-20 141239" src="https://github.com/user-attachments/assets/a2ba2b05-4b8b-4bc4-8ab4-28b538d05a6b" />





 
