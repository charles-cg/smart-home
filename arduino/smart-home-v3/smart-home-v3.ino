// Smart Home v3 - Incremental Implementation
// Step 6: Add rain sensor and servo motor roof

#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include <Servo.h>

// Pin definitions
const int PIN_BUZZER = 13;     // Buzzer for alarms
const int PIN_BTN_SEGURIDAD = 9;  // Security button
const int PIN_BTN_GARAGE = 7;     // Garage door button
const int PIN_BTN_LED1 = 5;       // Button for LED 1
const int PIN_BTN_LED2 = 6;       // Button for LED 2
const int PIN_LED1 = 3;           // LED output 1 (series of LEDs)
const int PIN_LED2 = 4;           // LED output 2 (series of LEDs)
const int PIN_LASER_EMITTER = 12; // Laser module (KY-008)
const int PIN_LDR = A0;        // Photoresistor (light sensor)
const int PIN_MQ2 = A2;        // MQ2 smoke/gas sensor (kitchen)
const int PIN_RAIN = A1;       // Rain sensor
const int PIN_LASER_SENSOR = A3;  // Photoresistor (laser receiver)
const int PIN_SERVO_TECHO = 10; // Servo motor for roof
const int PIN_SERVO_GARAGE = 11; // Servo motor for garage door

// WiFi credentials
char ssid[] = "Nihon";
char pass[] = "Ximena10@";

// MQTT setup
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "test.mosquitto.org";
int port = 1883;
const char topic[] = "EQ8/sensors/data";

// BMP280 sensor (temperature + pressure)
Adafruit_BMP280 bmp;
// AHT20 sensor (temperature + humidity)
Adafruit_AHTX0 aht;
// VL53L0X distance sensor (garage)
Adafruit_VL53L0X vl53 = Adafruit_VL53L0X();

// OLED display (main status screen)
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire);

// LCD (pantalla de la cava en la cocina)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Servo for roof
Servo servoTecho;
// Servo for garage door
Servo servoGarage;

float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;
int smokeValue = 0;
float smoke_ppm = 0.0;
int rainValue = 0;
int rainPercentage = 0;
int ldrValue = 0;
int luminosityPercentage = 0;
int distanceMm = 0;            // Distance reading from VL53L0X
int baselineDistance = 0;      // Baseline distance (floor)
bool garageAbierta = false;    // Garage door state (false = closed, true = open)
bool garageManualMode = false; // True if garage opened manually by button
unsigned long garageDetectionTime = 0; // Time when car was detected in manual mode

// Wine cellar ideal ranges
const float TEMP_MIN = 10.0;   // °C - minimum ideal temperature
const float TEMP_MAX = 25.0;   // °C - maximum ideal temperature
const float HUMID_MIN = 40.0;  // % - minimum ideal humidity
const float HUMID_MAX = 80.0;  // % - maximum ideal humidity

// Smoke sensor threshold
const float MQ2_UMBRAL_HUMO = 400.0;  // >400 PPM = smoke/gas detected

// Rain sensor threshold
const int RAIN_UMBRAL_MOJADO = 65;  // >50% wet = rain detected (close roof)

// Light sensor threshold
const int LDR_UMBRAL_SOL_ALTO = 15;  // >70% luminosity = high sun (close roof)

// Security laser sensor threshold
const int LASER_UMBRAL_CORTE = 5000;  // <5000 = laser beam interrupted (red laser on photoresistor reads ~8000-12000)

// Distance sensor threshold for garage
const int DIST_UMBRAL_COCHE_MM = 10;  // >300mm change from baseline = car detected

// MQ2 smoke sensor constants for PPM calculation
const float RL_VALUE = 5.0;
const float RO_CONSTANT = 10.0;  // Calibrated resistance
const float VCC = 5.0;

// Alarm state variables
bool alarmaTempActiva = false;
bool alarmaHumidActiva = false;
bool humoDetectado = false;
bool techoCerrado = false;  // Roof state (false = open, true = closed)
bool modoSeguridad = false;   // Security mode ON/OFF
bool intrusoDetectado = false; // Intruder detected flag
bool lastBtnSeguridad = HIGH;  // Last security button state
bool lastBtnGarage = HIGH;     // Last garage button state
bool lastBtnLed1 = HIGH;       // Last LED 1 button state
bool lastBtnLed2 = HIGH;       // Last LED 2 button state
bool led1Encendido = false;    // LED 1 state (OFF at startup)
bool led2Encendido = false;    // LED 2 state (OFF at startup)
unsigned long lastAlarmaCellarBeep = 0;
const unsigned long CELLAR_ALARM_INTERVAL = 30000;  // 30 seconds

// Timing for sending messages
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 2000;  // Send every 2 seconds

// Timing for LCD updates
unsigned long lastLCDUpdate = 0;
const unsigned long LCD_UPDATE_MS = 1000;  // Update every 1 second

// Function declarations
void actualizarOLED();
void actualizarLCDCava();
void verificarCondicionesCava();
void verificarHumo();
void verificarLluvia();
void verificarSeguridad();
void verificarDistancia();
void abrirTecho();
void cerrarTecho();
void abrirGarage();
void cerrarGarage();
void beepAlarma();
void beepAlarmaHumo();
void beepAlarmaIntruso();
float calculatePPM_Smoke(float ratio);

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("\n======================");
  Serial.println("Smart Home v3 - Step 6");
  Serial.println("WiFi, MQTT, Sensors, LCD, OLED, Smoke & Rain+Roof");
  Serial.println("======================\n");
  
  // Initialize buzzer pin
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Initialize security system pins
  pinMode(PIN_BTN_SEGURIDAD, INPUT_PULLUP);
  pinMode(PIN_BTN_GARAGE, INPUT_PULLUP);
  pinMode(PIN_BTN_LED1, INPUT_PULLUP);
  pinMode(PIN_BTN_LED2, INPUT_PULLUP);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  digitalWrite(PIN_LED1, LOW);  // LEDs off at startup
  digitalWrite(PIN_LED2, LOW);
  pinMode(PIN_LASER_EMITTER, OUTPUT);
  digitalWrite(PIN_LASER_EMITTER, LOW);  // Laser off at startup
  Serial.println("Security system pins configured");
  
  // Initialize servo
  Serial.println("Initializing roof servo...");
  servoTecho.attach(PIN_SERVO_TECHO);
  servoTecho.write(90);  // Stop (90 = stop for continuous rotation servo)
  Serial.println("Roof servo initialized");
  
  // Initialize garage servo
  Serial.println("Initializing garage servo...");
  servoGarage.attach(PIN_SERVO_GARAGE);
  servoGarage.write(90);  // Stop
  Serial.println("Garage servo initialized");
  
  // Set analog resolution
  Serial.println("Setting analog resolution...");
  analogReadResolution(14);
  Serial.println("Analog resolution set");
  
  // Initialize I2C
  Serial.println("Initializing I2C...");
  Wire.begin();
  delay(100);
  Serial.println("I2C started");
  
  // Initialize OLED
  Serial.println("Initializing OLED...");
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: Could not find OLED display!");
  } else {
    Serial.println("OLED initialized successfully!");
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(F("Smart Home v3"));
    oled.println(F("Iniciando..."));
    oled.display();
  }
  
  // Initialize LCD
  Serial.println("Initializing LCD...");
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cava de vino");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  Serial.println("LCD initialized");
  
  // Initialize BMP280 (pressure + temperature)
  Serial.println("Initializing BMP280...");
  bool bmpStatus = bmp.begin(0x76);
  if (!bmpStatus) {
    Serial.println("BMP280 not found at 0x76, trying 0x77...");
    bmpStatus = bmp.begin(0x77);
  }
  
  if (!bmpStatus) {
    Serial.println("ERROR: Could not find BMP280 sensor!");
    Serial.println("Check wiring: SDA, SCL, VCC, GND");
  } else {
    Serial.println("BMP280 initialized successfully!");
    
    // Configure BMP280
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
  }
  
  // Initialize AHT20 (humidity + temperature)
  Serial.println("Initializing AHT20...");
  if (!aht.begin()) {
    Serial.println("ERROR: Could not find AHT20 sensor!");
    Serial.println("Check wiring: SDA, SCL, VCC, GND");
  } else {
    Serial.println("AHT20 initialized successfully!");
  }
  
  // Initialize VL53L0X distance sensor (garage)
  Serial.println("Initializing VL53L0X distance sensor...");
  if (!vl53.begin()) {
    Serial.println("ERROR: Could not find VL53L0X sensor!");
    Serial.println("Check wiring: SDA, SCL, VCC, GND");
  } else {
    Serial.println("VL53L0X initialized successfully!");
    
    // Calibrate baseline distance (measure floor distance)
    delay(500);
    VL53L0X_RangingMeasurementData_t measure;
    vl53.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      baselineDistance = measure.RangeMilliMeter;
      Serial.print("Baseline distance calibrated: ");
      Serial.print(baselineDistance);
      Serial.println(" mm");
    } else {
      baselineDistance = 2000;  // Default 2 meters if calibration fails
      Serial.println("Baseline calibration failed, using default 2000mm");
    }
  }
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  int attempts = 0;
  while (WiFi.begin(ssid, pass) != WL_CONNECTED && attempts < 10) {
    Serial.print(".");
    attempts++;
    delay(5000);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
    return;
  }
  
  // Connect to MQTT broker
  Serial.print("Connecting to MQTT broker: ");
  Serial.println(broker);
  
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code: ");
    Serial.println(mqttClient.connectError());
  } else {
    Serial.println("MQTT connected!");
  }
  
  Serial.println("\nSetup complete. Starting to send messages...\n");
}

void loop() {
  // Reconnect MQTT if disconnected
  if (!mqttClient.connected()) {
    Serial.println("MQTT disconnected. Reconnecting...");
    if (mqttClient.connect(broker, port)) {
      Serial.println("MQTT reconnected!");
    } else {
      Serial.print("Reconnection failed. Error: ");
      Serial.println(mqttClient.connectError());
      delay(5000);
      return;
    }
  }
  
  // Send message every SEND_INTERVAL
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = millis();
    
    // Read BMP280 sensor (temperature + pressure)
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure() / 100.0F;  // Convert Pa to hPa
    
    // Read AHT20 sensor (humidity + temperature)
    sensors_event_t humidity_event, temp_event;
    aht.getEvent(&humidity_event, &temp_event);
    humidity = humidity_event.relative_humidity;
    
    // Use AHT20 temperature if BMP280 fails (AHT20 is more accurate for temp)
    if (isnan(temperature) || temperature < -40 || temperature > 85) {
      temperature = temp_event.temperature;
    }
    
    // Validate readings
    if (isnan(temperature) || temperature < -40 || temperature > 85) {
      temperature = 0.0;
    }
    if (isnan(humidity) || humidity < 0 || humidity > 100) {
      humidity = 0.0;
    }
    if (isnan(pressure) || pressure < 300 || pressure > 1100) {
      pressure = 0.0;
    }
    
    // Read MQ2 smoke sensor
    smokeValue = analogRead(PIN_MQ2);
    
    // Calculate smoke PPM
    float sensor_volt = (float)smokeValue / 16384.0 * VCC;
    if (sensor_volt <= 0.1) sensor_volt = 0.1;
    
    float RS_gas = ((VCC * RL_VALUE) / sensor_volt) - RL_VALUE;
    if (RS_gas < 0) RS_gas = 0.1;
    
    float ratio = RS_gas / RO_CONSTANT;
    smoke_ppm = calculatePPM_Smoke(ratio);
    
    // Read rain sensor
    rainValue = analogRead(PIN_RAIN);
    rainPercentage = map(rainValue, 16383, 0, 0, 100);  // 0 = wet, 100 = dry
    
    // Read light sensor (LDR)
    ldrValue = analogRead(PIN_LDR);
    luminosityPercentage = map(ldrValue, 0, 16383, 0, 100);  // 0 = dark, 100 = bright
    
    // Display readings in Serial Monitor
    Serial.println("--- Sensor Readings ---");
    Serial.print("Temperature (BMP280): ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity (AHT20): ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Pressure (BMP280): ");
    Serial.print(pressure);
    Serial.println(" hPa");
    Serial.print("Smoke (MQ2): ");
    Serial.print(smoke_ppm, 2);
    Serial.println(" PPM");
    Serial.print("Rain: ");
    Serial.print(rainValue);
    Serial.print(" | Wet: ");
    Serial.print(rainPercentage);
    Serial.println("%");
    Serial.print("Light: ");
    Serial.print(ldrValue);
    Serial.print(" | Luminosity: ");
    Serial.print(luminosityPercentage);
    Serial.println("%");
    
    // Create JSON message with sensor data
    Serial.println("Sending message to MQTT...");
    
    mqttClient.beginMessage(topic);
    mqttClient.print("{");
    mqttClient.print("\"distance\":");
    mqttClient.print(distanceMm);
    mqttClient.print(",\"light\":");
    mqttClient.print(luminosityPercentage);
    mqttClient.print(",\"rain\":");
    mqttClient.print(rainPercentage);
    mqttClient.print(",\"smoke\":");
    mqttClient.print(smoke_ppm, 2);
    mqttClient.print(",\"temperature\":");
    mqttClient.print(temperature, 2);
    mqttClient.print(",\"pressure\":");
    mqttClient.print(pressure, 2);
    mqttClient.print(",\"humidity\":");
    mqttClient.print(humidity, 2);
    mqttClient.print(",\"roof_closed\":");
    mqttClient.print(techoCerrado ? "true" : "false");
    mqttClient.print(",\"garage_open\":");
    mqttClient.print(garageAbierta ? "true" : "false");
    mqttClient.print(",\"led1\":");
    mqttClient.print(led1Encendido ? "true" : "false");
    mqttClient.print(",\"led2\":");
    mqttClient.print(led2Encendido ? "true" : "false");
    mqttClient.print(",\"uptime\":");
    mqttClient.print(millis() / 1000);
    mqttClient.print("}");
    mqttClient.endMessage();
    
    Serial.println("Message sent!");
    Serial.println("----------------------\n");
  }
  
  // Check wine cellar conditions
  verificarCondicionesCava();
  
  // Check smoke levels
  verificarHumo();
  
  // Check security system
  verificarSeguridad();
  
  // Check garage button
  bool btnGarage = (digitalRead(PIN_BTN_GARAGE) == LOW);
  if (btnGarage && !lastBtnGarage) {
    lastBtnGarage = btnGarage;  // Update state immediately to prevent re-triggering
    // Toggle garage door
    if (garageAbierta) {
      Serial.println("Button pressed: Closing garage...");
      cerrarGarage();
      garageAbierta = false;
      garageManualMode = false;
      garageDetectionTime = 0;
    } else {
      Serial.println("Button pressed: Opening garage (manual mode)...");
      abrirGarage();
      garageAbierta = true;
      garageManualMode = true;  // Mark as manually opened
      garageDetectionTime = 0;  // Reset detection timer
    }
  } else {
    lastBtnGarage = btnGarage;
  }
  
  // Check LED 1 button
  bool btnLed1 = (digitalRead(PIN_BTN_LED1) == LOW);
  if (btnLed1 && !lastBtnLed1) {
    led1Encendido = !led1Encendido;
    digitalWrite(PIN_LED1, led1Encendido ? HIGH : LOW);
    Serial.print("LED 1 ");
    Serial.println(led1Encendido ? "ON" : "OFF");
  }
  lastBtnLed1 = btnLed1;
  
  // Check LED 2 button
  bool btnLed2 = (digitalRead(PIN_BTN_LED2) == LOW);
  if (btnLed2 && !lastBtnLed2) {
    led2Encendido = !led2Encendido;
    digitalWrite(PIN_LED2, led2Encendido ? HIGH : LOW);
    Serial.print("LED 2 ");
    Serial.println(led2Encendido ? "ON" : "OFF");
  }
  lastBtnLed2 = btnLed2;
  
  // Check distance sensor for garage door
  verificarDistancia();
  
  // Check rain and control roof
  verificarLluvia();
  
  // Update OLED
  actualizarOLED();
  
  // Update LCD every second
  if (millis() - lastLCDUpdate >= LCD_UPDATE_MS) {
    lastLCDUpdate = millis();
    actualizarLCDCava();
  }
  
  delay(100);
}

// Function to update LCD with sensor readings
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
  
  // Show alarm indicator if conditions are out of range
  if (alarmaTempActiva || alarmaHumidActiva || humoDetectado) {
    lcd.setCursor(14, 1);
    lcd.print("!!");
  }
}

// Function to check wine cellar conditions and trigger alarm
void verificarCondicionesCava() {
  bool tempFueraRango = (temperature < TEMP_MIN || temperature > TEMP_MAX);
  bool humidFueraRango = (humidity < HUMID_MIN || humidity > HUMID_MAX);
  bool alarmaActiva = tempFueraRango || humidFueraRango;

  // Update alarm states
  alarmaTempActiva = tempFueraRango;
  alarmaHumidActiva = humidFueraRango;

  // Beep every 30 seconds if conditions are out of range
  if (alarmaActiva) {
    // If first detection or 30 seconds have passed since last beep
    if (lastAlarmaCellarBeep == 0 || (millis() - lastAlarmaCellarBeep >= CELLAR_ALARM_INTERVAL)) {
      lastAlarmaCellarBeep = millis();
      
      if (tempFueraRango) {
        Serial.println("ALERT: Temperature out of range for wine cellar!");
        Serial.print("Current: ");
        Serial.print(temperature);
        Serial.print("°C | Ideal: ");
        Serial.print(TEMP_MIN);
        Serial.print("-");
        Serial.print(TEMP_MAX);
        Serial.println("°C");
      }
      if (humidFueraRango) {
        Serial.println("ALERT: Humidity out of range for wine cellar!");
        Serial.print("Current: ");
        Serial.print(humidity);
        Serial.print("% | Ideal: ");
        Serial.print(HUMID_MIN);
        Serial.print("-");
        Serial.print(HUMID_MAX);
        Serial.println("%");
      }
      
      beepAlarma();
    }
  } else {
    // Reset timer when conditions return to normal
    lastAlarmaCellarBeep = 0;
  }
}

// Function to update OLED with system status
void actualizarOLED() {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  
  // Priority 1: Intruder alarm
  if (intrusoDetectado) {
    oled.setTextSize(2);
    oled.println(F("ALERTA:"));
    oled.println(F("Intruso"));
    oled.setTextSize(1);
    oled.println();
    oled.print(F("Seguridad"));
  }
  // Priority 2: Smoke alarm
  else if (humoDetectado) {
    oled.setTextSize(2);
    oled.println(F("ALERTA:"));
    oled.println(F("Humo"));
    oled.setTextSize(1);
    oled.println();
    oled.print(F("Cocina"));
  }
  // Priority 3: Wine cellar alarm
  else if (alarmaTempActiva || alarmaHumidActiva) {
    oled.setTextSize(2);
    oled.println(F("ALERTA:"));
    oled.println(F("Cava"));
    oled.setTextSize(1);
    if (alarmaTempActiva) {
      oled.print(F("Temp: "));
      oled.print(temperature, 1);
      oled.println(F("C"));
    }
    if (alarmaHumidActiva) {
      oled.print(F("Hum: "));
      oled.print(humidity, 1);
      oled.println(F("%"));
    }
  }
  // Normal status - show all readings
  else {
    oled.println(F("Smart Home v3"));
    oled.println();
    oled.print(F("Temp: "));
    oled.print(temperature, 1);
    oled.println(F(" C"));
    
    oled.print(F("Hum:  "));
    oled.print(humidity, 1);
    oled.println(F(" %"));
    
    oled.print(F("Pres: "));
    oled.print(pressure, 1);
    oled.println(F(" hPa"));
    
    oled.print(F("Humo: "));
    oled.print(smoke_ppm, 1);
    oled.print(F(" ppm"));
  }
  
  oled.display();
}

// Function to check smoke levels and trigger alarm
void verificarHumo() {
  bool humoAlto = (smoke_ppm > MQ2_UMBRAL_HUMO);
  
  // If smoke detected for the first time
  if (humoAlto && !humoDetectado) {
    humoDetectado = true;
    Serial.println("ALERT: Smoke detected in kitchen!");
    Serial.print("Smoke PPM: ");
    Serial.println(smoke_ppm);
    beepAlarmaHumo();
  }
  
  // Clear flag when smoke level returns to normal
  if (!humoAlto) {
    humoDetectado = false;
  }
}

// Function for wine cellar alarm beep (short, less annoying)
void beepAlarma() {
  tone(PIN_BUZZER, 1800, 200);  // 1800 Hz for 200ms
  delay(250);
}

// Function for smoke alarm (longer, more urgent pattern)
void beepAlarmaHumo() {
  for (int i = 0; i < 2; i++) {
    tone(PIN_BUZZER, 1800, 800);  // Long beep (800ms)
    delay(900);
  }
}

// Function for intruder alarm (continuous until deactivated)
void beepAlarmaIntruso() {
  tone(PIN_BUZZER, 2500, 200);  // High pitched short beep
  delay(250);
}

// Function to check rain/sun and control roof automatically
void verificarSeguridad() {
  // Read security button (toggle security mode)
  bool btnSeguridad = (digitalRead(PIN_BTN_SEGURIDAD) == LOW);
  
  // Detect button press (rising edge)
  if (btnSeguridad && !lastBtnSeguridad) {
    modoSeguridad = !modoSeguridad;  // Toggle security mode
    
    // Control laser based on security mode
    if (modoSeguridad) {
      digitalWrite(PIN_LASER_EMITTER, HIGH);  // Turn on laser
      intrusoDetectado = false;  // Reset intruder flag
      Serial.println("Security mode ON - Laser activated");
    } else {
      digitalWrite(PIN_LASER_EMITTER, LOW);   // Turn off laser
      intrusoDetectado = false;  // Reset intruder flag
      noTone(PIN_BUZZER);  // Stop alarm if active
      Serial.println("Security mode OFF - Laser deactivated");
    }
  }
  lastBtnSeguridad = btnSeguridad;
  
  // Only check for intrusion if security mode is active
  if (!modoSeguridad) {
    return;
  }
  
  // Read photoresistor (laser receiver)
  int laserValor = analogRead(PIN_LASER_SENSOR);
  
  // Debug: Show laser sensor value periodically
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000) {  // Every 2 seconds
    Serial.print("Laser sensor value: ");
    Serial.print(laserValor);
    Serial.print(" | Beam interrupted: ");
    Serial.println(laserValor < LASER_UMBRAL_CORTE ? "YES" : "NO");
    lastDebug = millis();
  }
  
  // Check if laser beam is interrupted
  bool hazInterrumpido = (laserValor < LASER_UMBRAL_CORTE);
  
  // If beam interrupted, activate alarm
  if (hazInterrumpido) {
    if (!intrusoDetectado) {
      intrusoDetectado = true;
      Serial.println("ALERT: Intruder detected! Laser beam interrupted!");
      Serial.print("Sensor reading: ");
      Serial.println(laserValor);
    }
    // Sound alarm continuously while beam is interrupted
    beepAlarmaIntruso();
  } else {
    // Beam restored but keep intruder flag (only reset by button)
    if (intrusoDetectado) {
      beepAlarmaIntruso();  // Keep beeping even if beam restored
    }
  }
}

// Function to check distance sensor and control garage door
void verificarDistancia() {
  // Read distance sensor
  VL53L0X_RangingMeasurementData_t measure;
  vl53.rangingTest(&measure, false);
  
  // Check if reading is valid
  if (measure.RangeStatus != 4) {  // 4 = out of range
    distanceMm = measure.RangeMilliMeter;
  } else {
    distanceMm = baselineDistance;  // Use baseline if out of range
  }
  
  // Debug: Show distance reading periodically
  static unsigned long lastDistDebug = 0;
  if (millis() - lastDistDebug > 2000) {  // Every 2 seconds
    Serial.print("Distance: ");
    Serial.print(distanceMm);
    Serial.print(" mm | Baseline: ");
    Serial.print(baselineDistance);
    Serial.print(" mm | Change: ");
    Serial.print(baselineDistance - distanceMm);
    Serial.println(" mm");
    lastDistDebug = millis();
  }
  
  // Calculate distance change from baseline (floor level)
  int distanceChange = baselineDistance - distanceMm;
  
  // If distance decreased significantly from baseline, car is present
  bool cocheCerca = (distanceChange > DIST_UMBRAL_COCHE_MM);
  
  // If garage is in manual mode (opened by button)
  if (garageManualMode && garageAbierta) {
    // Check if car is detected
    if (cocheCerca) {
      // Start or update the timer when car is detected
      if (garageDetectionTime == 0) {
        garageDetectionTime = millis();
        Serial.println("Car detected in manual mode. Will close in 2 seconds...");
      }
      // Check if 2 seconds have passed since detection
      if (millis() - garageDetectionTime >= 2000) {
        Serial.println("2 seconds elapsed. Closing garage...");
        cerrarGarage();
        garageAbierta = false;
        garageManualMode = false;
        garageDetectionTime = 0;
      }
    } else {
      // No car detected, reset timer
      garageDetectionTime = 0;
    }
  }
  // Normal automatic mode (not manually opened)
  else if (!garageManualMode) {
    // Open garage door when car detected
    if (cocheCerca && !garageAbierta) {
      Serial.print("Car detected! Distance change: ");
      Serial.print(distanceChange);
      Serial.println(" mm. Opening garage...");
      abrirGarage();
      garageAbierta = true;
    }
    // Close garage door when car leaves
    else if (!cocheCerca && garageAbierta) {
      Serial.print("Car left. Distance change: ");
      Serial.print(distanceChange);
      Serial.println(" mm. Closing garage...");
      cerrarGarage();
      garageAbierta = false;
    }
  }
}

// Function to check rain/sun and control roof automatically
// Priority: Rain > Sun > Default (open)
void verificarLluvia() {
  // Don't control roof if security system is active
  if (modoSeguridad) {
    return;
  }
  
  bool hayLluvia = (rainPercentage > RAIN_UMBRAL_MOJADO);
  bool muchoSol = (luminosityPercentage > LDR_UMBRAL_SOL_ALTO);
  
  // Priority 1: Rain detected - close roof
  if (hayLluvia && !techoCerrado) {
    Serial.print("Rain detected (wet: ");
    Serial.print(rainPercentage);
    Serial.println("%)! Closing roof...");
    cerrarTecho();
    techoCerrado = true;
  }
  // Priority 2: High sun and no rain - close roof
  else if (!hayLluvia && muchoSol && !techoCerrado) {
    Serial.print("High sun detected (luminosity: ");
    Serial.print(luminosityPercentage);
    Serial.println("%)! Closing roof...");
    cerrarTecho();
    techoCerrado = true;
  }
  // No rain and no high sun - open roof
  else if (!hayLluvia && !muchoSol && techoCerrado) {
    Serial.print("No rain or high sun (wet: ");
    Serial.print(rainPercentage);
    Serial.print("%, luminosity: ");
    Serial.print(luminosityPercentage);
    Serial.println("%). Opening roof...");
    abrirTecho();
    techoCerrado = false;
  }
}

// Function to open roof (continuous rotation servo)
void abrirTecho() {
  servoTecho.write(80);   // Rotate counterclockwise with even more power (further from 90)
  delay(5600);            // Time for 180 degrees rotation + 2 seconds
  servoTecho.write(90);   // Stop (90 = stop for continuous rotation servos)
}

// Function to close roof (continuous rotation servo)
void cerrarTecho() {
  servoTecho.write(115);  // Rotate slowly clockwise (90-180 = clockwise)
  delay(4600);            // Time for 180 degrees rotation + 1 second
  servoTecho.write(90);   // Stop (90 = stop)
}

// Function to open garage door (continuous rotation servo)
void abrirGarage() {
  servoGarage.write(80);  // Rotate clockwise with more torque
  delay(3500);             // 4 seconds rotation time
  servoGarage.write(90);   // Stop
}

// Function to close garage door (continuous rotation servo)
void cerrarGarage() {
  servoGarage.write(100);   // Rotate counterclockwise with more torque
  delay(4500);             // 4 seconds rotation time
  servoGarage.write(90);   // Stop
}

// Calculate Smoke concentration in PPM
float calculatePPM_Smoke(float ratio) {
  // Smoke curve for MQ2: approximately ppm = 800 * pow(ratio, -2.70)
  return 800.0 * pow(ratio, -2.70);
}