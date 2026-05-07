# Faculdade de Informatica e Administracao Paulista

<p align="center">
  <img src="assets/logo-fiap.png" alt="FIAP" width="40%">
</p>

# CardioIA Conectada - IoT e Visualizacao de Dados para a Saude Digital

**FIAP | Tecnologo em Inteligencia Artificial | Fase 3 | Capitulo 1**

## Grupo 57

| Integrante | GitHub |
|---|---|
| Felipe Sabino da Silva | [@FelipeSabinoTMRS](https://github.com/FelipeSabinoTMRS) |
| Juan Felipe Voltolini | [@juanvoltolini-rm562890](https://github.com/juanvoltolini-rm562890) |
| Luiz Henrique Ribeiro de Oliveira | [@Luiz-FIAP](https://github.com/Luiz-FIAP) |
| Marco Aurelio Eberhardt Assimpção | [@marcofiap](https://github.com/marcofiap) |
| Paulo Henrique Senise | [@PauloSenise](https://github.com/PauloSenise) |

## Descricao

Este repositorio implementa a fase **CardioIA Conectada**, continuidade do projeto da Fase 2. Na etapa anterior, a CardioIA explorou diagnostico assistido por regras e classificacao textual de risco cardiologico. Nesta fase, a solucao avanca para um prototipo IoT de monitoramento continuo, simulando um dispositivo vestivel com ESP32, sensores, processamento local, comunicacao MQTT, dashboard Node-RED e alertas automaticos.

O sistema simula o fluxo completo de dados em saude digital:

```text
captura -> edge/resiliencia -> MQTT -> fog/cloud -> dashboard -> alerta
```

O prototipo usa dois sensores no Wokwi:

- DHT22 para temperatura e umidade;
- botao de pressao como sensor de pulso, simulando batimentos por minuto.

Tambem foram implementados os desafios "Ir Alem":

- **Ir Alem 1**: cliente REST em Python com verificacao de risco e simulacao de e-mail.
- **Ir Alem 2**: notebook comparando regressao logistica com um modelo neuromorfico LIF simples em serie temporal de batimentos.

## Estrutura de pastas

```text
.
|-- assets/
|   `-- logo-fiap.png
|-- config/
|   `-- mosquitto.conf
|-- docs/
|   |-- referencias_apostilas_resumo.txt
|   |-- relatorio_parte1_edge.md
|   |-- relatorio_parte2_mqtt_dashboard.md
|   |-- relatorio_ir_alem1_rest_email.md
|   `-- relatorio_ir_alem2_ia_series_temporais.md
|-- document/
|   `-- ai_project_document_fiap.md
|-- notebooks/
|   `-- ir_alem2_series_temporais_saude.ipynb
|-- src/
|   |-- wokwi/
|   |   |-- sketch.ino
|   |   |-- diagram.json
|   |   `-- libraries.txt
|   |-- node-red/
|   |   `-- flows_cardioia_node_red.json
|   `-- rest-email/
|       |-- cardioia_rest_email.py
|       `-- requirements.txt
`-- README.md
```

## Como executar

### Parte 1 e Parte 2 - ESP32 no Wokwi

1. Crie um projeto ESP32 no Wokwi.
2. Copie `src/wokwi/sketch.ino`, `src/wokwi/diagram.json` e `src/wokwi/libraries.txt`.
3. Ajuste no firmware, se necessario:
   - `MQTT_SERVER`
   - `MQTT_PORT`
   - `MQTT_USER`
   - `MQTT_PASSWORD`
4. Execute a simulacao.
5. Use o Monitor Serial para acompanhar:
   - leituras coletadas;
   - estado online/offline;
   - fila local de resiliencia;
   - publicacoes MQTT.

### Dashboard Node-RED

1. Instale Node-RED e os dashboards:

```bash
npm install -g node-red
cd ~/.node-red
npm install node-red-dashboard
node-red
```

2. Acesse `http://localhost:1880`.
3. Importe o fluxo `src/node-red/flows_cardioia_node_red.json`.
4. Configure o broker MQTT no node `mqtt in`, se estiver usando HiveMQ Cloud ou Mosquitto local.
5. Abra `http://localhost:1880/ui`.

### Ir Alem 1 - REST e e-mail

```bash
cd src/rest-email
pip install -r requirements.txt
python cardioia_rest_email.py
```

Por padrao, o script usa uma API REST simulada em memoria e imprime o e-mail no console. Para envio SMTP real, defina variaveis de ambiente:

```bash
set SMTP_HOST=smtp.example.com
set SMTP_PORT=587
set SMTP_USER=usuario
set SMTP_PASSWORD=senha
set ALERT_TO=destino@example.com
```

### Ir Alem 2 - Notebook

```bash
pip install numpy pandas scikit-learn matplotlib notebook
python -m notebook notebooks/ir_alem2_series_temporais_saude.ipynb
```

## Links para entrega

Preencher apos publicacao:

- Link Wokwi: `INSERIR_LINK_DO_WOKWI`
- Link GitHub publico: `INSERIR_LINK_DO_GITHUB`
- Link video YouTube nao listado: `INSERIR_LINK_DO_VIDEO`

## Checklist do enunciado

- [x] ESP32 com no minimo dois sensores.
- [x] DHT22 para temperatura e umidade.
- [x] Sensor adicional de pulso simulado por botao.
- [x] Processamento local e regras de alerta na borda.
- [x] Resiliencia offline por fila circular limitada.
- [x] Envio de dados via MQTT.
- [x] Dashboard Node-RED com grafico, gauge e alerta.
- [x] Relatorios das Partes 1 e 2.
- [x] Ir Alem 1 com REST, risco e e-mail.
- [x] Ir Alem 2 com comparacao entre modelo tradicional e neuromorfico.

## Observacao academica

Este projeto e uma simulacao academica. Os alertas e classificacoes nao substituem avaliacao medica, validacao clinica, certificacao regulatoria ou protocolos reais de atendimento.
