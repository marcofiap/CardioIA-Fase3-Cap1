# Relatorio Parte 1 - Armazenamento e processamento local (Edge Computing)

## Contexto

A Parte 1 da atividade implementa o nucleo do dispositivo vestivel da CardioIA usando ESP32 simulado no Wokwi. O foco e o papel da **Edge Computing**: capturar sinais vitais, decidir localmente sobre alertas e manter resiliencia mesmo quando a rede esta indisponivel. As decisoes seguem o material da Fase 3, capitulos 8 (Cloud Computing como Pilar da IoT Moderna) e 9 (Inteligencia na Borda: Fog e Edge Computing Transformando a IoT).

O prototipo usa tres sensores, alem dos atuadores de demonstracao:

- **DHT22** (sensor obrigatorio do enunciado): mede temperatura e umidade do ambiente proximo ao paciente.
- **Botao de pressao** (segundo sensor de livre escolha): cada acionamento e contado como um batimento cardiaco simulado, permitindo demonstrar o fluxo de pulso.
- **MPU6050** (sensor adicional, alinhado com as apostilas Cap 8 e 9 e com o desafio Ir Alem 1): acelerometro I2C que detecta movimento do paciente.

Tambem ha uma chave deslizante usada como variavel booleana de conectividade Wi-Fi, um LED de alerta e um resistor de protecao.

## Diagrama do circuito

```text
+---------------------+        +-----------------+
|        ESP32        |--3V3---| DHT22 (temp/um) |
|                     |--GND---|                 |
|                     |--D15---| SDA             |
|                     |        +-----------------+
|                     |
|                     |--3V3---+-----------------+
|                     |--GND---|     MPU6050     |
|                     |--D21---| SDA  (I2C)      |
|                     |--D22---| SCL  (I2C)      |
|                     |        +-----------------+
|                     |
|                     |--D18---+ Botao PULSO (PULL-UP)
|                     |--GND---|
|                     |
|                     |--D19---+ Slide ONLINE/OFFLINE
|                     |--3V3---|  (3V3 = ONLINE, GND = OFFLINE)
|                     |--GND---|
|                     |
|                     |--D2----R220--LED ALERTA--GND
+---------------------+
```

## Fluxo de funcionamento

1. **Boot**: o firmware inicializa Serial, DHT22, I2C/MPU6050 e atribui os modos dos pinos. Loga o estado inicial da chave de conectividade.
2. **Coleta** a cada 5 segundos (`SAMPLE_INTERVAL_MS`):
   - le temperatura e umidade do DHT22;
   - calcula BPM com base na contagem de pulsos numa janela de 15 segundos;
   - calcula movimento como variacao da magnitude do vetor de aceleracao (`|delta| > 0.4 m/s^2 -> movement = 1`);
   - aplica regras locais de alerta (`temperatura > 38 C` ou `BPM > 120`) e atualiza o LED de alerta na borda.
3. **Decisao de transporte**:
   - se a chave esta em `ONLINE` e ha Wi-Fi e MQTT, publica o JSON no topico `fiap/cardioia/grupo57/vitals` com flag `retained=true`;
   - caso contrario, enfileira a amostra na fila circular local de 120 posicoes.
4. **Sincronizacao** automatica: quando a conectividade volta, `syncOfflineQueue()` drena a fila publicando uma amostra de cada vez e logando cada publicacao no Serial.
5. **Heartbeat**: a cada 2 segundos o firmware imprime no Serial um resumo (`HB millis= wifi= mqtt= online= fila=`) que ajuda diagnosticar a simulacao no Wokwi e nas evidencias do video.

## Resiliencia offline (Edge Computing)

O enunciado destaca que SPIFFS nao persiste no Wokwi e indica como alternativa o Monitor Serial. A entrega usa uma estrategia mais robusta sem depender de hardware fisico: uma **fila circular em memoria** com capacidade configuravel, exibida pelo Serial.

A capacidade foi fixada em 120 amostras. Considerando intervalo de 5 segundos por amostra, isso representa 10 minutos de autonomia offline:

```text
120 amostras * 5 segundos = 600 segundos = 10 minutos
```

Justificativa do dimensionamento:

- modelo de negocio CardioIA pressupoe wearable com Wi-Fi residencial. Quedas curtas de rede (mudanca de comodo, oscilacao do roteador) sao absorvidas localmente sem perda de dado.
- 10 minutos cobre boa parte das interrupcoes residenciais reais sem inflar memoria do ESP32.
- quando a fila atinge a capacidade, a amostra mais antiga e descartada: em monitoramento cardiologico, o estado atual e mais relevante para o alerta imediato do que dados antigos.
- queda longa de rede deveria gerar alerta operacional, troca de gateway ou reconexao do paciente, fora do escopo do firmware.

Para um dispositivo real, o mesmo desenho pode ser adaptado para SPIFFS ou microSD, persistindo o array em arquivo. Como o enunciado considera SPIFFS opcional e apenas para chips fisicos, o relatorio mantem o registro da estrategia em memoria.

## Lendo o JSON gerado

Cada amostra coletada vira o seguinte JSON (publicado por MQTT ou logado por Serial em modo offline):

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

O mesmo schema e consumido pelo dashboard Node-RED da Parte 2 e pelo cliente REST do Ir Alem 1.

## Decisoes de seguranca aplicadas na borda

- LED de alerta acionado localmente, nao depende da nuvem: garante que o paciente ou cuidador veja o sinal mesmo offline.
- regras simples e auditaveis (`>38 C`, `>120 bpm`), explicaveis a profissional de saude.
- `Serial.print` mascarado: nenhum dado pessoal e enviado, apenas identificador tecnico do dispositivo.
- broker MQTT pode ser facilmente trocado para HiveMQ Cloud com TLS e credenciais sem mudar o restante do firmware. As implicacoes de seguranca/LGPD estao detalhadas em `docs/reflexao_seguranca_lgpd.md`.

## Link Wokwi

Preencher apos publicacao do projeto:

`INSERIR_LINK_DO_WOKWI`
