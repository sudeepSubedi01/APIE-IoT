#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "APIE-Proxmox"; // Enter your Wi-Fi name
const char *password = "apiecampnet";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic_sub = "apie_camp/iot/led";
const char *mqtt_username = "sushant";
const char *mqtt_password = "sushan";
const int mqtt_port = 1883;

unsigned long lastTime = 0;
unsigned long intervalTime = 10000;

const int ledPin = 2;

WiFiClient espClient;
PubSubClient client(espClient);

void connectToMqttBroker(){
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Public EMQX MQTT broker connected");
    } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  String sMsg;
  int iValue;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    sMsg += (char) payload[i];
  }
  Serial.println(sMsg);
  Serial.println("-----------------------");

  iValue = sMsg.toInt();
  if(iValue>0){
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }
}

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  // Connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the Wi-Fi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  connectToMqttBroker();

  if (client.subscribe(topic_sub)) {
    Serial.printf("ESP32 subscribed to topic: %s", topic_sub);
  }
  else {
    Serial.println("Failed to subscribe!!!");
  }
}

void loop() {
  client.loop();

  if ((millis() - lastTime) > intervalTime) {
    if (!client.connected()) {
      connectToMqttBroker();
    }
  }  
}