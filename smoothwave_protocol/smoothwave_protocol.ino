#include <Adafruit_NeoPixel.h>

#define LED_PIN    6    // NeoPixel-Streifen an Pin 6
#define BUZZER_PIN 13   // Buzzer an Pin 13
#define ANALOG_PIN A0   // Analoger Eingang an Pin A0
#define LED_COUNT  5    // 5 LEDs

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

float Vin = 5.0;
float Vout = 0;
float R1 = 100000.0;      // Bekannter Widerstand: 100 kOhm
float R2 = 0.0;
float Schwellwert = 3000.0; // Schwellenwert in Ohm

bool triggered = false;

// Neue Helligkeitsfunktionen für 150 ms Gesamtzyklus

// Mittlere LED: fade in von 0-12ms, konstant 12-37ms, fade out 37-62ms (ungefähr)
uint8_t getBrightnessMiddle(unsigned long t) {
  if (t < 12) {
    return map(t, 0, 12, 0, 255);
  } else if (t < 37) {
    return 255;
  } else if (t < 62) {
    return map(t, 37, 62, 255, 0);
  } else {
    return 0;
  }
}

// Mittlere Nachbarn (LED 1 und 3): fade in 37-50ms, konstant 50-75ms, fade out 75-87ms
uint8_t getBrightnessMiddleNeighbors(unsigned long t) {
  if (t < 37) {
    return 0;
  } else if (t < 50) {
    return map(t, 37, 50, 0, 255);
  } else if (t < 75) {
    return 255;
  } else if (t < 87) {
    return map(t, 75, 87, 255, 0);
  } else {
    return 0;
  }
}

// Äußerste LEDs (LED 0 und 4): fade in 75-87ms, konstant 87-125ms, fade out 125-150ms
uint8_t getBrightnessOutermost(unsigned long t) {
  if (t < 75) {
    return 0;
  } else if (t < 87) {
    return map(t, 75, 87, 0, 255);
  } else if (t < 125) {
    return 255;
  } else if (t < 150) {
    return map(t, 125, 150, 255, 0);
  } else {
    return 0;
  }
}

// smoothWavePulse: Erzeugt einen LED-Impuls über totalDuration ms (hier 150ms) mit smoothen Überblendungen,
// sodass während des Fade-In bereits die nächste LED einblendet, ohne dass zwischendurch alle LEDs dunkel werden.
// Während des gesamten Zyklus bleibt der Buzzer aktiv.
void smoothWavePulse(uint32_t color, int totalDuration) {
  unsigned long startTime = millis();
  while (millis() - startTime < totalDuration) {
    unsigned long t = millis() - startTime; // Zeit in ms seit Beginn des Zyklus
    
    uint8_t bMiddle = getBrightnessMiddle(t);
    uint8_t bNeighbor = getBrightnessMiddleNeighbors(t);
    uint8_t bOuter = getBrightnessOutermost(t);
    
    // Setze LED 2 (mittlere LED)
    strip.setPixelColor(2, strip.Color(
      (((color >> 16) & 0xFF) * bMiddle) / 255,
      (((color >> 8) & 0xFF) * bMiddle) / 255,
      ((color & 0xFF) * bMiddle) / 255
    ));
    
    // Setze LEDs 1 und 3 (mittlere Nachbarn)
    strip.setPixelColor(1, strip.Color(
      (((color >> 16) & 0xFF) * bNeighbor) / 255,
      (((color >> 8) & 0xFF) * bNeighbor) / 255,
      ((color & 0xFF) * bNeighbor) / 255
    ));
    strip.setPixelColor(3, strip.Color(
      (((color >> 16) & 0xFF) * bNeighbor) / 255,
      (((color >> 8) & 0xFF) * bNeighbor) / 255,
      ((color & 0xFF) * bNeighbor) / 255
    ));
    
    // Setze äußerste LEDs (Indices 0 und 4)
    strip.setPixelColor(0, strip.Color(
      (((color >> 16) & 0xFF) * bOuter) / 255,
      (((color >> 8) & 0xFF) * bOuter) / 255,
      ((color & 0xFF) * bOuter) / 255
    ));
    strip.setPixelColor(LED_COUNT - 1, strip.Color(
      (((color >> 16) & 0xFF) * bOuter) / 255,
      (((color >> 8) & 0xFF) * bOuter) / 255,
      ((color & 0xFF) * bOuter) / 255
    ));
    
    strip.show();
    digitalWrite(BUZZER_PIN, HIGH); // Buzzer an während der Animation
    delay(10);
  }
  digitalWrite(BUZZER_PIN, LOW);
  // Am Ende sicherstellen, dass alle LEDs aus sind
  strip.clear();
  strip.show();
}

void setup() {
  Serial.begin(11550);
  pinMode(BUZZER_PIN, OUTPUT);
  strip.begin();
  strip.clear();
  strip.show();
  strip.setBrightness(50);
}

void loop() {
  int samples = 10;
  float total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(ANALOG_PIN);
    delay(1);
  }
  
  float raw = total / samples;
  Vout = (raw * Vin) / 1023.0;
  
  if (Vout > 0) {
    R2 = R1 * ((Vin / Vout) - 1.0);
    Serial.println(R2);
  }
  
  // Triggern, wenn R2 unter dem Schwellenwert liegt
  if (R2 < Schwellwert && !triggered) {
    triggered = true;
    // Wiederhole das gesamte Muster 3-mal, mit 25ms Pause zwischen den Zyklen
    for (int cycle = 0; cycle < 3; cycle++) {
      smoothWavePulse(strip.Color(255, 0, 0), 150);  // Gesamtzyklus 150ms
      delay(25);
    }
  } 
  else if (R2 >= Schwellwert) {
    triggered = false;
  }
}
