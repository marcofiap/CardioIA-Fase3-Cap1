const mqtt = require(process.env.USERPROFILE + "/.node-red/node_modules/mqtt");

const brokerUrl = process.env.MQTT_URL || "mqtt://broker.hivemq.com:1883";
const topic = process.env.MQTT_TOPIC || "fiap/cardioia/grupo57/vitals";
const payload = {
  deviceId: "teste-terminal",
  timestamp: Date.now(),
  temperature: 39.4,
  humidity: 40,
  bpm: 88,
  alert: true,
};

const client = mqtt.connect(brokerUrl, {
  clientId: "cardioia-terminal-publisher-" + Math.random().toString(16).slice(2),
  connectTimeout: 10000,
  reconnectPeriod: 0,
});

const timeout = setTimeout(() => {
  console.error("[MQTT] timeout ao publicar");
  client.end(true);
}, 15000);

client.on("connect", () => {
  console.log(`[MQTT] conectado em ${brokerUrl}`);
  client.publish(topic, JSON.stringify(payload), { qos: 0 }, (error) => {
    clearTimeout(timeout);
    if (error) {
      console.error("[MQTT] erro ao publicar:", error.message);
    } else {
      console.log(`[MQTT] publicado em ${topic}`);
      console.log(JSON.stringify(payload, null, 2));
    }
    client.end(true);
  });
});

client.on("error", (error) => {
  clearTimeout(timeout);
  console.error("[MQTT] erro:", error.message);
  client.end(true);
});
