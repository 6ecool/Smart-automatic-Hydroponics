#include "PCF8574.h"
#include <Wire.h>
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
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

String RelayPath = "/Relay/" + String(WiFi.macAddress()) + "/";
//String RelayPath = "Relay";

const int output1 = 12;
const int output2 = 13;
const int output3 = 14;
int relay_default_position = 1;

PCF8574 pcf8574(0x20);

void startpcf8574() {
  Serial.print("Init pcf8574...");
  if (pcf8574.begin()){
    Serial.println("OK");}
  else{
    Serial.println("ERROR");}
  delay(50);     
}

void sendINT(String path, int value){
  if (Firebase.RTDB.setInt(&stream, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + stream.dataPath());
    Serial.println("TYPE: " + stream.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + stream.errorReason());
  }
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

void streamCallback(FirebaseStream data){
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data);
  Serial.println();

  String streamPath = String(data.dataPath());

  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer){
    String gpio = streamPath.substring(1);
    int state = data.intData();
    Serial.print("GPIO: ");
    Serial.println(gpio);
    Serial.print("STATE: ");
    Serial.println(state);
    pcf8574.digitalWrite(gpio.toInt(), state);
  }

  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json){
    FirebaseJson json = data.to<FirebaseJson>();
    size_t count = json.iteratorBegin();
    Serial.println("\n---------");
    for (size_t i = 0; i < count; i++){
        FirebaseJson::IteratorValue value = json.valueAt(i);
        int gpio = value.key.toInt();
        int state = value.value.toInt();
        Serial.print("STATE: ");
        Serial.println(state);
        Serial.print("GPIO:");
        Serial.println(gpio);
        pcf8574.digitalWrite(gpio, state);
        Serial.printf("Name: %s, Value: %s, Type: %s\n", value.key.c_str(), value.value.c_str(), value.type == FirebaseJson::JSON_OBJECT ? "object" : "array");
    }
    Serial.println();
    json.iteratorEnd();
  }
  
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}

void streamTimeoutCallback(bool timeout){
  if (timeout)
    Serial.println("stream timeout, resuming...\n");
  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup(){
  Serial.begin(115200);
  initWiFi();
  
  for (int i = 0; i < 8; i++) {
    pcf8574.pinMode(i, OUTPUT);
  }

  startpcf8574();

  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  
  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);

  config.token_status_callback = tokenStatusCallback;

  config.max_token_generation_retry = 5;

  Firebase.begin(&config, &auth);

  delay(2000); 
  
  for (int i = 0; i < 8; i++) {
    String topic = RelayPath + String(i);
    sendINT(topic, relay_default_position);
  }

  if (!Firebase.RTDB.beginStream(&stream, RelayPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
  
  
  delay(2000);
}

void loop(){
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
}
