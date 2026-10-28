// Ultrasonic Sensor with Buzzer Alert Using ESP32
// No library required. Serial Monitor: 115200 baud.
//
// Wiring:
//   HC-SR04 VCC  -> 5V (VIN)      Buzzer +  -> GPIO 23
//   HC-SR04 GND  -> GND           Buzzer -  -> GND
//   HC-SR04 TRIG -> GPIO 5
//   HC-SR04 ECHO -> GPIO 18
//
// Behaviour (like a car parking sensor):
//   farther than ALERT_DISTANCE  -> silent
//   between the two distances    -> beeps, faster as the object gets closer
//   closer than DANGER_DISTANCE  -> continuous tone

#define TRIG_PIN   5
#define ECHO_PIN   18
#define BUZZER_PIN 13

#define ALERT_DISTANCE  50.0   // cm: start beeping below this
#define DANGER_DISTANCE 10.0   // cm: continuous tone below this

// Set to true if your buzzer is passive (needs a tone signal to make sound).
// Active buzzers (the common type with a sealed top) just need HIGH/LOW.
#define PASSIVE_BUZZER false
#define TONE_FREQ 2000         // Hz, used only for passive buzzers

unsigned long lastMeasure = 0;
unsigned long lastToggle = 0;
bool buzzerOn = false;
float distanceCm = -1;

void buzzer(bool on) {
  if (on == buzzerOn) return;
  buzzerOn = on;
  if (PASSIVE_BUZZER) {
    if (on) tone(BUZZER_PIN, TONE_FREQ);
    else    noTone(BUZZER_PIN);
  } else {
    digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
  }
}

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);        // 10 µs trigger pulse
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);  // timeout ≈ 5 m
  if (duration < 116) return -1;       // no echo, or noise below the 2 cm minimum
  return duration * 0.0343 / 2.0;      // speed of sound 343 m/s, there and back
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  buzzer(false);
  Serial.println("Ultrasonic buzzer alert ready");
}

void loop() {
  unsigned long now = millis();

  // Measure about every 60 ms (the sensor needs time for echoes to fade)
  if (now - lastMeasure >= 60) {
    lastMeasure = now;
    distanceCm = readDistanceCm();

    if (distanceCm < 0) Serial.println("Out of range");
    else                Serial.printf("Distance: %.1f cm\n", distanceCm);
  }

  // Buzzer pattern (non-blocking, so measuring never pauses)
  if (distanceCm < 0 || distanceCm >= ALERT_DISTANCE) {
    buzzer(false);                                   // nothing close: silent
  } else if (distanceCm < DANGER_DISTANCE) {
    buzzer(true);                                    // very close: continuous
  } else {
    // 10 cm -> beep every 80 ms, 50 cm -> every 600 ms
    unsigned long interval = map((long)distanceCm, (long)DANGER_DISTANCE,
                                 (long)ALERT_DISTANCE, 80, 600);
    if (now - lastToggle >= interval) {
      lastToggle = now;
      buzzer(!buzzerOn);
    }
  }
}
