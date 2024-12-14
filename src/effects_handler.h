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
    if (now - lastUpdate > (unsigned long)random(CANDLE_DELAY_MIN, CANDLE_DELAY_MAX)) {
        lastUpdate = now;

        for (int i = 0; i < pixels.numPixels(); i++) {
            int flicker = random(CANDLE_BRIGHTNESS_MIN, CANDLE_BRIGHTNESS_MAX);
            int adjustedFlicker = (flicker * currentBrightness) / 255; // Helligkeit berücksichtigen
            int red = (adjustedFlicker * 255) / CANDLE_BRIGHTNESS_MAX;
            int green = (adjustedFlicker * 140) / CANDLE_BRIGHTNESS_MAX; // Warme Farben
            int blue = 0; // Kein Blau für warmes Licht

            pixels.setPixelColor(i, pixels.Color(red, green, blue));
        }
        pixels.show();
    }
}

void rainbowEffect() {
    static uint16_t firstPixelHue = 0;
    for (int i = 0; i < pixels.numPixels(); i++) {
        int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
        pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show();
    firstPixelHue += 256;
}

void sunriseEffect() {
    static unsigned long lastUpdate = 0;
    static int step = 0;
    unsigned long now = millis();

    if (now - lastUpdate > 1000) { // Aktualisierung alle 1 Sekunde
        lastUpdate = now;

        int r = map(step, 0, 100, 0, 255);
        int g = map(step, 0, 100, 0, 160);
        int b = map(step, 0, 100, 0, 80);

        for (int i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, pixels.Color(r, g, b));
        }
        pixels.show();

        if (step < 100) {
            step++;
        }
    }
}

void thunderstormEffect() {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();

    if (now - lastUpdate > random(50, 500)) { // Zufällige Verzögerung für Blitze
        lastUpdate = now;

        int brightness = random(100, 255);
        for (int i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, pixels.Color(brightness, brightness, brightness));
        }
        pixels.show();

        delay(random(50, 200)); // Blitzdauer
        for (int i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, 0);
        }
        pixels.show();
    }
}

void auroraEffect() {
    static uint16_t firstPixelHue = 0;
    for (int i = 0; i < pixels.numPixels(); i++) {
        int pixelHue = firstPixelHue + (i * 32768L / pixels.numPixels()); // Grün bis Violett
        pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue, 255, 128)));
    }
    pixels.show();
    firstPixelHue += 128;
}

void colorExplosionEffect() {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();

    if (now - lastUpdate > 100) { // Schnelle Farbwechsel
        lastUpdate = now;
        for (int i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, pixels.Color(random(0, 255), random(0, 255), random(0, 255)));
        }
        pixels.show();
    }
}

#endif // EFFECTS_HANDLER_H
