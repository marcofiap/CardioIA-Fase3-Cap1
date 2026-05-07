# Relatorio Ir Alem 2 - IA em series temporais de saude

## Objetivo

O Ir Alem 2 compara um classificador tradicional com um modelo neuromorfico simples para analise de serie temporal de batimentos cardiacos. A proposta e demonstrar vantagens e limitacoes de cada abordagem dentro de um contexto academico da CardioIA.

## Dados

O notebook gera dados sinteticos de series temporais com 60 pontos por amostra. Foram criadas duas classes:

- ritmo normal;
- ritmo de risco, com picos e maior variabilidade.

Os dados sinteticos evitam uso de informacao pessoal e facilitam reproducao da entrega.

## Modelos comparados

### Regressao logistica

A regressao logistica usa estatisticas extraidas da serie:

- media;
- desvio padrao;
- valor maximo;
- valor minimo;
- amplitude;
- quantidade de picos.

E um modelo simples, interpretavel e rapido para bases pequenas.

### Modelo LIF simples

O modelo LIF, inspirado em neuronios do tipo Leaky Integrate-and-Fire, transforma a serie em contagem de disparos. A intuicao e que sinais com maior intensidade e variabilidade geram mais spikes. A classificacao ocorre por limiar sobre a quantidade de disparos.

## Comparacao

A regressao logistica tende a ser mais estavel no dataset sintetico porque aprende pesos diretamente a partir das features. O LIF e mais simples e energeticamente interessante como conceito neuromorfico, mas depende muito da escolha de limiar, decaimento e escala do sinal.

## Limitacoes

O experimento nao substitui validacao clinica, pois usa sinais simulados e uma arquitetura LIF didatica. Para evolucao, a equipe poderia usar datasets reais de ECG, calibrar parametros neuromorficos e comparar contra redes recorrentes ou convolucionais.

## Entregaveis relacionados

- Notebook: `notebooks/ir_alem2_series_temporais_saude.ipynb`
- README principal com link reservado para GitHub e video.
