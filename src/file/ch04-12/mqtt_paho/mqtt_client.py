# !/usr/bin/env python3
# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import time

mqtt_client = mqtt.Client()

# 连接成功回调
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected with result code " + str(rc))
        # 连接成功后订阅主题
        client.subscribe("/topic/temperature")
    else:
        print(f"Failed to connect, return code {rc}")

# 接收到消息回调
def on_message(client, userdata, msg):
    print(f"Received message on topic '{msg.topic}': {msg.payload.decode()}")

mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

broker_address = "localhost"
broker_port = 1883
mqtt_client.connect(broker_address, broker_port)

mqtt_client.loop_start()

try:
    while True:
        # 发送消息
        mqtt_client.publish("test/topic", "Hello World!")
        time.sleep(5)
except KeyboardInterrupt:
    print("Disconnecting from MQTT broker...")
    mqtt_client.loop_stop()
    mqtt_client.disconnect()
