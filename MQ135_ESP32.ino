/**
 * Sensor MQ-135 — ESP32 — Calidad del Aire
 * Pin Digital  : GPIO 23  (D0 del módulo)
 * Pin Analógico: GPIO 34  (A0 del módulo — ADC1, solo lectura)
 * Librería     : MQUnifiedsensor  (https://github.com/miguel5612/MQSensorsLib)
 *
 * Conexión física:
 *   VCC → 5 V  (pin VIN del ESP32)
 *   GND → GND
 *   D0  → GPIO 23  (salida digital — LOW cuando gas supera umbral)
 *   A0  → GPIO 34  (salida analógica)
 *
 * El MQ-135 posee un único elemento resistivo: no distingue gases.
 * Este sketch muestra el ratio Rs/R0, un nivel de calidad del aire
 * y la estimación de CO₂ equivalente (uso estándar en interiores).
 */

#include <MQUnifiedsensor.h>

#define BOARD             "ESP-32"
#define PIN_ANALOGICO     34
#define PIN_DIGITAL       23
#define SENSOR_TYPE       "MQ-135"
#define VOLTAGE           3.3f
#define ADC_RESOLUTION    12
#define RL_VALUE          10.0f
#define RATIO_CLEAN_AIR   3.6f

MQUnifiedsensor MQ135(BOARD, VOLTAGE, ADC_RESOLUTION, PIN_ANALOGICO, SENSOR_TYPE);

float r0 = 0;

const char* clasificarAire(float ratio) {
  if (ratio > 3.0f) return "EXCELENTE";
  if (ratio > 2.0f) return "BUENO";
  if (ratio > 1.2f) return "MODERADO";
  if (ratio > 0.8f) return "MALO";
  return "PELIGROSO";
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_DIGITAL, INPUT);

  Serial.println(F("=== MQ-135 en ESP32 — Calidad del Aire ==="));

  MQ135.setRegressionMethod(1);
  MQ135.setRL(RL_VALUE);
  MQ135.init();

  Serial.print(F("Calibrando R0 en aire limpio"));
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();
    calcR0 += MQ135.calibrate(RATIO_CLEAN_AIR);
    Serial.print(F("."));
  }
  r0 = calcR0 / 10.0f;
  MQ135.setR0(r0);
  Serial.println(F(" listo!"));

  if (isinf(r0)) {
    Serial.println(F("[ERROR] R0 infinito — circuito abierto. Revisar cableado."));
    while (1);
  }
  if (r0 == 0) {
    Serial.println(F("[ERROR] R0 = 0 — corto a GND. Revisar cableado."));
    while (1);
  }

  Serial.print(F("R0 calibrado  : "));
  Serial.print(r0, 2);
  Serial.println(F(" kΩ"));
  Serial.println(F("----------------------------------"));
}

void loop() {
  MQ135.update();
  int estadoDigital = digitalRead(PIN_DIGITAL);

  /** Ratio Rs/R0: con a=1 b=1 la fórmula a*(Rs/R0)^b devuelve el ratio crudo */
  MQ135.setA(1.0);  MQ135.setB(1.0);
  float ratio = MQ135.readSensor();

  /** CO₂ equivalente (uso estándar del MQ-135 en interiores) */
  MQ135.setA(110.47);  MQ135.setB(-2.862);
  float co2 = MQ135.readSensor() + 400;

  Serial.print(F("Rs/R0         : "));
  Serial.println(ratio, 3);

  Serial.print(F("Calidad aire  : "));
  Serial.println(clasificarAire(ratio));

  Serial.print(F("CO2 equiv.    : "));
  Serial.print(co2, 0);
  Serial.println(F(" ppm"));

  Serial.print(F("Digital D0    : "));
  Serial.println(estadoDigital == LOW ? F("ALERTA") : F("OK"));

  Serial.println(F("----------------------------------"));
  delay(1000);
}
