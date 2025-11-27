#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
//Sensor de distancia
#include "Adafruit_VL53L0X.h"  //Importa libreria

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Create BME280 object
Adafruit_BME280 bme;

// Variables to store sensor readings
float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

#define MQ2_PIN A0  // Analog pin for MQ2 sensor

const float RL_VALUE = 5.0;
const float RO_CLEAN_AIR_FACTOR = 9.83;  // Sensor resistance in clean air / RO

const float VCC = 5.0;


float sensor_volt;
float RS_gas;      // Sensor resistance in gas
float ratio;       // RS_gas / RO
float RO = 10.0;   // Sensor resistance in clean air 
	
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

const char subscribe_topic_dist[]  = "EQ8/dist/log";
const char subscribe_topic_light[] = "EQ8/light/log";
const char subscribe_topic_rain[] = "EQ8/rain/log";
const char subscribe_topic_smoke[] = "EQ8/smoke/log";
const char subscribe_topic_temp[] = "EQ8/temp/log";
const char subscribe_topic_press[] = "EQ8/press/log";
const char subscribe_topic_humid[] = "EQ8/humid/log";

const char publish_topic_dist[]  = "EQ8/dist/message";
const char publish_topic_light[] = "EQ8/light/message";
const char publish_topic_rain[] = "EQ8/rain/message";
const char publish_topic_smoke[] = "EQ8/smoke/message";
const char publish_topic_temp[] = "EQ8/temp/message";
const char publish_topic_press[] = "EQ8/press/message";
const char publish_topic_humid[] = "EQ8/humid/message";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //Monitor serial
  lox.begin();
  analogReadResolution(14);  
  
  Serial.println("MQ2 Smoke Sensor Initialization");
  Serial.println("Warming up sensor (60 seconds)...");
  
  delay(60000);
  
  Serial.println("Calibrating sensor in clean air...");
  RO = calibrateSensor();
  Serial.print("Calibration complete. RO = ");
  Serial.print(RO);
  Serial.println(" kOhm");
  Serial.println("Starting measurements...");
  Serial.println();

    Serial.println("BME280 Sensor Test");
  Serial.println();

  // Initialize I2C
  Wire.begin();

  // Try to initialize BME280 at address 0x76
  bool status = bme.begin(0x76);
  
  // If not found at 0x76, try 0x77
  if (!status) {
    Serial.println("BME280 not found at 0x76, trying 0x77...");
    status = bme.begin(0x77);
  }

  if (!status) {
    Serial.println("Could not find a valid BME280 sensor!");
    Serial.println("Check wiring and I2C address.");
    while (1) delay(10);
  }

  Serial.println("BME280 sensor found and initialized!");
  Serial.println();
  
  // Configure sensor settings (optional)
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating mode
                  Adafruit_BME280::SAMPLING_X2,      // Temperature oversampling
                  Adafruit_BME280::SAMPLING_X16,     // Pressure oversampling
                  Adafruit_BME280::SAMPLING_X1,      // Humidity oversampling
                  Adafruit_BME280::FILTER_X16,       // Filtering
                  Adafruit_BME280::STANDBY_MS_500);  // Standby time

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
  Serial.println(subscribe_topic_smoke);
  Serial.println(subscribe_topic_temp);
  Serial.println(subscribe_topic_press);
  Serial.println(subscribe_topic_humid);
  // subscribe to a topic
  mqttClient.subscribe(subscribe_topic_dist);
  mqttClient.subscribe(subscribe_topic_light);
  mqttClient.subscribe(subscribe_topic_rain);
  mqttClient.subscribe(subscribe_topic_smoke);
  mqttClient.subscribe(subscribe_topic_temp);
  mqttClient.subscribe(subscribe_topic_press);
  mqttClient.subscribe(subscribe_topic_humid);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Waiting for messages on s: ");
  Serial.println(subscribe_topic_dist);
  Serial.println(subscribe_topic_light);
  Serial.println(subscribe_topic_light);
  Serial.println(subscribe_topic_smoke);
  Serial.println(subscribe_topic_temp);
  Serial.println(subscribe_topic_press);
  Serial.println(subscribe_topic_humid);
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

  // Check if measurement is valid
  int distance_mm = 0;
  if (measure.RangeStatus != 4) {  // 4 means out of range
    distance_mm = measure.RangeMilliMeter;
  } else {
    distance_mm = -1;  // Invalid reading
    Serial.println("Distance sensor out of range");
  }
  
  Serial.print("Distance: ");
  Serial.print(distance_mm);
  Serial.println(" mm");

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

  // Read analog value from sensor
  int sensorValue = analogRead(MQ2_PIN);
  
  // Convert to voltage using 14-bit resolution (16384 max)
  sensor_volt = (float)sensorValue / 16384.0 * VCC;
  
  // Prevent division by zero
  if (sensor_volt <= 0.1) {
    sensor_volt = 0.1;
  }
  
  // Calculate RS (sensor resistance in gas)
  RS_gas = ((VCC * RL_VALUE) / sensor_volt) - RL_VALUE;
  
  // Ensure positive value
  if (RS_gas < 0) {
    RS_gas = 0.1;
  }
  
  // Calculate ratio RS/RO
  if (RO <= 0) {
    Serial.println("Error: Invalid RO value. Recalibrate sensor.");
    RO = 1.0;  // Set default instead of returning
  }
  
  ratio = RS_gas / RO;
  
  float smoke_ppm = calculatePPM_Smoke(ratio);
  
  Serial.print("Smoke PPM: ");
  Serial.println(smoke_ppm);

  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;  // Convert Pa to hPa

  // Check if readings are valid
  if (isnan(temperature) || isnan(humidity) || isnan(pressure)) {
    Serial.println("Failed to read from BME280 sensor!");
    return;
  }

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
  delay(2000);
  Serial.println("sending to mqtt!");

  // Only send valid distance readings
  if (distance_mm > 0) {
    mqttClient.beginMessage(publish_topic_dist);
    mqttClient.println(distance_mm);
    mqttClient.endMessage();
  }

  delay(2000);
  mqttClient.beginMessage(publish_topic_light);
  mqttClient.println(photoVoltage);
  mqttClient.endMessage();

  delay(2000);
  mqttClient.beginMessage(publish_topic_rain);
  mqttClient.println(rainVPercentage);
  mqttClient.endMessage();

  delay(2000);
  mqttClient.beginMessage(publish_topic_smoke);
  mqttClient.println(smoke_ppm);
  mqttClient.endMessage();

  delay(2000);
  mqttClient.beginMessage(publish_topic_temp);
  mqttClient.println(temperature);
  mqttClient.endMessage();

  delay(2000);
  mqttClient.beginMessage(publish_topic_press);
  mqttClient.println(pressure);
  mqttClient.endMessage();

  delay(2000);
  mqttClient.beginMessage(publish_topic_humid);
  mqttClient.println(humidity);
  mqttClient.endMessage();
}

// Calibrate sensor in clean air
float calibrateSensor() {
  float val = 0;
  
  // Take average of 50 readings
  for (int i = 0; i < 50; i++) {
    int sensorValue = analogRead(MQ2_PIN);
    // Use 14-bit resolution (16384 max) since you set analogReadResolution(14)
    float sensor_volt = (float)sensorValue / 16384.0 * VCC;
    
    // Prevent division by zero
    if (sensor_volt <= 0.1) {
      sensor_volt = 0.1;
    }
    
    float RS_air = ((VCC * RL_VALUE) / sensor_volt) - RL_VALUE;
    
    // Ensure positive values
    if (RS_air < 0) {
      RS_air = 0.1;
    }
    
    val += RS_air;
    delay(100);
  }
  
  val = val / 50.0;  // Calculate average
  val = val / RO_CLEAN_AIR_FACTOR;  // Divide by RO factor
  
  // Ensure a valid minimum value
  if (val <= 0) {
    val = 1.0;  // Default fallback value
  }
  
  return val;
}

// Calculate Smoke concentration in PPM
float calculatePPM_Smoke(float ratio) {
  // Smoke curve for MQ2: approximately ppm = 800 * pow(ratio, -2.70)
  return 800.0 * pow(ratio, -2.70);
}