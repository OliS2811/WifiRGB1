#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include "effects_handler.h"
#include "names.h" // Enthält die RGB-Definition
#include "web_admin.h"
#include "web_interface.h"

int currentBrightness = 255; // Standardhelligkeit (maximal)

// Globale Variable für aktive Effekte
String activeEffect = "";

// WLAN-Konfiguration
const char* ssid = "FRITZ!BoxSA";
const char* password = "Raissa2200!";
const char* deviceName = "WifiRGB-Test";

// NeoPixel-Konfiguration
#define NEOPIXEL_PIN D4 // GPIO-Pin für NeoPixel-Daten
#define NUMPIXELS 9    // Anzahl der NeoPixel im Streifen
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Server und Netzwerk
ESP8266WebServer server(80);
IPAddress clientIP(192, 168, 178, 254); // Statische IP des Geräts
IPAddress gateway(192, 168, 178, 1);    // Gateway
IPAddress subnet(255, 255, 255, 0);     // Subnetzmaske

// Funktionsprototypen
void handleRoot();
void handleNotFound();
void handleApiRequest();
void handleEffectRequest();
void resetOutputs();
void setNeoPixelColor(int r, int g, int b);
RGB hsvToRgb(double h, double s, double v); // Umrechnung HSV -> RGB

void setup(void) {
  Serial.begin(115200);

  // WLAN-Verbindung herstellen
  WiFi.mode(WIFI_STA);
  WiFi.hostname(deviceName);
  //WiFi.config(clientIP, gateway, subnet); // Statische IP, für DHCP entfernen
  WiFi.begin(ssid, password);

  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Verbunden mit: ");
  Serial.println(ssid);
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // mDNS-Responder starten
  if (MDNS.begin("RGB-Strip-Test")) {
    Serial.println("MDNS-Responder gestartet");
  }

  // OTA-Setup
  ArduinoOTA.setHostname("RGB-Strip-Test");

  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
          type = "Sketch";
      } else {
          type = "Filesystem";
      }
      Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
      Serial.println("\nUpdate abgeschlossen");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Fortschritt: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Fehler[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth-Fehler");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Start-Fehler");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Verbindungs-Fehler");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Empfangs-Fehler");
      else if (error == OTA_END_ERROR) Serial.println("End-Fehler");
  });

  // OTA starten
  ArduinoOTA.begin();
  Serial.println("OTA ist bereit");
  Serial.print("Hostname: ");
  Serial.println(ArduinoOTA.getHostname());

  // NeoPixel-Setup
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Web-Interface und REST-API
  server.on("/ui", HTTP_GET, []() {
    server.send_P(200, "text/html", WEBINTERFACE);
  });
  server.on("/admin", HTTP_GET, []() {
    server.send_P(200, "text/html", WEBADMIN);
  });
  server.on("/api/v1/state", HTTP_POST, handleApiRequest);
  server.on("/api/v1/reset", HTTP_GET, resetOutputs);

  // Effekt-API hinzufügen
  server.on("/api/v1/effect", HTTP_GET, handleEffectRequest);

  server.begin();
  Serial.println("WifiRGB HTTP server started");
}

void loop(void) {
  server.handleClient();

  // Aktiven Effekt ausführen
  if (activeEffect == "candle") {
    candleEffect();
  }
  // Weitere Effekte können hier hinzugefügt werden
}

// Effekt-Handler
void handleEffectRequest() {
  Serial.println("Endpunkt /api/v1/effect wurde aufgerufen");

  if (!server.hasArg("effect")) {
    server.send(400, "application/json", "{\"error\":\"No effect specified\"}");
    Serial.println("Fehler: Kein Effekt angegeben");
    return;
  }

  String effectName = server.arg("effect");
  activeEffect = effectName; // Effekt aktivieren
  Serial.println("Effekt-Anfrage: " + effectName);

  server.send(200, "application/json", "{\"effect\":\"" + effectName + "\"}");
}

// API-Handler
void handleApiRequest() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Kein JSON-Daten empfangen\"}");
    return;
  }

  DynamicJsonDocument jsonDocument(256); // Statt StaticJsonDocument
  DeserializationError error = deserializeJson(jsonDocument, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\":\"JSON-Parsing fehlgeschlagen\"}");
    return;
  }

  JsonObject root = jsonDocument.as<JsonObject>();
  const char* state = root["state"]; // ON oder OFF

  if (strcmp(state, "OFF") == 0) {
    resetOutputs();
    server.send(200, "application/json", "{\"status\":\"OFF\"}");
    return;
  }

  // Wenn ein Effekt aktiv ist, ändern wir nur die Helligkeit
  if (!activeEffect.isEmpty()) {
    if (root["brightness"].is<int>()) {
      currentBrightness = root["brightness"];
      Serial.println("Helligkeit während Effekt gesetzt auf: " + String(currentBrightness));
    }
    server.send(200, "application/json", "{\"status\":\"OK\"}");
    return;
  }

  // Wenn kein Effekt aktiv ist, setzen wir die Farbe und Helligkeit
  if (root["brightness"].is<int>()) {
    currentBrightness = root["brightness"];
    Serial.println("Helligkeit gesetzt auf: " + String(currentBrightness));
  }

  if (root["color"].is<JsonObject>()) {
    JsonObject color = root["color"];
    if (color["mode"] == "rgb") {
      int r = map(color["r"], 0, 100, 0, currentBrightness);
      int g = map(color["g"], 0, 100, 0, currentBrightness);
      int b = map(color["b"], 0, 100, 0, currentBrightness);
      setNeoPixelColor(r, g, b);
    }
  }

  server.send(200, "application/json", "{\"status\":\"OK\"}");
}

// Outputs zurücksetzen
void resetOutputs() {
  setNeoPixelColor(0, 0, 0);
  activeEffect = ""; // Effekt deaktivieren
  currentBrightness = 255;   // Helligkeit zurücksetzen
  Serial.println("Alle LEDs aus, Effekt deaktiviert, Helligkeit zurückgesetzt");
}

// Funktion zum Setzen der NeoPixel-Farben
void setNeoPixelColor(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

// Funktion zur Umrechnung von HSV in RGB
RGB hsvToRgb(double h, double s, double v) {
  int i;
  double f, p, q, t;
  byte r, g, b;

  h = max(0.0, min(360.0, h));
  s = max(0.0, min(100.0, s));
  v = max(0.0, min(100.0, v));

  s /= 100;
  v /= 100;

  if (s == 0) {
    r = g = b = round(v * 255);
    return {r, g, b};
  }

  h /= 60;
  i = floor(h);
  f = h - i;
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));

  switch (i) {
    case 0: r = round(255 * v); g = round(255 * t); b = round(255 * p); break;
    case 1: r = round(255 * q); g = round(255 * v); b = round(255 * p); break;
    case 2: r = round(255 * p); g = round(255 * v); b = round(255 * t); break;
    case 3: r = round(255 * p); g = round(255 * q); b = round(255 * v); break;
    case 4: r = round(255 * t); g = round(255 * p); b = round(255 * v); break;
    default: r = round(255 * v); g = round(255 * p); b = round(255 * q); break;
  }

  return {r, g, b};
}
