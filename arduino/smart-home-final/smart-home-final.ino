// Librerías
#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
#include <Wire.h>               // Habilita el bus I2C (usado por BME280, OLED, VL53L0X, LCD I2C)
#include <Adafruit_Sensor.h>    // Librería base de sensores Adafruit
#include <Adafruit_BME280.h>    // Librería del sensor BME280
#include <Adafruit_GFX.h>       // Librería gráfica base
#include <Adafruit_SSD1306.h>   // Librería para la pantalla OLED
#include <Adafruit_VL53L0X.h>   // Librería para el sensor de distancia VL53L0X
#include <Servo.h>              // Librería para controlar servos
#include <LiquidCrystal_I2C.h>  // Librería para LCD por I2C (16x2)

// WiFi y MQTT
char ssid[] = "Nihon";
char pass[] = "Ximena10@";
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "test.mosquitto.org";
int port = 1883;

// OLED (pantalla principal)
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire);

// LCD (pantalla de la cava en la cocina)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensores
Adafruit_BME280 bme;
Adafruit_VL53L0X vl53 = Adafruit_VL53L0X();

// Variables para lecturas de sensores
float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;
int distMm = -1;
float smoke_ppm = 0.0;

// Pines analógicos para sensores (A0-A5 available)
const int PIN_LDR_EXTERIOR = A0;    // LDR que mide luz exterior (para toldo)
const int PIN_RAIN         = A1;    // Sensor de lluvia
const int PIN_MQ2          = A2;    // Sensor de humo/gas MQ-2 (cocina)
const int PIN_LASER_SENSOR = A3;    // Fotoresistor/fotodiodo que recibe el láser KY-008

// Módulo láser HW-493 (KY-008)
const int PIN_LASER_EMITTER = 6;    // Pin para controlar el emisor láser HW-493

// Constantes MQ2
const float RL_VALUE = 5.0;
const float RO_CLEAN_AIR_FACTOR = 9.83;
const float VCC = 5.0;
float sensor_volt;
float RS_gas;
float ratio;
float RO = 10.0;

int rainValue = 0;
float rainVoltage = 0.0;
int rainVPercentage = 0;
int photoValue = 0;
float photoVoltage = 0.0;

// Actuadores
const int PIN_BUZZER        = 9;
const int PIN_SERVO_TOLDO   = 10;
const int PIN_SERVO_COCHERA = 11;

Servo servoToldo;
Servo servoCochera;

// Luces: 3 pisos (floors)
const int NUM_HABITACIONES = 3;
const int PIN_LUCES[NUM_HABITACIONES] = {3, 4, 5};  // Lights for 3 floors
const int PIN_BTN_LUCES[NUM_HABITACIONES] = {2, 13, 12};  // Buttons for 3 floors

// Botones
const int PIN_BTN_SEGURIDAD  = 1;      // Security button
const int PIN_BTN_COCHERA_IN = 0;      // Cochera button - RE-ENABLED

bool lucesEncendidas[NUM_HABITACIONES] = {false, false, false};
bool lastBtnLuces[NUM_HABITACIONES];

// Umbrales
const int LDR_UMBRAL_LUZ_ALTA   = 700;
const int RAIN_UMBRAL_MOJADO    = 600;
const int MQ2_UMBRAL_HUMO       = 400;
const int LASER_UMBRAL_CORTE    = 300;
const int DIST_UMBRAL_COCHE_MM  = 800;

// Tiempos
const unsigned long COCHERA_OPEN_MS      = 15000;
const unsigned long SERVO_TOLDO_MS       = 2000;
const unsigned long SERVO_COCHERA_MS     = 2500;

// Configuración servos MG90S (estándar 180°)
const int SERVO_TOLDO_RETRAIDO   = 0;    // Toldo retraído (0 grados)
const int SERVO_TOLDO_EXTENDIDO  = 90;   // Toldo extendido (90 grados)
const int SERVO_COCHERA_CERRADA  = 0;    // Cochera cerrada (0 grados)
const int SERVO_COCHERA_ABIERTA  = 90;   // Cochera abierta (90 grados)

// Variables de estado
bool modoSeguridad     = false;
bool toldoExtendido    = false;
bool intrusoDetectado  = false;
bool humoDetectado     = false;
bool cocheraAbierta    = false;
unsigned long tInicioCochera = 0;

bool lastBtnSeguridad  = HIGH;
bool lastBtnCochera    = HIGH;  // RE-ENABLED

unsigned long lastLCDUpdate      = 0;
const unsigned long LCD_UPDATE_MS = 1000;

unsigned long lastMQTTSend = 0;
const unsigned long MQTT_SEND_INTERVAL = 1000;

// Declaración de funciones
void actualizarOLED(bool hayLluvia, bool muchaLuz);
void actualizarLCDCava();
void beepCortoSuave();
void beepAlarmaIntruso();
void beepAlarmaHumo();
void extenderToldo();
void retraerToldo();
void abrirCochera();
void cerrarCochera();
float calibrateSensor();
float calculatePPM_Smoke(float ratio);
void enviarDatosViaJSON();

void setup() {
  Serial.begin(9600);
  
  // Configuración de pines
  pinMode(PIN_BTN_SEGURIDAD, INPUT_PULLUP);
  pinMode(PIN_BTN_COCHERA_IN, INPUT_PULLUP);  // RE-ENABLED
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LASER_EMITTER, OUTPUT);  // Láser HW-493
  digitalWrite(PIN_LASER_EMITTER, LOW);  // Láser apagado al inicio

  for (int i = 0; i < NUM_HABITACIONES; i++) {
    pinMode(PIN_LUCES[i], OUTPUT);
    digitalWrite(PIN_LUCES[i], LOW);
    pinMode(PIN_BTN_LUCES[i], INPUT_PULLUP);
    lastBtnLuces[i] = HIGH;
  }

  // Inicia I2C
  Wire.begin();
  delay(100);

  // Inicializar OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("ERROR: No se detecta la OLED"));
  } else {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(F("Casa inteligente IoT"));
    oled.display();
  }

  // Inicializar LCD
  lcd.begin(16, 2);  // Inicializar con 16 columnas y 2 filas
  lcd.backlight();
  lcd.clear();
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Cava de vino");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  Serial.println("LCD initialized");

  // Inicializar VL53L0X
  if (!vl53.begin()) {
    Serial.println("Failed to initialize VL53L0X!");
  } else {
    Serial.println("VL53L0X OK");
  }

  // Inicializar BME280
  bool status = bme.begin(0x76);
  if (!status) {
    Serial.println("BME280 not found at 0x76, trying 0x77...");
    status = bme.begin(0x77);
  }
  
  if (!status) {
    Serial.println("Could not find BME280 sensor!");
  } else {
    Serial.println("BME280 OK");
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,
                    Adafruit_BME280::SAMPLING_X16,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_500);
  }

  // Set analog resolution
  analogReadResolution(14);

  // Servos
  servoToldo.attach(PIN_SERVO_TOLDO);
  servoCochera.attach(PIN_SERVO_COCHERA);
  servoToldo.write(SERVO_TOLDO_RETRAIDO);      // Iniciar retraído
  servoCochera.write(SERVO_COCHERA_CERRADA);   // Iniciar cerrado

  Serial.println("MQ2 Smoke Sensor Initialization");
  Serial.println("Warming up sensor (60 seconds)...");
  delay(60000);
  
  Serial.println("Calibrating smoke sensor...");
  RO = calibrateSensor();
  Serial.print("Calibration complete. RO = ");
  Serial.print(RO);
  Serial.println(" kOhm");

  // Conectar WiFi
  Serial.print("Attempting to connect to WiFi: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }
  Serial.println("\nYou're connected to the network");

  // Conectar MQTT
  Serial.print("Attempting to connect to MQTT broker: ");
  Serial.println(broker);
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
  } else {
    Serial.println("You're connected to the MQTT broker!");
  }

  actualizarOLED(false, false);
}

void loop() {
  // Reconectar MQTT si es necesario
  if (!mqttClient.connected()) {
    Serial.println("MQTT connection lost. Attempting to reconnect...");
    if (mqttClient.connect(broker, port)) {
      Serial.println("Reconnected to MQTT broker!");
    }
  }

  // Lectura de botones de luces
  for (int i = 0; i < NUM_HABITACIONES; i++) {
    bool btn = (digitalRead(PIN_BTN_LUCES[i]) == LOW);
    if (btn && !lastBtnLuces[i]) {
      lucesEncendidas[i] = !lucesEncendidas[i];
      digitalWrite(PIN_LUCES[i], lucesEncendidas[i] ? HIGH : LOW);
    }
    lastBtnLuces[i] = btn;
  }

  // Botón de seguridad
  bool btnSeguridad = (digitalRead(PIN_BTN_SEGURIDAD) == LOW);
  if (btnSeguridad && !lastBtnSeguridad) {
    modoSeguridad = !modoSeguridad;
    intrusoDetectado = false;
    
    // Controlar láser según modo seguridad
    if (modoSeguridad) {
      digitalWrite(PIN_LASER_EMITTER, HIGH);  // Encender láser
      Serial.println("Security mode ON - Laser activated");
    } else {
      digitalWrite(PIN_LASER_EMITTER, LOW);   // Apagar láser
      Serial.println("Security mode OFF - Laser deactivated");
    }
  }
  lastBtnSeguridad = btnSeguridad;

  // Botón de cochera - RE-ENABLED
  bool btnCochera = (digitalRead(PIN_BTN_COCHERA_IN) == LOW);
  if (btnCochera && !lastBtnCochera) {
    if (cocheraAbierta) {
      cerrarCochera();
    } else {
      abrirCochera();
    }
  }
  lastBtnCochera = btnCochera;

  // Lectura de sensores analógicos
  int ldrValor = analogRead(PIN_LDR_EXTERIOR);
  int lluviaValor = analogRead(PIN_RAIN);
  int mq2Valor = analogRead(PIN_MQ2);
  int laserValor = analogRead(PIN_LASER_SENSOR);  // Lee el photoresistor que recibe el láser

  bool hayLluvia = (lluviaValor < RAIN_UMBRAL_MOJADO);
  bool muchaLuz = (ldrValor > LDR_UMBRAL_LUZ_ALTA);
  bool hazInterrumpido = (laserValor < LASER_UMBRAL_CORTE);  // Si el valor es bajo, el haz fue interrumpido
  
  // Debug: Mostrar valor del sensor láser cuando seguridad está activa
  if (modoSeguridad) {
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 2000) {  // Cada 2 segundos
      Serial.print("Laser sensor value: ");
      Serial.print(laserValor);
      Serial.print(" | Beam interrupted: ");
      Serial.println(hazInterrumpido ? "YES" : "NO");
      lastDebug = millis();
    }
  }

  // Sensor de distancia VL53L0X
  distMm = -1;
  VL53L0X_RangingMeasurementData_t medida;
  vl53.rangingTest(&medida, false);
  if (medida.RangeStatus != 4) {
    distMm = (int)medida.RangeMilliMeter;
  }
  bool cocheCerca = (distMm > 0 && distMm < DIST_UMBRAL_COCHE_MM);

  // Calcular valores para MQTT
  photoValue = ldrValor;
  photoVoltage = ((photoValue * 5.0) / 16384);
  
  rainValue = lluviaValor;
  rainVoltage = ((rainValue * 5.0) / 16384);
  rainVPercentage = map(rainValue, 16383, 0, 0, 100);

  // MQ2 Smoke sensor
  int sensorValue = mq2Valor;
  sensor_volt = (float)sensorValue / 16384.0 * VCC;
  if (sensor_volt <= 0.1) sensor_volt = 0.1;
  
  RS_gas = ((VCC * RL_VALUE) / sensor_volt) - RL_VALUE;
  if (RS_gas < 0) RS_gas = 0.1;
  
  if (RO <= 0) RO = 1.0;
  ratio = RS_gas / RO;
  smoke_ppm = calculatePPM_Smoke(ratio);

  // BME280
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  
  if (isnan(temperature)) temperature = 0.0;
  if (isnan(humidity)) humidity = 0.0;
  if (isnan(pressure)) pressure = 0.0;

  // Lógica del toldo
  bool debeExtenderToldo = hayLluvia || muchaLuz;
  if (debeExtenderToldo && !toldoExtendido) {
    extenderToldo();
    toldoExtendido = true;
  } else if (!debeExtenderToldo && toldoExtendido) {
    retraerToldo();
    toldoExtendido = false;
  }

  // Seguridad y láser
  if (modoSeguridad && hazInterrumpido && !intrusoDetectado) {
    intrusoDetectado = true;
    beepAlarmaIntruso();
  }
  if (!modoSeguridad) {
    intrusoDetectado = false;
  }

  // Cochera automática
  if (!modoSeguridad && cocheCerca && !cocheraAbierta) {
    abrirCochera();
  }
  if (cocheraAbierta && (millis() - tInicioCochera >= COCHERA_OPEN_MS)) {
    cerrarCochera();
  }

  // Detector de humo
  bool humoAlto = (mq2Valor > MQ2_UMBRAL_HUMO);
  if (humoAlto && !humoDetectado) {
    humoDetectado = true;
    beepAlarmaHumo();
  }
  if (!humoAlto) {
    humoDetectado = false;
  }

  // Actualizar OLED
  actualizarOLED(hayLluvia, muchaLuz);

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

void enviarDatosViaJSON() {
  if (!mqttClient.connected()) return;

  Serial.println("Sending data to MQTT...");
  
  mqttClient.beginMessage("EQ8/sensors/data");
  mqttClient.print("{");
  mqttClient.print("\"distance\":");
  mqttClient.print(distMm);
  mqttClient.print(",\"light\":");
  mqttClient.print(photoVoltage, 2);
  mqttClient.print(",\"rain\":");
  mqttClient.print(rainVPercentage);
  mqttClient.print(",\"smoke\":");
  mqttClient.print(smoke_ppm, 2);
  mqttClient.print(",\"temperature\":");
  mqttClient.print(temperature, 2);
  mqttClient.print(",\"pressure\":");
  mqttClient.print(pressure, 2);
  mqttClient.print(",\"humidity\":");
  mqttClient.print(humidity, 2);
  mqttClient.print("}");
  mqttClient.endMessage();
}

float calibrateSensor() {
  float val = 0;
  for (int i = 0; i < 50; i++) {
    int sensorValue = analogRead(PIN_MQ2);
    float sensor_volt = (float)sensorValue / 16384.0 * VCC;
    if (sensor_volt <= 0.1) sensor_volt = 0.1;
    
    float RS_air = ((VCC * RL_VALUE) / sensor_volt) - RL_VALUE;
    if (RS_air < 0) RS_air = 0.1;
    
    val += RS_air;
    delay(100);
  }
  val = val / 50.0;
  val = val / RO_CLEAN_AIR_FACTOR;
  if (val <= 0) val = 1.0;
  return val;
}

float calculatePPM_Smoke(float ratio) {
  return 800.0 * pow(ratio, -2.70);
}

void beepCortoSuave() {
  tone(PIN_BUZZER, 1500, 150);
  delay(160);
}

void beepAlarmaIntruso() {
  for (int i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 2000, 200);
    delay(250);
  }
}

void beepAlarmaHumo() {
  for (int i = 0; i < 2; i++) {
    tone(PIN_BUZZER, 1800, 800);
    delay(900);
  }
}

void extenderToldo() {
  beepCortoSuave();
  // Mover lentamente de 0 a 90 grados
  for (int pos = SERVO_TOLDO_RETRAIDO; pos <= SERVO_TOLDO_EXTENDIDO; pos += 2) {
    servoToldo.write(pos);
    delay(50);  // Movimiento lento
  }
  servoToldo.write(SERVO_TOLDO_EXTENDIDO);
}

void retraerToldo() {
  // Mover lentamente de 90 a 0 grados
  for (int pos = SERVO_TOLDO_EXTENDIDO; pos >= SERVO_TOLDO_RETRAIDO; pos -= 2) {
    servoToldo.write(pos);
    delay(50);  // Movimiento lento
  }
  servoToldo.write(SERVO_TOLDO_RETRAIDO);
}

void abrirCochera() {
  // Mover lentamente de 0 a 90 grados
  for (int pos = SERVO_COCHERA_CERRADA; pos <= SERVO_COCHERA_ABIERTA; pos += 2) {
    servoCochera.write(pos);
    delay(200);  // Movimiento muy lento
  }
  servoCochera.write(SERVO_COCHERA_ABIERTA);
  cocheraAbierta = true;
  tInicioCochera = millis();
}

void cerrarCochera() {
  // Mover lentamente de 90 a 0 grados
  for (int pos = SERVO_COCHERA_ABIERTA; pos >= SERVO_COCHERA_CERRADA; pos -= 2) {
    servoCochera.write(pos);
    delay(200);  // Movimiento muy lento
  }
  servoCochera.write(SERVO_COCHERA_CERRADA);
  cocheraAbierta = false;
}

void actualizarOLED(bool hayLluvia, bool muchaLuz) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);

  if (intrusoDetectado) {
    oled.println(F("ALERTA: Intruso"));
  } else if (humoDetectado) {
    oled.println(F("ALERTA: Humo en cocina"));
  } else {
    if (hayLluvia) {
      oled.println(F("Clima: Lluvia"));
    } else {
      oled.println(F("Clima: Soleado"));
    }
  }

  oled.setCursor(0, 16);
  oled.print(F("Toldo: "));
  oled.println(toldoExtendido ? F("Extendido") : F("Contraido"));

  oled.setCursor(0, 40);
  oled.print(F("Seguridad: "));
  oled.println(modoSeguridad ? F("ON") : F("OFF"));

  oled.setCursor(0, 52);
  oled.print(F("Cochera: "));
  oled.println(cocheraAbierta ? F("Abierta") : F("Cerrada"));

  oled.display();
}

void actualizarLCDCava() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  if (!isnan(temperature)) {
    lcd.print(temperature, 1);
    lcd.print("C ");
  } else {
    lcd.print("--.-C ");
  }

  lcd.print("H:");
  if (!isnan(humidity)) {
    lcd.print(humidity, 1);
    lcd.print("%");
  } else {
    lcd.print("--%");
  }

  lcd.setCursor(0, 1);
  lcd.print("P:");
  if (!isnan(pressure)) {
    lcd.print(pressure, 1);
    lcd.print("hPa");
  } else {
    lcd.print("----hPa");
  }
}
