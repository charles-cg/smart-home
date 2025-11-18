	//Sensor de distancia
	#include "Adafruit_VL53L0X.h"  //Importa libreria
	

	Adafruit_VL53L0X lox = Adafruit_VL53L0X();  //Declara el objeto para usar el sensor
	void setup() {
	  Serial.begin(9600);  //Monitor serial
	  lox.begin();         //Inicia el sensor
	}
	

	void loop() {
	  VL53L0X_RangingMeasurementData_t measure;  //Estructura donde se guardara la medicion
	  lox.rangingTest(&measure, false);          //Obtiene una medicion desde el sensor
	  Serial.print("Distancia en (mm): ");
	  Serial.println(measure.RangeMilliMeter);  //Imprime la medicion en milimetros
	  delay(500);                               //Retardo
	}
