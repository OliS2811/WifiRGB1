#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <ArduinoOTA.h>
#include "names.h"
#include "web_admin.h"
#include "web_interface.h"

#define PIN D4 // GPIO für NeoPixel
#define NUMPIXELS 9 // Anzahl der NeoPixel
#define MAX_BRIGHTNESS 255 // Maximale Helligkeit

const char* ssid = "FRITZ!BoxSA";  // WLAN-SSID
const char* password = "Raissa2200!";  // WLAN-Passwort

ESP8266WebServer server(80);
Adafruit_NeoPixel np(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Globale Variablen
int redValue = 0;
int greenValue = 0;
int blueValue = 0;
int brightness = MAX_BRIGHTNESS;

void setColor(int red, int green, int blue);
void applyHSV(int h, int s, int v);
void handleRoot();
void handleUI();
void handleAdmin();
void handleApiStateRequest();
void handleReset();

void setup() {
  Serial.begin(115200);

  // WLAN-Verbindung herstellen
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nVerbunden mit WLAN");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // OTA initialisieren
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "Sketch";
    } else { // U_SPIFFS
      type = "SPIFFS";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnde der Aktualisierung");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Fortschritt: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Fehler [%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Fehler");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Fehler");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Verbindungsfehler");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Empfangsfehler");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Fehler");
    }
  });
  ArduinoOTA.begin();
  Serial.println("OTA bereit");

  // NeoPixel initialisieren
  np.begin();
  np.setBrightness(brightness);
  setColor(redValue, greenValue, blueValue);

  // Routen definieren
  server.on("/", HTTP_GET, handleRoot);
  server.on("/ui", HTTP_GET, handleUI);
  server.on("/admin", HTTP_GET, handleAdmin);
  server.on("/api/v1/state", HTTP_POST, handleApiStateRequest);
  server.on("/api/v1/reset", HTTP_GET, handleReset);
  server.begin();
  Serial.println("HTTP-Server gestartet");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/plain", "NeoPixel WiFi RGB Controller ist aktiv");
}

void handleUI() {
  server.send_P(200, "text/html", WEBINTERFACE);
}

void handleAdmin() {
  server.send_P(200, "text/html", WEBADMIN);
}

void handleApiStateRequest() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Kein Body empfangen\"}");
    return;
  }

  StaticJsonDocument<512> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, server.arg("plain"));
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Ungültiges JSON\"}");
    return;
  }

  const char* state = jsonDoc["state"];
  if (strcmp(state, "OFF") == 0) {
    setColor(0, 0, 0);
    server.send(200, "application/json", "{\"state\":\"OFF\"}");
    return;
  }

  if (strcmp(state, "ON") != 0) {
    server.send(400, "application/json", "{\"error\":\"Ungültiger Status\"}");
    return;
  }

  if (jsonDoc["color"].is<JsonObject>()) {
    JsonObject color = jsonDoc["color"];
    if (color["r"].is<int>() && color["g"].is<int>() && color["b"].is<int>()) {
      int red = color["r"];
      int green = color["g"];
      int blue = color["b"];
      setColor(red, green, blue);
    } else {
      server.send(400, "application/json", "{\"error\":\"RGB-Werte fehlen\"}");
      return;
    }
  } else if (jsonDoc["hsv"].is<JsonObject>()) {
    JsonObject hsv = jsonDoc["hsv"];
    if (hsv["h"].is<int>() && hsv["s"].is<int>() && hsv["v"].is<int>()) {
      int h = hsv["h"];
      int s = hsv["s"];
      int v = hsv["v"];
      applyHSV(h, s, v);
    } else {
      server.send(400, "application/json", "{\"error\":\"HSV-Werte fehlen\"}");
      return;
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Farbwerte fehlen\"}");
    return;
  }

  if (jsonDoc["brightness"].is<int>()) {
    brightness = jsonDoc["brightness"];
    np.setBrightness(brightness);
  }

  server.send(200, "application/json", "{\"state\":\"ON\"}");
  np.show();
}

void handleReset() {
  setColor(0, 0, 0);
  np.setBrightness(MAX_BRIGHTNESS);
  np.show();
  server.send(200, "text/plain", "NeoPixel zurückgesetzt");
}

void setColor(int red, int green, int blue) {
  for (int i = 0; i < NUMPIXELS; i++) {
    np.setPixelColor(i, np.Color(red, green, blue));
  }
  np.show();
}

void applyHSV(int h, int s, int v) {
  uint32_t rgb = np.ColorHSV(h * 65536L / 360, s * 255 / 100, v * 255 / 100);
  for (int i = 0; i < NUMPIXELS; i++) {
    np.setPixelColor(i, rgb);
  }
  np.show();
}
