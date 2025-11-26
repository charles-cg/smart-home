#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
//Sensor de distancia
#include "Adafruit_VL53L0X.h"  //Importa libreria
	
#define photoRes A2
#define rainSensor A1

int rainValue = 0;
float rainVoltage = 0.0;
int rainVPercentage = 0;

int photoValue = 0;
float photoVoltage = 0.0;

Adafruit_VL53L0X lox = Adafruit_VL53L0X();  //Declara el objeto para usar el sensor

char ssid[] = "Tec-IoT";    // your network SSID (name)
char pass[] = "spotless.magnetic.bridge";    // your network password 

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "test.mosquitto.org"; //IP address of the EMQX broker.
int        port     = 1883;

const char subscribe_topic_dist[]  = "EQ8/dist/test/log";
const char subscribe_topic_light[] = "EQ8/light/test/log";
const char subscribe_topic_rain[] = "EQ8/rain/test/log";

const char publish_topic_dist[]  = "EQ8/dist/test/message";
const char publish_topic_light[] = "EQ8/light/test/message";
const char publish_topic_rain[] = "EQ8/rain/test/message";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //Monitor serial
  lox.begin();
  analogReadResolution(14);     
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

  Serial.print("Subscribing to topics: ");
  Serial.println(subscribe_topic_dist);
  Serial.println(subscribe_topic_light);
  Serial.println(subscribe_topic_rain);
  // subscribe to a topic
  mqttClient.subscribe(subscribe_topic_dist);
  mqttClient.subscribe(subscribe_topic_light);
  mqttClient.subscribe(subscribe_topic_rain);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Waiting for messages on s: ");
  Serial.println(subscribe_topic_dist);
  Serial.println(subscribe_topic_light);
  Serial.println(subscribe_topic_light);
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

  photoValue = analogRead(photoRes);
  Serial.print("Digital Value is: ");
  Serial.println(photoValue);
  photoVoltage = ((photoValue * 5.0)/ 16384);
  Serial.print("Voltaje es: ");
  Serial.println(photoVoltage);

  rainValue = analogRead(rainSensor);
  Serial.print("Digital Value is: ");
  Serial.println(rainValue);
  Serial.print("Voltaje es: ");
  rainVoltage = ((rainValue * 5.0)/ 16384);
  Serial.println(rainVoltage);
  rainVPercentage = map(rainValue, 16383,0 , 0, 100); // 0 = está mojado, 16383 = está seco
  Serial.print("Porcentaje de mojado es: ");
  Serial.print(rainVPercentage);
  Serial.println("%");
  Serial.println("------");

  delay(200);

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

  mqttClient.beginMessage(publish_topic_dist);
  mqttClient.println(measure.RangeMilliMeter);
  mqttClient.endMessage();

  mqttClient.beginMessage(publish_topic_light);
  mqttClient.println(photoVoltage);
  mqttClient.endMessage();

  mqttClient.beginMessage(publish_topic_rain);
  mqttClient.println(rainVPercentage);
  mqttClient.endMessage();
}