#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* MQTT_SERVER = "host.wokwi.internal";
const int MQTT_PORT = 1883;

const char* MQTT_TOPIC = "iot/esp32/esp32_01/sensor";

const int DHT_PIN = 15;
const unsigned long SEND_INTERVAL_MS = 5000;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
DHTesp dht;

unsigned long lastSend = 0;
unsigned long sequenceNo = 0;

void connectWiFi() {
  Serial.print("Connecting WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
}

void connectMQTT() {
  while (!mqttClient.connected()) {

    Serial.print("Connecting MQTT... ");

    String clientId =
      "ESP32-" +
      String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  delay(1000);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  connectWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  Serial.println("Setup completed!");
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();

  unsigned long now = millis();

  if (now - lastSend < SEND_INTERVAL_MS) {
    return;
  }

  lastSend = now;

  TempAndHumidity data = dht.getTempAndHumidity();

  if (!isfinite(data.temperature) ||
      !isfinite(data.humidity)) {

    Serial.println("Invalid DHT22 data!");
    return;
  }

  sequenceNo++;

  char payload[256];

  snprintf(
    payload,
    sizeof(payload),
    "{\"device_id\":\"esp32_01\","
    "\"temperature\":%.2f,"
    "\"humidity\":%.2f,"
    "\"sequence\":%lu,"
    "\"uptime_s\":%lu}",

    data.temperature,
    data.humidity,
    sequenceNo,
    millis() / 1000
  );

  bool ok = mqttClient.publish(
    MQTT_TOPIC,
    payload
  );

  Serial.print("JSON: ");
  Serial.println(payload);

  Serial.print("Publish: ");

  if (ok) {
    Serial.println("OK");
  } else {
    Serial.println("FAILED");
  }
}