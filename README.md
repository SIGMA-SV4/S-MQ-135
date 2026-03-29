# S-MQ-135 — Sensor de Calidad del Aire con ESP32

Sketch base para leer el sensor **MQ-135** desde un **ESP32** usando la librería **MQUnifiedsensor**. Mide concentraciones individuales de CO, Alcohol, CO₂, Tolueno, NH₄ y Acetona en ppm, con calibración automática de R0 al inicio.

---

## Tabla de contenidos

- [Descripción del sensor](#descripción-del-sensor)
- [Hardware requerido](#hardware-requerido)
- [Diagrama de conexión](#diagrama-de-conexión)
- [Conexión de pines](#conexión-de-pines)
- [Librería](#librería)
- [Calibración](#calibración)
- [Configuración del sketch](#configuración-del-sketch)
- [Salida por Serial Monitor](#salida-por-serial-monitor)
- [Interpretación del ratio Rs/R0](#interpretación-del-ratio-rsr0)
- [Repositorio](#repositorio)

---

## Descripción del sensor

El **MQ-135** es un sensor electroquímico de calidad del aire basado en SnO₂ (dióxido de estaño). Su resistencia disminuye al aumentar la concentración de gases contaminantes. Dispone de salida analógica proporcional a la concentración y salida digital con umbral configurable mediante potenciómetro.

| Parámetro | Valor |
|---|---|
| Voltaje de operación | 5 V |
| Consumo de corriente | ≤ 150 mA |
| Salida analógica | 0 V – 5 V |
| Salida digital | 0 V / 5 V (TTL) |
| Gases detectables | NH₃, NOₓ, CO₂, Alcohol, Benceno, Humo |
| Resistencia de carga (RLOAD) | 10 kΩ |
| Tiempo de precalentamiento | 20 – 30 s |
| Vida útil | > 5 años |

---

## Hardware requerido

- ESP32 (cualquier variante)
- Módulo MQ-135
- Fuente de alimentación 5 V
- Cables de conexión

---

## Diagrama de conexión

![Diagrama de conexión MQ-135](https://microcontrollerslab.com/wp-content/uploads/2022/06/MQ-135-gas-sensor-with-Arduino-schematic-diagram.jpg)

> La imagen muestra la conexión general del módulo MQ-135. Para este proyecto los pines de datos van a los **GPIO 23 (digital) y GPIO 34 (analógico)** del ESP32.

---

## Conexión de pines

| Pin Módulo | Señal | ESP32 |
|---|---|---|
| `VCC` | Alimentación | 5 V (VIN) |
| `GND` | Tierra | GND |
| `D0` | Salida digital | GPIO 23 |
| `A0` | Salida analógica | GPIO 34 |

---

## Librería

Instalar desde el **Library Manager** de Arduino IDE:

| Librería | Autor | Versión mínima |
|---|---|---|
| `MQUnifiedsensor` | Miguel Califa U. | 3.0.0 |

> Repositorio oficial: [miguel5612/MQSensorsLib](https://github.com/miguel5612/MQSensorsLib)
>
> Compatible con ESP32 de forma nativa: permite configurar voltaje de referencia (3.3 V) y resolución ADC (12 bits).

---

## Calibración

El sketch calibra R0 automáticamente al encender, tomando 10 muestras en aire limpio y promediándolas. El valor se asigna con `MQ135.setR0()`.

Para resultados óptimos, encender el sensor en **aire limpio al exterior** durante el primer arranque. Si R0 sale infinito o cero, el sketch se detiene e indica error de cableado.

---

## Configuración del sketch

Parámetros al inicio del archivo `.ino`:

```cpp
#define BOARD             "ESP-32"
#define PIN_ANALOGICO     34      // ADC1, solo lectura
#define PIN_DIGITAL       23      // D0 del módulo
#define VOLTAGE           3.3f    // Referencia del ADC del ESP32
#define ADC_RESOLUTION    12      // 12 bits → 0–4095
#define RL_VALUE          10.0f   // Resistencia de carga del módulo (kΩ)
#define RATIO_CLEAN_AIR   3.6f    // Rs/R0 en aire limpio (datasheet)
```

---

## Salida por Serial Monitor

Abrir el Serial Monitor a **115200 bps**. Salida esperada:

```
=== MQ-135 en ESP32 — Calidad del Aire ===
Calibrando R0 en aire limpio.......... listo!
R0 calibrado  : 12.45 kΩ
----------------------------------
Rs/R0         : 3.580
Calidad aire  : EXCELENTE
CO2 equiv.    : 405 ppm
Digital D0    : OK
----------------------------------
Rs/R0         : 0.620
Calidad aire  : PELIGROSO
CO2 equiv.    : 8530 ppm
Digital D0    : ALERTA
----------------------------------
```

---

## Interpretación del ratio Rs/R0

El MQ-135 posee **un único elemento resistivo** (SnO₂). No puede distinguir entre gases individuales — su resistencia baja ante cualquier contaminante presente. El **ratio Rs/R0** es el dato real del sensor:

| Rs/R0 | Nivel | Significado |
|---|---|---|
| > 3.0 | EXCELENTE | Aire limpio, sin contaminantes detectables |
| 2.0 – 3.0 | BUENO | Niveles normales en interiores ventilados |
| 1.2 – 2.0 | MODERADO | Contaminantes leves, considerar ventilación |
| 0.8 – 1.2 | MALO | Concentración elevada, ventilar inmediatamente |
| < 0.8 | PELIGROSO | Nivel alto de gases, evacuar o ventilar con urgencia |

> **CO₂ equivalente**: se calcula con la curva de CO₂ del datasheet ($a = 110.47$, $b = -2.862$) más un offset de 400 ppm (nivel atmosférico actual). Es una estimación, no una medición selectiva.

> **Obtención del ratio Rs/R0**: la librería `MQUnifiedsensor` no expone un método `getResistance()` directo. El ratio se obtiene llamando a `readSensor()` con coeficientes `a=1, b=1`, ya que la fórmula interna es $a \times (R_s/R_0)^b$, que con esos valores devuelve el ratio puro.

---

## Repositorio

[https://github.com/SIGMA-SV4/S-MQ-135](https://github.com/SIGMA-SV4/S-MQ-135)
