// Test del OLED con dirección 0x3F
// Este código prueba el funcionamiento del display OLED

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3F

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("\n======================");
  Serial.println("OLED Test Starting...");
  Serial.println("======================\n");
  
  // Inicializar I2C
  Wire.begin();
  Serial.println("I2C initialized");
  
  // Inicializar OLED
  Serial.print("Initializing OLED at address 0x");
  Serial.println(OLED_ADDRESS, HEX);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED allocation failed!");
    Serial.println("Check wiring and I2C address");
    for(;;); // Loop forever
  }
  
  Serial.println("OLED initialized successfully!");
  
  // Limpiar pantalla
  display.clearDisplay();
  
  // Configurar texto
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  // Mostrar mensaje
  display.println("OLED Test");
  display.println("Address: 0x3F");
  display.println("");
  display.println("Display OK!");
  display.display();
  
  Serial.println("Text displayed on OLED");
}

void loop() {
  // Actualizar contador cada segundo
  static int counter = 0;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("OLED Working!");
  display.println("");
  display.print("Counter: ");
  display.println(counter);
  display.println("");
  display.setTextSize(2);
  display.println(counter);
  display.display();
  
  Serial.print("Counter: ");
  Serial.println(counter);
  
  counter++;
  delay(1000);
}
