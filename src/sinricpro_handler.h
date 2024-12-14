#ifndef SINRICPRO_HANDLER_H
#define SINRICPRO_HANDLER_H

#include <SinricPro.h>
#include <SinricProLight.h>
#include "effects_handler.h"
#include "names.h"

// Sinric Pro Credentials
#define APP_KEY       "74476a59-2de0-4a1d-87b3-ef613aa3618c"       // Ihr Sinric Pro App Key
#define APP_SECRET    "09b47197-eb14-43b1-9853-2f51babaf8ba-e0b8c780-9163-4aa9-b13a-8230011d4c76"    // Ihr Sinric Pro App Secret
#define LIGHT_ID      "67585b5413f98f141605b10d"    // Ihre Geräte-ID

// Externe Funktionsprototypen
extern void setNeoPixelColor(int r, int g, int b); // Funktion zum Setzen der Farbe
extern void resetOutputs();                       // Funktion zum Zurücksetzen der LEDs
extern int currentBrightness;                    // Globale Helligkeitsvariable
extern String activeEffect;                      // Globale Variable für aktive Effekte

// Prototypen für SinricPro-Callbacks
bool onPowerState(const String &deviceId, bool &state);
bool onBrightness(const String &deviceId, int &brightness);
bool onColor(const String &deviceId, byte &r, byte &g, byte &b);
bool onColorTemperature(const String &deviceId, int &colorTemperature);

// Setup SinricPro
void setupSinricPro() {
    SinricProLight &myLight = SinricPro[LIGHT_ID];

    // Callback-Funktionen registrieren
    myLight.onPowerState(onPowerState);
    myLight.onBrightness(onBrightness);
    myLight.onColor(onColor); // Registrierung der Farbfunktion
    myLight.onColorTemperature(onColorTemperature); // Registrierung der Farbtemperatur-Funktion

    // SinricPro initialisieren
    SinricPro.begin(APP_KEY, APP_SECRET);
    Serial.println("SinricPro initialisiert");
}

// Loop-Handler für SinricPro
void handleSinricPro() {
    SinricPro.handle();
}

// Callback-Funktion: Ein-/Ausschalten
bool onPowerState(const String &deviceId, bool &state) {
    Serial.printf("Gerät %s: Power %s\n", deviceId.c_str(), state ? "ON" : "OFF");
    if (state) {
        if (activeEffect.isEmpty()) { // Nur aktivieren, wenn keine Szene aktiv ist
            activeEffect = "candle"; // Kerzenlichteffekt aktivieren
            Serial.println("Kerzenlichteffekt aktiviert.");
        }
    } else {
        resetOutputs();
        activeEffect = ""; // Effekt deaktivieren
        Serial.println("Alle Effekte deaktiviert.");
    }
    return true;
}

// Callback-Funktion: Helligkeit ändern
bool onBrightness(const String &deviceId, int &brightness) {
    Serial.printf("Gerät %s: Helligkeit %d\n", deviceId.c_str(), brightness);
    currentBrightness = brightness;
    if (!activeEffect.isEmpty()) {
        Serial.printf("Helligkeit geändert während der Szene '%s'.\n", activeEffect.c_str());
    }
    return true;
}

// Callback-Funktion: Farbe ändern
bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
    Serial.printf("Gerät %s: Farbe geändert zu R:%d, G:%d, B:%d\n", deviceId.c_str(), r, g, b);
    // Deaktivieren Sie die Szene, wenn die Farbe geändert wird
    activeEffect = "";
    setNeoPixelColor(r, g, b);
    return true;
}

// Callback-Funktion: Farbtemperatur ändern
bool onColorTemperature(const String &deviceId, int &colorTemperature) {
    Serial.printf("Gerät %s: Farbtemperatur auf %dK gesetzt\n", deviceId.c_str(), colorTemperature);

    // Deaktivieren Sie die Szene
    activeEffect = "";

    // Konvertieren Sie die Farbtemperatur in RGB-Werte
    int r, g, b;
    if (colorTemperature < 3000) { // Warmweiß
        r = 255;
        g = 160;
        b = 100;
    } else if (colorTemperature > 5000) { // Kaltweiß
        r = 200;
        g = 200;
        b = 255;
    } else { // Neutralweiß
        r = 255;
        g = 240;
        b = 220;
    }

    setNeoPixelColor(r, g, b); // LEDs entsprechend der Farbtemperatur einstellen
    return true; // Signalisiert Sinric Pro, dass das Kommando erfolgreich verarbeitet wurde
}

#endif
