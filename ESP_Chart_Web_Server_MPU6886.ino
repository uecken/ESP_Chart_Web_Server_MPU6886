/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

//まずはマイコンをサーバーとして実装する
//その後柔軟性を上げるためにマイコンはクライアントとし、スマホかクラウドにサーバーを実装する


// Import required libraries
#ifdef ESP32
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#endif
#include <Wire.h>

#include <M5StickC.h>
float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;
float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;
int mode = -1;

//8000 point makse esp32 reboot.
float accXarray[6000];
float accYarray[6000];
float accZarray[6000];
String accXstr = "0";
String accYstr = "0";
String accZstr = "0";
long acc_i = 0;
String name_ = "";
//String acc_buffer[];
//int top,bottom;

// Replace with your network credentials
const char* ssid = "elecom2g-6c8919";
const char* password = "lsnwple4aqr8";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);


  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  delay(1000);
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", String(accX).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    /*String cmd = request->getParam(0)->name().c_str();
    if(cmd=="start") acc_i=0;
    */
    String requested_index = request->getParam(0)->value().c_str();
    if(requested_index == "0"){
      request->send_P(200, "text/plain", "0");
      acc_i =0;
    }
    else{
      request->send_P(200, "text/plain", arrayToStr(accXarray, requested_index.toInt(),acc_i).c_str());
    }
    Serial.println(requested_index);
  });

  // Start server
  server.begin();
  Serial.printf("%3.3f,%3.3f,%3.3f\n", accX, accY, accZ);

  //Start MPU6886
  M5.begin();
  M5.IMU.Init();
}

void loop() {
  M5.update();

  // データ取得
  //M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  //M5.IMU.getAhrsData(&pitch, &roll, &yaw);

  accXarray[acc_i] = accX;  accYarray[acc_i] = accY; accZarray[acc_i] = accZ;
  Serial.printf("%d,%5.2f,%5.2f,%5.2f,\n", acc_i, accXarray[acc_i], accYarray[acc_i], accZarray[acc_i]);
  acc_i = acc_i + 1;

/*
  accXstr = accXstr + "," + accX;
  accYstr = accYstr + "," + accY;
  accZstr = accZstr + "," + accZ;
  if (acc_i % 3000 == 0 ) {
    Serial.println(accXstr);
    Serial.println(accYstr);
    Serial.println(accZstr);
  }
*/
  /*
    // モードチェンジ
    if ( mode == -1 || M5.BtnA.wasReleased() ) {
      mode++;
      mode = mode % 3;
    }
  */
  delay(10);//10sps 3second pollig NG
}

String arrayToStr(float* array, int requested_index,long current_index) {
//  int array_length = sizeof(array) / sizeof(array[0]));
//  Serial.println(array_length);

  String array_str = "0";
  Serial.println("request:"+String(requested_index));
  Serial.println("current:"+String(current_index));
  for (int i = requested_index; i < current_index; i++) {
    if(i==requested_index){
      array_str = array[i]; 
    }
    else{
      array_str = array_str + "," + array[i];
    }
  }
  return array_str;
}
