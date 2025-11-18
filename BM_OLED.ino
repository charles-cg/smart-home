#define _DEBUG_


#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <Wire.h>


#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// Definir constantes
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;


#define ANCHO_PANTALLA 128 // ancho pantalla OLED
#define ALTO_PANTALLA 64 // alto pantalla OLED


// Objeto de la clase Adafruit_SSD1306
Adafruit_SSD1306 oled(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);


void setup() {
#ifdef _DEBUG_
  Serial.begin(9600);
  delay(100);
  Serial.println("Iniciando pantalla OLED");
#endif


  // Iniciar pantalla OLED en la dirección 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
#ifdef _DEBUG_
    Serial.println("No se encuentra la pantalla OLED");
#endif
    while (true);
  }


  // Limpiar buffer
  oled.clearDisplay();


  // Tamaño del texto
  oled.setTextSize(1);
  // Color del texto
  //oled.setTextColor(SSD1306_WHITE);
  oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  // Posición del texto
  oled.setCursor(10, 32);
  // Escribir texto
  oled.println("¡¡protoboard!!");


  // Enviar a pantalla
  oled.display();


  Serial.begin(9600);


  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }


}


void loop() {
   // Limpiar buffer
  oled.clearDisplay();


  // Tamaño del texto
  oled.setTextSize(1.5);
  // Color del texto
  //oled.setTextColor(SSD1306_WHITE);
  oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  // Posición del texto
  oled.setCursor(0, 0);
  // Escribir texto
  oled.print("temp:");
  oled.print(bme.readTemperature());
  oled.println("*C");
 


  oled.print("Pressure = ");
  oled.print(bme.readPressure() / 100.0F);
  oled.println("hPa");


  oled.print("h = ");
  oled.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  oled.println("m");


  oled.print("Humidity = ");
  oled.print(bme.readHumidity());
  oled.println("%");


   oled.display();


  Serial.println();
  delay(1000);
}


void printValues() {


}