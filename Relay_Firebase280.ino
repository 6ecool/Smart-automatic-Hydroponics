#include <Wire.h>
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <TimeLib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define WIFI_SSID "TechnoPark"
#define WIFI_PASSWORD "techno2020"
#define API_KEY "AIzaSyCv6az41mVzr3pKbzz-ETw_b2XnWS_vEVk"
#define USER_EMAIL "iot.technopark@gmail.com"
#define USER_PASSWORD "iot.technopark2023"
#define DATABASE_URL "https://hydroponics-48941-default-rtdb.europe-west1.firebasedatabase.app/"

FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

String settingsPath = "/BME-280/" + String(WiFi.macAddress()) + "/Settings/" + "/";
String dataPath = "/BME-280/" + String(WiFi.macAddress()) + "/Data/" + "/";

Adafruit_BME280 bme; // I2C
float temperature;
float humidity;
float pressure;

const int output1 = 12;
const int output2 = 13;
const int output3 = 14;
int packNum = 0;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 6*3600;
const int   daylightOffset_sec = 3600;

void initBME(){
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void sendINT(String path, int value) {
  if (Firebase.RTDB.setInt(&stream, path.c_str(), value)) {
    Serial.print("Writing value: ");
    Serial.print(value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + stream.dataPath());
    Serial.println("TYPE: " + stream.dataType());
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + stream.errorReason());
  }
}

void sendFLOAT(String path, float value) {
  if (Firebase.RTDB.setInt(&stream, path.c_str(), value)) {
    Serial.print("Writing value: ");
    Serial.print(value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + stream.dataPath());
    Serial.println("TYPE: " + stream.dataType());
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + stream.errorReason());
  }
}
void PrintEveryThing(){
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure()/100.0F;
  int delay = 10;
  sendFLOAT(dataPath + "Temperature", temperature);
  sendFLOAT(dataPath + "Humidity", humidity);
  sendFLOAT(dataPath + "Pressure", pressure);
  sendINT(settingsPath + "Packnum", packNum);
  sendINT(settingsPath + "Delay", delay);
  packNum ++;
  printLocalTimeAndSendToFirebase(settingsPath + "Time");
}
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

void printLocalTimeAndSendToFirebase(String firebasePath) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char formattedTime[20];
  snprintf(formattedTime, sizeof(formattedTime), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  Serial.println(formattedTime);  // Print the formatted time

  // Send the formatted time to Firebase
  sendTime(firebasePath, formattedTime);
}

void sendTime(String path, const char* value) {
  if (Firebase.RTDB.setString(&stream, path.c_str(), value)) {
    Serial.print("Writing value: ");
    Serial.print(value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + stream.dataPath());
    Serial.println("TYPE: " + stream.dataType());
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + stream.errorReason());
  }
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  initBME();

  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);

  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);

  config.max_token_generation_retry = 5;

  Firebase.begin(&config, &auth);

  delay(1000);

  time_t now;
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (true) {
    time(&now);
    if (now != 0) {
      break;
    }
    delay(1000);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  if (!Firebase.RTDB.beginStream(&stream, settingsPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());
  if (!Firebase.RTDB.beginStream(&stream, dataPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());
}

void loop() {
  if (Firebase.isTokenExpired()) {
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
  PrintEveryThing();
  delay(9000);
}
