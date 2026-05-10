#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// CardioIA Conectada - firmware do dispositivo vestivel simulado.
// Captura temperatura, umidade, batimentos cardiacos simulados e movimento.
// Mantem fila circular de resiliencia offline e publica via MQTT.

#define DHT_PIN 15
#define DHT_TYPE DHT22
#define PULSE_BUTTON_PIN 18
#define FORCE_OFFLINE_SWITCH_PIN 19
#define ALERT_LED_PIN 2

const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";
const int WIFI_CHANNEL = 6;

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
// Limiar empirico em m/s^2 para detectar variacao na magnitude do acelerometro.
// Ajusta a sensibilidade do "estou em movimento" usado no campo movement do JSON.
const float MOVEMENT_DELTA_THRESHOLD = 0.4f;

struct VitalSample {
  unsigned long timestamp;
  float temperature;
  float humidity;
  int bpm;
  int movement;
  float accelMagnitude;
  bool alert;
};

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_MPU6050 mpu;
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
bool mpuReady = false;
float lastAccelMagnitude = 9.8f;

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
  // Mantem o mesmo schema usado pelo dashboard Node-RED e pelo cliente REST do Ir Alem 1.
  String payload = "{";
  payload += "\"deviceId\":\"cardioia-esp32-grupo57\",";
  payload += "\"timestamp\":" + String(sample.timestamp) + ",";
  payload += "\"temperature\":" + String(sample.temperature, 1) + ",";
  payload += "\"humidity\":" + String(sample.humidity, 1) + ",";
  payload += "\"bpm\":" + String(sample.bpm) + ",";
  payload += "\"movement\":" + String(sample.movement) + ",";
  payload += "\"accelMagnitude\":" + String(sample.accelMagnitude, 2) + ",";
  payload += "\"alert\":" + String(sample.alert ? "true" : "false");
  payload += "}";
  return payload;
}

bool isConnectivityEnabled() {
  return digitalRead(FORCE_OFFLINE_SWITCH_PIN) == HIGH;
}

String mqttStateDescription(int state) {
  switch (state) {
    case MQTT_CONNECTION_TIMEOUT:
      return "timeout";
    case MQTT_CONNECTION_LOST:
      return "conexao perdida";
    case MQTT_CONNECT_FAILED:
      return "falha TCP";
    case MQTT_DISCONNECTED:
      return "desconectado";
    case MQTT_CONNECTED:
      return "conectado";
    case MQTT_CONNECT_BAD_PROTOCOL:
      return "protocolo recusado";
    case MQTT_CONNECT_BAD_CLIENT_ID:
      return "clientId recusado";
    case MQTT_CONNECT_UNAVAILABLE:
      return "broker indisponivel";
    case MQTT_CONNECT_BAD_CREDENTIALS:
      return "credenciais invalidas";
    case MQTT_CONNECT_UNAUTHORIZED:
      return "nao autorizado";
    default:
      return "estado " + String(state);
  }
}

void printWifiStatus() {
  wl_status_t status = WiFi.status();
  Serial.print("Status Wi-Fi=");
  Serial.print(status);
  Serial.print(" chave=");
  Serial.println(isConnectivityEnabled() ? "ONLINE" : "OFFLINE");
}

void ensureWifi() {
  if (!isConnectivityEnabled()) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Chave OFFLINE acionada. Desconectando Wi-Fi.");
      WiFi.disconnect(true);
    }
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Conectando ao Wi-Fi simulado...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 15000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi conectado. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi indisponivel. Coleta seguira em modo offline.");
    printWifiStatus();
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
  Serial.print("Conectando ao broker MQTT ");
  Serial.print(MQTT_SERVER);
  Serial.print(":");
  Serial.print(MQTT_PORT);
  Serial.print(" topico=");
  Serial.println(MQTT_TOPIC);

  bool connected;
  if (String(MQTT_USER).length() > 0) {
    connected = mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
  } else {
    connected = mqtt.connect(clientId.c_str());
  }

  if (connected) {
    Serial.println("MQTT conectado.");
  } else {
    Serial.print("Falha ao conectar MQTT. Estado=");
    Serial.print(mqtt.state());
    Serial.print(" (");
    Serial.print(mqttStateDescription(mqtt.state()));
    Serial.println(")");
  }
}

bool publishSample(const VitalSample &sample) {
  if (!isConnectivityEnabled()) {
    Serial.println("Publicacao bloqueada: chave em OFFLINE.");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Publicacao bloqueada: Wi-Fi nao conectado. Status=");
    Serial.println(WiFi.status());
    return false;
  }

  if (!mqtt.connected()) {
    Serial.print("Publicacao bloqueada: MQTT nao conectado. Estado=");
    Serial.print(mqtt.state());
    Serial.print(" (");
    Serial.print(mqttStateDescription(mqtt.state()));
    Serial.println(")");
    return false;
  }

  String payload = sampleToJson(sample);
  bool published = mqtt.publish(MQTT_TOPIC, payload.c_str(), true);

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

// Le o acelerometro e estima movimento como variacao da magnitude do vetor de aceleracao.
// Em repouso a magnitude e proxima de 9.8 m/s^2 (gravidade). Movimentos do paciente,
// ou o usuario movendo o slider do MPU6050 no Wokwi, fazem essa magnitude variar.
void readMovement(int &movement, float &magnitudeOut) {
  movement = 0;
  magnitudeOut = lastAccelMagnitude;

  if (!mpuReady) {
    return;
  }

  sensors_event_t accelEvent;
  sensors_event_t gyroEvent;
  sensors_event_t tempEvent;
  if (!mpu.getEvent(&accelEvent, &gyroEvent, &tempEvent)) {
    return;
  }

  float ax = accelEvent.acceleration.x;
  float ay = accelEvent.acceleration.y;
  float az = accelEvent.acceleration.z;
  float magnitude = sqrtf(ax * ax + ay * ay + az * az);
  float delta = fabsf(magnitude - lastAccelMagnitude);

  movement = (delta > MOVEMENT_DELTA_THRESHOLD) ? 1 : 0;
  magnitudeOut = magnitude;
  lastAccelMagnitude = magnitude;
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

  int movement = 0;
  float accelMagnitude = lastAccelMagnitude;
  readMovement(movement, accelMagnitude);

  // Regras locais de alerta: febre, taquicardia ou sinal de queda/ausencia prolongada
  // de movimento podem disparar o LED na borda, antes mesmo do dashboard receber.
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
    movement,
    accelMagnitude,
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

  // Inicializa o barramento I2C padrao do ESP32 (SDA=21, SCL=22) usado pelo MPU6050.
  Wire.begin();
  mpuReady = mpu.begin();
  if (mpuReady) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 inicializado.");
  } else {
    Serial.println("MPU6050 nao detectado. Coleta seguira sem movimento.");
  }

  randomSeed(analogRead(0));
  pulseWindowStartedAt = millis();

  Serial.println("CardioIA Conectada - ESP32 iniciado.");
  Serial.println("Chave em ONLINE: conecta Wi-Fi/MQTT. Chave em OFFLINE: guarda na fila local.");
  Serial.print("Capacidade da fila offline: ");
  Serial.print(MAX_OFFLINE_SAMPLES);
  Serial.println(" amostras.");
  Serial.print("Estado inicial da chave (D19): ");
  Serial.println(digitalRead(FORCE_OFFLINE_SWITCH_PIN) == HIGH ? "HIGH (ONLINE)" : "LOW (OFFLINE)");
  printWifiStatus();
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
