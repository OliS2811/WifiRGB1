#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "FRITZ!BoxSA";  // WLAN-SSID
const char* password = "Raissa2200!";  // WLAN-Passwort

void setup() {
    Serial.begin(115200);

    // WLAN-Verbindung herstellen
    WiFi.begin("FRITZ!BoxSA", "Raissa2200!");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi verbunden");

    // OTA-Setup
    ArduinoOTA.setHostname("ESP-0D1FF8");
    ArduinoOTA.setPassword("OTA_Test");

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
}

void loop() {
    ArduinoOTA.handle();
}
