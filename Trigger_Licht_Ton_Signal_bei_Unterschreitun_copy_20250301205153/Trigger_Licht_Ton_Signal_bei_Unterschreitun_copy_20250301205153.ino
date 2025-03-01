#include <Adafruit_NeoPixel.h>

// Pin-Zuordnungen und Einstellungen
#define LED_PIN    6        // NeoPixel-Streifen an Pin 6
#define BUZZER_PIN 13       // Buzzer an Pin 13
#define ANALOG_PIN A0       // Analoger Eingang an Pin A0
#define LED_COUNT  5        // Anzahl der genutzten LEDs

// NeoPixel-Objekt initialisieren
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

float Vin = 5.0;
float Vout = 0;
float R1 = 100000.0;
float R2 = 0.0;
float Schwellwert = 3000.0;  // Schwellenwert in Ohm

bool triggered = false;      // Flag, um wiederholtes Triggern zu vermeiden

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  strip.begin();
  strip.show();              // Alle LEDs ausschalten
  strip.setBrightness(50);   // Helligkeit setzen
}

void loop() {
  int samples = 10;  // Anzahl der Messungen für Mittelwertbildung
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

  // Triggern, wenn R2 unter den Schwellwert fällt und noch nicht getriggert wurde
  if (R2 < Schwellwert && !triggered) {
    triggered = true;
    tone(BUZZER_PIN, 2000, 80);          // 2000 Hz Ton für 80ms
    wavePulse(strip.Color(255, 0, 0), 20); // LED-Wellenimpuls mit 20ms pro Schritt
  }
  else if (R2 >= Schwellwert) {
    triggered = false;
  }
}

// Funktion zum sanften Ausblenden einer einzelnen LED (Fading)
void fadeOutLED(int index, uint32_t color, int duration) {
  int steps = 10;  // Anzahl der Fading-Schritte
  int delayTime = duration / steps;  
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  for (int i = steps; i >= 0; i--) {
    uint8_t nr = (r * i) / steps;
    uint8_t ng = (g * i) / steps;
    uint8_t nb = (b * i) / steps;
    strip.setPixelColor(index, strip.Color(nr, ng, nb));
    strip.show();
    delay(delayTime);
  }
}
// Funktion, die einen LED-Wellenimpuls erzeugt, der dreimal hintereinander ausgeführt wird.
// Das Muster: Zuerst wird in 3 Schritten von der Mitte aus (einschalten) das Licht aktiviert,
// danach wird in derselben Reihenfolge (von der Mitte aus) das Licht ausgeschaltet.
// Zwischen den Wiederholungen gibt es eine kurze Pause.
void wavePulse(uint32_t color, int stepDelay) {
  for (int rep = 0; rep < 3; rep++) {
    strip.clear();
    strip.show();
    int mid = LED_COUNT / 2;  // Bei 5 LEDs ist Index 2 die Mitte

    // Einschaltphase: Von der Mitte aus nach außen
    // Schritt 1: Mittlere LED anschalten
    strip.setPixelColor(mid, color);
    strip.show();
    delay(stepDelay);
    
    // Schritt 2: LEDs neben der Mitte anschalten (Index mid-1 und mid+1)
    if (mid - 1 >= 0) {
      strip.setPixelColor(mid - 1, color);
    }
    if (mid + 1 < LED_COUNT) {
      strip.setPixelColor(mid + 1, color);
    }
    strip.show();
    delay(stepDelay);
    
    // Schritt 3: Äußerste LEDs anschalten (Index 0 und LED_COUNT-1)
    strip.setPixelColor(0, color);
    strip.setPixelColor(LED_COUNT - 1, color);
    strip.show();
    delay(stepDelay);
    
    // Ausschaltphase: In derselben Reihenfolge, also von der Mitte aus nach außen
    fadeOutLED(mid, color, 50);  // Mittlere LED ausfaden
    if (mid - 1 >= 0) {
      fadeOutLED(mid - 1, color, 50);
    }
    if (mid + 1 < LED_COUNT) {
      fadeOutLED(mid + 1, color, 50);
    }
    fadeOutLED(0, color, 50);
    fadeOutLED(LED_COUNT - 1, color, 50);
    
    // Kurze Pause zwischen den Wiederholungen
    delay(50);
  }
  // Am Ende sicherstellen, dass alle LEDs aus sind
  strip.clear();
  strip.show();
}
