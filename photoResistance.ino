#define photoRes A2

int photoValue = 0;
float photoVoltage = 0.0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  analogReadResolution(14);
}

void loop() {
  // put your main code here, to run repeatedly:
  photoValue = analogRead(photoRes);
  Serial.print("Digital Value is: ");
  Serial.println(photoValue);
  photoVoltage = ((photoValue * 5.0)/ 16384);
  Serial.print("Voltaje es: ");
  Serial.println(photoVoltage);
  delay(200);
}
