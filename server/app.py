import os
import threading
import time
from typing import Optional

from flask import Flask, jsonify
import paho.mqtt.client as mqtt


app = Flask(__name__)

MQTT_HOST = os.getenv("MQTT_HOST", "localhost")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_USERNAME = os.getenv("MQTT_USERNAME")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD")
MQTT_KEEPALIVE = int(os.getenv("MQTT_KEEPALIVE", "60"))
MQTT_CLIENT_ID = os.getenv("MQTT_CLIENT_ID", "hkvlib-rest")


class MqttService:
    def __init__(self) -> None:
        self.client = mqtt.Client(client_id=MQTT_CLIENT_ID, clean_session=True)
        if MQTT_USERNAME:
            self.client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self._connected = False
        self._last_error: Optional[str] = None

    def _on_connect(self, _client, _userdata, _flags, rc):
        self._connected = rc == 0
        self._last_error = None if self._connected else f"connect failed rc={rc}"

    def _on_disconnect(self, _client, _userdata, rc):
        self._connected = False
        if rc != 0:
            self._last_error = f"disconnect rc={rc}"

    def start(self) -> None:
        self.client.connect_async(MQTT_HOST, MQTT_PORT, MQTT_KEEPALIVE)
        self.client.loop_start()

    def stop(self) -> None:
        self.client.loop_stop()
        self.client.disconnect()

    def is_connected(self) -> bool:
        return self._connected

    def last_error(self) -> Optional[str]:
        return self._last_error


mqtt_service = MqttService()


@app.get("/heartbeat")
def heartbeat():
    return "OK", 200


@app.get("/health")
def health():
    payload = {
        "status": "ok",
        "mqtt_connected": mqtt_service.is_connected(),
        "mqtt_host": MQTT_HOST,
        "mqtt_port": MQTT_PORT,
        "mqtt_error": mqtt_service.last_error(),
    }
    return jsonify(payload), 200


def run_http() -> None:
    host = os.getenv("HTTP_HOST", "0.0.0.0")
    port = int(os.getenv("HTTP_PORT", "8080"))
    app.run(host=host, port=port, debug=False)


if __name__ == "__main__":
    mqtt_service.start()
    try:
        run_http()
    finally:
        mqtt_service.stop()
