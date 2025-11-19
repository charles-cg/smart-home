int buzzerPin = 9; // Pin de salida para el buzzer
int duration = 1000; // Duración del sonido en milisegundos
int frequency = 1000; // Frecuencia en Hz

void setup() {
  // No se necesita configurar el pin como OUTPUT con tone(), pero puede ser útil para otros fines.
  // pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // Reproduce un tono de 1000 Hz durante 1000 ms
  tone(buzzerPin, frequency, duration);
  // Espera a que termine el tono (la duración ya está incluida en tone())
  delay(duration);
  // Detiene el sonido (necesario para que el sonido termine realmente)
  noTone(buzzerPin);
  delay(1000); // Espera 1 segundo antes de repetir
}

