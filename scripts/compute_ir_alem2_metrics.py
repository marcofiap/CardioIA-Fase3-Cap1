"""Replica o experimento Ir Alem 2 em dois cenarios e imprime metricas para o relatorio.

Cenario "facil" reproduz o dataset original com classes bem separadas; cenario
"dificil" reduz amplitude dos picos e injeta mais ruido para revelar diferencas
entre regressao logistica e o modelo LIF simples.
"""

from __future__ import annotations

import json
from typing import Callable

import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import (
    accuracy_score,
    classification_report,
    confusion_matrix,
    f1_score,
    precision_score,
    recall_score,
    roc_auc_score,
)
from sklearn.model_selection import train_test_split

SEED = 57


def make_generator(spike_amp_mean: float, spike_amp_std: float, base_noise: float, drift_mean: float) -> Callable:
    rng = np.random.default_rng(SEED)

    def _generate(n_samples: int = 400, length: int = 60) -> tuple[np.ndarray, np.ndarray]:
        x: list[np.ndarray] = []
        y: list[int] = []
        t = np.linspace(0, 2 * np.pi, length)
        for i in range(n_samples):
            label = i >= n_samples // 2
            base = 75 + 5 * np.sin(t * 3) + rng.normal(0, base_noise, length)
            if label:
                spikes = rng.choice(length, size=5, replace=False)
                base += rng.normal(drift_mean, 5, length)
                base[spikes] += rng.normal(spike_amp_mean, spike_amp_std, len(spikes))
            x.append(base)
            y.append(int(label))
        return np.array(x), np.array(y)

    return _generate


def extract_features(series: np.ndarray) -> np.ndarray:
    peaks = (series > 110).sum(axis=1)
    return np.column_stack(
        [
            series.mean(axis=1),
            series.std(axis=1),
            series.max(axis=1),
            series.min(axis=1),
            series.max(axis=1) - series.min(axis=1),
            peaks,
        ]
    )


def lif_spike_count(series: np.ndarray, threshold: float = 1.0, leak: float = 0.85, scale: float = 35.0) -> np.ndarray:
    centered = (series - 70) / scale
    counts: list[int] = []
    for row in centered:
        v = 0.0
        spikes = 0
        for value in row:
            v = leak * v + max(value, 0)
            if v >= threshold:
                spikes += 1
                v = 0.0
        counts.append(spikes)
    return np.array(counts)


def evaluate(generator: Callable) -> dict:
    X, y = generator()

    features = extract_features(X)
    x_train, x_test, y_train, y_test = train_test_split(features, y, test_size=0.25, random_state=SEED, stratify=y)
    clf = LogisticRegression(max_iter=1000)
    clf.fit(x_train, y_train)
    pred_lr = clf.predict(x_test)
    proba_lr = clf.predict_proba(x_test)[:, 1]

    x_train_s, x_test_s, y_train_s, y_test_s = train_test_split(X, y, test_size=0.25, random_state=SEED, stratify=y)
    train_spikes = lif_spike_count(x_train_s)
    test_spikes = lif_spike_count(x_test_s)

    candidate_thresholds = np.arange(train_spikes.min(), train_spikes.max() + 1)
    best_threshold = max(
        candidate_thresholds,
        key=lambda th: accuracy_score(y_train_s, train_spikes >= th),
    )
    pred_lif = (test_spikes >= best_threshold).astype(int)

    return {
        "logistic": {
            "accuracy": round(float(accuracy_score(y_test, pred_lr)), 4),
            "precision": round(float(precision_score(y_test, pred_lr)), 4),
            "recall": round(float(recall_score(y_test, pred_lr)), 4),
            "f1": round(float(f1_score(y_test, pred_lr)), 4),
            "roc_auc": round(float(roc_auc_score(y_test, proba_lr)), 4),
            "confusion_matrix": confusion_matrix(y_test, pred_lr).tolist(),
        },
        "lif": {
            "best_threshold": int(best_threshold),
            "accuracy": round(float(accuracy_score(y_test_s, pred_lif)), 4),
            "precision": round(float(precision_score(y_test_s, pred_lif)), 4),
            "recall": round(float(recall_score(y_test_s, pred_lif)), 4),
            "f1": round(float(f1_score(y_test_s, pred_lif)), 4),
            "confusion_matrix": confusion_matrix(y_test_s, pred_lif).tolist(),
        },
    }


def main() -> None:
    facil = make_generator(spike_amp_mean=35, spike_amp_std=8, base_noise=2.0, drift_mean=8)
    dificil = make_generator(spike_amp_mean=8, spike_amp_std=3, base_noise=6.0, drift_mean=0)
    metrics = {
        "facil": evaluate(facil),
        "dificil": evaluate(dificil),
    }
    print(json.dumps(metrics, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
