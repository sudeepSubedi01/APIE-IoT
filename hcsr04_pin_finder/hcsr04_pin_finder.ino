// HC-SR04 pin finder (works for any Bluino ESP32 board layout)
// Tries every Trig/Echo combination on the usable ESP32 GPIOs
// and prints the pairs that get an echo back.
// Open Serial Monitor at 115200 and hold a flat object ~20 cm in front of the sensor.
// One full scan takes ~20 seconds.
//
// Skipped on purpose: 1, 3 (USB serial), 6-11 (flash), 21, 22 (I2C / OLED).

int trigCandidates[] = {2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 23, 25, 26, 27, 32, 33};
int echoCandidates[] = {2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 23, 25, 26, 27, 32, 33,
                       34, 35, 36, 39};   // 34-39 are input-only, so Echo only

// int trigCandidates[] = {4};
// int echoCandidates[] = {39};

long tryPair(int trig, int echo) {
  pinMode(echo, INPUT_PULLDOWN);           // stop a floating pin from reading noise
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 25000UL);
  pinMode(trig, INPUT);                    // release the pin again
  return duration;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  Serial.println("---- scanning ----");
  bool found = false;
  for (int t : trigCandidates) {
    for (int e : echoCandidates) {
      if (t == e) continue;
      long d = tryPair(t, e);
      if (d > 116) {                       // < 116 µs (< 2 cm) is noise, not a real echo
        Serial.printf("ECHO FOUND: TRIG=%d  ECHO=%d  -> %.1f cm\n", t, e, d * 0.0343 / 2.0);
        found = true;
      }
      delay(30);                           // let echoes die down between tries
    }
  }
  if (!found) Serial.println("No pair responded - check wiring, 5V, and jumpers");
  delay(2000);
}
