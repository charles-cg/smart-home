#define buttonOne 2

int state = 0;

void setup() {
  pinMode(buttonOne, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  state = digitalRead(buttonOne);
  if(state == LOW){
    Serial.println("Boton presionado");
  }else{
    Serial.println("Boton no presionado");
  }
  delay(200);

}