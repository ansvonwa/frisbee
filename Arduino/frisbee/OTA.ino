#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "OTACredentials.h"

bool _OTA_running = false;
bool _OTA_connected = false;

size_t wifiIdx = 0;

void OTA_setup() {
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssids[0], passwords[0]);
  Serial.print("Trying to connect to ");
  Serial.println(ssids[0]);

  ArduinoOTA.setHostname("Frisbee");
  ArduinoOTA.setPassword(NULL);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  _OTA_running = true;
}

inline bool OTA_check() {
  Serial.println("OTA_check()");
  if (WiFi.status() == WL_CONNECTED) {
    _OTA_running = true;
  } else {
    _OTA_running = false;
  }
  return _OTA_running;
}

inline bool OTA_running() {
  return _OTA_running;
}

inline void OTA_stop() {
  _OTA_running = false;
}

inline void OTA_loop() {
  if (_OTA_running) {
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      Serial.print("Wifi network ");
      Serial.print(ssids[wifiIdx]);
      Serial.println(" not available");
      wifiIdx++;
      if (wifiIdx < num_ssids) {
        WiFi.begin(ssids[wifiIdx], passwords[wifiIdx]);
        Serial.print("Trying to connect to ");
        Serial.println(ssids[wifiIdx]);
        ArduinoOTA.begin();
      } else {
        _OTA_running = false;
        Serial.println("Giving OTA up");
      }
    } else if (WiFi.status() == WL_CONNECTED) {
      if (!_OTA_connected) {
        Serial.print("Connected to ");
        Serial.println(ssids[wifiIdx]);
        _OTA_connected = true;
      }
      ArduinoOTA.handle();
    } else if (WiFi.status() == WL_DISCONNECTED) {
      ;
    } else {
      Serial.println(WiFi.status());
    }
  }
}
