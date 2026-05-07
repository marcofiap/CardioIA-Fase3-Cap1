<img src="../assets/logo-fiap.png" alt="FIAP" border="0" width="30%">

# AI Project Document - CardioIA Conectada

## Grupo 57

Felipe Sabino da Silva, Juan Felipe Voltolini, Luiz Henrique Ribeiro de Oliveira, Marco Aurelio Eberhardt Assimpção e Paulo Henrique Senise.

## Sumario

[1. Introducao](#c1)  
[2. Visao Geral do Projeto](#c2)  
[3. Desenvolvimento do Projeto](#c3)  
[4. Resultados e Avaliacoes](#c4)  
[5. Conclusoes e Trabalhos Futuros](#c5)  
[6. Referencias](#c6)  
[Anexos](#c7)

# <a name="c1"></a>1. Introducao

## 1.1 Escopo do Projeto

O projeto CardioIA Conectada simula um sistema de monitoramento continuo para pacientes cardiologicos. A solucao combina sensores em ESP32, processamento local, comunicacao MQTT, dashboard e automacao de alertas.

## 1.2 Contexto

Solucoes de saude digital precisam capturar sinais em tempo real, reduzir latencia e manter funcionamento mesmo com conectividade instavel. Por isso, a arquitetura adotada divide responsabilidades entre borda, nevoa e nuvem.

# <a name="c2"></a>2. Visao Geral do Projeto

## 2.1 Objetivos

Desenvolver um prototipo funcional que capture temperatura, umidade e batimentos simulados, mantenha resiliencia offline, publique dados via MQTT e exiba indicadores em dashboard.

## 2.2 Publico-alvo

O publico-alvo conceitual inclui pacientes cardiologicos monitorados remotamente, equipes de enfermagem, cardiologistas e operadoras de saude. A entrega e academica e nao tem finalidade clinica real.

## 2.3 Metodologia

A equipe reaproveitou o contexto da Fase 2, na qual a CardioIA tratava diagnostico assistido, e evoluiu para monitoramento IoT. A implementacao foi dividida em firmware, dashboard, automacao REST/e-mail e experimento de IA em series temporais.

# <a name="c3"></a>3. Desenvolvimento do Projeto

## 3.1 Tecnologias

- ESP32 no Wokwi;
- DHT22;
- botao de pulso;
- MQTT;
- Node-RED Dashboard;
- Python;
- httpx;
- scikit-learn;
- notebook Jupyter.

## 3.2 Modelagem e algoritmos

No ESP32, a logica principal e baseada em regras de risco e fila circular de resiliencia. No Ir Alem 2, foram comparados regressao logistica e modelo LIF simples.

## 3.3 Treinamento e teste

O notebook usa dados sinteticos de sinais vitais para comparar os modelos. O firmware pode ser testado no Wokwi, alterando temperatura do DHT22, pressionando o botao de pulso e alternando a chave de conectividade.

# <a name="c4"></a>4. Resultados e Avaliacoes

## 4.1 Analise dos resultados

A solucao atende aos criterios da atividade: leitura de sensores, resiliencia offline, envio MQTT, dashboard com alertas e documentacao. A fila circular demonstra comportamento resiliente sem depender de SPIFFS fisico.

## 4.2 Feedback

Como trabalho academico, os proximos testes devem registrar prints do Wokwi e do Node-RED para evidenciar a execucao em sala ou video.

# <a name="c5"></a>5. Conclusoes e Trabalhos Futuros

O projeto demonstra um ciclo completo de IoT em saude digital. Trabalhos futuros incluem uso de ESP32 fisico com SPIFFS ou microSD, MQTT com TLS, Grafana Cloud, persistencia historica e modelos de IA validados com bases reais.

# <a name="c6"></a>6. Referencias

- Apostilas FIAP Fase 3, Capitulos 1, 2, 8 e 9.
- Material FIAP Fase 2 utilizado como continuidade do projeto CardioIA.
- Documentacao Arduino ESP32.
- Documentacao Node-RED.
- Documentacao MQTT.

# <a name="c7"></a>Anexos

Arquivos complementares:

- firmware ESP32 em `src/wokwi/sketch.ino`;
- fluxo Node-RED em `src/node-red/flows_cardioia_node_red.json`;
- relatorios em `docs/`;
- notebook em `notebooks/`.
