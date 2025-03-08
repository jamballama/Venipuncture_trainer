#include <Adafruit_NeoPixel.h>

#define LED_PIN    9    // NeoPixel-Streifen an Pin 6
#define BUZZER_PIN 6   // Buzzer an Pin 13
#define ANALOG_PIN A7   // Analoger Eingang an Pin A0
#define LED_COUNT  5    // 5 LEDs

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

float Vin = 5.0;
float Vout = 0;
float R1 = 100000.0;      // Bekannter Widerstand: 100 kOhm
float R2 = 0.0;
float Schwellwert = 2000.0; // Schwellenwert in Ohm

bool triggered = false;

// -----------------------------------------------------------------
// Kurze Impulse (Morse-Punkte) im Propagation-Stil
// -----------------------------------------------------------------
// Parameter:
//   toneDuration: z. B. 80ms
//   ledCycleDuration: z. B. 120ms
//   globalDuration: z. B. 160ms
// Die LED-Offsets werden relativ zu einer globalen Dauer von 1000ms skaliert.
void smoothWavePulseShortCustomWithPropagation(int toneDuration, int ledCycleDuration, int globalDuration) {
  float scale = (float)globalDuration / 1000.0;
  int offsets[LED_COUNT] = { (int)(200 * scale), (int)(100 * scale), 0, (int)(100 * scale), (int)(200 * scale) };
  
  unsigned long startTime = millis();
  while (millis() - startTime < globalDuration) {
    unsigned long t = millis() - startTime;
    
    // Buzzer: Aktiv nur während toneDuration
    if (t < toneDuration) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
    
    // Für jede LED individuell, mit eigenem Offset
    for (int i = 0; i < LED_COUNT; i++) {
      int effTime = t - offsets[i];
      uint32_t color;
      if (effTime < 0 || effTime > ledCycleDuration) {
        color = strip.Color(0, 0, 0);
      } else {
        float fraction = (float)effTime / ledCycleDuration;
        // Angepasster Farbverlauf:
        // 0.00 - 0.03: Weiß → Gelb
        // 0.03 - 0.08: Gelb → Orange
        // 0.08 - 0.30: Orange → Rot (schneller Übergang)
        // 0.30 - 0.95: Reines Rot
        // 0.95 - 1.00: Rot → Schwaches Rot (20% Intensität, ca. 51)
        if (fraction < 0.03) {
          float localFrac = fraction / 0.03;
          uint8_t r = 255, g = 255, b = 255 - (uint8_t)(255 * localFrac);
          color = strip.Color(r, g, b);
        } else if (fraction < 0.08) {
          float localFrac = (fraction - 0.03) / 0.05;
          uint8_t r = 255, g = 255 - (uint8_t)((255 - 165) * localFrac), b = 0;
          color = strip.Color(r, g, b);
        } else if (fraction < 0.30) {
          float localFrac = (fraction - 0.08) / 0.22;
          uint8_t r = 255, g = 165 - (uint8_t)(165 * localFrac), b = 0;
          color = strip.Color(r, g, b);
        } else if (fraction < 0.95) {
          color = strip.Color(255, 0, 0);
        } else {
          float localFrac = (fraction - 0.95) / 0.05;
          uint8_t r = 255 - (uint8_t)(204 * localFrac); // 255-204=51
          color = strip.Color(r, 0, 0);
        }
      }
      strip.setPixelColor(i, color);
    }
    
    strip.show();
    delay(10);
  }
  digitalWrite(BUZZER_PIN, LOW);
  strip.clear();
  strip.show();
}

// -----------------------------------------------------------------
// Langer Impuls (Morse-Strich) im Propagation-Stil
// -----------------------------------------------------------------
// Parameter:
//   toneDuration: z. B. 360ms
//   ledCycleDuration: z. B. 640ms
//   globalDuration: z. B. 800ms
void smoothWavePulseLongCustomWithPropagation(int toneDuration, int ledCycleDuration, int globalDuration) {
  const int offsets[LED_COUNT] = {200, 100, 0, 100, 200};
  
  unsigned long startTime = millis();
  while (millis() - startTime < globalDuration) {
    unsigned long t = millis() - startTime;
    
    if (t < toneDuration) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
    
    for (int i = 0; i < LED_COUNT; i++) {
      int effTime = t - offsets[i];
      uint32_t color;
      if (effTime < 0 || effTime > ledCycleDuration) {
        color = strip.Color(0, 0, 0);
      } else {
        float fraction = (float)effTime / ledCycleDuration;
        if (fraction < 0.03) {
          float localFrac = fraction / 0.03;
          uint8_t r = 255, g = 255, b = 255 - (uint8_t)(255 * localFrac);
          color = strip.Color(r, g, b);
        } else if (fraction < 0.08) {
          float localFrac = (fraction - 0.03) / 0.05;
          uint8_t r = 255, g = 255 - (uint8_t)((255 - 165) * localFrac), b = 0;
          color = strip.Color(r, g, b);
        } else if (fraction < 0.30) {
          float localFrac = (fraction - 0.08) / 0.22;
          uint8_t r = 255, g = 165 - (uint8_t)(165 * localFrac), b = 0;
          color = strip.Color(r, g, b);
        } else if (fraction < 0.95) {
          color = strip.Color(255, 0, 0);
        } else {
          float localFrac = (fraction - 0.95) / 0.05;
          uint8_t r = 255 - (uint8_t)(204 * localFrac);
          color = strip.Color(r, 0, 0);
        }
      }
      strip.setPixelColor(i, color);
    }
    
    strip.show();
    delay(10);
  }
  digitalWrite(BUZZER_PIN, LOW);
  strip.clear();
  strip.show();
}

void setup() {
  Serial.begin(115200);
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
  
  if (R2 < Schwellwert && !triggered) {
    triggered = true;
    Serial.println(R2);
    // Drei kurze Impulse (Morse-Punkte) im Propagation-Stil:
    // Jetzt: toneDuration = 80ms, ledCycleDuration = 120ms, globalDuration = 160ms
    for (int cycle = 0; cycle < 3; cycle++) {
      smoothWavePulseShortCustomWithPropagation(80, 120, 160);
      delay(20);
    }
    // Langer Impuls (Morse-Strich) im Propagation-Stil:
    // Jetzt: toneDuration = 360ms, ledCycleDuration = 640ms, globalDuration = 800ms
    smoothWavePulseLongCustomWithPropagation(360, 640, 800);
  } 
  else if (R2 >= Schwellwert) {
    triggered = false;
  }
}
