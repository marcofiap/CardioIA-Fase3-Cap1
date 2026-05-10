# Validacao local

Data da ultima validacao: 10/05/2026.

Este arquivo resume os testes executados no workspace para reduzir risco antes da entrega da Fase 3 Cap 1.

## Firmware ESP32

Comando:

```powershell
pio run
```

Resultado: compilacao concluida com sucesso para `esp32dev`, ja com bibliotecas `Adafruit MPU6050` e `Adafruit Unified Sensor` adicionadas.

Resumo de memoria informado pelo PlatformIO:

- RAM: 14,9%;
- Flash: 60,2%.

## JSONs de integracao

Comandos:

```powershell
python -m json.tool src/node-red/flows_cardioia_node_red.json > $null
python -m json.tool src/wokwi/diagram.json > $null
```

Resultado: os arquivos JSON do fluxo Node-RED (com 6 saidas e novo node de movimento) e do diagrama Wokwi (com MPU6050) sao validos.

## REST e e-mail

Comando:

```powershell
python src/rest-email/cardioia_rest_email.py
```

Resultado: o script enviou uma leitura (com `accelMagnitude`) para a API REST simulada, recuperou a ultima leitura, classificou risco `alto` por taquicardia e febre e gerou a simulacao de e-mail no console.

## MQTT

Comando:

```powershell
node scripts/mqtt_loopback_test.js
```

Resultado: o cliente assinou o topico `fiap/cardioia/grupo57/vitals`, publicou uma mensagem de teste no broker publico `broker.hivemq.com:1883` e recebeu o payload de volta.

## Notebook Ir Alem 2

Comando:

```powershell
python -m jupyter nbconvert --to notebook --execute notebooks/ir_alem2_series_temporais_saude.ipynb --output ir_alem2_series_temporais_saude.ipynb
```

Resultado: notebook executado em ambos os cenarios (facil e dificil). Em paralelo, o script reproductivel `python scripts/compute_ir_alem2_metrics.py` confirmou:

- Cenario facil: regressao logistica e LIF com acuracia 1.0;
- Cenario dificil: regressao logistica F1 0.951 e ROC-AUC 0.988; LIF F1 0.82.

## Lint

Comando interno (Cursor `read_lints`) sobre `src/rest-email/cardioia_rest_email.py`, `scripts/compute_ir_alem2_metrics.py` e `src/wokwi/sketch.ino`: sem erros.

## Pendencias que dependem de execucao visual

- publicar/preencher o link publico do Wokwi;
- importar o fluxo no Node-RED, executar e salvar prints em `assets/evidencias/`;
- gravar o video de ate 4 minutos, publicar como nao listado e preencher o link no README.
