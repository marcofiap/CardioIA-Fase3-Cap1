# Relatorio Ir Alem 2 - IA em series temporais de saude

## 1. Objetivo

O desafio Ir Alem 2 do CardioIA Conectada compara um classificador tradicional com um modelo neuromorfico simples para analise de **series temporais de batimentos cardiacos**. A pergunta de pesquisa e: "Em qual cenario um modelo neuromorfico LIF (Leaky Integrate-and-Fire) compete com uma regressao logistica baseada em features estatisticas?". O experimento esta no notebook `notebooks/ir_alem2_series_temporais_saude.ipynb` e e reproduzivel via `python scripts/compute_ir_alem2_metrics.py`.

## 2. Dados sinteticos

Cada amostra e uma serie de 60 pontos representando uma janela de batimentos. A classe 0 simula ritmo normal (senoide com ruido); a classe 1 simula ritmo de risco (drift na linha de base mais picos esparsos). O experimento usa **dois cenarios** controlados pelo gerador parametrizado, para evidenciar diferenca entre os modelos:

| Cenario | spike_amp_mean | spike_amp_std | base_noise | drift_mean | Comentario |
|---|---:|---:|---:|---:|---|
| Facil | 35 | 8 | 2.0 | 8 | picos altos, classes bem separadas |
| Dificil | 8 | 3 | 6.0 | 0 | picos curtos, sem drift, ruido alto |

A escolha de gerar dois cenarios evita o pitfall classico de avaliar so em conjunto facil e concluir, equivocadamente, que ambos os modelos sao equivalentes.

## 3. Modelos comparados

### 3.1 Regressao logistica

Recebe seis features estatisticas extraidas da serie:

- media;
- desvio padrao;
- valor maximo;
- valor minimo;
- amplitude (max - min);
- numero de picos com valor acima de 110.

Sao caracteristicas tabulares, leves e interpretaveis. A regressao logistica converge rapido e fornece pesos analisaveis por profissional clinico.

### 3.2 Modelo neuromorfico LIF simples

Inspirado em Leaky Integrate-and-Fire, o modelo:

1. centraliza a serie em torno de 70 e divide por uma escala (`scale = 35`);
2. acumula potencial `v = leak * v + max(value, 0)` com `leak = 0.85`;
3. dispara um spike quando `v >= 1.0` e zera `v`;
4. classifica pela contagem total de spikes contra um limiar otimizado no conjunto de treino.

A motivacao e demonstrar como uma dinamica temporal se reduz a uma feature unica derivada do comportamento de um neuronio biologico simplificado, permitindo paralelo com chips neuromorficos discutidos no Cap 3 e Cap 8 da Fase 3.

## 4. Resultados

Resultados gerados pelo script reprodutivel:

### 4.1 Cenario facil

| Modelo | Acuracia | Precisao | Recall | F1 | ROC-AUC |
|---|---:|---:|---:|---:|---:|
| Regressao logistica | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| LIF (limiar=6) | 1.000 | 1.000 | 1.000 | 1.000 | n/a |

Matriz de confusao: 50 normais e 50 risco classificados sem erro em ambos.

### 4.2 Cenario dificil

| Modelo | Acuracia | Precisao | Recall | F1 | ROC-AUC |
|---|---:|---:|---:|---:|---:|
| Regressao logistica | 0.950 | 0.941 | 0.960 | 0.951 | 0.988 |
| LIF (limiar=6) | 0.820 | 0.820 | 0.820 | 0.820 | n/a |

Matriz de confusao da regressao logistica:

| | predito normal | predito risco |
|---|---:|---:|
| **real normal** | 47 | 3 |
| **real risco** | 2 | 48 |

Matriz de confusao do LIF:

| | predito normal | predito risco |
|---|---:|---:|
| **real normal** | 41 | 9 |
| **real risco** | 9 | 41 |

## 5. Analise critica

**Quando os modelos empatam**: no cenario facil, a separacao entre as classes e dada por picos amplos (`~35 unidades acima da base`). Ambos os modelos detectam isso facilmente: a regressao logistica usa o feature `peaks` como dominante, e o LIF acumula spikes rapidamente nas amostras de risco. Ambos chegam a 1.000 em todas as metricas. **Nesse cenario, nao adianta comparar.**

**Quando o tradicional ganha**: no cenario dificil, a regressao logistica mantem F1 0.951 e ROC-AUC 0.988 porque tem **multiplas features redundantes** (media, desvio, max, min, amplitude e picos). Mesmo quando `peaks` se torna pouco informativo (poucos pontos passam de 110), o desvio padrao e a amplitude continuam sinalizando a classe de risco. O LIF cai para 0.82 porque depende exclusivamente da **contagem de spikes**, e com picos curtos a integracao + leak nem sempre ultrapassa o limiar dentro de uma janela de 60 pontos.

**Vantagens estruturais do LIF**:

- representacao temporal natural (event-driven), aderente ao paradigma neuromorfico discutido no Cap 3 da Fase 3 (TrueNorth, Loihi);
- baixo consumo energetico em hardware dedicado, util em wearables com bateria limitada;
- decisoes baseadas em eventos podem ser combinadas com latencia muito baixa, ideal para alertas em tempo real na borda.

**Limitacoes estruturais do LIF (modelo academico desta entrega)**:

- uma unica feature (contagem de spikes) e fragilidade ao ruido;
- depende fortemente de calibracao de `threshold`, `leak` e `scale`. Mudar `scale` de 35 para 25 ja altera a contagem;
- sem aprendizado de pesos sinapticos (modelo esta sem STDP ou SLAYER), nao se adapta a paciente individual;
- na pratica, redes spiking modernas (SNN) usam tipicamente camadas e codificacoes mais ricas (rate, time-to-first-spike).

**Vantagens da regressao logistica neste experimento**:

- robusta com poucas amostras;
- pesos interpretaveis - permite ao profissional clinico saber qual feature dispara o alerta;
- treina e infere em milissegundos, com requisitos minimos.

**Limitacoes da regressao logistica**:

- e linear em features ja extraidas: para sinais reais com nao linearidades complexas (ECG patologico, fibrilacao atrial), seria necessario engenharia de features pesada ou um modelo nao linear;
- nao explora a temporalidade da serie;
- tende a overfit em datasets sinteticos com sinais demasiado faceis (como o cenario facil deste relatorio).

## 6. Implicacoes para a CardioIA

A entrega evidencia que, para o estagio atual do CardioIA Conectada:

1. um classificador tradicional ja e suficiente como linha de base de monitoramento de risco com dados tabulares;
2. um modelo neuromorfico ainda nao esta pronto para substituir, mas serve como prova de conceito para o caminho de inteligencia na borda em wearables com hardware spiking;
3. validacao em dados reais (por exemplo, **MIT-BIH Arrhythmia Database** ou **PhysioNet 2017**) e indispensavel antes de qualquer uso assistencial.

## 7. Reprodutibilidade

```bash
pip install numpy pandas scikit-learn matplotlib notebook
python -m notebook notebooks/ir_alem2_series_temporais_saude.ipynb
# ou de forma headless
python scripts/compute_ir_alem2_metrics.py
```

## 8. Entregaveis relacionados

- Notebook: `notebooks/ir_alem2_series_temporais_saude.ipynb`.
- Script reprodutivel: `scripts/compute_ir_alem2_metrics.py`.
- README: `https://github.com/marcofiap/CardioIA-Fase3-Cap1`.
- Video YouTube nao listado: `INSERIR_LINK_DO_VIDEO`.

## 9. Limitacoes do experimento

- dados sinteticos nao substituem ECG real;
- LIF aqui e didatico, sem aprendizado online ou STDP;
- ausencia de validacao cruzada k-fold (foi usado apenas hold-out estratificado 75/25);
- ausencia de teste estatistico de significancia entre os F1 dos modelos;
- ausencia de revisao por profissional de saude;
- ambos os modelos rodam em CPU, sem benchmark energetico real.

## 10. Trabalhos futuros

- usar dados reais (MIT-BIH, PhysioNet);
- comparar com SNN treinada via SLAYER ou snntorch;
- avaliar consumo energetico em hardware como Loihi 2 ou simuladores;
- adicionar validacao cruzada estratificada e teste de McNemar;
- integrar este modelo de IA com o stream MQTT da Parte 2, exibindo a classificacao em tempo real no Node-RED ou em Grafana Cloud.
