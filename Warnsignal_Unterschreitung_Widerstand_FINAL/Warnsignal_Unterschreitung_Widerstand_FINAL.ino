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
float Schwellwert = 3000.0; // Schwellenwert in Ohm

bool triggered = false;

// -----------------------------------------------------------------
// Hilfsfunktion: Berechnet die LED-Farbe anhand des relativen Zeitanteils (fraction)
// gemäß der gewünschten Sequenz:
// Weiß → Gelb → Orange → Rot (255,0,0) → Rot (51,0,0) → Rot (25,0,0) → Rot (10,0,0) → Schwarz
// fraction liegt im Bereich [0,1]
uint32_t getColorForFraction(float fraction) {
  uint8_t r, g, b;
  if (fraction < 0.02) {
    float localFrac = fraction / 0.02;
    // Weiß (255,255,255) → Gelb (255,255,0)
    r = 255;
    g = 255;
    b = 255 - (uint8_t)(255 * localFrac);
  }
  else if (fraction < 0.04) {
    float localFrac = (fraction - 0.02) / 0.02;
    // Gelb (255,255,0) → Orange (255,165,0)
    r = 255;
    g = 255 - (uint8_t)((255 - 165) * localFrac);
    b = 0;
  }
  else if (fraction < 0.06) {
    float localFrac = (fraction - 0.04) / 0.02;
    // Orange (255,165,0) → Rot (255,0,0)
    r = 255;
    g = 165 - (uint8_t)(165 * localFrac);
    b = 0;
  }
  else if (fraction < 0.46) {
    float localFrac = (fraction - 0.06) / 0.40;
    // Rot (255,0,0) → Rot (51,0,0)
    r = 255 - (uint8_t)((255 - 51) * localFrac);
    g = 0;
    b = 0;
  }
  else if (fraction < 0.76) {
    float localFrac = (fraction - 0.46) / 0.30;
    // Rot (51,0,0) → Rot (25,0,0)
    r = 51 - (uint8_t)((51 - 25) * localFrac);
    g = 0;
    b = 0;
  }
  else if (fraction < 0.91) {
    float localFrac = (fraction - 0.76) / 0.15;
    // Rot (25,0,0) → Rot (10,0,0)
    r = 25 - (uint8_t)((25 - 10) * localFrac);
    g = 0;
    b = 0;
  }
  else if (fraction < 0.96) {
    float localFrac = (fraction - 0.91) / 0.05;
    // Rot (10,0,0) → Schwarz (0,0,0)
    r = 10 - (uint8_t)(10 * localFrac);
    g = 0;
    b = 0;
  }
  else {
    r = 0; g = 0; b = 0;
  }
  return strip.Color(r, g, b);
}

// -----------------------------------------------------------------
// Kurze Impulse (Morse-Punkte) im Propagation-Stil
// Parameter:
//   toneDuration: z. B. 80ms
//   ledCycleDuration: z. B. 120ms
//   globalDuration: z. B. 160ms
void smoothWavePulseShortCustomWithPropagation(int toneDuration, int ledCycleDuration, int globalDuration) {
  float scale = (float)globalDuration / 1000.0; // Skalierung der Offsets
  int offsets[LED_COUNT] = { (int)(200 * scale), (int)(100 * scale), 0, (int)(100 * scale), (int)(200 * scale) };
  
  unsigned long startTime = millis();
  while (millis() - startTime < globalDuration) {
    unsigned long t = millis() - startTime;
    
    // Buzzer aktiv während toneDuration
    if (t < toneDuration)
      digitalWrite(BUZZER_PIN, HIGH);
    else
      digitalWrite(BUZZER_PIN, LOW);
    
    // Berechne für jede LED den effektiven Zeitwert (mit Offset)
    for (int i = 0; i < LED_COUNT; i++) {
      int effTime = t - offsets[i];
      uint32_t color;
      if (effTime < 0 || effTime > ledCycleDuration)
        color = strip.Color(0, 0, 0);
      else {
        float fraction = (float)effTime / ledCycleDuration;
        color = getColorForFraction(fraction);
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
// Parameter:
//   toneDuration: z. B. 360ms
//   ledCycleDuration: z. B. 640ms
//   globalDuration: z. B. 800ms
void smoothWavePulseLongCustomWithPropagation(int toneDuration, int ledCycleDuration, int globalDuration) {
  const int offsets[LED_COUNT] = {200, 100, 0, 100, 200}; // Feste Offsets
  unsigned long startTime = millis();
  while (millis() - startTime < globalDuration) {
    unsigned long t = millis() - startTime;
    
    if (t < toneDuration)
      digitalWrite(BUZZER_PIN, HIGH);
    else
      digitalWrite(BUZZER_PIN, LOW);
    
    for (int i = 0; i < LED_COUNT; i++) {
      int effTime = t - offsets[i];
      uint32_t color;
      if (effTime < 0 || effTime > ledCycleDuration)
        color = strip.Color(0, 0, 0);
      else {
        float fraction = (float)effTime / ledCycleDuration;
        color = getColorForFraction(fraction);
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
  
  if (Vout > 0)
    R2 = R1 * ((Vin / Vout) - 1.0);
  
  Serial.println(R2);
  
  if (R2 < Schwellwert && !triggered) {
    triggered = true;
    // Drei kurze Impulse (Punkte) im Propagation-Stil:
    // toneDuration = 80ms, ledCycleDuration = 120ms, globalDuration = 160ms
    for (int cycle = 0; cycle < 3; cycle++) {
      smoothWavePulseShortCustomWithPropagation(80, 120, 160);
      delay(20);
    }
    // Langer Impuls (Strich) im Propagation-Stil:
    // toneDuration = 360ms, ledCycleDuration = 640ms, globalDuration = 800ms
    smoothWavePulseLongCustomWithPropagation(360, 640, 800);
  } 
  else if (R2 >= Schwellwert) {
    triggered = false;
  }
}
