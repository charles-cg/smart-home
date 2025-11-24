#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
//Sensor de distancia
#include "Adafruit_VL53L0X.h"  //Importa libreria
	

Adafruit_VL53L0X lox = Adafruit_VL53L0X();  //Declara el objeto para usar el sensor

char ssid[] = "Tec-IoT";    // your network SSID (name)
char pass[] = "spotless.magnetic.bridge";    // your network password 

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "broker.mqtt.cool"; //IP address of the EMQX broker.
int        port     = 1883;
const char subscribe_topic[]  = "/EQ8/test/log";
const char publish_topic[]  = "/EQ8/test/message";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //Monitor serial
  lox.begin();     
  while (!Serial) {
    ; 
  }

  // Connect to WiFi
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    delay(5000);
    return;
  }

  Serial.println("You're connected to the MQTT broker!");

  Serial.print("Subscribing to topic: ");
  Serial.println(subscribe_topic);
  // subscribe to a topic
  mqttClient.subscribe(subscribe_topic);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(subscribe_topic);
    //Inicia el sensor
}

void loop() {
  // Check if MQTT is still connected
  if (!mqttClient.connected()) {
    Serial.println("MQTT connection lost. Attempting to reconnect...");
    if (!mqttClient.connect(broker, port)) {
      Serial.print("Reconnection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      delay(5000);
      return;
    }
    Serial.println("Reconnected to MQTT broker!");
  }

	VL53L0X_RangingMeasurementData_t measure;  //Estructura donde se guardara la medicion
	lox.rangingTest(&measure, false);          //Obtiene una medicion desde el sensor

  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    // use the Stream interface to print the contents
    while (mqttClient.available()) {
      Serial.print((char)mqttClient.read());
    }
    Serial.println();
  }

  // send message, the Print interface can be used to set the message contents
  delay(3000);
  Serial.println("sending to mqtt!");

  mqttClient.beginMessage(publish_topic);
  mqttClient.print("Distance in (mm)");
  mqttClient.println(measure.RangeMilliMeter);
  mqttClient.endMessage();
}