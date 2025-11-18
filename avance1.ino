\
//Mostrar datos en la OLED de todos los sensores.
//Al presionar el botón suena el buzzer y se mueve el servo


#include <Wire.h> //Habilita el bus I2C (SDL y SDA)
#include <SPI.h> // habilita SPI aunque no se ocupa aquí
#include <Adafruit_Sensor.h> //para sesnores de Adafruit
#include <Adafruit_BME280.h> 
#include <Adafruit_GFX.h> //Dibujar textos 
#include <Adafruit_SSD1306.h> //pantalla OLED
#include <Adafruit_VL53L0X.h> //distancia
#include <Servo.h>

//Pines const para que no cambien
const int PIN_BUTTON = 2;
const int PIN_LDR    = A0; //Este es el de luz
const int PIN_RAIN   = A1;
const int PIN_BUZZER = 9;
const int PIN_SERVO  = 10;

// OLED
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C //Dirección
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire); //Le pasa a la pantalla la referencia de esa conexión (al obj  I2C)

// Sensores
Adafruit_BME280 bme;           // I2C (0x76 o 0x77)
Adafruit_VL53L0X vl53 = Adafruit_VL53L0X();
Servo servo360;

// Estado
unsigned long lastPrint = 0; // guarda el tiempo (en ms) de la última actualización 
const unsigned long PRINT_EVERY_MS = 500; //Cada 500 ms se refreshea (se puede aumentar)
bool buttonWasLow = false; // para un beep corto al presionar

void setup() {
  Serial.begin(115200); //preguntar si se debe cambiar a 9600
  pinMode(PIN_BUTTON, INPUT_PULLUP); //Pull-up : suelto = HIGH, presionado = LOW
  pinMode(PIN_BUZZER, OUTPUT);
  // A0 y A1 analógicos no requieren pinMode

  // I2C
  Wire.begin();

  // OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("ERROR: OLED no encontrada en 0x3C"));
  } else {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(F("Integrado de componentes"));
    oled.display();
  }

  // BME280 ( Si no es 0x76, es 0x77) 
  bool bmeOk = bme.begin(0x76);
  if (!bmeOk) bmeOk = bme.begin(0x77);
  Serial.println(bmeOk ? F("BME280 OK") : F("BME280 NO detectado"));

  // VL53L0X
  bool vlOk = vl53.begin();
  Serial.println(vlOk ? F("VL53L0X OK") : F("VL53L0X NO detectado")); //reporta si fue detectado

  // Servo continuo – detener
  servo360.attach(PIN_SERVO);
  servo360.write(90); // 90 = stop 0 = sentido horario, 180 = sntido antihorario

  // Pantalla inicial
  drawScreen(NAN, NAN, -1, -1, -1);
}

void loop() {
  // Lee botón y hace beep :) en el pulso de bajada
  // Lee botón, hace beep y mueve el servo al presionar
bool btnLow = (digitalRead(PIN_BUTTON) == LOW);
if (btnLow && !buttonWasLow) { //detectar la pulsación del botón
  tone(PIN_BUZZER, 1200, 500);      // beep
  servo360.write(0);               // gira en sentido horario
  delay(1000);                     // tiempo de giro (1 seg)
  servo360.write(180);               // gira en sentido horario
  delay(1000);       
  servo360.write(90);              // detener
}
buttonWasLow = btnLow; //recordar el estado en la siguiente vuelta


  // Cada PRINT_EVERY_MS refresca lecturas y OLED
  if (millis() - lastPrint >= PRINT_EVERY_MS) { //millis() es el reloj interno en milisegundos.
    lastPrint = millis();

    // Lectura BME280
    float tempC = NAN, pressHpa = NAN;
    // Si el BME está, lee, devuelven NaN si fallan
    float t = bme.readTemperature();
    float p = bme.readPressure();
    if (!isnan(t)) tempC = t;
    if (!isnan(p)) pressHpa = p / 100.0f; //Convierte a hPa dividiendo entre 100

    // LDR y lluvia (analógicos)
    int ldrRaw = analogRead(PIN_LDR); //Del 0–1023
    int rainRaw = analogRead(PIN_RAIN);

    // Distancia VL53L0X
    int distMm = -1;
    VL53L0X_RangingMeasurementData_t m;
    vl53.rangingTest(&m, false);
    if (m.RangeStatus != 4) { //si es válida (4) lee en ms
      distMm = (int)m.RangeMilliMeter;
    }

    // Serialop cond ternario: (condición) ? valor_si_verdadero : valor_si_falso
    Serial.print(F("T=")); Serial.print(isnan(tempC)?-999:tempC, 2); // se muestra con 2 decimales, el -999 es solo para detectar que falló 
    Serial.print(F("C  P=")); Serial.print(isnan(pressHpa)?-999:pressHpa, 0);
    Serial.print(F("hPa  LDR=")); Serial.print(ldrRaw);
    Serial.print(F("  Rain=")); Serial.print(rainRaw);
    Serial.print(F("  Dist=")); Serial.print(distMm);
    Serial.println(F("mm"));

    // OLED
    drawScreen(tempC, pressHpa, ldrRaw, rainRaw, distMm);
  }
}

// Mostrar datos en pantalla
void drawScreen(float tempC, float pressHpa, int ldr, int rain, int distMm) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println(F("Integrado_simple"));

  oled.print(F("Temp: "));
  if (isnan(tempC)) oled.println(F("--"));
  else { oled.print(tempC, 1); oled.println(F(" C")); }

  oled.print(F("Pres: "));
  if (isnan(pressHpa)) oled.println(F("--"));
  else { oled.print(pressHpa, 0); oled.println(F(" hPa")); }

  oled.print(F("LDR : ")); oled.println(ldr < 0 ? 0 : ldr);
  oled.print(F("Rain: ")); oled.println(rain < 0 ? 0 : rain);

  oled.print(F("Dist: "));
  if (distMm < 0) oled.println(F("--"));
  else { oled.print(distMm); oled.println(F(" mm")); }

  oled.drawLine(0, 54, 127, 54, SSD1306_WHITE); //Dibuja una línea horizontal en la pantalla
  oled.setCursor(0, 56); //Mueve el cursor de texto a la posición de inicio del siguiente texto
  oled.print(F("Btn->Beep  Servo:Stop"));
  oled.display(); //envía todo lo del buffer al OLED
}
