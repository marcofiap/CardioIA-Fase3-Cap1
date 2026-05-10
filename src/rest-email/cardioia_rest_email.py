from __future__ import annotations

import json
import os
import smtplib
from dataclasses import dataclass
from email.message import EmailMessage
from typing import Any

import httpx


@dataclass
class VitalReading:
    patient_id: str
    bpm: int
    temperature: float
    humidity: float
    movement: int
    accel_magnitude: float | None = None

    def to_dict(self) -> dict[str, Any]:
        payload: dict[str, Any] = {
            "patientId": self.patient_id,
            "bpm": self.bpm,
            "temperature": self.temperature,
            "humidity": self.humidity,
            "movement": self.movement,
        }
        if self.accel_magnitude is not None:
            payload["accelMagnitude"] = self.accel_magnitude
        return payload


class CardioIaApiClient:
    def __init__(self, base_url: str, client: httpx.Client | None = None) -> None:
        self.base_url = base_url.rstrip("/")
        self.client = client or httpx.Client(timeout=10)

    def send_reading(self, reading: VitalReading) -> dict[str, Any]:
        response = self.client.post(f"{self.base_url}/vitals", json=reading.to_dict())
        response.raise_for_status()
        return response.json()

    def get_latest_reading(self, patient_id: str) -> dict[str, Any]:
        response = self.client.get(f"{self.base_url}/patients/{patient_id}/latest")
        response.raise_for_status()
        return response.json()


def evaluate_risk(reading: dict[str, Any]) -> dict[str, Any]:
    bpm = int(reading["bpm"])
    temperature = float(reading["temperature"])
    movement = int(reading["movement"])
    reasons: list[str] = []

    if bpm >= 120:
        reasons.append("taquicardia")
    elif bpm >= 100:
        reasons.append("frequencia cardiaca elevada")

    if temperature >= 38.0:
        reasons.append("febre")

    if movement == 0:
        reasons.append("ausencia de movimento")

    if "taquicardia" in reasons or ("febre" in reasons and "ausencia de movimento" in reasons):
        level = "alto"
        recommendation = "acionar equipe de saude e revisar paciente imediatamente"
    elif reasons:
        level = "moderado"
        recommendation = "acompanhar tendencia e repetir leitura"
    else:
        level = "baixo"
        recommendation = "manter monitoramento continuo"

    return {
        "riskLevel": level,
        "reasons": reasons,
        "recommendation": recommendation,
    }


def send_alert_email(reading: dict[str, Any], risk: dict[str, Any]) -> None:
    alert_to = os.getenv("ALERT_TO", "equipe.cardioia@example.com")
    alert_from = os.getenv("ALERT_FROM", os.getenv("SMTP_USER", "cardioia@example.com"))

    message = EmailMessage()
    message["Subject"] = f"CardioIA - alerta {risk['riskLevel']} para {reading['patientId']}"
    message["From"] = alert_from
    message["To"] = alert_to
    message.set_content(
        "\n".join(
            [
                "Alerta automatizado CardioIA",
                "",
                f"Paciente: {reading['patientId']}",
                f"BPM: {reading['bpm']}",
                f"Temperatura: {reading['temperature']} C",
                f"Umidade: {reading['humidity']}%",
                f"Movimento: {reading['movement']}",
                f"Nivel de risco: {risk['riskLevel']}",
                f"Motivos: {', '.join(risk['reasons']) or 'sem motivos criticos'}",
                f"Recomendacao: {risk['recommendation']}",
            ]
        )
    )

    smtp_host = os.getenv("SMTP_HOST")
    smtp_user = os.getenv("SMTP_USER")
    smtp_password = os.getenv("SMTP_PASSWORD")
    smtp_port = int(os.getenv("SMTP_PORT", "587"))

    if not smtp_host or not smtp_user or not smtp_password:
        print("\n--- SIMULACAO DE E-MAIL ---")
        print(message)
        print("--- FIM DA SIMULACAO ---\n")
        return

    with smtplib.SMTP(smtp_host, smtp_port) as server:
        server.starttls()
        server.login(smtp_user, smtp_password)
        server.send_message(message)


def build_mock_client() -> httpx.Client:
    database: dict[str, dict[str, Any]] = {}

    def handler(request: httpx.Request) -> httpx.Response:
        if request.method == "POST" and request.url.path == "/vitals":
            payload = json.loads(request.content.decode("utf-8"))
            database[payload["patientId"]] = payload
            return httpx.Response(201, json={"status": "stored", "data": payload})

        if request.method == "GET" and request.url.path.startswith("/patients/"):
            patient_id = request.url.path.split("/")[2]
            latest = database.get(patient_id)
            if latest is None:
                return httpx.Response(404, json={"error": "patient not found"})
            return httpx.Response(200, json=latest)

        return httpx.Response(404, json={"error": "route not found"})

    return httpx.Client(transport=httpx.MockTransport(handler), base_url="https://mock.cardioia.local")


def main() -> None:
    client = CardioIaApiClient("https://mock.cardioia.local", build_mock_client())
    reading = VitalReading(
        patient_id="paciente-simulado-001",
        bpm=128,
        temperature=38.4,
        humidity=54.0,
        movement=1,
        accel_magnitude=11.4,
    )

    stored = client.send_reading(reading)
    latest = client.get_latest_reading(reading.patient_id)
    risk = evaluate_risk(latest)

    print("Leitura enviada para API REST:")
    print(json.dumps(stored, indent=2, ensure_ascii=False))
    print("\nRisco calculado:")
    print(json.dumps(risk, indent=2, ensure_ascii=False))

    if risk["riskLevel"] in {"moderado", "alto"}:
        send_alert_email(latest, risk)


if __name__ == "__main__":
    main()
