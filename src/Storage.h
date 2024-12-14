#ifndef STORAGE_H
#define STORAGE_H

#include <EEPROM.h>
#include <Arduino.h>

// Externe Variablen für den aktuellen Zustand
extern int currentBrightness;
extern String activeEffect;

// EEPROM-Größe definieren
#define EEPROM_SIZE 512

// Initialisierung des EEPROM
void initStorage() {
    EEPROM.begin(EEPROM_SIZE);
    Serial.println("EEPROM initialisiert.");
}

// Zustand speichern
void saveState() {
    Serial.println("Speichere Zustand:");
    Serial.println("Helligkeit: " + String(currentBrightness));
    Serial.println("Effekt: " + activeEffect);

    EEPROM.write(0, currentBrightness); // Helligkeit speichern
    EEPROM.write(1, activeEffect.length()); // Länge des Effekt-Namens speichern

    for (size_t i = 0; i < activeEffect.length(); i++) {
        EEPROM.write(2 + i, activeEffect[i]); // Effekt-Name speichern
    }

    EEPROM.commit(); // Änderungen schreiben
    Serial.println("Zustand gespeichert.");
}

// Zustand laden
void loadState() {
    currentBrightness = EEPROM.read(0); // Helligkeit laden

    size_t effectLength = EEPROM.read(1); // Länge des Effekt-Namens lesen
    activeEffect = ""; // Effekt zurücksetzen

    for (size_t i = 0; i < effectLength; i++) {
        activeEffect += char(EEPROM.read(2 + i)); // Effekt-Name lesen
    }

    Serial.println("Geladener Zustand:");
    Serial.println("Helligkeit: " + String(currentBrightness));
    Serial.println("Effekt: " + activeEffect);

    // Anwenden des geladenen Zustands
    if (!activeEffect.isEmpty()) {
        Serial.println("Wende Effekt an: " + activeEffect);
        if (activeEffect == "candle") {
            candleEffect();
        } else if (activeEffect == "rainbow") {
            rainbowEffect();
        } else if (activeEffect == "sunrise") {
            sunriseEffect();
        } else if (activeEffect == "thunderstorm") {
            thunderstormEffect();
        } else if (activeEffect == "aurora") {
            auroraEffect();
        } else if (activeEffect == "colorExplosion") {
            colorExplosionEffect();
        }
    }
}

#endif // STORAGE_H
