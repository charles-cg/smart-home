// Test del módulo láser HW-493
// Este código simplemente enciende el láser y lee el valor del fotoresistor

const int PIN_LASER_EMITTER = 6;    // Emisor láser HW-493
const int PIN_LASER_SENSOR = A3;    // Fotoresistor que recibe el láser

void setup() {
  Serial.begin(9600);
  
  // Configurar pines
  pinMode(PIN_LASER_EMITTER, OUTPUT);
  
  // Set analog resolution
  analogReadResolution(14);
  
  // Encender láser
  digitalWrite(PIN_LASER_EMITTER, HIGH);
  Serial.println("Laser ON");
  Serial.println("Reading photoresistor values every second...");
  Serial.println("Point laser at photoresistor to see high values");
  Serial.println("Block laser beam to see low values");
  Serial.println("---");
}

void loop() {
  // Leer fotoresistor
  int laserValor = analogRead(PIN_LASER_SENSOR);
  
  // Mostrar en Serial
  Serial.print("Laser sensor value: ");
  Serial.print(laserValor);
  Serial.print(" | Status: ");
  
  if (laserValor > 300) {
    Serial.println("RECEIVING LASER (beam not blocked)");
  } else {
    Serial.println("NO LASER / BLOCKED (beam interrupted)");
  }
  
  delay(1000);
}
