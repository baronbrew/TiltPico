#include <BTstackLib.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LEAmDNS.h>
#include <StreamString.h>

#ifndef STASSID
#define STASSID "cameowifi"
#define STAPSK "jalabean"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

WebServer server(80);

const int led = LED_BUILTIN;

char macChar[17];

void handleRoot() {
  static int cnt = 0;
  digitalWrite(led, 1);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  StreamString temp;
  temp.reserve(500); // Preallocate a large chunk to avoid memory fragmentation
  temp.printf("<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>Pico-W Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>iBeacon found: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Free Memory: %d</p>\
    <p>Page Count: %d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>", macChar[0], macChar[1], macChar[2], macChar[3], macChar[4], macChar[5], macChar[6], macChar[7], macChar[8], macChar[9], macChar[10], macChar[11], macChar[12], macChar[13], macChar[14], macChar[15], macChar[16], hr, min % 60, sec % 60, rp2040.getFreeHeap(), ++cnt);
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void drawGraph() {
  String out;
  out.reserve(2600);
  char temp[70];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}

/* EXAMPLE_START(iBeaconScanner): iBeacon Scanner

   @section Setup

   @text After BTstack.setup(), BTstack is configured to call
   advertisementCallback whenever an Advertisement was received.
   Then, a device discovery is started
*/

/* LISTING_START(iBeaconCallback): iBeacon Scanner Callback */
void advertisementCallback(BLEAdvertisement *adv) {
  if (adv->isIBeacon()) {
    Serial.print("iBeacon found ");
    Serial.print(adv->getBdAddr()->getAddressString());
    const char *mac = adv->getBdAddr()->getAddressString();
    strncpy(macChar, mac, 17);
    Serial.print(", RSSI ");
    Serial.print(adv->getRssi());
    Serial.print(", UUID ");
    Serial.print(adv->getIBeaconUUID()->getUuidString());
    Serial.print(", MajorID ");
    Serial.print(adv->getIBeaconMajorID());
    Serial.print(", MinorID ");
    Serial.print(adv->getIBecaonMinorID());
    Serial.print(", Measured Power ");
    Serial.println(adv->getiBeaconMeasuredPower());
  }
}
/* LISTING_END(iBeaconCallback): iBeacon Scanner Callback */

/* LISTING_START(iBeaconSetup): iBeacon Scanner Setup */
void setup(void) {
  Serial.begin(9600);
  BTstack.setup();
  BTstack.setBLEAdvertisementCallback(advertisementCallback);
  BTstack.bleStartScanning();
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("tiltpico-01")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}
/* LISTING_END(iBeaconSetup): iBeacon Scanner Setup */

void loop(void) {
  BTstack.loop();
  server.handleClient();
  MDNS.update();
}

/*
   @section Advertisement Callback

   @text Whenever an Advertisement is received, isIBeacon() checks if
   it contains an iBeacon. If yes, the Major ID, Minor ID, and UUID
   is printed.
   If it's not an iBeacon, only the BD_ADDR and the received signal strength
   (RSSI) is shown.
*/
