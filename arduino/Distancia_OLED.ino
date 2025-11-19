#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VL53L0X.h"

// Crear objeto para el sensor
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Tamaño de la pantalla
#define ANCHO_PANTALLA 128
#define ALTO_PANTALLA 64

// Crear objeto para el display
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);

void setup() {
  Serial.begin(9600);

  // Inicializar sensor
  if (!lox.begin()) {
    Serial.println(F("Error al iniciar el sensor VL53L0X"));
    while (1);
  }

  // Inicializar display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se pudo inicializar el display SSD1306"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.println("Sensor VL53L0X listo");
  display.display();
  delay(1500);
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); // Obtener medición

  display.clearDisplay(); // Limpiar pantalla

  if (measure.RangeStatus != 4) { // Si la medición es válida
    Serial.print("Distancia (mm): ");
    Serial.println(measure.RangeMilliMeter);

    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print(measure.RangeMilliMeter);
    display.println(" mm");
  } else {
    Serial.println("Fuera de rango");
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.println("Fuera de rango");
  }

  display.display();
  delay(500); // Actualiza cada medio segundo
}
