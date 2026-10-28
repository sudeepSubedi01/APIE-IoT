#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
//#include "secrets.h"

// ============================================================
// CONFIGURATION
// ============================================================
const char* ssid     = "APIE-Proxmox";
const char* password = "apiecampnet";

const char* aws_endpoint = "a38wosdsp9k3ir-ats.iot.us-east-1.amazonaws.com";
const int   aws_port     = 8883;
const char* client_id    = "esp32-deomo-apie11";
const char* pub_topic    = "esp32/sensor/data";
const char* sub_topic    = "esp32/commands";


#define ROOT_CA R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF"

#define DEVICE_CERT R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUVrvWnBKigzV126wjXgm3n8SW6oEwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MTAyNzA1MDM0
MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKcsegM0V4w4LaNLL0Sl
yyGKd6XZF/7H0uZKnT7r/2+cIBlgpmsHV5g7NKx1ZZJjA5ioEG+5hkWXVQ1oaR2K
zWfE+Ihrml7dEad+/Q3Cpjpo9Fyu1q7mnAiEh/GKpzmDbqvYJynVQ1QUIzqY5GAa
XHa6zDoMleDBVY0ECm5n2VRQaae2y27tJazr6KWHDw2t7QXU4WnERIS51vGDY2gW
ia23t3YJ20Trsb7iWlAeTFHpcQHhVrr5V08exZHDtateZReWCs9+ZFCEiiN5LSxX
k0zOa9Y8hxhdk2SA1qUbYamxD3ESmTp8M0g7uVvYoYoDkvqP9Q7HAj6LT//3R3Mb
4tECAwEAAaNgMF4wHwYDVR0jBBgwFoAURHTOFrm46sXpfQCPi8+2RUePD3cwHQYD
VR0OBBYEFAdPKK0tMCLSlPgSef9foGwrORH9MAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBofosNsYg3/Zz2o9/RWovoCWOK
YBWHIAQzN5IK7sUyGr4vJ9TCviPNYF1rUMNre5Pnx9LnFGifZ5iOnk+juEMo8meU
k1HDH2b7Qaiiv3+Oltwu+jBdIDaA6sb9j9jVuSA6WUqegas9pFrUyEuYtpjcsPqz
NCedjKsgiSXMswAs7VFuj0qhWdpqnP4wQneww6t9MCOo5sonlWQqWsgAjNcTaxsK
Sdi8OwoQNmDzMmqJUrGeAeao6BO9VebZ7AULc37RZzVpJBiY3sOy0C/+kZMqLe+u
vmRTKxikwLCa4QuN1nww8NBiSZHgNVPAlQxTx/Me+MyAIZ1lACnrMB4JWleb
-----END CERTIFICATE-----
)EOF"

#define PRIVATE_KEY R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEApyx6AzRXjDgto0svRKXLIYp3pdkX/sfS5kqdPuv/b5wgGWCm
awdXmDs0rHVlkmMDmKgQb7mGRZdVDWhpHYrNZ8T4iGuaXt0Rp379DcKmOmj0XK7W
ruacCISH8YqnOYNuq9gnKdVDVBQjOpjkYBpcdrrMOgyV4MFVjQQKbmfZVFBpp7bL
bu0lrOvopYcPDa3tBdThacREhLnW8YNjaBaJrbe3dgnbROuxvuJaUB5MUelxAeFW
uvlXTx7FkcO1q15lF5YKz35kUISKI3ktLFeTTM5r1jyHGF2TZIDWpRthqbEPcRKZ
OnwzSDu5W9ihigOS+o/1DscCPotP//dHcxvi0QIDAQABAoIBABoH/fPTybIm6Uxd
H74jxu6xHhJwLKFawkdj5PpwSzU47XKmLyPJX+VvirM0EJ3r1xqFSRHspLDfcLoG
wR99hqCa9kxf3Qjaia44QvVKkxHTWAG6o+lWNYPKB1UgTwvQoIIXxXlpIPPNsvKj
7iPc1vsR+U0suoXl9QlhULKCb0AGiQwiFC4i93Fe9RC97wKk2heB7DrjvBNUER+7
1OT4ZhBtL6wicQ0yzzxhRwxLeDT1gxkl7yvX7tBzowG+ZJTKG7tM2wLLbbEeFcdl
3yRFMjG1YvcJBs5bu2o3YxBjSYUheajQpc+Z4o1ZB2QU/Q/0/Ne9lkAe+kK/Q6JS
hu46f7ECgYEA015UeL3E8FhbCR/YUfrK1LdQhKAmf6ZHcPFT1c6oeRAe+309deCH
nL8+ZpqM659JK5Ozbe6+Xxg+7UK88abznbj7oOa+5pww2y398yHevFrbiQUaOw88
eYfjrgnH+Epwv/+mrDzMO37BUoClZBI50iwkFNJhFqXqPwY/GdPv2h0CgYEAynkr
U8Pq4l0CgCdcNnL+eFna/XdpQ/OkCCKHfOwa7g1FpdaUH8Neeh1N0mf29l+mQ+if
PR3U539izl19qflDATtrEbD/GKU0rhgIfv+AR0gNtfpmdX4kCq+qS+h0pwwka5T8
R0scDvxJipNYVprulYPhgNEtCpWBX5rEEABrLUUCgYBKd1RJSYpFpstD1q1OjI3D
DWZXo1iM5lcLCmqfy/P82toffgsWKWl9pr7kRRTauj58FMhFNbRqKk+ZJUcN1+TC
L3avRDwDYQmxXm22p7siHtxzVJHeNsr2ghKLsTR7XDaG3hmpWq4VU93mJB6gt+8Z
BaYck9XStddmJtw6TDmXXQKBgQDA8tOdUdjmuugFJH01zkQ+gLK2VTNVdDtfHTis
jXvkW3WB4ojp9A52pLJiznyEsQYLNQV7ApC/w59vNZzpfAakh/EyvM98twMK3RBL
pkSiqUGXp/Y4BWtnW5wYFoAEKQgUFbiMYEFu7nFOw7etYJv4k+M0FSKtMWhGqYAq
F5wLWQKBgQCtsEtfv7qLG04IwSwT1MdUflWCWfRw37uY/exEVFr4PNk+wq6aEe1c
9LnM7+ncdB/EOB4qKUEqYC6fWPUMi3bO70E8PicSgGNZwF79g4S1eIMgBovC59Kx
OvUWkTUapJTJDY7MBQ8lCmdH9Q5bTNKmM7/dEpqJDJOyH8EMAdCLJA==
-----END RSA PRIVATE KEY-----
)EOF"

// ============================================================
// DHT SENSOR CONFIGURATION
// ============================================================
#define DHTPIN 27       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// ============================================================
// CERTIFICATES (loaded from secrets.h)
// ============================================================
const char* root_ca     = ROOT_CA;
const char* device_cert = DEVICE_CERT;
const char* private_key = PRIVATE_KEY;

// ============================================================
// GLOBAL OBJECTS
// ============================================================
WiFiClientSecure net;
PubSubClient     client(net);

unsigned long lastPublish = 0;
const long    interval    = 5000;  // publish every 5 seconds
int           msgCount    = 0;

// ============================================================
// CALLBACK — Invoked when a message arrives from AWS
// ============================================================
void messageCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[SUBSCRIBE] Message on topic: ");
  Serial.println(topic);

  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.print("Payload: ");
  Serial.println(msg);
}

// ============================================================
// WIFI CONNECTION
// ============================================================
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.enableIPv6();
  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++retry > 30) {
      Serial.println("\nFailed to connect. Restarting...");
      ESP.restart();
    }
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IPv4 address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
  Serial.print("IPv6 link-local address: ");
  Serial.println(WiFi.linkLocalIPv6());
  
  delay(1000);
  Serial.print("IPv6 global address: ");
  Serial.println(WiFi.globalIPv6());
}

// ============================================================
// AWS IoT CONNECTION
// ============================================================
void connectAWS() {
  net.setCACert(root_ca);
  net.setCertificate(device_cert);
  net.setPrivateKey(private_key);

  client.setServer(aws_endpoint, aws_port);
  client.setCallback(messageCallback);
  client.setBufferSize(512);

  Serial.print("Connecting to AWS IoT Core");
  while (!client.connected()) {
    Serial.print(".");
    if (client.connect(client_id)) {
      Serial.println("\nConnected to AWS IoT Core!");
      client.subscribe(sub_topic);
      Serial.print("Subscribed to topic: ");
      Serial.println(sub_topic);
    } else {
      Serial.print("\nFailed. State: ");
      Serial.print(client.state());
      Serial.println(" — Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// ============================================================
// PUBLISH DHT SENSOR DATA
// ============================================================
void publishSensorData() {
  StaticJsonDocument<200> doc;

  float humidity = dht.readHumidity();              // Read humidity
  float temperature = dht.readTemperature();        // Read temperature in Celsius
  float fahrenheit = dht.readTemperature(true);     // Read temperature in Fahrenheit

  doc["device_id"]               = client_id;
  doc["humidity"]                = humidity;
  doc["temperature_celsius"]     = temperature;
  doc["temperature_fahrenheit"]  = fahrenheit;
  doc["msg_count"]               = ++msgCount;
  doc["uptime_ms"]               = millis();

  char buffer[200];
  serializeJson(doc, buffer);

  if (client.publish(pub_topic, buffer)) {
    Serial.print("[PUBLISH] ");
    Serial.print(humidity);
    Serial.print(" | Temperature: ");
    Serial.print(temperature);
    Serial.print("°C | ");
    Serial.print(fahrenheit);
    Serial.print("°F | ");
    Serial.println(buffer);
  } else {
    Serial.println("[ERROR] Publish failed!");
  }
}

// ============================================================
// SETUP & LOOP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== ESP32 AWS IoT DHT Demo ===");

  dht.begin();

  connectWiFi();
  connectAWS();
}

void loop() {
  // Reconnect if disconnected
  if (!client.connected()) {
    Serial.println("Connection lost. Reconnecting...");
    connectAWS();
  }

  client.loop();

  // Publish data every 'interval' ms
  unsigned long now = millis();
  if (now - lastPublish >= interval) {
    lastPublish = now;
    publishSensorData();
  }
}