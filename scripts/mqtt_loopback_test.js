const mqtt = require(process.env.USERPROFILE + "/.node-red/node_modules/mqtt");

const brokerUrl = process.env.MQTT_URL || "mqtt://broker.hivemq.com:1883";
const topic = process.env.MQTT_TOPIC || "fiap/cardioia/grupo57/vitals";
const payload = {
  deviceId: "teste-loopback",
  timestamp: Date.now(),
  temperature: 39.4,
  humidity: 40,
  bpm: 88,
  alert: true,
};

const sub = mqtt.connect(brokerUrl, {
  clientId: "cardioia-loopback-sub-" + Math.random().toString(16).slice(2),
  connectTimeout: 10000,
  reconnectPeriod: 0,
});

const pub = mqtt.connect(brokerUrl, {
  clientId: "cardioia-loopback-pub-" + Math.random().toString(16).slice(2),
  connectTimeout: 10000,
  reconnectPeriod: 0,
});

let received = false;
let publisherReady = false;
let subscribed = false;

function maybePublish() {
  if (!publisherReady || !subscribed) return;
  pub.publish(topic, JSON.stringify(payload), { qos: 0 }, (error) => {
    if (error) {
      console.error("[MQTT] erro ao publicar:", error.message);
      finish(1);
    } else {
      console.log("[MQTT] publicado apos assinatura ativa");
    }
  });
}

function finish(code) {
  sub.end(true);
  pub.end(true);
  setTimeout(() => process.exit(code), 100);
}

sub.on("connect", () => {
  console.log(`[MQTT] subscriber conectado em ${brokerUrl}`);
  sub.subscribe(topic, { qos: 0 }, (error) => {
    if (error) {
      console.error("[MQTT] erro ao assinar:", error.message);
      finish(1);
      return;
    }
    subscribed = true;
    console.log(`[MQTT] assinatura ativa: ${topic}`);
    maybePublish();
  });
});

pub.on("connect", () => {
  publisherReady = true;
  console.log(`[MQTT] publisher conectado em ${brokerUrl}`);
  maybePublish();
});

sub.on("message", (receivedTopic, message) => {
  received = true;
  console.log(`[MQTT] recebido em ${receivedTopic}`);
  console.log(message.toString());
  finish(0);
});

sub.on("error", (error) => {
  console.error("[MQTT] erro subscriber:", error.message);
  finish(1);
});

pub.on("error", (error) => {
  console.error("[MQTT] erro publisher:", error.message);
  finish(1);
});

setTimeout(() => {
  if (!received) {
    console.error("[MQTT] timeout: nenhuma mensagem retornou do broker");
    finish(1);
  }
}, 15000);
