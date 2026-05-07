# Relatorio Ir Alem 1 - REST e e-mail

## Objetivo

O Ir Alem 1 simula um fluxo de automacao em saude digital no qual sinais vitais sao enviados para uma API REST, avaliados por regras de risco e, em caso de alerta, disparam uma comunicacao por e-mail.

Essa etapa conecta os conceitos da Fase 3 sobre REST, automacao e e-mail ao projeto CardioIA.

## Implementacao

O codigo esta em:

```text
src/rest-email/cardioia_rest_email.py
```

O script contem:

- uma classe `CardioIaApiClient`, responsavel por enviar e buscar sinais vitais;
- uma funcao `evaluate_risk`, que aplica regras de risco;
- uma funcao `send_alert_email`, que envia ou simula e-mail;
- um fluxo demonstrativo em `main`.

Por padrao, o cliente usa `MockTransport` da biblioteca `httpx`. Isso permite demonstrar consumo REST sem depender de servidor externo. O mesmo cliente pode ser apontado para uma API real alterando a URL base.

## Regras de risco

As regras foram definidas de forma transparente:

- taquicardia: BPM maior ou igual a 120;
- febre: temperatura maior ou igual a 38 C;
- ausencia de movimento: `movement` igual a zero;
- risco moderado: BPM entre 100 e 119.

O retorno da avaliacao contem:

- nivel do risco;
- lista de motivos;
- recomendacao operacional.

## E-mail

Se variaveis SMTP estiverem configuradas, o script envia uma mensagem real. Se nao estiverem, imprime no console uma simulacao do e-mail. Essa decisao evita expor credenciais no repositorio e segue boas praticas de seguranca.

Variaveis suportadas:

```text
SMTP_HOST
SMTP_PORT
SMTP_USER
SMTP_PASSWORD
ALERT_TO
ALERT_FROM
```

## Conclusao

O modulo demonstra como o monitoramento IoT pode acionar automacoes fora do ESP32. Em uma solucao real, esse fluxo poderia abrir chamado, notificar equipe medica, registrar ocorrencia em prontuario ou disparar protocolo de atendimento, sempre com controles de seguranca e supervisao humana.
