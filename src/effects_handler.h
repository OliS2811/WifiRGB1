#ifndef EFFECTS_HANDLER_H
#define EFFECTS_HANDLER_H

#include <Adafruit_NeoPixel.h>
#include "names.h"

extern Adafruit_NeoPixel pixels; // Der LED-Streifen wird aus der Hauptdatei referenziert
extern int currentBrightness;

#define CANDLE_BRIGHTNESS_MIN 150
#define CANDLE_BRIGHTNESS_MAX 255
#define CANDLE_DELAY_MIN 50
#define CANDLE_DELAY_MAX 200

void candleEffect() {
    static unsigned long lastUpdate = 0; // Zeitstempel für das letzte Update
    unsigned long now = millis();

    // Nur aktualisieren, wenn die Verzögerung abgelaufen ist
    if (now - lastUpdate > random(CANDLE_DELAY_MIN, CANDLE_DELAY_MAX)) {
        lastUpdate = now;

        for (int i = 0; i < pixels.numPixels(); i++) {
            int flicker = random(CANDLE_BRIGHTNESS_MIN, CANDLE_BRIGHTNESS_MAX);
            int adjustedFlicker = (flicker * currentBrightness) / 255; // Helligkeit berücksichtigen
            int red = (adjustedFlicker * 255) / CANDLE_BRIGHTNESS_MAX;
            int green = (adjustedFlicker * 140) / CANDLE_BRIGHTNESS_MAX; // Warme Farben
            int blue = 0; // Kein Blau für warmes Licht

            pixels.setPixelColor(i, pixels.Color(red, green, blue));

            // Debugging-Ausgabe
            //Serial.print("LED ");
            //Serial.print(i);
            //Serial.print(": Red: ");
            //Serial.print(red);
            //Serial.print(", Green: ");
            //Serial.print(green);
            //Serial.print(", Flicker: ");
            //Serial.println(flicker);
        }
        pixels.show();
    }
}
#endif // EFFECTS_HANDLER_H