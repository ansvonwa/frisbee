/*
   Captive Portal by: M. Ray Burnette 20150831
   See Notes tab for original code references and compile requirements
   Sketch uses 300,640 bytes (69%) of program storage space. Maximum is 434,160 bytes.
   Global variables use 50,732 bytes (61%) of dynamic memory, leaving 31,336 bytes for local variables. Maximum is 81,920 bytes.
*/

#include <ESP8266WiFi.h>
#include "./DNSServer.h"                  // Patched lib
#include <ESP8266WebServer.h>

const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress         apIP(10, 10, 10, 1);    // Private network for server
DNSServer         dnsServer;              // Create the DNS object
ESP8266WebServer  webServer(80);          // HTTP server

String responseHTML = String("") +
                   "<!DOCTYPE html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">" +
                   "<html><head><title>CaptivePortal</title></head><body>" +
                   "<h1>Frisbee Settings!</h1><p>Choose Animation:</p><p>" +
                   "<a href=\"/anim/" + ThreeDotsRGB + "\">ThreeDotsRGB</a><br/>" +
                   "<a href=\"/anim/" + Glow + "\">Glow</a><br/>" +
                   "<a href=\"/anim/" + RedToGreen + "\">RedToGreen</a><br/>" +
                   "<a href=\"/anim/" + HueFuu + "\">HueFuu</a><br/>" +
                   "<a href=\"/anim/" + TwoWayRotation + "\">TwoWayRotation</a><br/>" +
                   "<a href=\"/anim/" + SineRotation + "\">SineRotation</a><br/>" +
                   "<a href=\"/anim/" + Radioactive + "\">Radioactive</a><br/>" +
                   "<a href=\"/anim/" + Randots + "\">Randots</a><br/>" +
                   "<a href=\"/anim/" + FadingDot + "\">FadingDot</a><br/>" +
                   "<a href=\"/anim/" + Sparkling + "\">Sparkling</a><br/>" +
                   "<a href=\"/anim/" + ColorByDirection + "\">ColorByDirection</a><br/>" +
                   "<a href=\"/anim/" + ReversedDots + "\">ReversedDots</a><br/>" +
                   "</p></body></html>";

bool _CP_running = false;

void CP_setup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Frisbee Settings");
  WiFi.hostname("Frisbee");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", responseHTML);
    String uri = webServer.uri();
    Serial.println(uri);
    if (uri.startsWith("/anim/")) {
      Serial.println(uri.substring(6));
      currentAnim = (Animation) uri.substring(6).toInt();
      lastChangeTime = micros();
      Serial.print("currentAnim = ");
      Serial.println(currentAnim);
    }
  });
  webServer.begin();
  _CP_running = true;
}

inline bool CP_running() {
  return _CP_running;
}

inline void CP_loop() {
  if (_CP_running) {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }
}



