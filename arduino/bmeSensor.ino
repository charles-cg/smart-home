/*
 * BME280 Sensor Reader
 * Reads Temperature (°C), Humidity (%), and Pressure (hPa)
 * 
 * Connections (I2C):
 * BME280 VCC -> 3.3V or 5V
 * BME280 GND -> GND
 * BME280 SCL -> SCL (or A5 on Uno)
 * BME280 SDA -> SDA (or A4 on Uno)
 * 
 * I2C Address: 0x76 or 0x77 (check your module)
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Create BME280 object
Adafruit_BME280 bme;

// Variables to store sensor readings
float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

void setup() {
  Serial.begin(9600);
  Serial.println("BME280 Sensor Test");
  Serial.println();

  // Initialize I2C
  Wire.begin();

  // Try to initialize BME280 at address 0x76
  bool status = bme.begin(0x76);
  
  // If not found at 0x76, try 0x77
  if (!status) {
    Serial.println("BME280 not found at 0x76, trying 0x77...");
    status = bme.begin(0x77);
  }

  if (!status) {
    Serial.println("Could not find a valid BME280 sensor!");
    Serial.println("Check wiring and I2C address.");
    while (1) delay(10);
  }

  Serial.println("BME280 sensor found and initialized!");
  Serial.println();
  
  // Configure sensor settings (optional)
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating mode
                  Adafruit_BME280::SAMPLING_X2,      // Temperature oversampling
                  Adafruit_BME280::SAMPLING_X16,     // Pressure oversampling
                  Adafruit_BME280::SAMPLING_X1,      // Humidity oversampling
                  Adafruit_BME280::FILTER_X16,       // Filtering
                  Adafruit_BME280::STANDBY_MS_500);  // Standby time

  delay(1000);
  Serial.println("Starting measurements...");
  Serial.println();
}

void loop() {
  // Read sensor values
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;  // Convert Pa to hPa

  // Check if readings are valid
  if (isnan(temperature) || isnan(humidity) || isnan(pressure)) {
    Serial.println("Failed to read from BME280 sensor!");
    delay(2000);
    return;
  }

  // Display readings
  Serial.println("========== BME280 Readings ==========");
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  Serial.println("=====================================");
  Serial.println();

  // Wait 2 seconds before next reading
  delay(2000);
}
