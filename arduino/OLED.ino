#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// Define el tamaño de la pantalla. Puede ser 128x64 o 128x32.
#define ANCHO_PANTALLA 128
#define ALTO_PANTALLA 64

// Crea una instancia del objeto de pantalla.
// La última dirección (0x3C) depende de tu modelo de pantalla.
// Si no funciona, prueba 0x3D.
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);

void setup() {
  // Inicializa la comunicación y la pantalla
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se pudo inicializar el display SSD1306"));
    for (;;); // Bucle infinito si falla
  }

  // Muestra un mensaje de prueba
  display.display();
  delay(2000);

  // Limpia el búfer de pantalla
  display.clearDisplay();

  // Establece el color del texto (blanco)
  display.setTextColor(SSD1306_WHITE);

  // Establece el tamaño del texto
  display.setTextSize(1);

  // Establece la posición del texto (columna 5, fila 2)
  display.setCursor(5, 2);

  // Muestra el texto
  display.println("¡Hola Mundo!");

  // Muestra todo lo escrito en el búfer
  display.display();
}

void loop() {
  // Puedes añadir más código aquí para actualizar la pantalla en tiempo real
}
