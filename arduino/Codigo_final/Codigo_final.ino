// Librerías
#include <Wire.h>               // Habilita el bus I2C (usado por BME280, OLED, VL53L0X, LCD I2C)
#include <Adafruit_Sensor.h>    // Librería base de sensores Adafruit
#include <Adafruit_BME280.h>    // Librería del sensor BME280
#include <Adafruit_GFX.h>       // Librería gráfica base
#include <Adafruit_SSD1306.h>   // Librería para la pantalla OLED
#include <Adafruit_VL53L0X.h>   // Librería para el sensor de distancia VL53L0X
#include <Servo.h>              // Librería para controlar servos
#include <LiquidCrystal_I2C.h>  // Librería para LCD por I2C (16x2)

// OLED (pantalla principal)
#define OLED_WIDTH 128          // Ancho de la OLED en pixeles
#define OLED_HEIGHT 64          // Alto de la OLED
#define OLED_ADDR 0x3C          // Dirección I2C típica de la OLED
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire); // Objeto OLED usando I2C

// LCD (pantalla de la cava en la cocina)
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD I2C en dirección 0x27, de 16 columnas x 2 filas

// Sensores
Adafruit_BME280 bme;             // Sensor BME280 (usado para la cava de vino)
Adafruit_VL53L0X vl53 = Adafruit_VL53L0X();  // Sensor de distancia VL53L0X (cochera)

// Pines analógicos para sensores
const int PIN_LDR_EXTERIOR = A0;    // LDR que mide luz exterior (para toldo)
const int PIN_RAIN         = A1;    // Sensor de lluvia
const int PIN_MQ2          = A2;    // Sensor de humo/gas MQ-2 (cocina)
const int PIN_LASER_SENSOR = A3;    // Fotoresistor/fotodiodo que recibe el láser KY-008

// Actuadores
const int PIN_BUZZER        = 9;    // Buzzer para beeps y alarmas
const int PIN_SERVO_TOLDO   = 10;   // Servo 360° del toldo (terraza)
const int PIN_SERVO_COCHERA = 11;   // Servo 360° puerta de cochera

Servo servoToldo;                   // Objeto servo para el toldo
Servo servoCochera;                 // Objeto servo para la cochera

// Luces: 6 habitaciones, cada pin controla 4 focos (24 focos en total)
const int NUM_HABITACIONES = 6;
const int PIN_LUCES[NUM_HABITACIONES] = {3, 4, 5};  // (Pueden ajustar los pines Mat)

const int PIN_BTN_LUCES[NUM_HABITACIONES] = {2, A4, A5};  // Pines de los 6 botones (uno por habitación) (cambiarlos si es necesario)
 
// Botones
const int PIN_BTN_SEGURIDAD  = 12;  // Botón para activar/desactivar modo seguridad
const int PIN_BTN_COCHERA_IN = 13;  // Botón interior para abrir/cerrar cochera

bool lucesEncendidas[NUM_HABITACIONES] = {false, false, false, false, false, false};

// Estado anterior de cada botón (para detectar el clic)
bool lastBtnLuces[NUM_HABITACIONES]; 

// Constantes de umbrales (creo que debemos ajustarlos en las pruebas)
const int LDR_UMBRAL_LUZ_ALTA   = 700;   // >700 = mucha luz (depende de tu LDR)
const int RAIN_UMBRAL_MOJADO    = 600;   // <600 = lluvia detectada (depende del sensor)
const int MQ2_UMBRAL_HUMO       = 400;   // >400 = hay mucho humo/gas (depende de tu MQ-2)
const int LASER_UMBRAL_CORTE    = 300;   // Valor que indica que se interrumpió el haz
const int DIST_UMBRAL_COCHE_MM  = 800;   // Distancia (mm) para detectar coche llegando

// Tiempo que la cochera queda abierta antes de cerrarse sola
const unsigned long COCHERA_OPEN_MS      = 15000; // 15 segundos (le puedo aumentar)
// Tiempos de movimiento de los servos (pues tammbién hay que ajustar estos)
const unsigned long SERVO_TOLDO_MS       = 2000;  // 2 segundos para extender/retraer
const unsigned long SERVO_COCHERA_MS     = 2500;  // 2.5 segundos para abrir/cerrar

// Configuración de servos 360° (velocidad muy lenta)
const int SERVO_STOP                = 90;  // Parado
const int SERVO_TOLDO_EXTENDER_LENTO = 87;  // Extender toldo muy lento
const int SERVO_TOLDO_RETRAER_LENTO  = 93;  // Retraer toldo muy lento
const int SERVO_COCHERA_ABRIR_LENTO  = 87;  // Abrir cochera muy lento
const int SERVO_COCHERA_CERRAR_LENTO = 93;  // Cerrar cochera muy lento

// Variables de estado
bool modoSeguridad     = false;    // ON/OFF del modo seguridad
bool toldoExtendido    = false;    // TRUE si el toldo está extendido
bool intrusoDetectado  = false;    // TRUE si se detectó intruso
bool humoDetectado     = false;    // TRUE si se detectó humo

bool cocheraAbierta    = false;    // TRUE si la cochera está abierta
unsigned long tInicioCochera = 0;  // Momento en que se abrió la cochera (para temporizador)

// Para detectar flancos (pulsaciones) de los botones
bool lastBtnSeguridad  = HIGH;     // Último estado leído del botón de seguridad
bool lastBtnCochera    = HIGH;     // Último estado del botón interior de la cochera

// Para actualizar LCD cada cierto tiempo
unsigned long lastLCDUpdate      = 0;
const unsigned long LCD_UPDATE_MS = 1000; // Actualizar cada 1 segundo (se puede subir el tiempo)

// funciones auxiliares
void actualizarOLED(bool hayLluvia, bool muchaLuz); // Dibuja texto en OLED según estado (si no hya nada, ambos FAlSE)

void actualizarLCDCava();                           // Muestra BME280 en la LCD


void beepCortoSuave();                              // Beep corto (poco molesto porque me harta)
void beepAlarmaIntruso();                           // Patrón de alarma intruso (este vaya que me va a hartar)
void beepAlarmaHumo();                              // Patrón de alarma humo (otro que me va a hartar pero que le hago)

void extenderToldo();                               // Mueve servo para extender toldo
void retraerToldo();                                // Mueve servo para retraer toldo

void abrirCochera();                                // Mueve servo para abrir cochera
void cerrarCochera();                               // Mueve servo para cerrar cochera

// El setup
void setup() {
  Serial.begin(115200);                // Inicia comunicación serie para depuración

  // Configuración de pines de botones (pull-up))
  pinMode(PIN_BTN_SEGURIDAD, INPUT_PULLUP);  
  pinMode(PIN_BTN_COCHERA_IN, INPUT_PULLUP);

  for (int i = 0; i < NUM_HABITACIONES; i++) {
    pinMode(PIN_LUCES[i], OUTPUT);           // Pin de luces como salida
    digitalWrite(PIN_LUCES[i], LOW);         // Luces apagadas al inicio

    pinMode(PIN_BTN_LUCES[i], INPUT_PULLUP); // Botón con pull-up
    lastBtnLuces[i] = HIGH;                  // Al inicio no están presionados
  }

  // Buzzer como salida
  pinMode(PIN_BUZZER, OUTPUT);


  // Inicia bus I2C
  Wire.begin();

  // Inicializar la OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("ERROR: No se detecta la OLED")); //Aquí lloramos
  } else { //Yipi
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(F("Casa inteligente IoT"));
    oled.display();
  }

  // Inicializamos la LCD de la cava 
  lcd.init();               // Inicializa LCD I2C
  lcd.backlight();          // Enciende la luz de fondo
  lcd.clear();              // Limpia pantalla
  lcd.setCursor(0, 0);      // Coloca cursor en columna 0, fila 0
  lcd.print("Cava de vino"); // Mensaje inicial
  lcd.setCursor(0, 1);      // Fila 1
  lcd.print("Iniciando...");

  // Inicializamos el BME280 de la cava
  bool bmeOk = bme.begin(0x76);       // Intenta dirección 0x76
  if (!bmeOk) bmeOk = bme.begin(0x77); // Si falla, prueba 0x77
  Serial.println(bmeOk ? F("BME280 OK") : F("BME280 NO detectado")); // F = lloramos pt 2

  // Inicializamos VL53L0X (cochera) 
  bool vlOk = vl53.begin();          // Inicializa sensor de distancia
  Serial.println(vlOk ? F("VL53L0X OK") : F("VL53L0X NO detectado"));

  // Los servos
  servoToldo.attach(PIN_SERVO_TOLDO);     // Servo del toldo
  servoCochera.attach(PIN_SERVO_COCHERA); // Servo de cochera

  servoToldo.write(SERVO_STOP);   //  está detenido
  servoCochera.write(SERVO_STOP); //  está detenido

  // Pantalla inicial en OLED
  actualizarOLED(false, false);
}

// Loop gigante
void loop() {
  // lectura de botones (que detecta el flanco)
  bool btnSeguridad= (digitalRead(PIN_BTN_SEGURIDAD) == LOW);
  bool btnCochera  = (digitalRead(PIN_BTN_COCHERA_IN) == LOW);

  // Botón de luces: un click enciende, otro apaga
  for (int i = 0; i < NUM_HABITACIONES; i++) {
    bool btn = (digitalRead(PIN_BTN_LUCES[i]) == LOW); // TRUE si el botón i está presionado

    // Si AHORA está presionado y ANTES no lo estaba -> click
    if (btn && !lastBtnLuces[i]) {
      // Cambiar el estado de las luces de ESTA habitación
      lucesEncendidas[i] = !lucesEncendidas[i];   // toggle solo de la habitación i

      // Actualizar el pin de las luces de esa habitación
      digitalWrite(PIN_LUCES[i], lucesEncendidas[i] ? HIGH : LOW);
    }

    // Guardar el estado actual para la siguiente vuelta
    lastBtnLuces[i] = btn;
  }

  // Botón de seguridad: activa/desactiva modo seguridad
  if (btnSeguridad && !lastBtnSeguridad) {
    modoSeguridad = !modoSeguridad;      // Cambia de ON a OFF o viceversa
    intrusoDetectado = false;            // Se limpia bandera de intruso 
  }
  lastBtnSeguridad = btnSeguridad;

  // Botón interior de cochera: abre o cierra manualmente
  if (btnCochera && !lastBtnCochera) {
    if (cocheraAbierta) {
      cerrarCochera();                   // Si ya está abierta, la cerramos :D
    } else {
      abrirCochera();                    // Si está cerrada, la abrimos :D simple
    }
  }
  lastBtnCochera = btnCochera;

  // Lectura de los sensores anlógicos
  int ldrValor   = analogRead(PIN_LDR_EXTERIOR); // Luz exterior
  int lluviaValor = analogRead(PIN_RAIN);        // Lluvia
  int mq2Valor   = analogRead(PIN_MQ2);          // Humo/gas
  int laserValor = analogRead(PIN_LASER_SENSOR); // El del láser

  // Flags lógicas basadas en umbrales
  bool hayLluvia = (lluviaValor < RAIN_UMBRAL_MOJADO);   // TRUE si hay agua
  bool muchaLuz  = (ldrValor   > LDR_UMBRAL_LUZ_ALTA);   // TRUE si hay mucha luz

  // Si la lectura cae por debajo del umbral, se interrumpió el láser
  bool hazInterrumpido = (laserValor < LASER_UMBRAL_CORTE);

  // Sensor de distancia de coche (cochera)
  int distMm = -1;                                   // Valor por defecto sin lectura (muy irreal para decir que aun no hay lecturas)
  VL53L0X_RangingMeasurementData_t medida;           // Estructura para la medida
  vl53.rangingTest(&medida, false);                  // Lee distancia
  if (medida.RangeStatus != 4) {                     // 4 = fuera de rango
    distMm = (int)medida.RangeMilliMeter;            // Guardar distancia válida
  }

  bool cocheCerca = (distMm > 0 && distMm < DIST_UMBRAL_COCHE_MM); // TRUE si hay coche

  // Clima y Toldo
  // Lógica: si hay lluvia O mucha luz -> toldo extendido, si no -> retraído
  bool debeExtenderToldo = hayLluvia || muchaLuz;

  // Cambios de estado del toldo
  if (debeExtenderToldo && !toldoExtendido) {
    extenderToldo();          // Extiende el toldo
    toldoExtendido = true;    // Actualiza estado
  } else if (!debeExtenderToldo && toldoExtendido) {
    retraerToldo();           // Contrae el toldo
    toldoExtendido = false;   // Actualiza estado
  }

  // Seguridad y movimeinto (láser)
  if (modoSeguridad && hazInterrumpido && !intrusoDetectado) {
    intrusoDetectado = true;     // Marca que hubo intruso
    beepAlarmaIntruso();         // Patrón de alarma
  }

  // Si se desactiva la seguridad, dejamos de considerar intruso
  if (!modoSeguridad) {
    intrusoDetectado = false;
  }

  //  Cochera automática por distancia
  // Solo abre si NO está el modo seguridad y se detecta coche
  if (!modoSeguridad && cocheCerca && !cocheraAbierta) {
    abrirCochera();               // Abre cochera
  }

  // Si la cochera está abierta, revisa si ya se cumplió el tiempo para cerrarla
  if (cocheraAbierta && (millis() - tInicioCochera >= COCHERA_OPEN_MS)) {
    cerrarCochera();              // Cierra cochera automáticamente
  }

  // Humito de la cocina
  bool humoAlto = (mq2Valor > MQ2_UMBRAL_HUMO); // TRUE si MQ2 pasó el umbral

  // Si antes no había humo y ahora sí -> dispara alarma una vez (va a sonar horrible)
  if (humoAlto && !humoDetectado) {
    humoDetectado = true;         // Marca que hay humo
    beepAlarmaHumo();             // Alarma de humo
  }
  // Si baja del umbral, se limpia la marca
  if (!humoAlto) {
    humoDetectado = false;
  }

  // Actualizar la OLED
  actualizarOLED(hayLluvia, muchaLuz);

  //  Actualizar LCD de la cava (BME280) cada cierto tiempo (1 seg )
  if (millis() - lastLCDUpdate >= LCD_UPDATE_MS) {
    lastLCDUpdate = millis();     // Actualiza marca de tiempo
    actualizarLCDCava();          // Muestra T/H/P en LCD
  }

  // Pequeña pausa opcional (no muy grande para no hacer el sistema lento)
  delay(50);
}

// Funciones auxiliares

// Beep corto, frecuencia media y duración corta (menos molesto para el toldo, yipi)
void beepCortoSuave() {
  tone(PIN_BUZZER, 1500, 150); // Suena a 1500 Hz durante 150 ms
  delay(160);                  // Espera un poquito más que el beep
}

// Patrón de alarma para intruso (rápido, que lata)
void beepAlarmaIntruso() {
  for (int i = 0; i < 3; i++) {     // Repite 3 veces
    tone(PIN_BUZZER, 2000, 200);   // Tono de 2000 Hz por 200 ms
    delay(250);                    // Pequeña pausa
  }
}

// Patrón de alarma para humo (más largo y repetitivo, más lata)
void beepAlarmaHumo() {
  for (int i = 0; i < 2; i++) {       // Repite 2 veces
    tone(PIN_BUZZER, 1800, 800);     // Tono largo (800 ms)
    delay(900);                      // Pausa entre tonos
  }
}

// Extiende el toldo: servo en sentido horario + beep corto
void extenderToldo() {
  servoToldo.write(SERVO_TOLDO_EXTENDER_LENTO); // gira muuuy lento -> sentido horario (ajusta si el servo es al revés)
  beepCortoSuave();                  // Beep al moverse
  delay(SERVO_TOLDO_MS * 3);             // Tiempo para que llegue a posición extendida (ni idea, hay que hacer prueba y error) * 3 ya que necesita más 
  servoToldo.write(SERVO_STOP);              // Detener el servo
}

// Retrae el toldo: servo en sentido antihorario
void retraerToldo() {
  servoToldo.write(SERVO_TOLDO_RETRAER_LENTO);    // sentido antihorario
  delay(SERVO_TOLDO_MS * 3);             // Tiempo para retraer
  servoToldo.write(SERVO_STOP);              // Detener
}

// Abre la cochera: mover servo y marcar tiempo de apertura
void abrirCochera() {
  servoCochera.write(SERVO_COCHERA_ABRIR_LENTO);  // Gira para abrir
  delay(SERVO_COCHERA_MS * 3);           // Tiempo estimado para abrir
  servoCochera.write(SERVO_STOP);            // Detener
  cocheraAbierta   = true;           // Marca como abierta
  tInicioCochera   = millis();       // Guarda el tiempo para el temporizador
}

// Cierra la cochera
void cerrarCochera() {
  servoCochera.write(SERVO_COCHERA_CERRAR_LENTO);  // Gira para cerrar
  delay(SERVO_COCHERA_MS * 3);           // Tiempo para cerrar
  servoCochera.write(SERVO_STOP);            // Detener
  cocheraAbierta   = false;          // Marca como cerrada
}

// Actualiza OLED según clima y alertas
void actualizarOLED(bool hayLluvia, bool muchaLuz) {
  oled.clearDisplay();               // Limpia la pantalla
  oled.setTextSize(1);               // Tamaño de texto pequeño
  oled.setCursor(0, 0);              // Inicio de la pantalla

  // Prioridad 1: intruso
  if (intrusoDetectado) {
    oled.println(F("ALERTA: Intruso"));
  }
  // Prioridad 2: humo en cocina
  else if (humoDetectado) {
    oled.println(F("ALERTA: Humo en cocina"));
  }
  // Si no hay alertas, mostramos clima
  else {
    if (hayLluvia) {
      oled.println(F("Clima: Lluvia"));
    } else {
      oled.println(F("Clima: Soleado"));
    }
  }

  // Segunda línea: estado del toldo
  oled.setCursor(0, 16);
  oled.print(F("Toldo: "));
  oled.println(toldoExtendido ? F("Extendido") : F("Contraido"));

  // Cuarta línea: modo seguridad
  oled.setCursor(0, 40);
  oled.print(F("Seguridad: "));
  oled.println(modoSeguridad ? F("ON") : F("OFF"));

  // Quinta línea: cochera
  oled.setCursor(0, 52);
  oled.print(F("Cochera: "));
  oled.println(cocheraAbierta ? F("Abierta") : F("Cerrada"));

  oled.display();                    // Actualiza la pantalla OLED
}

// Muestra en la LCD los datos de la cava (BME280)
void actualizarLCDCava() {
  float tempC = bme.readTemperature();     // Temperatura en °C
  float hum   = bme.readHumidity();        // Humedad relativa %
  float pres  = bme.readPressure() / 100.0; // Presión en hPa

  lcd.clear();                             // Limpia pantalla
  lcd.setCursor(0, 0);                     // Fila 0
  lcd.print("T:");
  if (!isnan(tempC)) {
    lcd.print(tempC, 1);                   // Temperatura con 1 decimal
    lcd.print("C ");
  } else {
    lcd.print("--.-C ");
  }

  lcd.print("H:");
  if (!isnan(hum)) {
    lcd.print(hum, 1);                     // Humedad con 1 decimal
    lcd.print("%");
  } else {
    lcd.print("--%");
  }

  lcd.setCursor(0, 1);                     // Fila 1
  lcd.print("P:");
  if (!isnan(pres)) {
    lcd.print(pres, 1);                    // Presión con 1 decimal
    lcd.print("hPa");
  } else {
    lcd.print("----hPa");
  }
}
