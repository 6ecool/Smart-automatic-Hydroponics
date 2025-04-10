
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 868E6

#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "TechnoPark"
#define WIFI_PASSWORD "techno2020"
//#define WIFI_SSID "HONOR"
//#define WIFI_PASSWORD "HONOR2023"

#define API_KEY "AIzaSyA3txSt2Tm0q4XByth4ligkMqmsaWF7xuc"
#define USER_EMAIL "samayryn.smart.office@gmail.com"
#define USER_PASSWORD "$m@rt.office.301123"
#define DATABASE_URL "https://smart-office-e6873-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

String RelayPath = "/Master-1/Relay/";
String WiFi_Config = "/WiFi Config";

String uid;
String R1;


int rssi;
int counter = 0;
int relay_default_position = 1;

unsigned long previousMillis = 0;
unsigned long interval = 5000;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void startOLED(){
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.display();
  delay(2000);
}


void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.setCursor(0,30);
  display.print("Connecting to WiFi .."); 
  display.display();
  
  while ((WiFi.status() != WL_CONNECTED) && counter < 10) {
    counter++;
    delay(1000);
  }
  if (counter == 10) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(5,0);
    display.print("SMART OFFICE MODEM");
    display.setCursor(0,30);
    display.print("WiFi is failed!"); 
    display.display();
    delay(2000);
    ESP.restart();
  }

  counter = 0;
  
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.setCursor(0,30);
  display.print("WiFi is Connected!"); 
  display.display();
  delay(2000);
}


void streamCallback(FirebaseStream data){
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); //see addons/RTDBHelper.h
  Serial.println();

  String streamPath = String(data.dataPath());

  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer){
    String GPIO = streamPath.substring(1);
    int state = data.intData();
    Serial.print("GPIO: ");
    Serial.println(GPIO);
    Serial.print("STATE: ");
    Serial.println(state);
    LoRa.beginPacket();
    LoRa.print(R1 + "?" + GPIO + "&" +String(state));
    LoRa.endPacket();
  }
}

void streamTimeoutCallback(bool timeout){
  if (timeout)
    Serial.println("stream timeout, resuming...\n");
  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
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

void startLoRA(){
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(BAND) && counter < 10) {
    counter++;
    delay(500);
  }
  if (counter == 10) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(5,0);
    display.print("SMART OFFICE MODEM");
    display.setCursor(0,30);
    display.print("WiFi is Connected!"); 
    display.setCursor(0,50);
    display.print("Starting LoRa failed!");
    display.display();
    ESP.restart();
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.setCursor(0,30);
  display.print("WiFi is Connected!"); 
  display.setCursor(0,50);
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}


void connect_RTDB(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.setCursor(0,30);
  display.print("WiFi is Connected!"); 
  display.setCursor(0,50);
  display.print("Connecting to Server");
  display.display();

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  
  Firebase.reconnectWiFi(true);
  stream.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);
  while ((auth.token.uid) == "") {
    delay(1000);
  }
  uid = auth.token.uid.c_str();
  Serial.println(uid);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.setCursor(0,30);
  display.print("WiFi is Connected!"); 
  display.setCursor(0,50);
  display.print("Server is connected!");
  display.display();
  delay(2000);
}


void defaultposition(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.setCursor(15,30);
  display.print("Default positions"); 
  display.setCursor(28,40);
  display.print("are recorded"); 
  display.setCursor(28,50);
  display.print("Please wait!");
  display.display();
  
  for (int i = 0; i < 8; i++) {
    String topic = RelayPath + String(i);
    sendINT(topic, relay_default_position);
  }

  LoRa.beginPacket();
  LoRa.print(R1 + "?" + 0 + "&" + String(relay_default_position));
  LoRa.endPacket();
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(R1 + "?" + 1 + "&" + String(relay_default_position));
  LoRa.endPacket();
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(R1 + "?" + 2 + "&" + String(relay_default_position));
  LoRa.endPacket();
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(R1 + "?" + 3 + "&" + String(relay_default_position));
  LoRa.endPacket();
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(R1 + "?" + 4 + "&" + String(relay_default_position));
  LoRa.endPacket();
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(R1 + "?" + 5 + "&" + String(relay_default_position));
  LoRa.endPacket();
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(R1 + "?" + 6 + "&" + String(relay_default_position));
  LoRa.endPacket();
  delay(1000);
  LoRa.beginPacket();
  LoRa.print(R1 + "?" + 7 + "&" + String(relay_default_position));
  LoRa.endPacket();

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.setCursor(28,30);
  display.print("READY TO GO!"); 
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5,0);
  display.print("SMART OFFICE MODEM");
  display.setCursor(0,30);
  display.print("WiFI: Connected"); 
  display.setCursor(0,50);
  display.print("WiFI SSID: " + String(WiFi.SSID()));
  display.display();
}

void setup(){    
  Serial.begin(115200);
  
  startOLED();
  initWiFi();
  startLoRA();
  connect_RTDB();
  defaultposition();
  
  if (!Firebase.RTDB.beginStream(&stream, RelayPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
  delay(2000);
}

void loop(){
  unsigned long currentMillis = millis();

    if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
  
  if (((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval))) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(5,0);
    display.print("SMART OFFICE MODEM");
    display.setCursor(0,30);
    display.print("Reconnecting to WiFi!"); 
    display.display();
    delay(2000);
    ESP.restart();
    previousMillis = currentMillis;
  }
}
