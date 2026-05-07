# Relatorio Parte 2 - MQTT, Fog/Cloud e dashboard

## Arquitetura

A Parte 2 conecta o ESP32 simulado a uma camada de visualizacao em tempo real. O fluxo implementado e:

```text
ESP32/Wokwi -> MQTT -> Node-RED -> Dashboard -> alerta visual
```

O ESP32 publica as leituras no topico:

```text
fiap/cardioia/grupo57/vitals
```

O Node-RED assina esse topico, interpreta o JSON e distribui os valores para componentes visuais.

## MQTT

MQTT foi escolhido por ser leve, adequado a IoT e compatível com conexoes instaveis. O dispositivo atua como publisher e o dashboard atua como subscriber. Isso desacopla a captura da visualizacao: se futuramente houver mais dashboards, persistencia em banco ou integracao com Grafana, novos consumidores podem assinar o mesmo topico.

Payload publicado:

```json
{
  "deviceId": "cardioia-esp32-grupo57",
  "timestamp": 123456,
  "temperature": 36.8,
  "humidity": 52.4,
  "bpm": 82,
  "alert": false
}
```

Para teste local, o projeto esta configurado com `broker.hivemq.com:1883`. Para uma entrega mais controlada, pode-se usar HiveMQ Cloud com TLS e credenciais ou Mosquitto local com a configuracao em `config/mosquitto.conf`.

## Dashboard Node-RED

O fluxo exportado esta em:

```text
src/node-red/flows_cardioia_node_red.json
```

Componentes da dashboard:

- grafico de linha para BPM;
- gauge para temperatura corporal;
- texto de status;
- LED virtual para alerta.

As regras de alerta sao aplicadas no firmware e refletidas no campo `alert`. O dashboard tambem pode ser estendido para recalcular regras na camada de fog, mas a decisao principal ficou na borda para reduzir latencia.

## Papel de Fog e Cloud

Nesta entrega, o Node-RED representa a camada de Fog Computing, pois recebe dados proximos da operacao, trata o payload e entrega visualizacao imediata. O broker MQTT representa a camada de comunicacao com nuvem ou infraestrutura remota. Em uma solucao real, a nuvem agregaria historico, auditoria, dashboards gerenciais e modelos de IA.

Essa divisao segue o material da Fase 3: a borda coleta e decide localmente, a nevoa agrega e exibe rapidamente, e a nuvem escala armazenamento e analise.

## Prints e evidencias

Os prints devem ser salvos em `assets/` apos execucao:

- `assets/wokwi_execucao.png`
- `assets/node_red_flow.png`
- `assets/node_red_dashboard.png`

Como alternativa, o export do fluxo Node-RED ja foi incluído no repositorio.

## Boas praticas

Para uso real em saude digital, a arquitetura deve incluir:

- MQTT sobre TLS;
- autenticacao por dispositivo;
- controle de acesso no broker;
- mascaramento de identificadores pessoais;
- logs auditaveis;
- armazenamento criptografado;
- politicas de alerta com supervisao humana.
