# Roteiro sugerido para video de ate 4 minutos

## 0:00 - 0:30 | Contexto

Apresentar a CardioIA Conectada como evolucao da Fase 2. Explicar que agora o foco e monitoramento continuo com IoT, Edge, Fog, Cloud e visualizacao.

## 0:30 - 1:30 | Wokwi e ESP32

Mostrar:

- ESP32 com DHT22 (temperatura/umidade);
- MPU6050 (movimento via acelerometro);
- botao simulando pulso (BPM);
- chave online/offline;
- LED de alerta;
- Monitor Serial com leituras JSON e heartbeat.

Explicar que a fila circular de 120 amostras representa resiliencia offline no simulador, substituindo SPIFFS por causa da limitacao do Wokwi, e cobre 10 minutos sem rede.

## 1:30 - 2:30 | MQTT e Node-RED

Mostrar:

- topico MQTT `fiap/cardioia/grupo57/vitals`;
- fluxo Node-RED importado;
- dashboard com grafico de BPM, gauge de temperatura, indicador de movimento, texto de alerta e LED virtual;
- citar HiveMQ Cloud com TLS como upgrade para producao.

Explicar que o ESP32 publica dados e o Node-RED consome e visualiza em tempo real.

## 2:30 - 3:20 | Ir Alem 1

Executar ou mostrar o script:

```bash
python src/rest-email/cardioia_rest_email.py
```

Explicar envio REST, verificacao de risco e simulacao de e-mail.

## 3:20 - 4:00 | Ir Alem 2 e conclusao

Mostrar rapidamente o notebook com regressao logistica e LIF simples nos cenarios facil e dificil, citando as metricas (F1 0.95 para a regressao no cenario dificil contra 0.82 do LIF). Fechar reforcando seguranca, LGPD (`docs/reflexao_seguranca_lgpd.md`), uso academico e necessidade de validacao clinica em uma solucao real.
