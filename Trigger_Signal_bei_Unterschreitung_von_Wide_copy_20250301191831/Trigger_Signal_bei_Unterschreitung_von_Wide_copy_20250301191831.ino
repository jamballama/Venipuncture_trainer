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
  
  // Triggern, wenn der Widerstand unter dem Schwellwert liegt
  if (R2 < Schwellwert && !triggered) {
    triggered = true;
    
    // Wiederhole das Muster 3-mal
    for (int cycle = 0; cycle < 3; cycle++) {
      // Schritt 1: Mittlere LED (Index 2) an
      digitalWrite(BUZZER_PIN, HIGH); // Buzzer an (einfach 5V an)
      setLEDsCenter();
      delay(50);
      clearLEDs();
      
      // Schritt 2: LEDs neben der Mitte (Indices 1 und 3) an
      digitalWrite(BUZZER_PIN, HIGH);
      setLEDsMiddle();
      delay(50);
      clearLEDs();
      
      // Schritt 3: Äußerste LEDs (Indices 0 und 4) an
      digitalWrite(BUZZER_PIN, HIGH);
      setLEDsOuter();
      delay(50);
      clearLEDs();
      
      digitalWrite(BUZZER_PIN, LOW); // Kurze Pause zwischen den Zyklen
      delay(50);
    }
  }
  // Reset: Sobald R2 wieder über dem Schwellwert liegt, darf neu getriggert werden
  else if (R2 >= Schwellwert) {
    triggered = false;
  }
}

// Schaltet die mittlere LED (Index 2) ein
void setLEDsCenter() {
  strip.clear();
  strip.setPixelColor(2, strip.Color(255, 0, 0));
  strip.show();
}

// Schaltet die LEDs neben der Mitte ein (Indices 1 und 3)
void setLEDsMiddle() {
  strip.clear();
  if (LED_COUNT > 1) strip.setPixelColor(1, strip.Color(255, 0, 0));
  if (LED_COUNT > 3) strip.setPixelColor(3, strip.Color(255, 0, 0));
  strip.show();
}

// Schaltet die äußersten LEDs ein (Indices 0 und 4)
void setLEDsOuter() {
  strip.clear();
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.setPixelColor(LED_COUNT - 1, strip.Color(255, 0, 0));
  strip.show();
}

void clearLEDs() {
  strip.clear();
  strip.show();
}
