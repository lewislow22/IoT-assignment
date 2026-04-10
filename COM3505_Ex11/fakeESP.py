import requests
import time

while True:
    data = {
        "temperature": 20 + time.time() % 5,
        "led": "blink"
    }

    requests.post("http://127.0.0.1:5000/data", json=data)
    time.sleep(1)