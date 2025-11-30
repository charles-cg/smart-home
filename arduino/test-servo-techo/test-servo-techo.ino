// Test del servo MG90S para el techo
// Este código prueba el movimiento lento del servo del techo

#include <Servo.h>

const int PIN_SERVO_TECHO = 10;  // Pin del servo del techo

Servo servoTecho;

// Posiciones del servo
const int TECHO_CERRADO = 0;    // Techo cerrado (0 grados)
const int TECHO_ABIERTO = 90;   // Techo abierto (90 grados)

void setup() {
  Serial.begin(9600);
  
  // Configurar servo
  servoTecho.attach(PIN_SERVO_TECHO);
  servoTecho.write(TECHO_CERRADO);  // Iniciar cerrado
  
  Serial.println("Test del servo del techo");
  Serial.println("El techo se abrirá y cerrará lentamente");
  delay(2000);
}

void loop() {
  Serial.println("Abriendo techo...");
  abrirTecho();
  
  Serial.println("Techo abierto - esperando 3 segundos");
  delay(3000);
  
  Serial.println("Cerrando techo...");
  cerrarTecho();
  
  Serial.println("Techo cerrado - esperando 3 segundos");
  delay(3000);
}

void abrirTecho() {
  // Para servo de rotación continua: girar en una dirección por tiempo fijo
  servoTecho.write(100);  // Girar lentamente en una dirección (90-180 = clockwise)
  delay(3600);            // Doble de tiempo para 180 grados
  servoTecho.write(90);   // Detener (90 = stop para servos continuos)
}

void cerrarTecho() {
  // Para servo de rotación continua: girar en dirección opuesta
  servoTecho.write(80);   // Girar lentamente en dirección contraria (0-90 = counterclockwise)
  delay(3600);            // Doble de tiempo para volver 180 grados
  servoTecho.write(90);   // Detener (90 = stop)
}
