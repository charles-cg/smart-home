const int MQ2_PIN = A0;  // Analog pin for MQ2 sensor

const float RL_VALUE = 5.0;
const float RO_CLEAN_AIR_FACTOR = 9.83;  // Sensor resistance in clean air / RO

// Sensor voltage
const float VCC = 5.0;

// Variables
float sensor_volt;
float RS_gas;      // Sensor resistance in gas
float ratio;       // RS_gas / RO
float RO = 10.0;   // Sensor resistance in clean air (calibrated value)

void setup() {
  Serial.begin(9600);
  Serial.println("MQ2 Smoke Sensor Initialization");
  Serial.println("Warming up sensor (60 seconds)...");
  
  // MQ2 needs 20-60 seconds to heat up for accurate readings
  delay(60000);
  
  // Calibrate sensor in clean air
  Serial.println("Calibrating sensor in clean air...");
  RO = calibrateSensor();
  Serial.print("Calibration complete. RO = ");
  Serial.print(RO);
  Serial.println(" kOhm");
  Serial.println("Starting measurements...");
  Serial.println();
}

void loop() {
  // Read analog value from sensor
  int sensorValue = analogRead(MQ2_PIN);
  
  // Convert to voltage (0-5V for Arduino Uno, 0-3.3V for some boards)
  sensor_volt = (float)sensorValue / 1024.0 * VCC;
  
  // Calculate RS (sensor resistance in gas)
  RS_gas = ((VCC * RL_VALUE) / sensor_volt) - RL_VALUE;
  
  // Calculate ratio RS/RO
  if (RO <= 0) {
    Serial.println("Error: Invalid RO value. Recalibrate sensor.");
    delay(1000);
    return;
  }
  
  ratio = RS_gas / RO;
  
  float smoke_ppm = calculatePPM_Smoke(ratio);
  
  // Display readings
  Serial.print("Smoke: ");
  Serial.print(smoke_ppm);
  Serial.println(" ppm");
  Serial.println("=========================================");
  Serial.println();
  
  delay(2000);  // Read every 2 seconds
}

// Calibrate sensor in clean air
float calibrateSensor() {
  float val = 0;
  
  // Take average of 50 readings
  for (int i = 0; i < 50; i++) {
    int sensorValue = analogRead(MQ2_PIN);
    float sensor_volt = (float)sensorValue / 1024.0 * VCC;
    float RS_air = ((VCC * RL_VALUE) / sensor_volt) - RL_VALUE;
    val += RS_air;
    delay(100);
  }
  
  val = val / 50.0;  // Calculate average
  val = val / RO_CLEAN_AIR_FACTOR;  // Divide by RO factor
  
  return val;
}

// Calculate Smoke concentration in PPM
float calculatePPM_Smoke(float ratio) {
  // Smoke curve for MQ2: approximately ppm = 800 * pow(ratio, -2.70)
  return 800.0 * pow(ratio, -2.70);
}
