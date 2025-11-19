#define rainSensor A1

int rainValue = 0;
float rainVoltage = 0.0;
int rainVPercentage = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  analogReadResolution(14);

}

void loop() {
  // put your main code here, to run repeatedly:
  rainValue = analogRead(rainSensor);
  Serial.print("Digital Value is: ");
  Serial.println(rainValue);
  Serial.print("Voltaje es: ");
  rainVoltage = ((rainValue * 5.0)/ 16384);
  Serial.println(rainVoltage);
  rainVPercentage = map(rainValue, 16383,0 , 0, 100); // 0 = está mojado, 16383 = está seco
  Serial.print("Porcentaje de mojado es: ");
  Serial.print(rainVPercentage);
  Serial.println("%");
  Serial.println("------");
  

  delay(200);
}
