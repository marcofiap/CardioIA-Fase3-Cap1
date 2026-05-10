# Checklist do enunciado

## Parte 1 - Edge Computing

| Exigencia | Status | Evidencia |
|---|---:|---|
| Projeto ESP32 no Wokwi | Parcial | Arquivos em `src/wokwi/`; falta inserir link publico do Wokwi no README |
| Minimo de 2 sensores distintos | Atendido com folga | DHT22, botao de pulso e MPU6050 em `src/wokwi/diagram.json` |
| Sensor obrigatorio DHT22 | Atendido | `DHT_PIN`, `DHT_TYPE` e leitura em `src/wokwi/sketch.ino` |
| Segundo sensor livre | Atendido | Botao de pressao para simular BPM |
| Sensor adicional (alinhado com Cap 8 e 9) | Atendido | MPU6050 acelerometro I2C, lido em `readMovement()` |
| Armazenamento local ou alternativa ao SPIFFS | Atendido | Fila circular em memoria, documentada em `docs/relatorio_parte1_edge.md` |
| Simular conectividade Wi-Fi booleana | Atendido | Chave `OFFLINE` no Wokwi, pino `FORCE_OFFLINE_SWITCH_PIN` |
| Quando conectado, enviar dados e limpar pendencias | Atendido | `syncOfflineQueue()` publica MQTT e remove amostras da fila |
| Resiliencia offline | Atendido | `MAX_OFFLINE_SAMPLES = 120`, fila circular |
| Codigo C++ comentado | Atendido | `src/wokwi/sketch.ino` |
| Relatorio minimo de uma pagina | Atendido com folga | `docs/relatorio_parte1_edge.md` (mais de duas paginas) |

## Parte 2 - Fog/Cloud e visualizacao

| Exigencia | Status | Evidencia |
|---|---:|---|
| Envio via MQTT | Atendido | `PubSubClient`, topico `fiap/cardioia/grupo57/vitals` |
| Broker MQTT | Atendido | HiveMQ publico configurado por padrao; Mosquitto local opcional em `config/mosquitto.conf` |
| Dashboard Node-RED | Atendido | Export em `src/node-red/flows_cardioia_node_red.json` |
| Grafico de sinal vital | Atendido | `ui_chart` para BPM |
| Gauge de parametro | Atendido | `ui_gauge` para temperatura |
| Indicador visual de alerta | Atendido | `ui_text` e LED virtual em `ui_template`, alem de `ui_text` para movimento |
| Prints ou export do dashboard | Parcial | Export incluido; faltam prints reais em `assets/evidencias/` |
| Relatorio minimo de duas paginas | Atendido com folga | `docs/relatorio_parte2_mqtt_dashboard.md` (com diagramas, tabelas e mapeamento Node-RED) |

## Ir Alem 1 - REST e e-mail

| Exigencia | Status | Evidencia |
|---|---:|---|
| Cliente REST em Python | Atendido | `src/rest-email/cardioia_rest_email.py` |
| Envio e recebimento de dados | Atendido | `send_reading()` e `get_latest_reading()` |
| Logica de risco | Atendido | `evaluate_risk()` |
| Simulacao de e-mail | Atendido | `send_alert_email()` imprime ou envia via SMTP |
| Relatorio curto | Atendido | `docs/relatorio_ir_alem1_rest_email.md` |

## Ir Alem 2 - IA em series temporais

| Exigencia | Status | Evidencia |
|---|---:|---|
| Notebook Python comentado | Atendido | `notebooks/ir_alem2_series_temporais_saude.ipynb` |
| Classificador tradicional | Atendido | Regressao logistica em dois cenarios (facil/dificil) |
| Rede/modelo neuromorfico simples | Atendido | LIF simples em dois cenarios (facil/dificil) |
| Relatorio comparativo | Atendido com folga | `docs/relatorio_ir_alem2_ia_series_temporais.md` (mais de duas paginas, com matrizes de confusao, F1, ROC-AUC e analise critica) |
| GitHub publico | Atendido | <https://github.com/marcofiap/CardioIA-Fase3-Cap1> |
| Video ate 4 minutos | Pendente | Falta gravar, publicar como nao listado e preencher link no README |

## Reflexao sobre seguranca, eficiencia e LGPD

| Exigencia | Status | Evidencia |
|---|---:|---|
| Reflexao sobre eficiencia, seguranca e boas praticas em IoT medico | Atendido | `docs/reflexao_seguranca_lgpd.md` |
| Documento estruturado seguindo Template FIAP | Atendido | `document/ai_project_document_fiap.md` |
| Diagrama de arquitetura Edge/Fog/Cloud | Atendido | README, relatorios e `document/ai_project_document_fiap.md` |

## Pendencias finais antes de entregar no portal

1. Publicar o projeto no Wokwi e preencher o link no README.
2. Importar o fluxo no Node-RED, executar e salvar prints em `assets/evidencias/`.
3. Gravar video de ate 4 minutos e preencher o link no README.
4. Fazer commit e push das evidencias finais.

## Validacao local executada

Os testes locais de firmware, JSONs, REST/e-mail, MQTT e notebook estao registrados em `docs/validacao_local.md`.
