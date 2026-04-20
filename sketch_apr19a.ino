#define BLYNK_TEMPLATE_ID "TMPL6yKLQ4Xmj"
#define BLYNK_TEMPLATE_NAME "Health Monitoring System"
#define BLYNK_AUTH_TOKEN "Y3rIwf0zyyJ9ZaXNzzoPFSVAqJPsUN4v"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
#include <OneWire.h>
#include <DallasTemperature.h>

char ssid[] = "Rav";
char pass[] = "12345678";

// -------------------- Pin setup --------------------
#define DS18B20_PIN D4   // DS18B20 DATA pin

// -------------------- DS18B20 ----------------------
OneWire oneWire(DS18B20_PIN);
DallasTemperature bodyTempSensor(&oneWire);

// -------------------- MAX30102 ---------------------
MAX30105 particleSensor;
bool max30102Found = false;

// renamed from BUFFER_SIZE to avoid warning/conflict
#define SPO2_SAMPLES 100

uint32_t irBuffer[SPO2_SAMPLES];
uint32_t redBuffer[SPO2_SAMPLES];

int32_t spo2 = 0;
int8_t validSPO2 = 0;
int32_t heartRateCalc = 0;
int8_t validHeartRate = 0;

// Beat average
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute = 0;
int beatAvg = 0;

// -------------------- Blynk ------------------------
BlynkTimer timer;

// -------------------- Functions --------------------
float getBodyTemperature()
{
  bodyTempSensor.requestTemperatures();
  float t = bodyTempSensor.getTempCByIndex(0);

  if (t == DEVICE_DISCONNECTED_C) {
    return -127.0;
  }
  return t;
}

void updateRealtimeHeartRate()
{
  if (!max30102Found) return;

  long irValue = particleSensor.getIR();

  // Stronger finger detection
  if (irValue < 70000) {
    beatAvg = 0;
    lastBeat = 0;
    return;
  }

  if (checkForBeat(irValue))
  {
    long currentBeat = millis();

    // Ignore first beat
    if (lastBeat == 0) {
      lastBeat = currentBeat;
      return;
    }

    long delta = currentBeat - lastBeat;
    lastBeat = currentBeat;

    float bpm = 60.0 / (delta / 1000.0);

    // Accept only realistic BPM
    if (bpm >= 45 && bpm <= 150)
    {
      rates[rateSpot++] = (byte)bpm;
      rateSpot %= RATE_SIZE;

      int sum = 0;
      int count = 0;

      for (byte i = 0; i < RATE_SIZE; i++) {
        if (rates[i] >= 45 && rates[i] <= 150) {
          sum += rates[i];
          count++;
        }
      }

      if (count > 0) {
        beatAvg = sum / count;
      }
    }
  }
}
bool fingerPresent()
{
  if (!max30102Found) return false;

  long irValue = particleSensor.getIR();
  return (irValue > 70000);
}

void updateSPO2andHR()
{
  if (!max30102Found) {
    validSPO2 = 0;
    validHeartRate = 0;
    spo2 = 0;
    heartRateCalc = 0;
    return;
  }

  if (!fingerPresent()) {
    validSPO2 = 0;
    validHeartRate = 0;
    spo2 = 0;
    heartRateCalc = 0;
    return;
  }

  for (int i = 0; i < SPO2_SAMPLES; i++)
  {
    while (particleSensor.available() == false) {
      particleSensor.check();
      yield();   // prevents ESP8266 watchdog reset
    }

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(
    irBuffer,
    SPO2_SAMPLES,
    redBuffer,
    &spo2,
    &validSPO2,
    &heartRateCalc,
    &validHeartRate
  );
}








void sendDataToBlynk()
{
  float bodyTemp = getBodyTemperature();
  updateSPO2andHR();

  int finalHR = 0;
  int finalSPO2 = 0;

  if (beatAvg >= 45 && beatAvg <= 150) {
    finalHR = beatAvg;
  } else if (validHeartRate && heartRateCalc >= 45 && heartRateCalc <= 150) {
    finalHR = heartRateCalc;
  }

  if (validSPO2 && spo2 >= 70 && spo2 <= 100) {
    finalSPO2 = spo2;
  }

  // 🚨 ALERT SECTION START

  // High Temperature
  if (bodyTemp > 38.0) {
    Blynk.logEvent("high_temperature", "High body temperature detected!");
  }

  // Low Temperature
  if (bodyTemp != -127.0 && bodyTemp < 35.0) {
    Blynk.logEvent("low_temperature", "Low body temperature detected!");
  }

  // High Heart Rate
  if (finalHR > 120) {
    Blynk.logEvent("high_heart_rate", "High heart rate detected!");
  }

  // Low Heart Rate
  if (finalHR > 0 && finalHR < 50) {
    Blynk.logEvent("low_heart_rate", "Low heart rate detected!");
  }

  // Low SpO2
  if (finalSPO2 > 0 && finalSPO2 < 90) {
    Blynk.logEvent("low_spo2_level", "Low SpO2 detected!");
  }

  // 🚨 ALERT SECTION END

  Serial.println("----------- HEALTH DATA -----------");

  if (bodyTemp == -127.0) {
    Serial.println("DS18B20 not detected. Check wiring/resistor.");
  } else {
    Serial.print("Body Temperature (C): ");
    Serial.println(bodyTemp);
    Blynk.virtualWrite(V0, bodyTemp);
  }

  if (!max30102Found) {
    Serial.println("MAX30102 not detected. Check wiring.");
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 0);
  } else if (!fingerPresent()) {
    Serial.println("Place finger on MAX30102");
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 0);
  } else {
    Serial.print("Heart Rate (BPM): ");
    Serial.println(finalHR);

    Serial.print("SpO2 (%): ");
    Serial.println(finalSPO2);

    if (finalHR > 0) {
      Blynk.virtualWrite(V1, finalHR);
    }
    if (finalSPO2 > 0) {
      Blynk.virtualWrite(V2, finalSPO2);
    }
  }

  Serial.println("-----------------------------------");
}






void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Starting Health Monitoring System...");

  // ESP8266 I2C pins
  Wire.begin(D2, D1); // SDA, SCL

  bodyTempSensor.begin();
  Serial.println("DS18B20 initialized.");

  max30102Found = particleSensor.begin(Wire, I2C_SPEED_STANDARD);

  if (!max30102Found)
  {
    Serial.println("MAX30102 not found. Check wiring.");
    Serial.println("System will continue for DS18B20 + Blynk debugging.");
  }
  else
  {
    Serial.println("MAX30102 detected successfully.");

    byte ledBrightness = 60;   // 0-255
    byte sampleAverage = 4;
    byte ledMode = 2;          // Red + IR
    int sampleRate = 100;
    int pulseWidth = 411;
    int adcRange = 4096;

    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
    particleSensor.setPulseAmplitudeRed(0x24);
    particleSensor.setPulseAmplitudeIR(0x24);
    particleSensor.setPulseAmplitudeGreen(0);
  }

  Serial.println("Connecting to WiFi/Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(40L, updateRealtimeHeartRate);
  timer.setInterval(3000L, sendDataToBlynk);

  Serial.println("System started...");
}

void loop()
{
  Blynk.run();
  timer.run();
}