# Relatorio Parte 1 - Armazenamento e processamento local

## Contexto

A Parte 1 da atividade foi implementada como uma simulacao de dispositivo vestivel da CardioIA usando ESP32 no Wokwi. O objetivo e demonstrar como a camada de borda pode continuar coletando sinais vitais mesmo quando a comunicacao com a nuvem esta indisponivel.

O prototipo usa dois sensores:

- DHT22, considerado um sensor fisico, para temperatura e umidade;
- botao de pressao, usado como sensor adicional para simular batimentos cardiacos.

## Fluxo de funcionamento

O firmware executa um ciclo continuo:

1. Le a temperatura e a umidade do DHT22.
2. Conta acionamentos do botao em uma janela temporal e converte essa contagem em BPM.
3. Aplica regras locais de risco:
   - temperatura acima de 38 C;
   - BPM acima de 120.
4. Acende o LED local de alerta quando alguma regra e violada.
5. Em operacao normal, tenta publicar o JSON no topico MQTT.
6. Se a chave `OFFLINE` estiver fechada ou houver falha de rede, grava a amostra em uma fila circular na memoria.
7. Quando a conectividade retorna, sincroniza as amostras pendentes em ordem.

Esse desenho segue o conceito de Edge Computing das apostilas da Fase 3: decisoes imediatas sao tomadas perto da origem do dado, reduzindo dependencia da nuvem em uma situacao de saude que pode ser critica.

## Resiliencia offline

O enunciado explica que o SPIFFS no Wokwi e volatil e nao preserva arquivos quando a simulacao termina. Por isso, a entrega usa uma estrategia alternativa alinhada ao simulador: uma fila circular em memoria, exibida pelo Monitor Serial.

A fila foi limitada a 120 amostras. Com intervalo de coleta de 5 segundos, isso representa cerca de 10 minutos de autonomia offline:

```text
120 amostras * 5 segundos = 600 segundos = 10 minutos
```

Essa escolha e coerente com o modelo da CardioIA como solucao vestivel conectada: quedas curtas de rede devem ser absorvidas localmente, mas uma indisponibilidade longa exige alerta operacional, troca de gateway ou reconexao do paciente.

Quando a fila atinge a capacidade maxima, a amostra mais antiga e descartada. Essa decisao prioriza dados recentes, pois em monitoramento continuo de risco cardiologico o estado atual do paciente e mais relevante para alertas imediatos.

No Wokwi, a chave `OFFLINE` foi configurada para facilitar a demonstracao:

- aberta: sistema tenta conectar Wi-Fi e publicar MQTT;
- fechada: sistema simula queda de conectividade e guarda leituras na fila local.

## Dados coletados

Cada amostra e representada como JSON:

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

## Consideracoes de seguranca

Em uma aplicacao real, o dispositivo nao deveria transmitir dados de saude em broker publico sem criptografia. A versao academica usa broker MQTT simples para facilitar a reproducao, mas o desenho recomenda:

- MQTT com TLS;
- usuario e senha por dispositivo;
- topicos segregados por identificador tecnico, sem nome do paciente;
- politicas de retencao minima;
- tratamento conforme LGPD;
- validacao clinica antes de qualquer uso assistencial.

## Link Wokwi

Preencher apos publicacao do projeto:

`INSERIR_LINK_DO_WOKWI`
