#include "LoraReciever.h"
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
   

WiFiClient mqttIPClientWifi;
PubSubClient mqttIPClient( mqttIPClientWifi );
const String deviceId = "PLANT_1";

#include <mqtt.h>



void mqttCallback(char* topic, byte* payload, unsigned int length){
  String parameter = getCommand(deviceId, topic, payload, length);
    if (parameter == "execute"){
       Serial.println("feed rabbit method");
       // hacer algo?
  }
}

void setup()
{
  Serial.begin(9600);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  mqttIPClient.setServer(mqtt_ip, mqtt_ip_port);
  mqttIPClient.setCallback(mqttCallback);
  bootUp(deviceId, &mqttIPClient, mqtt_ip, mqtt_ip_port, mqtt_ip_user, mqtt_ip_password, mqtt_ip_topic_subscribe, mqtt_ip_topic, MQTT_RETRYMS);
}

void loop()
{
  mqttLoop(&mqttIPClient, mqtt_ip_user, mqtt_ip_password, mqtt_ip_topic_subscribe, mqtt_ip_topic, MQTT_RETRYMS);
}
