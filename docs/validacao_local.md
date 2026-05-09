# Validacao local

Data da validacao: 09/05/2026.

Este arquivo resume os testes executados no workspace para reduzir risco antes da entrega da Fase 3 Cap 1.

## Firmware ESP32

Comando:

```powershell
pio run
```

Resultado: compilacao concluida com sucesso para `esp32dev`.

Resumo de memoria informado pelo PlatformIO:

- RAM: 14,5%;
- Flash: 58,0%.

## JSONs de integracao

Comandos:

```powershell
python -m json.tool .\src\node-red\flows_cardioia_node_red.json > $null
python -m json.tool .\src\wokwi\diagram.json > $null
```

Resultado: os arquivos JSON do fluxo Node-RED e do diagrama Wokwi sao validos.

## REST e e-mail

Comando:

```powershell
python .\src\rest-email\cardioia_rest_email.py
```

Resultado: o script enviou uma leitura para a API REST simulada, recuperou a ultima leitura, classificou risco `alto` por taquicardia e febre, e gerou a simulacao de e-mail no console.

## MQTT

Comando:

```powershell
node .\scripts\mqtt_loopback_test.js
```

Resultado: o cliente assinou o topico `fiap/cardioia/grupo57/vitals`, publicou uma mensagem de teste no broker publico `broker.hivemq.com:1883` e recebeu o payload de volta.

## Notebook Ir Alem 2

O comando `jupyter nbconvert` nao estava disponivel no PATH do ambiente local. Para validar a logica, as celulas Python do notebook foram executadas diretamente em um processo Python com backend grafico `Agg`.

Resultado:

- regressao logistica: acuracia `1.0` no conjunto sintetico de teste;
- modelo LIF simples: acuracia `1.0` no conjunto sintetico de teste;
- notebook corrigido para usar quebras de linha reais nas celulas de codigo.

## Pendencias que dependem de execucao visual

- publicar/preencher o link publico do Wokwi;
- importar o fluxo no Node-RED e salvar prints em `assets/evidencias/`;
- gravar o video de ate 4 minutos, publicar como nao listado e preencher o link no README.
