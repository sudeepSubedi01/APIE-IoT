#include <DHT.h>
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() { Serial.begin(115200); dht.begin(); }

void loop() {
  delay(2000);
  float t = dht.readTemperature(), h = dht.readHumidity();
  if (isnan(t) || isnan(h)) { Serial.println("Read failed"); return; }
  Serial.printf("Temp: %.1f C  Humidity: %.1f %%\n", t, h);
}