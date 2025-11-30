// Test del buzzer
// Este código prueba el buzzer con diferentes tonos y patrones

const int PIN_BUZZER = 8;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BUZZER, OUTPUT);
  
  Serial.println("Buzzer Test Starting...");
  Serial.println("You should hear 3 different beep patterns");
  delay(1000);
}

void loop() {
  Serial.println("Test 1: Simple tone 1000Hz for 1 second");
  tone(PIN_BUZZER, 1000, 1000);
  delay(1500);
  
  Serial.println("Test 2: Three short beeps (1800Hz)");
  for (int i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 1800, 300);
    delay(400);
  }
  delay(1000);
  
  Serial.println("Test 3: Two long beeps (2000Hz)");
  for (int i = 0; i < 2; i++) {
    tone(PIN_BUZZER, 2000, 800);
    delay(900);
  }
  delay(1000);
  
  Serial.println("Test 4: Siren effect");
  for (int freq = 1000; freq <= 2000; freq += 100) {
    tone(PIN_BUZZER, freq, 100);
    delay(100);
  }
  delay(1000);
  
  Serial.println("---");
  Serial.println("All tests complete. Repeating in 3 seconds...");
  delay(3000);
}
