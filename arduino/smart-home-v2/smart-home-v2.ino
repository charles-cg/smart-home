// Librerías
#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
#include <Wire.h>               // Habilita el bus I2C
#include <Adafruit_Sensor.h>    // Librería base de sensores Adafruit
#include <Adafruit_BME280.h>    // Librería del sensor BME280
#include <LiquidCrystal_I2C.h>  // Librería para LCD por I2C (16x2)
#include <Servo.h>              // Librería para controlar servos

// WiFi y MQTT
char ssid[] = "Tec-IoT";
char pass[] = "spotless.magnetic.bridge";
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "test.mosquitto.org";
int port = 1883;

// LCD (pantalla de la cava en la cocina)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensor BME280
Adafruit_BME280 bme;

// Variables para lecturas de sensores
float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

// Sensor de humo MQ2
const int PIN_MQ2 = A2;

// Sensor de lluvia y servo del techo
const int PIN_RAIN = A1;              // Sensor de lluvia
const int PIN_SERVO_TECHO = 10;       // Servo del techo
const int RAIN_UMBRAL_MOJADO = 600;   // Umbral para detectar lluvia

// Servo del techo (rotación continua)
Servo servoTecho;
bool techoCerrado = true;       // Estado del techo

int rainValue = 0;
float rainVoltage = 0.0;
int rainPercentage = 0;

// Sistema de seguridad con láser
const int PIN_LASER_EMITTER = 8;    // Emisor láser HW-493
const int PIN_LASER_SENSOR = A3;    // Fotoresistor que recibe el láser
const int PIN_BTN_SEGURIDAD = 12;    // Botón para activar/desactivar seguridad

// Constantes MQ2
const float RL_VALUE = 5.0;
const float RO_CONSTANT = 10.0;  // Resistencia constante calibrada
const float VCC = 5.0;
float sensor_volt;
float RS_gas;
float ratio;
float smoke_ppm = 0.0;

// Pin del buzzer para alarma
const int PIN_BUZZER = 13;

// Umbrales ideales para cava de vino
const float TEMP_MIN = 10.0;   // °C - temperatura mínima ideal
const float TEMP_MAX = 25.0;   // °C - temperatura máxima ideal
const float HUMID_MIN = 40.0;  // % - humedad mínima ideal
const float HUMID_MAX = 80.0;  // % - humedad máxima ideal

// Umbral para humo
const float SMOKE_UMBRAL_PPM = 400.0;  // PPM de humo para alarma

// Umbral para sistema de seguridad
const int LASER_UMBRAL_CORTE = 300;  // Umbral para detección de interrupción del haz láser

// Variables de estado
bool alarmaTempActiva = false;
bool alarmaHumidActiva = false;
bool alarmaHumoActiva = false;
bool modoSeguridad = false;
bool intrusoDetectado = false;
bool lastBtnSeguridad = HIGH;

unsigned long lastAlarmaCellarBeep = 0;
const unsigned long CELLAR_ALARM_INTERVAL = 30000;  // 30 segundos

// Tiempos
unsigned long lastLCDUpdate = 0;
const unsigned long LCD_UPDATE_MS = 1000;
unsigned long lastMQTTSend = 0;
const unsigned long MQTT_SEND_INTERVAL = 1000;

// Declaración de funciones
void actualizarLCDCava();
void verificarCondicionesCava();
void verificarHumo();
void verificarLluvia();
void verificarSeguridad();
void abrirTecho();
void cerrarTecho();
void beepAlarma();
void beepAlarmaHumo();
void beepAlarmaIntruso();
float calculatePPM_Smoke(float ratio);
void enviarDatosViaJSON();

void setup() {
  Serial.begin(9600);
  delay(1000);  // Esperar a que el Serial Monitor se conecte
  
  Serial.println("\n\n======================");
  Serial.println("Smart Home v2 Starting...");
  Serial.println("======================\n");
  
  // Configuración de pines
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LASER_EMITTER, OUTPUT);
  pinMode(PIN_BTN_SEGURIDAD, INPUT_PULLUP);
  digitalWrite(PIN_LASER_EMITTER, LOW);  // Láser apagado al inicio
  
  Serial.println("Pins configured");

  // Inicializar servo del techo
  Serial.println("Initializing servo...");
  servoTecho.attach(PIN_SERVO_TECHO);
  servoTecho.write(90);  // Iniciar detenido (90 = stop para servo continuo)
  Serial.println("Servo initialized");

  // Set analog resolution
  Serial.println("Setting analog resolution...");
  analogReadResolution(14);
  Serial.println("Analog resolution set");

  // Inicia I2C
  Serial.println("Starting I2C...");
  Wire.begin();
  delay(100);
  Serial.println("I2C started");

  // Inicializar LCD
  Serial.println("Initializing LCD...");
  lcd.init();        // Inicializar LCD
  lcd.backlight();   // Encender backlight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cava de vino");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  Serial.println("LCD initialized");

  // Inicializar BME280
  Serial.println("Scanning for BME280...");
  delay(500);
  
  bool status = bme.begin(0x76);
  if (!status) {
    Serial.println("BME280 not found at 0x76, trying 0x77...");
    delay(500);
    status = bme.begin(0x77);
  }
  
  if (!status) {
    Serial.println("Could not find BME280 sensor!");
    Serial.println("Check wiring: SDA, SCL, VCC, GND");
    Serial.println("Try running I2C scanner to find address");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR: BME280");
    lcd.setCursor(0, 1);
    lcd.print("No detectado");
    delay(3000);
  } else {
    Serial.println("BME280 OK");
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_500);
  }

  // Conectar WiFi
  Serial.print("Attempting to connect to WiFi: ");
  Serial.println(ssid);
  int wifiAttempts = 0;
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    wifiAttempts++;
    if (wifiAttempts > 10) {
      Serial.println("\nWiFi connection failed after 10 attempts, continuing anyway...");
      break;
    }
    delay(5000);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nYou're connected to the network");
  }

  // Conectar MQTT
  Serial.print("Attempting to connect to MQTT broker: ");
  Serial.println(broker);
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
  } else {
    Serial.println("You're connected to the MQTT broker!");
  }

  delay(2000);
}

void loop() {
  // Reconectar MQTT si es necesario
  if (!mqttClient.connected()) {
    Serial.println("MQTT connection lost. Attempting to reconnect...");
    if (mqttClient.connect(broker, port)) {
      Serial.println("Reconnected to MQTT broker!");
    }
  }

  // Leer botón de seguridad
  bool btnSeguridad = (digitalRead(PIN_BTN_SEGURIDAD) == LOW);
  if (btnSeguridad && !lastBtnSeguridad) {
    modoSeguridad = !modoSeguridad;
    
    // Controlar láser según modo seguridad
    if (modoSeguridad) {
      digitalWrite(PIN_LASER_EMITTER, HIGH);  // Encender láser
      intrusoDetectado = false;  // Reset estado de intruso
      Serial.println("Security mode ON - Laser activated");
    } else {
      digitalWrite(PIN_LASER_EMITTER, LOW);   // Apagar láser
      intrusoDetectado = false;  // Reset estado de intruso
      Serial.println("Security mode OFF - Laser deactivated");
    }
  }
  lastBtnSeguridad = btnSeguridad;

  // Leer BME280
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  
  // Validar lecturas
  if (isnan(temperature) || temperature < -40 || temperature > 85) {
    temperature = 0.0;
  }
  if (isnan(humidity) || humidity < 0 || humidity > 100) {
    humidity = 0.0;
  }
  if (isnan(pressure) || pressure < 300 || pressure > 1100) {
    pressure = 0.0;
  }

  // Mostrar lecturas en Serial
  Serial.println("--- BME280 Readings ---");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  // Leer sensor de humo MQ2
  int mq2Valor = analogRead(PIN_MQ2);
  sensor_volt = (float)mq2Valor / 16384.0 * VCC;
  if (sensor_volt <= 0.1) sensor_volt = 0.1;
  
  RS_gas = ((VCC * RL_VALUE) / sensor_volt) - RL_VALUE;
  if (RS_gas < 0) RS_gas = 0.1;
  
  ratio = RS_gas / RO_CONSTANT;
  smoke_ppm = calculatePPM_Smoke(ratio);

  Serial.print("Smoke PPM: ");
  Serial.println(smoke_ppm);

  // Leer sensor de lluvia
  rainValue = analogRead(PIN_RAIN);
  rainVoltage = ((rainValue * 5.0) / 16384);
  rainPercentage = map(rainValue, 16383, 0, 0, 100);  // 0 = mojado, 100 = seco

  Serial.print("Rain value: ");
  Serial.print(rainValue);
  Serial.print(" | Wet percentage: ");
  Serial.print(rainPercentage);
  Serial.println("%");
  Serial.println("----------------------");

  // Verificar condiciones de la cava
  verificarCondicionesCava();
  
  // Verificar humo
  verificarHumo();
  
  // Verificar lluvia (techo automático)
  verificarLluvia();
  
  // Verificar sistema de seguridad
  verificarSeguridad();

  // Actualizar LCD cada segundo
  if (millis() - lastLCDUpdate >= LCD_UPDATE_MS) {
    lastLCDUpdate = millis();
    actualizarLCDCava();
  }

  // Enviar datos vía MQTT cada segundo
  if (millis() - lastMQTTSend >= MQTT_SEND_INTERVAL) {
    lastMQTTSend = millis();
    enviarDatosViaJSON();
  }

  delay(50);
}

// Funciones auxiliares

void verificarCondicionesCava() {
  bool tempFueraRango = (temperature < TEMP_MIN || temperature > TEMP_MAX);
  bool humidFueraRango = (humidity < HUMID_MIN || humidity > HUMID_MAX);
  bool alarmaActiva = tempFueraRango || humidFueraRango;

  // Actualizar estados
  alarmaTempActiva = tempFueraRango;
  alarmaHumidActiva = humidFueraRango;

  // Beep solo una vez cuando se detecta por primera vez
  if (alarmaActiva && lastAlarmaCellarBeep == 0) {
    lastAlarmaCellarBeep = millis();
    
    if (tempFueraRango) {
      Serial.println("ALERTA: Temperatura fuera de rango!");
    }
    if (humidFueraRango) {
      Serial.println("ALERTA: Humedad fuera de rango!");
    }
    
    beepAlarma();
  } else if (!alarmaActiva) {
    // Reset timer cuando vuelve a rango normal
    lastAlarmaCellarBeep = 0;
  }
}

void verificarHumo() {
  bool humoAlto = (smoke_ppm > SMOKE_UMBRAL_PPM);

  if (humoAlto && !alarmaHumoActiva) {
    alarmaHumoActiva = true;
    Serial.println("ALERTA: Humo detectado en cocina!");
    beepAlarmaHumo();
  } else if (!humoAlto) {
    alarmaHumoActiva = false;
  }
}

void verificarLluvia() {
  bool hayLluvia = (rainValue < RAIN_UMBRAL_MOJADO);
  
  // Si detecta lluvia y el techo está abierto, cerrarlo
  if (hayLluvia && !techoCerrado) {
    Serial.println("Lluvia detectada! Cerrando techo...");
    cerrarTecho();
    techoCerrado = true;
  }
  // Si no hay lluvia y el techo está cerrado, abrirlo
  else if (!hayLluvia && techoCerrado) {
    Serial.println("No hay lluvia. Abriendo techo...");
    abrirTecho();
    techoCerrado = false;
  }
}

void abrirTecho() {
  // Para servo de rotación continua: girar en una dirección por tiempo fijo
  servoTecho.write(100);  // Girar lentamente en una dirección (90-180 = clockwise)
  delay(3600);            // Tiempo para 180 grados
  servoTecho.write(90);   // Detener (90 = stop para servos continuos)
}

void cerrarTecho() {
  // Para servo de rotación continua: girar en dirección opuesta
  servoTecho.write(80);   // Girar lentamente en dirección contraria (0-90 = counterclockwise)
  delay(3600);            // Tiempo para volver 180 grados
  servoTecho.write(90);   // Detener (90 = stop)
}

void verificarSeguridad() {
  if (!modoSeguridad) {
    intrusoDetectado = false;
    return;
  }
  
  // Leer fotoresistor
  int laserValor = analogRead(PIN_LASER_SENSOR);
  bool hazInterrumpido = (laserValor < LASER_UMBRAL_CORTE);
  
  // Debug: Mostrar valor del sensor láser
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000) {  // Cada 2 segundos
    Serial.print("Laser sensor value: ");
    Serial.print(laserValor);
    Serial.print(" | Beam interrupted: ");
    Serial.println(hazInterrumpido ? "YES" : "NO");
    lastDebug = millis();
  }
  
  // Detectar intruso
  if (hazInterrumpido && !intrusoDetectado) {
    intrusoDetectado = true;
    Serial.println("ALERTA: Intruso detectado!");
    beepAlarmaIntruso();
  }
}

void beepAlarma() {
  tone(PIN_BUZZER, 1800, 200);
  delay(250);
}

void beepAlarmaHumo() {
  for (int i = 0; i < 2; i++) {
    tone(PIN_BUZZER, 2000, 800);
    delay(900);
  }
}

void beepAlarmaIntruso() {
  for (int i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 2500, 200);
    delay(250);
  }
}

float calculatePPM_Smoke(float ratio) {
  return 800.0 * pow(ratio, -2.70);
}

void actualizarLCDCava() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  if (temperature != 0.0) {
    lcd.print(temperature, 1);
    lcd.print("C ");
  } else {
    lcd.print("--.-C ");
  }

  lcd.print("H:");
  if (humidity != 0.0) {
    lcd.print(humidity, 1);
    lcd.print("%");
  } else {
    lcd.print("--%");
  }

  lcd.setCursor(0, 1);
  lcd.print("P:");
  if (pressure != 0.0) {
    lcd.print(pressure, 1);
    lcd.print("hPa");
  } else {
    lcd.print("----hPa");
  }
  
  // Mostrar estado de alarma
  if (alarmaTempActiva || alarmaHumidActiva || alarmaHumoActiva) {
    lcd.setCursor(14, 1);
    lcd.print("!!");
  }
}

void enviarDatosViaJSON() {
  if (!mqttClient.connected()) return;

  Serial.println("Sending data to MQTT...");
  
  mqttClient.beginMessage("EQ8/sensors/data");
  mqttClient.print("{");
  mqttClient.print("\"temperature\":");
  mqttClient.print(temperature, 2);
  mqttClient.print(",\"pressure\":");
  mqttClient.print(pressure, 2);
  mqttClient.print(",\"humidity\":");
  mqttClient.print(humidity, 2);
  mqttClient.print(",\"smoke\":");
  mqttClient.print(smoke_ppm, 2);
  mqttClient.print(",\"rain\":");
  mqttClient.print(rainPercentage);
  mqttClient.print(",\"roof_closed\":");
  mqttClient.print(techoCerrado ? "true" : "false");
  mqttClient.print(",\"temp_alarm\":");
  mqttClient.print(alarmaTempActiva ? "true" : "false");
  mqttClient.print(",\"humid_alarm\":");
  mqttClient.print(alarmaHumidActiva ? "true" : "false");
  mqttClient.print(",\"smoke_alarm\":");
  mqttClient.print(alarmaHumoActiva ? "true" : "false");
  mqttClient.print(",\"security_mode\":");
  mqttClient.print(modoSeguridad ? "true" : "false");
  mqttClient.print(",\"intruder_detected\":");
  mqttClient.print(intrusoDetectado ? "true" : "false");
  mqttClient.print("}");
  mqttClient.endMessage();
}
