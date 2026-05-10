# Reflexao - Seguranca, IoT medico e LGPD

Este documento resume a reflexao sobre eficiencia, seguranca e boas praticas em IoT medico, conforme exigido no objetivo geral do enunciado da Fase 3 Cap 1 (CardioIA Conectada). Tambem registra as decisoes academicas tomadas para a entrega e as recomendacoes para uma evolucao em ambiente real.

## 1. Por que isso importa nesse projeto

O CardioIA Conectada simula um wearable com sinais vitais. Mesmo em entrega academica, o desenho precisa demonstrar que a equipe entende as implicacoes reais de:

- captura continua de sinais corporais (temperatura, BPM, movimento);
- transmissao via Wi-Fi / MQTT pela internet publica;
- visualizacao em dashboard;
- automacao de alertas por e-mail.

Cada um desses passos toca em normas (LGPD, possivel ANVISA), em padroes tecnicos (HL7, FHIR no futuro) e em risco para o paciente.

## 2. Camadas de risco e controles correspondentes

### 2.1 Edge - dispositivo (ESP32)

Riscos:

- credenciais de Wi-Fi e MQTT em plain text no firmware;
- broker publico aceita qualquer cliente;
- LED de alerta nao deve ser confundido com diagnostico clinico.

Controles aplicados na entrega academica:

- nenhum dado pessoal no firmware: o `deviceId` e um identificador tecnico (`cardioia-esp32-grupo57`), nao o nome do paciente;
- regras simples e auditaveis (`>38 C`, `>120 bpm`) no firmware;
- LED claramente rotulado como `ALERTA`, nao como `DIAGNOSTICO`.

Controles recomendados para producao:

- credenciais por dispositivo, gravadas em area protegida (eFuse, NVS criptografada);
- conexao MQTT obrigatoria sobre TLS, com `WiFiClientSecure`;
- atualizacao OTA assinada digitalmente;
- watchdog de hardware para resetar dispositivo travado.

### 2.2 Fog - Node-RED no PC do operador

Riscos:

- dashboard exposto sem autenticacao;
- logs persistentes do Node-RED com payload bruto em disco;
- Node-RED rodando como root.

Controles aplicados:

- dashboard so escuta em `127.0.0.1` (localhost) na entrega academica;
- node `function` de normalizacao garante que somente os campos esperados seguem para os widgets;
- node `debug` claramente marcado e usado so como evidencia de execucao.

Controles recomendados para producao:

- ativar autenticacao do Node-RED (`adminAuth`) e dashboard atras de proxy reverso com SSO;
- `editorTheme.projects.enabled = true` com versionamento Git;
- logs sem PII;
- contexto persistido em arquivo criptografado.

### 2.3 Cloud - broker MQTT

Riscos:

- broker publico (HiveMQ public) e visivel a qualquer pessoa que assine o topico;
- topico previsivel (`fiap/cardioia/grupo57/vitals`);
- ausencia de ACL.

Controles aplicados na entrega academica:

- topico nao contem identificadores de paciente;
- nenhum dado real foi publicado;
- alternativa segura (HiveMQ Cloud com TLS e credenciais) esta documentada em `docs/relatorio_parte2_mqtt_dashboard.md`.

Controles recomendados para producao:

- HiveMQ Cloud / AWS IoT Core / Azure IoT Hub com mTLS;
- topicos hierarquicos com escopo (`hospitais/{hospitalId}/devices/{deviceId}/vitals`);
- ACL por dispositivo e por consumidor;
- last-will message para detectar dispositivos offline;
- Bridges para datalake corporativo via Kafka/MQTT bridge.

### 2.4 Automacao - REST e e-mail (Ir Alem 1)

Riscos:

- e-mail com dado clinico saindo por SMTP nao criptografado;
- ausencia de rate limiting (poderia gerar SPAM);
- log do payload em texto.

Controles aplicados na entrega academica:

- credenciais de SMTP via variavel de ambiente, nunca hardcoded;
- e-mail simulado por padrao (impressao no console), evitando vazamento durante desenvolvimento;
- e-mail nao contem CPF, nome real do paciente ou identificadores reidentificaveis;
- API REST em mock para nao publicar dados reais durante a entrega.

Controles recomendados para producao:

- SMTP com STARTTLS obrigatorio;
- pool/queue de envio com rate limiting e backoff exponencial;
- assinatura DKIM e SPF para evitar phishing;
- supervisao humana sempre antes de qualquer comunicacao com paciente real.

## 3. LGPD e dados sensiveis

A Lei Geral de Protecao de Dados (Lei 13.709/2018) classifica dados de saude como **dados pessoais sensiveis** (Art. 5, II). Para um produto real:

- a base legal mais provavel para tratamento e o **consentimento explicito** do titular (paciente), com finalidade especifica registrada em contrato;
- e necessario manter registro de operacoes de tratamento (Art. 37);
- o titular tem direito a **portabilidade** e a **eliminacao** dos dados (Art. 18);
- o operador deve adotar medidas tecnicas e administrativas para protecao (Art. 46);
- em caso de incidente, **comunicacao a ANPD** e obrigatoria em prazo razoavel (Art. 48).

Decisoes aplicadas a entrega academica:

- nenhum dado real de paciente foi capturado;
- todos os identificadores sao ficticios (`paciente-simulado-001`, `cardioia-esp32-grupo57`);
- relatorios e codigo nao contem CPF, RG, nome ou data de nascimento.

Recomendacoes para evolucao real:

- funcao de anonimizacao na borda (hash com sal por paciente) antes de qualquer envio;
- separacao logica entre identificadores tecnicos no MQTT e identidade clinica no prontuario;
- cifrar dados sensiveis em repouso na nuvem (AES-GCM) com chaves gerenciadas (KMS);
- auditoria de acesso ao prontuario;
- DPO (Data Protection Officer) designado;
- treinamento da equipe que opera o dashboard.

## 4. Etica e responsabilidade clinica

Mesmo com a melhor seguranca tecnica, e fundamental:

- deixar claro ao usuario que **a solucao nao substitui avaliacao medica**;
- usar termos como `alerta` ou `indicacao`, nunca `diagnostico`;
- supervisao humana de toda decisao critica (e-mail automatico ou alerta deve passar por equipe assistencial antes de chegar ao paciente);
- inclusao da regulamentacao da ANVISA para dispositivos medicos no roadmap (RDC 657/2022 sobre software como dispositivo medico);
- avaliacao de viesses do modelo de IA conforme a populacao de pacientes (Cap 7 da Fase 3 sobre Governanca em IA).

## 5. Resumo das decisoes na entrega

| Area | Decisao | Motivo | Risco residual |
|---|---|---|---|
| Wi-Fi | `Wokwi-GUEST` sem senha | facilita simulacao no Wokwi | aceitavel - simulacao academica |
| MQTT | broker publico HiveMQ:1883 | facilita correcao | dados nao sao sensiveis na simulacao |
| Topico | `fiap/cardioia/grupo57/vitals` | identificador academico | sem PII |
| Identificador | `cardioia-esp32-grupo57` | identificador tecnico | nao reidentificavel |
| Email | simulado no console | nao expor SMTP | aceitavel |
| Dataset IA | sintetico | nao usar dado clinico | viesses sao ilustrativos |
| LED de alerta | rotulo `ALERTA` no Wokwi | clareza ao avaliador | nao se confunde com diagnostico |

## 6. Conclusao

Eficiencia, seguranca e responsabilidade nao sao "extras" em IoT medico - sao requisitos fundamentais do produto. Esta entrega adota um conjunto coerente de decisoes para um cenario academico controlado e documenta como cada uma deveria evoluir em um deploy real, da borda ao broker e da automacao a IA. A seguranca esta presente em todas as camadas (Edge, Fog, Cloud) e o respeito a LGPD e a etica clinica estao explicitos como pre-condicoes para usar este desenho como produto comercial.

## 7. Referencias

- Lei 13.709/2018 - Lei Geral de Protecao de Dados Pessoais (LGPD).
- Resolucao ANVISA RDC 657/2022 - Software como Dispositivo Medico (SaMD).
- Apostila FIAP Fase 3 Cap 7 - Governanca em IA e Business Analytics.
- Apostila FIAP Fase 3 Cap 8 - Cloud Computing como Pilar da IoT Moderna.
- Apostila FIAP Fase 3 Cap 9 - Inteligencia na Borda: Fog e Edge Computing Transformando a IoT.
- HiveMQ Cloud Documentation - <https://docs.hivemq.com/hivemq-cloud/>.
- Node-RED Security Guide - <https://nodered.org/docs/user-guide/runtime/securing-node-red>.
