#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHT_PIN 15
#define DHT_TYPE DHT22
#define PULSE_BUTTON_PIN 18
#define FORCE_OFFLINE_SWITCH_PIN 19
#define ALERT_LED_PIN 2

const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

// Ajustar para o broker usado na entrega. O broker publico facilita teste local.
const char *MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char *MQTT_USER = "";
const char *MQTT_PASSWORD = "";
const char *MQTT_TOPIC = "fiap/cardioia/grupo57/vitals";

const unsigned long SAMPLE_INTERVAL_MS = 5000;
const unsigned long BPM_WINDOW_MS = 15000;
const int MAX_OFFLINE_SAMPLES = 120;
const float TEMP_ALERT_C = 38.0;
const int BPM_ALERT = 120;

struct VitalSample {
  unsigned long timestamp;
  float temperature;
  float humidity;
  int bpm;
  bool alert;
};

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

VitalSample offlineQueue[MAX_OFFLINE_SAMPLES];
int queueStart = 0;
int queueCount = 0;

unsigned long lastSampleAt = 0;
unsigned long lastHeartbeatAt = 0;
unsigned long pulseWindowStartedAt = 0;
int pulseCount = 0;
int lastPulseButtonState = HIGH;
float lastLoggedTemperature = -999;

void enqueueSample(const VitalSample &sample) {
  int index = (queueStart + queueCount) % MAX_OFFLINE_SAMPLES;

  if (queueCount == MAX_OFFLINE_SAMPLES) {
    // Fila circular: em caso de longa queda de rede, descarta a amostra mais antiga.
    queueStart = (queueStart + 1) % MAX_OFFLINE_SAMPLES;
    index = (queueStart + queueCount - 1) % MAX_OFFLINE_SAMPLES;
  } else {
    queueCount++;
  }

  offlineQueue[index] = sample;
}

bool dequeueSample(VitalSample &sample) {
  if (queueCount == 0) {
    return false;
  }

  sample = offlineQueue[queueStart];
  queueStart = (queueStart + 1) % MAX_OFFLINE_SAMPLES;
  queueCount--;
  return true;
}

String sampleToJson(const VitalSample &sample) {
  String payload = "{";
  payload += "\"deviceId\":\"cardioia-esp32-grupo57\",";
  payload += "\"timestamp\":" + String(sample.timestamp) + ",";
  payload += "\"temperature\":" + String(sample.temperature, 1) + ",";
  payload += "\"humidity\":" + String(sample.humidity, 1) + ",";
  payload += "\"bpm\":" + String(sample.bpm) + ",";
  payload += "\"alert\":" + String(sample.alert ? "true" : "false");
  payload += "}";
  return payload;
}

bool isConnectivityEnabled() {
  return digitalRead(FORCE_OFFLINE_SWITCH_PIN) == HIGH;
}

void ensureWifi() {
  if (!isConnectivityEnabled()) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.disconnect(true);
    }
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Conectando ao Wi-Fi simulado...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 8000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi conectado. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi indisponivel. Coleta seguira em modo offline.");
  }
}

void ensureMqtt() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  if (mqtt.connected()) {
    return;
  }

  String clientId = "cardioia-grupo57-" + String(random(0xffff), HEX);
  Serial.println("Conectando ao broker MQTT...");

  bool connected;
  if (String(MQTT_USER).length() > 0) {
    connected = mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
  } else {
    connected = mqtt.connect(clientId.c_str());
  }

  Serial.println(connected ? "MQTT conectado." : "Falha ao conectar MQTT.");
}

bool publishSample(const VitalSample &sample) {
  if (!isConnectivityEnabled() || WiFi.status() != WL_CONNECTED || !mqtt.connected()) {
    return false;
  }

  String payload = sampleToJson(sample);
  bool published = mqtt.publish(MQTT_TOPIC, payload.c_str());

  Serial.print(published ? "MQTT publicado: " : "Falha MQTT, mantendo local: ");
  Serial.println(payload);
  return published;
}

void syncOfflineQueue() {
  if (queueCount == 0 || !isConnectivityEnabled()) {
    return;
  }

  VitalSample sample;
  int sent = 0;

  while (queueCount > 0) {
    VitalSample candidate = offlineQueue[queueStart];
    if (!publishSample(candidate)) {
      break;
    }

    dequeueSample(sample);
    sent++;
    delay(80);
  }

  if (sent > 0) {
    Serial.print("Amostras sincronizadas da fila local: ");
    Serial.println(sent);
  }
}

void updatePulseCounter() {
  int state = digitalRead(PULSE_BUTTON_PIN);
  if (lastPulseButtonState == HIGH && state == LOW) {
    pulseCount++;
    Serial.print("PULSO detectado. Contagem na janela: ");
    Serial.println(pulseCount);
    Serial.flush();
  }
  lastPulseButtonState = state;
}

int calculateBpm() {
  unsigned long elapsed = millis() - pulseWindowStartedAt;
  if (elapsed == 0) {
    return 0;
  }

  int bpm = (int)((pulseCount * 60000UL) / elapsed);

  if (elapsed >= BPM_WINDOW_MS) {
    pulseCount = 0;
    pulseWindowStartedAt = millis();
  }

  return bpm;
}

VitalSample collectSample() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int bpm = calculateBpm();

  if (isnan(temperature)) {
    temperature = -1;
  }
  if (isnan(humidity)) {
    humidity = -1;
  }

  bool alert = temperature > TEMP_ALERT_C || bpm > BPM_ALERT;
  digitalWrite(ALERT_LED_PIN, alert ? HIGH : LOW);

  if (abs(temperature - lastLoggedTemperature) >= 0.1) {
    lastLoggedTemperature = temperature;
    Serial.print("DHT22 temperatura=");
    Serial.print(temperature, 1);
    Serial.print("C umidade=");
    Serial.print(humidity, 1);
    Serial.println("%");
    Serial.flush();
  }

  VitalSample sample = {
    millis(),
    temperature,
    humidity,
    bpm,
    alert
  };

  return sample;
}

void setup() {
  Serial.begin(115200, SERIAL_8N1, 3, 1);
  delay(1500);
  Serial.println();
  Serial.println("BOOT CardioIA - serial ativa");
  Serial.flush();

  pinMode(PULSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FORCE_OFFLINE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(ALERT_LED_PIN, OUTPUT);

  dht.begin();
  randomSeed(analogRead(0));
  pulseWindowStartedAt = millis();

  Serial.println("CardioIA Conectada - ESP32 iniciado.");
  Serial.println("Chave OFFLINE aberta: envia MQTT. Chave OFFLINE fechada: guarda na fila local.");
  Serial.print("Capacidade da fila offline: ");
  Serial.print(MAX_OFFLINE_SAMPLES);
  Serial.println(" amostras.");
}

void loop() {
  if (millis() - lastHeartbeatAt >= 2000) {
    lastHeartbeatAt = millis();
    Serial.print("HB millis=");
    Serial.print(millis());
    Serial.print(" wifi=");
    Serial.print(WiFi.status());
    Serial.print(" mqtt=");
    Serial.print(mqtt.connected() ? "1" : "0");
    Serial.print(" online=");
    Serial.print(isConnectivityEnabled() ? "1" : "0");
    Serial.print(" fila=");
    Serial.println(queueCount);
    Serial.flush();
  }

  updatePulseCounter();
  ensureWifi();
  ensureMqtt();
  mqtt.loop();

  if (millis() - lastSampleAt >= SAMPLE_INTERVAL_MS) {
    lastSampleAt = millis();
    VitalSample sample = collectSample();
    String payload = sampleToJson(sample);

    Serial.print("Leitura local: ");
    Serial.println(payload);

    if (!publishSample(sample)) {
      enqueueSample(sample);
      Serial.print("Modo offline/resiliente. Amostras na fila: ");
      Serial.println(queueCount);
    }
  }

  syncOfflineQueue();
  delay(20);
}
