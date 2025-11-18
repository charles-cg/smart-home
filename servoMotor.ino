#include <Servo.h>

Servo miServo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  miServo.attach(9); // El número es el pin conectado

}

void loop() {
  // put your main code here, to run repeatedly:
  miServo.write(0);    // Girar en sentido horario
  delay(2000);         // Esperar 2 segundos
  miServo.write(90);   // Detener
  delay(1000);         // Esperar 1 segundo
  miServo.write(180);  // Girar en sentido antihorario
  delay(2000);         // Esperar 2 segundos

}
