#include <ArduinoJson.h>
#include <BTstackLib.h>
#include <HTTPClient.h>
#include <LEAmDNS.h>
#include <LittleFS.h>
#include <SPI.h>
#include <WebServer.h>
#include <WiFi.h>


#define WIFI_CREDENTIAL_FILENAME ("wifi_cred.json")

String Router_SSID;
String Router_Pass;

#define SSID_MAX_LEN 32
#define PASS_MAX_LEN 64

char macChar[18];
int count = 0;

#define LED_ON HIGH
#define LED_OFF LOW

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw[PASS_MAX_LEN];
} WiFi_Credentials;

WiFi_Credentials WiFi_Creds;

typedef struct
{
  String uuid;
  int major;
  int minor;
  int tx_power;
  int rssi;
  char mac[17];
  char Color[30];
  String customCloudURL;
  bool logCloudData;
} JSON_FILE;



UUID tilt2REDUUID("A495BB10-C5B1-4B44-B512-1370F02D74DE");
UUID tilt2GREENUUID("A495BB20-C5B1-4B44-B512-1370F02D74DE");
UUID tilt2BLACKUUID("A495BB30-C5B1-4B44-B512-1370F02D74DE");
UUID tilt2PURPLEUUID("A495BB40-C5B1-4B44-B512-1370F02D74DE");
UUID tilt2ORANGEUUID("A495BB50-C5B1-4B44-B512-1370F02D74DE");
UUID tilt2BLUEUUID("A495BB60-C5B1-4B44-B512-1370F02D74DE");
UUID tilt2YELLOWUUID("A495BB70-C5B1-4B44-B512-1370F02D74DE");
UUID tilt2PINKUUID("A495BB80-C5B1-4B44-B512-1370F02D74DE");

WebServer server(80);

void handleRoot() {
  server.sendHeader("refresh", "5");
  Dir root = LittleFS.openDir("/");
  String output;
  while (root.next()) {
    File file = root.openFile("r");
    String fileName = file.name();
    if (fileName != WIFI_CREDENTIAL_FILENAME) {
      output += "tiltName: ";
      output += String(file.name());
      output += '\n';
      //server.streamFile(file, "text/plain");
    }
    file.close();
  }
  server.send(200, "text/plain", output);
}

void handleRead() {
  if (server.hasArg("Color")) {
    File file = LittleFS.open(server.arg("Color").c_str(), "r");
    if (file) {
      server.sendHeader("refresh", "5");
      server.streamFile(file, "text/plain");
      file.close();
    }
  }
}

void handleSet() {
  Serial.println(server.args());
  if (server.hasArg("Color")) {
    File file = LittleFS.open(server.arg("Color").c_str(), "r");
    if (file) {
      JSON_FILE JsonFile;
      int params = server.args();
      for (int i = 0; i < params; i++) {
        if (server.argName(i) == "Color") {
        } else if (server.argName(i) == "customCloudURL") {
          JsonFile.customCloudURL = server.arg(i);
        } else if (server.argName(i) == "logCloudDataCheck") {
          JsonFile.logCloudData = server.arg(i);
        }
      }
      updateJsonConfigData(server.arg("Color").c_str(), JsonFile);
    }
  }
}

unsigned long previousMillis = 0;  // will store last time LED was updated

// constants won't change:
const long interval = 30000;  // interval at which to blink (milliseconds)

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_LED, OUTPUT);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  Serial.begin(9600);
  while (!Serial && millis() < 5000)
    ;
  Serial.println("Application Begins");
  digitalWrite(PIN_LED, LED_OFF);
  if (!LittleFS.begin()) {
    Serial.println("\nLittleFS error");
  }
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  BTstack.setup();
  BTstack.setBLEAdvertisementCallback(advertisementCallback);
  gap_set_scan_params(0, 0x0012, 0x0012, 0);

  if (!loadWiFiConfigData()) {
    Serial.println("Loading WiFi Config Data Failed");
  } else {
    // Serial.println("Printing config file...");
    // printFile(WIFI_CREDENTIAL_FILENAME);
    WiFi.begin(WiFi_Creds.wifi_ssid, WiFi_Creds.wifi_pw);
    digitalWrite(PIN_LED, LED_OFF);
    BTstack.bleStopScanning();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    digitalWrite(PIN_LED, LED_ON);
    BTstack.bleStartScanning();
    if (MDNS.begin("tiltpico")) {
      Serial.println("MDNS responder started");
    }
    server.on("/", HTTP_GET, handleRoot);
    server.on("/set", HTTP_GET, handleSet);
    server.on("/read", HTTP_GET, handleRead);
    server.begin();
    Serial.println("HTTP server started");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    postData("ORANGE:D4:1B:F8:A6:2A:F3");
  }
  BTstack.loop();
  handleSerial();
  server.handleClient();
  MDNS.update();

  if (stringComplete) {
    //Serial.println(inputString);
    if (parseInput(inputString)) {
      //Serial.println(Router_SSID);
      //Serial.println(Router_Pass);
      strncpy(WiFi_Creds.wifi_ssid, Router_SSID.c_str(), sizeof(WiFi_Creds.wifi_ssid) - 1);
      strncpy(WiFi_Creds.wifi_pw, Router_Pass.c_str(), sizeof(WiFi_Creds.wifi_pw) - 1);
      saveWiFiConfigData();
      WiFi.begin(WiFi_Creds.wifi_ssid, WiFi_Creds.wifi_pw);
      digitalWrite(PIN_LED, LED_OFF);
      BTstack.bleStopScanning();
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      digitalWrite(PIN_LED, LED_ON);
      BTstack.bleStartScanning();
      if (MDNS.begin("tiltpico")) {
        Serial.println("MDNS responder started");
      }
      server.on("/", HTTP_GET, handleRoot);
      server.on("/set", HTTP_GET, handleSet);
      server.on("/read", HTTP_GET, handleRead);
      server.begin();
      Serial.println("HTTP server started");
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  } else {
    delay(5);
  }
}



void advertisementCallback(BLEAdvertisement *adv) {
  if (adv->isIBeacon()) {
    //Serial.print(count++);
    //listDir();
    //Serial.println("iBeacon found ");
    if (adv->getIBeaconUUID()->matches(&tilt2REDUUID)) {
      Serial.println("Found RED Tilt Hydrometer");
    } else if (adv->getIBeaconUUID()->matches(&tilt2GREENUUID)) {
      Serial.println("Found GREEN Tilt Hydrometer");
    } else if (adv->getIBeaconUUID()->matches(&tilt2BLACKUUID)) {
      Serial.println("Found BLACK Tilt Hydrometer");
    } else if (adv->getIBeaconUUID()->matches(&tilt2PURPLEUUID)) {
      Serial.println("Found PURPLE  Tilt Hydrometer");
    } else if (adv->getIBeaconUUID()->matches(&tilt2ORANGEUUID)) {
      Serial.println("Found ORANGE Tilt Hydrometer");
      char filename[30];
      strcpy(filename, "ORANGE:");
      strcat(filename, adv->getBdAddr()->getAddressString());
      //strcat(filename, ".json");
      File file = LittleFS.open(filename, "r");
      // if (file) {
      //   file.close();
      //   updateJsonSensorData(filename, adv);
      // }
      if (!file) {
        file.close();
        createJsonFile(filename, adv);
      } else {
        file.close();
        updateJsonSensorData(filename, adv);
        //printFile(filename);
      }
    } else if (adv->getIBeaconUUID()->matches(&tilt2BLUEUUID)) {
      Serial.println("Found BLUE Tilt Hydrometer");
      char filename[30];
      strcpy(filename, "BLUE:");
      strcat(filename, adv->getBdAddr()->getAddressString());
      //strcat(filename, ".json");
      File file = LittleFS.open(filename, "r");
      // if (file) {
      //   file.close();
      //   updateJsonSensorData(filename, adv);
      // }
      if (!file) {
        file.close();
        createJsonFile(filename, adv);
      } else {
        file.close();
        updateJsonSensorData(filename, adv);
        //printFile(filename);
      }
    } else if (adv->getIBeaconUUID()->matches(&tilt2YELLOWUUID)) {
      Serial.println("Found YELLOW Tilt Hydrometer");
    } else if (adv->getIBeaconUUID()->matches(&tilt2PINKUUID)) {
      Serial.println("Found PINK Tilt Hydrometer");
    } else {
      Serial.println("Not A Tilt Hydrometer");
    }
  }
}

void handleSerial() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\r') {
      stringComplete = true;
    }
  }
}

bool parseInput(String input) {
  //Serial.println(input.length());
  if (input.substring(0, 5) != "t2pwc") {
    Serial.println("Invalid Command");
    return false;
  }
  int ssid_length = input.substring(5, 7).toInt();
  Router_SSID = input.substring(7, 7 + ssid_length);
  //Serial.println(SSID);

  int pass_length = input.substring(7 + ssid_length, 7 + ssid_length + 2).toInt();
  Router_Pass = input.substring(7 + ssid_length + 2, 7 + ssid_length + 2 + pass_length);
  //Serial.println(Pass);

  if ((input.substring(7 + ssid_length + 2 + pass_length, input.length() - 1).toInt()) != (9 + pass_length + ssid_length)) {
    Serial.println("Invalid Input");
    return false;
  }

  Serial.println("Valid Input");
  return true;
}

bool loadWiFiConfigData() {
  File file = LittleFS.open(WIFI_CREDENTIAL_FILENAME, "r");
  Serial.print("LoadWiFiCfgFile ");
  if (file) {
    StaticJsonDocument<256> doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
      Serial.println(F("Failed to read file, using default configuration"));

    // Copy values from the JsonDocument to the Config
    strlcpy(WiFi_Creds.wifi_ssid,
            doc["ssid"],
            sizeof(WiFi_Creds.wifi_ssid));
    strlcpy(WiFi_Creds.wifi_pw,
            doc["pass"],
            sizeof(WiFi_Creds.wifi_pw));
    // Close the file (Curiously, File's destructor doesn't close the file)
    file.close();
    Serial.println("OK");
    return true;
  } else {
    Serial.println("Failed");
    return false;
  }
}

void saveWiFiConfigData() {
  LittleFS.remove(WIFI_CREDENTIAL_FILENAME);
  File file = LittleFS.open(WIFI_CREDENTIAL_FILENAME, "w");
  Serial.print("SaveWiFiCfgFile ");
  if (file) {
    StaticJsonDocument<256> doc;
    // Set the values in the document
    doc["ssid"] = WiFi_Creds.wifi_ssid;
    doc["pass"] = WiFi_Creds.wifi_pw;

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }
    file.close();
    Serial.println("OK");
  } else {
    Serial.println("Failed");
  }
}

// Prints the content of a file to the Serial
void printFile(const char *filename) {
  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

void createJsonFile(const char *filename, BLEAdvertisement *adv) {
  File file = LittleFS.open(filename, "w");
  Serial.println("CreateJSONFile ");
  if (file) {
    DynamicJsonDocument doc(1024);
    doc["uuid"] = adv->getIBeaconUUID()->getUuidString();
    doc["major"] = adv->getIBeaconMajorID();
    doc["minor"] = adv->getIBecaonMinorID();
    doc["tx_power"] = adv->getiBeaconMeasuredPower();
    doc["rssi"] = adv->getRssi();
    strcpy(macChar, adv->getBdAddr()->getAddressString());
    doc["mac"] = macChar;
    char Colour[30];
    doc["Color"] = filename;
    doc["customCloudURL"] = "";
    doc["logCloudDataCheck"] = true;

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }
    file.close();
    Serial.println("OK");
  } else {
    Serial.println("Failed");
  }
}

void updateJsonSensorData(const char *filename, BLEAdvertisement *adv) {
  Serial.println("UpdatingJSONSensor Data ");
  DynamicJsonDocument doc(1024);
  File file = LittleFS.open(filename, "r");
  deserializeJson(doc, file);
  file.close();

  doc["major"] = adv->getIBeaconMajorID();
  doc["minor"] = adv->getIBecaonMinorID();
  doc["tx_power"] = adv->getiBeaconMeasuredPower();
  doc["rssi"] = adv->getRssi();

  file = LittleFS.open(filename, "w");
  serializeJson(doc, file);
  file.close();
}

void updateJsonConfigData(const char *filename, JSON_FILE JsonConfigData) {
  Serial.println("Updating JSON ConfigData ");
  DynamicJsonDocument doc(1024);
  File file = LittleFS.open(filename, "r");
  deserializeJson(doc, file);
  file.close();

  if (JsonConfigData.customCloudURL != NULL) {
    doc["customCloudURL"] = JsonConfigData.customCloudURL;
  } else if (JsonConfigData.logCloudData != NULL) {
    doc["logCloudDataCheck"] = JsonConfigData.logCloudData;
  }


  file = LittleFS.open(filename, "w");
  serializeJson(doc, file);
  file.close();
}

void postData(const char *filename) {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.setInsecure();
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    Serial.println("Updating JSON ConfigData ");
    DynamicJsonDocument doc(1024);
    File file = LittleFS.open(filename, "r");
    if (file) {
      deserializeJson(doc, file);
      Serial.println("[HTTP] begin...\n");
      // configure target server and url
      String targetURL = doc["customCloudURL"];
      http.begin(targetURL);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
      int httpCode = http.POST("Timepoint=43331.79300001158&Temp=65.0&SG=1.01&Beer=Test%20Celsius%2C112&Color=ORANGE&Comment");
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {
          const String &payload = http.getString();
          Serial.println("received payload:\n<<");
          Serial.println(payload);
          Serial.println(">>");
        }
      } else {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
      file.close();
    }
  }
}
