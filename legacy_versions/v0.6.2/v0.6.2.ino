#define BLYNK_TEMPLATE_ID "TMPL2o-qm6hNn"
#define BLYNK_TEMPLATE_NAME "Medidor de nivel y consumo de agua"
#define BLYNK_FIRMWARE_VERSION "0.6.2"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG

#include "BlynkEdgent.h"
#include <LiquidCrystal_I2C.h>

// --- Pines ---
#define boton 25
#define wifi 26
#define flowsensor 27
#define rst 33
#define aire 34
#define agua 35

// --- Variables ---
const int brillo = 32;

// PWM settings
const int freq = 5000;
const int ledChannel2 = 4;
const int ledChannel3 = 5;
const int ledChannel4 = 6;
const int resolution = 8;

// Nivel de agua
volatile double Level;
volatile int porcentaje;
double total = 1.2;
volatile double Vair;
volatile double Vh2o;
double Vs = 5.00;
double slope_air = 25.697;
double slope_h2o = 25.69635;
double tol = 0.02;
double slope = 1.0106;
double intercept = 0.0841;

// Flujo
volatile int flow_frequency;
volatile int flow_frequency_2;
volatile float l_min;
volatile float Liters;
float cte = 6.6;
float cal = 0.72;

// LCD
bool modo = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Timer Blynk
BlynkTimer timer;

// --- Funciones Blynk ---
void myTimer() {
  Blynk.virtualWrite(V0, Level);
  Blynk.virtualWrite(V1, porcentaje);
  Blynk.virtualWrite(V5, l_min);
  Blynk.virtualWrite(V6, Liters);
  Blynk.virtualWrite(V11, Vair);
  Blynk.virtualWrite(V12, Vh2o);
}

// --- BLYNK_WRITE ---
BLYNK_WRITE(V3) { total = param.asDouble(); }
BLYNK_WRITE(V4) { Vs = param.asDouble(); }
BLYNK_WRITE(V7) { tol = param.asDouble(); }
BLYNK_WRITE(V8) { cte = param.asDouble(); }

// --- Interrupción flujo ---
void flow() {
  flow_frequency++;
  flow_frequency_2++;
}

// --- Task de medición y LCD (núcleo 1) ---
void TaskMedicion(void *pvParameters) {
  double aux1, aux2;
  const int n = 200;

  for (;;) {
    // --- Lectura sensores ---
    aux1 = 0;
    aux2 = 0;
    for (int i = 0; i < n; i++) {
      aux1 += analogRead(aire) * 3.3 / 4095.0;
      aux2 += analogRead(agua) * 3.3 / 4095.0;
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    Vair = aux1 / n;
    Vh2o = aux2 / n;

    // --- Cálculo nivel ---
    Level = slope * ((slope_h2o * Vh2o - slope_air * Vair) / Vs + tol) + intercept;
    if (Level < 0) Level = 0;
    porcentaje = Level * 100 / total;

    // --- Cálculo flujo cada segundo ---
    static unsigned long lastFlowTime = 0;
    unsigned long t = millis();
    if (t - lastFlowTime >= 1000) {
      lastFlowTime = t;
      l_min = flow_frequency / cte;
      Liters += flow_frequency_2 / 60 / cal / 1.4;
      flow_frequency = 0;
      flow_frequency_2 = 0;
    }

    // --- Actualización LCD ---
    if (digitalRead(boton) == 1) modo = !modo;

    lcd.clear();
    lcd.setCursor(0, 0);
    if (modo == 0) {
      lcd.print("Capacidad: ");
      lcd.setCursor(12, 0); lcd.print(porcentaje);
      lcd.setCursor(15, 0); lcd.print("%");
      lcd.setCursor(0, 1);  lcd.print("Nivel: ");
      lcd.setCursor(10, 1); lcd.print(Level, 2);
      lcd.setCursor(15, 1); lcd.print("m");
    } else {
      lcd.print(l_min, 2); lcd.print(" L/min");
      lcd.setCursor(0, 1); lcd.print(Liters, 2); lcd.print(" L");
      lcd.setCursor(12, 1); lcd.print(porcentaje);
      lcd.setCursor(15, 1); lcd.print("%");
    }
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);

  // Pines
  pinMode(boton, INPUT_PULLDOWN);
  pinMode(rst, INPUT_PULLUP);
  pinMode(agua, INPUT_PULLDOWN);
  pinMode(flowsensor, INPUT_PULLUP);
  digitalWrite(flowsensor, HIGH);
  attachInterrupt(digitalPinToInterrupt(flowsensor), flow, FALLING);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Iniciando...");

  // Blynk
  BlynkEdgent.begin();

  // Task de medición + LCD en núcleo 1
  xTaskCreatePinnedToCore(
    TaskMedicion, "MedicionLCD", 4096, NULL, 1, NULL, 1
  );

  // Timers para Blynk
  timer.setInterval(1000L, myTimer);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Sistema listo");
}

// --- Blynk conectado ---
BLYNK_CONNECTED() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Conectado con");
  lcd.setCursor(0, 1); lcd.print("exito!");
  delay(2000);
}

// --- Loop principal (núcleo 0) ---
void loop() {
  if(WiFi.status() != WL_CONNECTED) {
    BlynkEdgent.run();
  }

  BlynkEdgent.run();
  timer.run();
}