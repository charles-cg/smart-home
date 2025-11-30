// Librerías
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

// ======================= OLED (pantalla principal) =======================
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire);

// ======================= LCD (cava en la cocina) =========================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ======================= Sensores ===========================
Adafruit_BME280 bme;                        // BME280 cava
Adafruit_VL53L0X vl53 = Adafruit_VL53L0X(); // VL53L0X cochera

// Pines analógicos para sensores
const int PIN_LDR_EXTERIOR = A0;    // LDR exterior
const int PIN_RAIN         = A1;    // Sensor lluvia
const int PIN_MQ2          = A2;    // MQ-2 humo/gas
const int PIN_LASER_SENSOR = A3;    // Fotoresistor que recibe el láser

// Láser emisor KY-008
const int PIN_LASER_EMISOR = 6;     // <--- LÁSER EN PIN DIGITAL 6

// ======================= Actuadores ==========================
const int PIN_BUZZER        = 8;    // Buzzer
const int PIN_SERVO_TOLDO   = 10;   // Servo toldo
const int PIN_SERVO_COCHERA = 11;   // Servo cochera
const int PIN_LED_HUMO      = 7;    // LED rojo de humo (lo movimos al 7)

Servo servoToldo;
Servo servoCochera;

// ======================= Luces ===============================
const int NUM_HABITACIONES = 3;
const int PIN_LUCES[NUM_HABITACIONES]     = {3, 4, 5};       // Luces
const int PIN_BTN_LUCES[NUM_HABITACIONES] = {2, 13, 12};     // Botones luces

bool lucesEncendidas[NUM_HABITACIONES] = {false, false, false};
bool lastBtnLuces[NUM_HABITACIONES];

// ======================= Botones globales ====================
const int PIN_BTN_SEGURIDAD  = 9;   // Botón modo seguridad (OJO: RX)
const int PIN_BTN_COCHERA_IN = 0;   // Botón interior cochera (OJO: TX)

bool lastBtnSeguridad = HIGH;
bool lastBtnCochera   = HIGH;

// ======================= Umbrales ============================
const int LDR_UMBRAL_LUZ_ALTA   = 700;
const int RAIN_UMBRAL_MOJADO    = 600;
const int MQ2_UMBRAL_HUMO       = 400;
const int LASER_UMBRAL_CORTE    = 300;
const int DIST_UMBRAL_COCHE_MM  = 800;

// ======================= Tiempos =============================
const unsigned long COCHERA_OPEN_MS      = 15000;
const unsigned long SERVO_TOLDO_MS       = 2000;
const unsigned long SERVO_COCHERA_MS     = 2500;

// ======================= Servos 360° (lentos) ================
const int SERVO_STOP                 = 90;
const int SERVO_TOLDO_EXTENDER_LENTO = 87;
const int SERVO_TOLDO_RETRAER_LENTO  = 93;
const int SERVO_COCHERA_ABRIR_LENTO  = 87;
const int SERVO_COCHERA_CERRAR_LENTO = 93;

// ======================= Variables de estado =================
bool modoSeguridad     = false;
bool toldoExtendido    = false;
bool intrusoDetectado  = false;
bool humoDetectado     = false;

bool cocheraAbierta    = false;
unsigned long tInicioCochera = 0;

// Para lluvia (detectar inicio)
bool lluviaEstadoAnterior = false;

// LCD cava
unsigned long lastLCDUpdate       = 0;
const unsigned long LCD_UPDATE_MS = 1000;

// ======================= Prototipos ==========================
void actualizarOLED(bool hayLluvia, bool muchaLuz);
void actualizarLCDCava();

void beepCortoSuave();
void beepAlarmaIntruso();
void beepAlarmaHumo();

void extenderToldo();
void retraerToldo();
void abrirCochera();
void cerrarCochera();

// ======================= SETUP ===============================
void setup() {
  Serial.begin(115200);

  // Botones
  pinMode(PIN_BTN_SEGURIDAD,  INPUT_PULLUP);
  pinMode(PIN_BTN_COCHERA_IN, INPUT_PULLUP);

  // Luces y botones de luces
  for (int i = 0; i < NUM_HABITACIONES; i++) {
    pinMode(PIN_LUCES[i], OUTPUT);
    digitalWrite(PIN_LUCES[i], LOW);
    pinMode(PIN_BTN_LUCES[i], INPUT_PULLUP);
    lastBtnLuces[i] = HIGH;
  }

  // Buzzer y LED humo
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_HUMO, OUTPUT);
  digitalWrite(PIN_LED_HUMO, LOW);

  // Láser emisor
  pinMode(PIN_LASER_EMISOR, OUTPUT);
  digitalWrite(PIN_LASER_EMISOR, HIGH); // Láser encendido siempre en modo seguridad

  // I2C
  Wire.begin();

  // OLED
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

  // LCD cava
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cava de vino");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  // BME280
  bool bmeOk = bme.begin(0x76);
  if (!bmeOk) bmeOk = bme.begin(0x77);
  Serial.println(bmeOk ? F("BME280 OK") : F("BME280 NO detectado"));

  // VL53L0X
  bool vlOk = vl53.begin();
  Serial.println(vlOk ? F("VL53L0X OK") : F("VL53L0X NO detectado"));

  // Servos
  servoToldo.attach(PIN_SERVO_TOLDO);
  servoCochera.attach(PIN_SERVO_COCHERA);
  servoToldo.write(SERVO_STOP);
  servoCochera.write(SERVO_STOP);

  actualizarOLED(false, false);
}

// ======================= LOOP ================================
void loop() {
  // --- Botones globales ---
  bool btnSeguridad = (digitalRead(PIN_BTN_SEGURIDAD) == LOW);
  bool btnCochera   = (digitalRead(PIN_BTN_COCHERA_IN) == LOW);

  // --- Luces manuales ---
  for (int i = 0; i < NUM_HABITACIONES; i++) {
    bool btn = (digitalRead(PIN_BTN_LUCES[i]) == LOW);

    if (btn && !lastBtnLuces[i]) {
      lucesEncendidas[i] = !lucesEncendidas[i];
      digitalWrite(PIN_LUCES[i], lucesEncendidas[i] ? HIGH : LOW);
    }

    lastBtnLuces[i] = btn;
  }

  // --- Modo seguridad ON/OFF ---
  if (btnSeguridad && !lastBtnSeguridad) {
    modoSeguridad    = !modoSeguridad;
    intrusoDetectado = false;
  }
  lastBtnSeguridad = btnSeguridad;

  // --- Botón interior cochera ---
  if (btnCochera && !lastBtnCochera) {
    if (cocheraAbierta) {
      cerrarCochera();
    } else {
      abrirCochera();
    }
  }
  lastBtnCochera = btnCochera;

  // --- Lecturas analógicas ---
  int ldrValor    = analogRead(PIN_LDR_EXTERIOR);
  int lluviaValor = analogRead(PIN_RAIN);
  int mq2Valor    = analogRead(PIN_MQ2);
  int laserValor  = analogRead(PIN_LASER_SENSOR);

  bool hayLluvia = (lluviaValor < RAIN_UMBRAL_MOJADO);
  bool muchaLuz  = (ldrValor   > LDR_UMBRAL_LUZ_ALTA);

  // El haz está interrumpido si la fotoresistencia recibe MUCHA menos luz
  bool hazInterrumpido = (laserValor < LASER_UMBRAL_CORTE);

  // --- Beep al inicio de la lluvia ---
  if (hayLluvia && !lluviaEstadoAnterior) {
    beepCortoSuave();
  }
  lluviaEstadoAnterior = hayLluvia;

  // --- Distancia coche ---
  int distMm = -1;
  VL53L0X_RangingMeasurementData_t medida;
  vl53.rangingTest(&medida, false);
  if (medida.RangeStatus != 4) {
    distMm = (int)medida.RangeMilliMeter;
  }
  bool cocheCerca = (distMm > 0 && distMm < DIST_UMBRAL_COCHE_MM);

  // --- Toldo ---
  bool debeExtenderToldo = hayLluvia || muchaLuz;
  if (debeExtenderToldo && !toldoExtendido) {
    extenderToldo();
    toldoExtendido = true;
  } else if (!debeExtenderToldo && toldoExtendido) {
    retraerToldo();
    toldoExtendido = false;
  }

  // --- Seguridad y láser ---
  if (modoSeguridad && hazInterrumpido && !intrusoDetectado) {
    intrusoDetectado = true;
    beepAlarmaIntruso();
  }
  if (!modoSeguridad) {
    intrusoDetectado = false;
  }

  // --- Cochera automática ---
  if (!modoSeguridad && cocheCerca && !cocheraAbierta) {
    abrirCochera();
  }
  if (cocheraAbierta && (millis() - tInicioCochera >= COCHERA_OPEN_MS)) {
    cerrarCochera();
  }

  // --- Humo cocina ---
  bool humoAlto = (mq2Valor > MQ2_UMBRAL_HUMO);

  if (humoAlto) {
    digitalWrite(PIN_LED_HUMO, HIGH);
    if (!humoDetectado) {
      humoDetectado = true;
      beepAlarmaHumo();
    }
  } else {
    humoDetectado = false;
    digitalWrite(PIN_LED_HUMO, LOW);
  }

  // --- OLED ---
  actualizarOLED(hayLluvia, muchaLuz);

  // --- LCD cava ---
  if (millis() - lastLCDUpdate >= LCD_UPDATE_MS) {
    lastLCDUpdate = millis();
    actualizarLCDCava();
  }

  delay(50);
}

// ======================= FUNCIONES AUXILIARES ===================

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
  servoToldo.write(SERVO_TOLDO_EXTENDER_LENTO);
  delay(SERVO_TOLDO_MS * 3);
  servoToldo.write(SERVO_STOP);
}

void retraerToldo() {
  servoToldo.write(SERVO_TOLDO_RETRAER_LENTO);
  delay(SERVO_TOLDO_MS * 3);
  servoToldo.write(SERVO_STOP);
}

void abrirCochera() {
  servoCochera.write(SERVO_COCHERA_ABRIR_LENTO);
  delay(SERVO_COCHERA_MS * 3);
  servoCochera.write(SERVO_STOP);
  cocheraAbierta = true;
  tInicioCochera = millis();
}

void cerrarCochera() {
  servoCochera.write(SERVO_COCHERA_CERRAR_LENTO);
  delay(SERVO_COCHERA_MS * 3);
  servoCochera.write(SERVO_STOP);
  cocheraAbierta = false;
}

// OLED: clima, alertas, estado general
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
      float tempC = bme.readTemperature();
      oled.print(F("Clima: Soleado - "));
      oled.print(F("Temp: "));
      if (!isnan(tempC)) {
        oled.print(tempC, 1);
        oled.print(F("C"));
      } else {
        oled.print(F("--.-C"));
      }
    }
  }

  oled.setCursor(0, 16);
  oled.print(F("Toldo: "));
  oled.println(toldoExtendido ? F("Extendido") : F("Contraido"));

  oled.setCursor(0, 32);
  oled.print(F("Seguridad: "));
  oled.println(modoSeguridad ? F("ON") : F("OFF"));

  oled.setCursor(0, 48);
  oled.print(F("Cochera: "));
  oled.println(cocheraAbierta ? F("Abierta") : F("Cerrada"));

  oled.display();
}

// LCD cava: T/H/P
void actualizarLCDCava() {
  float tempC = bme.readTemperature();
  float hum   = bme.readHumidity();
  float pres  = bme.readPressure() / 100.0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  if (!isnan(tempC)) {
    lcd.print(tempC, 1);
    lcd.print("C ");
  } else {
    lcd.print("--.-C ");
  }

  lcd.print("H:");
  if (!isnan(hum)) {
    lcd.print(hum, 1);
    lcd.print("%");
  } else {
    lcd.print("--%");
  }

  lcd.setCursor(0, 1);
  lcd.print("P:");
  if (!isnan(pres)) {
    lcd.print(pres, 1);
    lcd.print("hPa");
  } else {
    lcd.print("----hPa");
  }
}