const mqtt = require(process.env.USERPROFILE + "/.node-red/node_modules/mqtt");

const brokerUrl = process.env.MQTT_URL || "mqtt://broker.hivemq.com:1883";
const topic = process.env.MQTT_TOPIC || "fiap/cardioia/grupo59/vitals";

const client = mqtt.connect(brokerUrl, {
  clientId: "cardioia-terminal-monitor-" + Math.random().toString(16).slice(2),
});

client.on("connect", () => {
  console.log(`[MQTT] conectado em ${brokerUrl}`);
  client.subscribe(topic, (error) => {
    if (error) {
      console.error("[MQTT] erro ao assinar topico:", error.message);
      client.end(true);
    } else {
      console.log(`[MQTT] assinatura ativa: ${topic}`);
    }
  });
});

client.on("message", (receivedTopic, payload) => {
  const text = payload.toString();
  console.log(`\n[${new Date().toISOString()}] ${receivedTopic}`);
  try {
    console.log(JSON.stringify(JSON.parse(text), null, 2));
  } catch {
    console.log(text);
  }
});

client.on("error", (error) => {
  console.error("[MQTT] erro:", error.message);
});

process.on("SIGINT", () => {
  console.log("\n[MQTT] encerrando monitor");
  client.end(true, () => process.exit(0));
});
