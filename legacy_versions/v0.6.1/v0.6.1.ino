#define BLYNK_TEMPLATE_ID "TMPL2o-qm6hNn"      // Actualiza con tu template real
#define BLYNK_TEMPLATE_NAME "Medidor de nivel y consumo de agua"
#define BLYNK_FIRMWARE_VERSION "0.6.1"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG

#include "BlynkEdgent.h"
#include <LiquidCrystal_I2C.h>

// Pines
#define boton 25
#define wifi 26
#define flowsensor 27
#define rst 33
#define aire 34
#define agua 35

const int rPin = 4;
const int gPin = 5;
const int zPin = 2;
const int brillo = 32;

// PWM settings
const int freq = 5000;
const int ledChannel2 = 4;
const int ledChannel3 = 5;
const int ledChannel4 = 6;
const int resolution = 8;

// Variables de nivel de agua
double Level;
int porcentaje;
double total = 4.5;
double Vair;
double Vh2o;
double Vout;
double Vs = 4.96;
double aux1 = 0;
double aux2 = 0;
int n = 200;
double slope_air = 25.697;
double slope_h2o = 25.69635;
double tol = 0.02;
double slope = 1.0106;
double intercept = 0.0841;

// Variables de flujo
volatile int flow_frequency;
volatile int flow_frequency_2;
float l_min;
unsigned long currentTime;
unsigned long cloopTime;
float Liters, cte = 9.9;

// Pantalla
bool modo = 0;

// LCDs
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Timer
BlynkTimer timer;

// --- Funciones ---
void myTimer() {
  Blynk.virtualWrite(V0, Level);
  Blynk.virtualWrite(V1, porcentaje);
  Blynk.virtualWrite(V5, l_min);
  Blynk.virtualWrite(V6, Liters);
  Blynk.virtualWrite(V11, Vair);
  Blynk.virtualWrite(V12, Vh2o);
}

void Medicion() {
  aux1 = 0;
  aux2 = 0;
  for (int i = 0; i < n; i++) {
    aux1 += analogRead(aire) * 3.3 / 4095.0;
    aux2 += analogRead(agua) * 3.3 / 4095.0;
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  Vair = aux1 / n;
  Vh2o = aux2 / n;
  Vout = Vh2o - Vair;

  Level = slope * ((slope_h2o * Vh2o - slope_air * Vair)/Vs + tol) + intercept;
  if (Level < 0) Level = 0;
  porcentaje = Level * 100 / total;
}

void Pantalla() {
  if (digitalRead(boton) == 1) modo = !modo;

  lcd.clear();
  lcd.setCursor(0, 0);

  if (modo == 0) {
    lcd.print("Depth: ");
    lcd.setCursor(10, 0);
    lcd.print(Level);
    lcd.setCursor(15, 0);
    lcd.print("m");
    lcd.setCursor(0, 1);
    lcd.print("Capacity: ");
    lcd.setCursor(12, 1);
    lcd.print(porcentaje);
    lcd.setCursor(15, 1);
    lcd.print("%");
  } else {
    lcd.print(l_min);
    lcd.print(" L/min");
    lcd.setCursor(0, 1);
    lcd.print(Liters);
    lcd.print(" L");
    lcd.setCursor(11, 1);
    lcd.print(porcentaje);
    lcd.setCursor(15, 1);
    lcd.print("%");
  }
}

// --- BLYNK_WRITE ---
BLYNK_WRITE(V3) { total = param.asDouble(); }
BLYNK_WRITE(V4) { Vs = param.asDouble(); }
BLYNK_WRITE(V7) { tol = param.asDouble(); }
BLYNK_WRITE(V8) { cte = param.asDouble(); }
BLYNK_WRITE(V9) { lon = param.asDouble(); }
BLYNK_WRITE(V10) { lat = param.asDouble(); }

// --- Interrupt ---
void flow() {
  flow_frequency++;
  flow_frequency_2++;
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

  // Timers
  timer.setInterval(1000L, myTimer);
  timer.setInterval(10L, Medicion);
  timer.setInterval(1000L, Pantalla);

  currentTime = millis();
  cloopTime = currentTime;
}

// --- Blynk conectado ---
BLYNK_CONNECTED() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectado con");
  lcd.setCursor(0, 1);
  lcd.print("exito!");
  delay(2000);
}

// --- Loop ---
void loop() {
  // Ejecuta BlynkEdgent solo si el WiFi está activo o intenta reconectar
  if(WiFi.status() != WL_CONNECTED) {
    BlynkEdgent.run();
  }

  // Flujo
  currentTime = millis();
  if(currentTime >= cloopTime + 1000){
    cloopTime = currentTime;
    l_min = flow_frequency / cte;
    Liters = flow_frequency_2 * cte / 3600.0;
    flow_frequency = 0;
  }

  // Ejecuta Blynk y timers
  BlynkEdgent.run();
  timer.run();
}

