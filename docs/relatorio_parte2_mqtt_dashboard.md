# Relatorio Parte 2 - MQTT, Fog/Cloud e dashboard

## 1. Arquitetura Edge/Fog/Cloud

A Parte 2 conecta o ESP32 simulado a uma camada de visualizacao em tempo real, integrando os tres niveis discutidos no Cap 8 (Cloud Computing como Pilar da IoT Moderna) e Cap 9 (Inteligencia na Borda) da Fase 3.

```text
+-----------------------+      +-------------------------+      +-----------------------+
|     EDGE (ESP32)      |      |   FOG (Node-RED local)  |      |   CLOUD (HiveMQ)      |
|                       |      |                         |      |                       |
|  DHT22, MPU6050,      | MQTT |  fluxo importavel,      | MQTT |  broker TLS opcional, |
|  botao, LED, fila     |----->|  funcao normalize,      |<-----|  retencao, escala     |
|  circular, regras     |      |  dashboard tempo real   |      |  multi-cliente        |
+-----------------------+      +-------------------------+      +-----------------------+
       captura/decisao              filtragem/visual                 transporte/agregacao
```

Cada amostra publicada pelo ESP32 viaja como mensagem MQTT no topico:

```text
fiap/cardioia/grupo57/vitals
```

O Node-RED atua como camada de Fog: consome o topico, normaliza o JSON, separa as variaveis para os widgets do dashboard e tambem encaminha o payload original para um node de debug. O broker, hospedado como servico publico (HiveMQ) ou em conta gerenciada (HiveMQ Cloud), representa a camada Cloud da arquitetura.

## 2. Payload e contrato

Toda mensagem segue o mesmo schema, alinhado com o cliente REST do Ir Alem 1:

```json
{
  "deviceId": "cardioia-esp32-grupo57",
  "timestamp": 123456,
  "temperature": 36.8,
  "humidity": 52.4,
  "bpm": 82,
  "movement": 0,
  "accelMagnitude": 9.81,
  "alert": false
}
```

Campos:

| Campo | Tipo | Origem | Observacao |
|---|---|---|---|
| `deviceId` | string | constante no firmware | identifica o wearable; sem dado pessoal |
| `timestamp` | int (ms) | `millis()` no ESP32 | facilita ordenacao client-side |
| `temperature` | float | DHT22 | Celsius, 1 casa decimal |
| `humidity` | float | DHT22 | percentual |
| `bpm` | int | botao + janela 15s | batimentos por minuto |
| `movement` | int (0/1) | MPU6050 | 1 quando ha variacao da magnitude > 0.4 m/s^2 |
| `accelMagnitude` | float | MPU6050 | magnitude do vetor de aceleracao |
| `alert` | bool | regra de borda | true quando regra de risco e violada |

## 3. Configuracao do broker MQTT

### 3.1 Broker publico (default)

Para entrega academica e demonstracao em sala, o firmware esta configurado em `broker.hivemq.com:1883` sem TLS e sem credenciais. Esse broker permite testes rapidos, publicacao retained e suporta varios clientes simultaneos. As constantes em `src/wokwi/sketch.ino`:

```c++
const char *MQTT_SERVER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;
const char *MQTT_USER   = "";
const char *MQTT_PASSWORD = "";
const char *MQTT_TOPIC = "fiap/cardioia/grupo57/vitals";
```

A publicacao usa `mqtt.publish(MQTT_TOPIC, payload.c_str(), true)` com `retained=true`, garantindo que um novo subscriber receba imediatamente a ultima leitura.

### 3.2 HiveMQ Cloud (recomendado para producao)

Para uma entrega proxima do mercado, basta:

1. criar conta em `https://www.hivemq.com/mqtt-cloud-broker/` e provisionar cluster gratuito;
2. cadastrar credenciais de dispositivo no painel `Access Management`;
3. trocar no firmware:
   - `MQTT_SERVER` para o host do cluster (ex.: `xxxxxxxx.s1.eu.hivemq.cloud`);
   - `MQTT_PORT` para `8883`;
   - `MQTT_USER` e `MQTT_PASSWORD` com as credenciais geradas;
   - usar `WiFiClientSecure` no lugar de `WiFiClient` e carregar o certificado raiz LetsEncrypt para TLS.
4. ajustar o node `mqtt-broker` do Node-RED para o mesmo host e marcar `Use TLS` + `Verify server certificate`.

A escolha entre broker publico e Cloud foi documentada para que a banca avalie ambos os caminhos. O firmware nao depende de TLS para compilar e simular no Wokwi: quando rodar em hardware real, basta alterar o cliente.

### 3.3 QoS e retained

A entrega usa **QoS 0** com **retained=true**. Justificativa:

- QoS 0 e suficiente para um stream continuo de telemetria a cada 5 segundos: a perda eventual de uma amostra nao compromete a tendencia clinica e nao adiciona round trips adicionais.
- Retained permite que um dashboard recem-aberto receba imediatamente a ultima leitura, melhorando a experiencia do operador.
- Para alarmes criticos em producao, a recomendacao seria adotar QoS 1 com last-will message no broker e dedup no consumer.

## 4. Dashboard Node-RED

### 4.1 Arquivo

O fluxo exportado esta em:

```text
src/node-red/flows_cardioia_node_red.json
```

### 4.2 Mapeamento dos nodes

| Node | Tipo | Funcao |
|---|---|---|
| `MQTT CardioIA` | `mqtt in` | assina `fiap/cardioia/grupo57/vitals` no broker configurado |
| `Teste dashboard` | `inject` | dispara um payload mock para validar o dashboard sem o ESP32 |
| `Normalizar, logar e separar` | `function` | converte string/Buffer em objeto, valida campos, gera log e divide em 8 saidas |
| `BPM em tempo real` | `ui_chart` | grafico de linha de batimentos (eixo Y de 40 a 180) |
| `Temperatura` | `ui_gauge` | gauge corporal de 30 a 42 C com bandas verde/amarelo/vermelho |
| `Umidade` | `ui_gauge` | gauge de 0 a 100% com banda saudavel entre 30 e 70 |
| `Magnitude aceleracao` | `ui_gauge` | gauge da magnitude do vetor de aceleracao (m/s^2), banda verde proxima de 9.8 (gravidade) |
| `Texto de alerta` | `ui_text` | mostra `Normal` ou `ALERTA: avaliar paciente` |
| `LED virtual` | `ui_template` | bola colorida vermelha/verde com sombra, refletindo o estado de alerta |
| `Movimento atual` | `ui_text` | mostra `Em movimento` ou `Em repouso` a partir do MPU6050 |
| `Payload recebido` | `debug` | imprime no debug panel e console o JSON original |
| `HiveMQ public` | `mqtt-broker` | configuracao de broker (host, porta, TLS, keepalive) |

### 4.3 Justificativa dos widgets

- **Grafico de BPM**: pedido explicito do enunciado como sinal vital escolhido pelo grupo.
- **Gauge de temperatura**: gauge claro com bandas faceis de interpretar pelo profissional de saude.
- **Gauge de umidade**: o DHT22 fornece um segundo canal (umidade) que merece ser exibido como sinal vital de ambiente; usamos faixa verde entre 30 e 70% como conforto.
- **Gauge de magnitude do acelerometro**: complementa o texto de movimento mostrando quanta variacao instantanea o MPU6050 esta detectando; em repouso fica proximo de 9.8 m/s^2 (gravidade).
- **Texto + LED virtual**: redundancia visual exigida pelo enunciado; um e textual, o outro e visual e tem sombra colorida quando ativo.
- **Texto de movimento**: traduz o `movement` (0/1) em algo legivel para o operador.
- **Debug node** mantido ativo durante a entrega: facil para a banca verificar o payload bruto recebido em `/ui` ou no editor.

### 4.4 Como rodar localmente

```bash
npm install -g node-red
mkdir -p ~/.node-red
npm install --prefix ~/.node-red node-red-dashboard
node-red --userDir ~/.node-red
```

Depois acessar `http://127.0.0.1:1880`, importar o fluxo, conferir o broker e clicar em `Deploy`. O dashboard final fica em `http://127.0.0.1:1880/ui`.

Para diagnosticar caminho MQTT do terminal, o repositorio inclui:

```bash
node scripts/mqtt_monitor.js          # assina o topico e imprime
node scripts/mqtt_publish_test.js     # publica um payload simulado
node scripts/mqtt_loopback_test.js    # publica e assina, fecha o loop
```

## 5. Papel de Fog e Cloud

Nesta entrega, o Node-RED representa a **Fog Computing**: recebe dados proximos da operacao (PC do operador), normaliza, aplica formatacao e entrega visualizacao imediata. Em uma solucao real, esse mesmo Node-RED poderia rodar em um gateway IoT do hospital ou clinica.

O **broker MQTT** e a representacao da camada Cloud para o transporte e a fan-out para multiplos consumidores. Em arquitetura mais ampla (Cap 8), o broker poderia encaminhar para `InfluxDB Cloud` e `Grafana Cloud` com persistencia historica e dashboards gerenciais. A entrega ja esta com o desenho preparado para essa evolucao opcional.

A divisao Edge/Fog/Cloud segue exatamente o material da Fase 3: a borda decide localmente para baixa latencia, a Fog agrega e exibe, a Cloud escala armazenamento e analise historica/IA.

## 6. Evidencias

Sao salvos em `assets/evidencias/`:

- `wokwi_execucao.png` - simulacao rodando no Wokwi com Monitor Serial.
- `node_red_flow.png` - fluxo importado e em deploy.
- `node_red_dashboard.png` - dashboard `/ui` em operacao.
- `alerta_dashboard.png` - exemplo com LED virtual em vermelho e texto de alerta.

## 7. Boas praticas para producao

Para uso real em saude digital, a arquitetura deve incluir:

- MQTT sobre TLS com certificados auditaveis;
- autenticacao por dispositivo (mTLS ou usuario/senha por wearable);
- ACLs no broker para isolar topicos por paciente ou clinica;
- mascaramento de identificadores pessoais e separacao do prontuario;
- logs auditaveis das publicacoes e dos acessos ao dashboard;
- politicas de alerta com supervisao humana (a entrega gera apenas indicacao visual);
- conformidade com LGPD (vide `docs/reflexao_seguranca_lgpd.md`).
