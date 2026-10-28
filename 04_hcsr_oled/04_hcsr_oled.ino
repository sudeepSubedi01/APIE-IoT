// HC-SR04 distance on 128x64 I2C OLED (SSD1306)
// Board: Bluino ESP32 IoT Starter Kit
// Libraries (Library Manager): "Adafruit SSD1306" and "Adafruit GFX Library"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C      // most 0.96" OLEDs use 0x3C

// Set these to the GPIOs the board's ultrasonic header (Ec / Tr) connects to.
// TRIG must be an output-capable pin (not 34, 35, 36 or 39). ECHO can be any input pin.
#define TRIG_PIN 5
#define ECHO_PIN 18

#define MAX_DISTANCE_CM 200  // full scale for the bar graph

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);      // 10 µs trigger pulse
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Echo pulse width in µs; 30 ms timeout ≈ 5 m max range
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  // 0 = no echo; < 116 µs (< 2 cm, below the sensor's range) = noise on the pin
  if (duration < 116) return -1;

  return duration * 0.0343 / 2.0;    // speed of sound 343 m/s, there and back
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT_PULLDOWN); // keeps the pin LOW if nothing drives it (not available on 34-39)
  digitalWrite(TRIG_PIN, LOW);

  Wire.begin(21, 22);                // SDA, SCL
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found - check address (0x3C / 0x3D) and wiring");
    while (true) delay(1000);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

void loop() {
  float d = readDistanceCm();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("HC-SR04 Distance");

  if (d < 0) {
    Serial.println("Out of range");
    display.setTextSize(2);
    display.setCursor(0, 24);
    display.print("No echo");
  } else {
    Serial.printf("Distance: %.1f cm\n", d);

    display.setTextSize(3);
    display.setCursor(0, 18);
    display.print(d, 1);

    display.setTextSize(2);
    display.setCursor(100, 24);
    display.print("cm");

    // Bar graph along the bottom
    int barWidth = (int)(min(d, (float)MAX_DISTANCE_CM) / MAX_DISTANCE_CM * 124);
    display.drawRect(0, 52, 128, 12, SSD1306_WHITE);
    display.fillRect(2, 54, barWidth, 8, SSD1306_WHITE);
  }

  display.display();
  delay(200);
}
