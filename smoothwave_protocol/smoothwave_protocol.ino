#include <Adafruit_NeoPixel.h>

#define LED_PIN    6      // NeoPixel-Streifen an Pin 6
#define BUZZER_PIN 13     // Buzzer an Pin 13
#define ANALOG_PIN A0     // Analoger Eingang an Pin A0
#define LED_COUNT  5      // 5 LEDs

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

float Vin = 5.0;
float Vout = 0;
float R1 = 100000.0;      // Bekannter Widerstand: 100 kOhm
float R2 = 0.0;
float Schwellwert = 3000.0; // Schwellenwert in Ohm

bool triggered = false;

// Hilfsfunktionen: Berechnen den Helligkeitswert (0-255) für jede LED basierend auf t (in ms)
// Gesamtdauer pro Impuls: 600ms

// Für die mittlere LED (Index 2)
uint8_t getBrightnessMiddle(unsigned long t) {
  if (t < 50) {
    return map(t, 0, 50, 0, 255);
  } else if (t < 150) {
    return 255;
  } else if (t < 250) {
    return map(t, 150, 250, 255, 0);
  } else {
    return 0;
  }
}

// Für die mittleren Nachbar-LEDs (Index 1 und 3)
uint8_t getBrightnessMiddleNeighbors(unsigned long t) {
  if (t < 150) {
    return 0;
  } else if (t < 200) {
    return map(t, 150, 200, 0, 255);
  } else if (t < 300) {
    return 255;
  } else if (t < 350) {
    return map(t, 300, 350, 255, 0);
  } else {
    return 0;
  }
}

// Für die äußersten LEDs (Index 0 und LED_COUNT-1, also 0 und 4)
uint8_t getBrightnessOutermost(unsigned long t) {
  if (t < 300) {
    return 0;
  } else if (t < 350) {
    return map(t, 300, 350, 0, 255);
  } else if (t < 500) {
    return 255;
  } else if (t < 600) {
    return map(t, 500, 600, 255, 0);
  } else {
    return 0;
  }
}

// Diese Funktion führt einen kompletten, smoothen Wellenimpuls (600ms) aus.
// Während der gesamten 600ms wird in kleinen Schritten (z.B. alle 20ms) 
// der aktuelle t-Wert berechnet, und die Helligkeit jeder LED wird neu gesetzt.
// So entsteht ein fließender Übergang, ohne dass zwischendurch alle LEDs dunkel sind.
void smoothWavePulse(uint32_t color, int totalDuration) {
  unsigned long startTime = millis();
  while (millis() - startTime < totalDuration) {
    unsigned long t = millis() - startTime; // t in [0, totalDuration)
    
    // Berechne für jede LED den Helligkeitswert (0-255) anhand des Zeitverlaufs
    // Wir gehen von der Reihenfolge aus: mittlere LED, dann mittlere Nachbarn, dann äußerste LEDs.
    uint8_t brightnessMiddle = getBrightnessMiddle(t);
    uint8_t brightnessNeighbor = getBrightnessMiddleNeighbors(t);
    uint8_t brightnessOuter = getBrightnessOutermost(t);
    
    // Setze LED 2 (mittlere LED)
    strip.setPixelColor(2, strip.Color(
      ((color >> 16) & 0xFF) * brightnessMiddle / 255,
      ((color >> 8) & 0xFF) * brightnessMiddle / 255,
      (color & 0xFF) * brightnessMiddle / 255
    ));
    
    // Setze LED 1 und LED 3 (mittlere Nachbarn)
    strip.setPixelColor(1, strip.Color(
      ((color >> 16) & 0xFF) * brightnessNeighbor / 255,
      ((color >> 8) & 0xFF) * brightnessNeighbor / 255,
      (color & 0xFF) * brightnessNeighbor / 255
    ));
    strip.setPixelColor(3, strip.Color(
      ((color >> 16) & 0xFF) * brightnessNeighbor / 255,
      ((color >> 8) & 0xFF) * brightnessNeighbor / 255,
      (color & 0xFF) * brightnessNeighbor / 255
    ));
    
    // Setze äußerste LEDs (LED 0 und LED 4)
    strip.setPixelColor(0, strip.Color(
      ((color >> 16) & 0xFF) * brightnessOuter / 255,
      ((color >> 8) & 0xFF) * brightnessOuter / 255,
      (color & 0xFF) * brightnessOuter / 255
    ));
    strip.setPixelColor(LED_COUNT - 1, strip.Color(
      ((color >> 16) & 0xFF) * brightnessOuter / 255,
      ((color >> 8) & 0xFF) * brightnessOuter / 255,
      (color & 0xFF) * brightnessOuter / 255
    ));
    
    strip.show();
    // Halte den Buzzer während der gesamten Animation an
    digitalWrite(BUZZER_PIN, HIGH);
    delay(20);
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
  
  // Triggern, wenn der gemessene Widerstand unter dem Schwellenwert liegt
  if (R2 < Schwellwert && !triggered) {
    triggered = true;
    // Wiederhole das gesamte Muster 3-mal, mit 50ms Pause zwischen den Zyklen
    for (int cycle = 0; cycle < 3; cycle++) {
      smoothWavePulse(strip.Color(255, 0, 0), 600);  // 600ms pro Zyklus
      delay(50);
    }
  } 
  else if (R2 >= Schwellwert) {
    triggered = false;
  }
}
