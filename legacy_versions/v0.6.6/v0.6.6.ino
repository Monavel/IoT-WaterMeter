#define BLYNK_TEMPLATE_ID "TMPL2o-qm6hNn"
#define BLYNK_TEMPLATE_NAME "Medidor de nivel y consumo de agua"
#define BLYNK_FIRMWARE_VERSION "0.6.6"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG

#include "BlynkEdgent.h"
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// --- Pines ---
#define boton 25
#define wifi 26
#define FLOW_SENSOR_PIN 27
#define rst 33
#define aire 34
#define agua 35

// Variables globales para acumular los datos
volatile double Level_sum = 0;
volatile double Vair_sum = 0;
volatile double Vh2o_sum = 0;
volatile int Level_count = 0;

//Reincio de la cuenta de los litros
WidgetRTC rtc;

int ultimoDiaReset = -1;

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
double slope = 1; //1.0106;
double intercept = 0; //0.0841;
const float c = 0.982;
const float b = 0.126;

// Flujo
volatile int flow_frequency;
volatile int flow_frequency_2;
volatile unsigned long pulsos = 0;
volatile float l_min;
volatile float Liters;
float cte = 6.6;
float m = 0.72;
unsigned long tiempoAnterior = 0;
volatile float flujo_Lmin = 0;
volatile float litros_totales = 0;
unsigned long currentTime;
unsigned long cloopTime;

void IRAM_ATTR contarPulsos(){
  pulsos++;
}

// LCD
bool modo = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Timer Blynk
BlynkTimer timer;

// --- Funciones Blynk ---
void myTimer() {
  double Level_avg = 0;
  double Vair_avg = 0;
  double Vh2o_avg = 0;

  if (Level_count > 0) {
    Level_avg = Level_sum / Level_count;
    Vair_avg  = Vair_sum  / Level_count;
    Vh2o_avg  = Vh2o_sum  / Level_count;
  }

  // --- Redondeo ---
  Level_avg = round(Level_avg * 100.0) / 100.0;
  Vair_avg  = round(Vair_avg  * 1000.0) / 1000.0;
  Vh2o_avg  = round(Vh2o_avg  * 1000.0) / 1000.0;

  // Reset acumuladores
  Level_sum = 0;
  Vair_sum  = 0;
  Vh2o_sum  = 0;
  Level_count = 0;

  // Enviar nivel promedio
  Blynk.virtualWrite(V0, Level_avg);

  // Enviar porcentaje promedio
  int porcentaje_avg = total > 0 ? round(Level_avg * 100.0 / total) : 0;
  Blynk.virtualWrite(V1, porcentaje_avg);

  // Enviar voltajes promedio
  Blynk.virtualWrite(V11, Vair_avg);
  Blynk.virtualWrite(V12, Vh2o_avg);

  // -----------------------------
  // RESET DIARIO DE LITROS
  // -----------------------------
  int hora = hour();
  int minuto = minute();
  int diaActual = day();

  if (hora == 0 && minuto == 0 && diaActual != ultimoDiaReset) {
    Blynk.virtualWrite(V9, Liters); // opcional: guardar litros del día anterior

    Liters = 0;
    flow_frequency_2 = 0;
    ultimoDiaReset = diaActual;

    Serial.println("Reinicio diario de litros");
  }
}

void sendFlowRealtime() {
  if (l_min > 0) {
    Blynk.virtualWrite(V5, l_min);
    Blynk.virtualWrite(V6, Liters);
  }
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

    Vair = c * Vair + b;
    Vh2o = c * Vh2o + b;

    Vair = round(Vair * 1000.0) / 1000.0;
    Vh2o = round(Vh2o * 1000.0) / 1000.0;

    // --- Cálculo nivel ---
    Level = slope * ((slope_h2o * Vh2o - slope_air * Vair) / Vs + tol) + intercept;
    Level = round(Level * 100.0) / 100.0;
    if (Level < 0) Level = 0;
    porcentaje = Level * 100 / total;

    // --- Acumular para promedio de 1 minuto ---
    Level_sum += Level;
    Vair_sum += Vair;
    Vh2o_sum += Vh2o;
    Level_count++;

    // --- Cálculo flujo cada segundo ---
     //Medición de flujo
   currentTime = millis();
   // Every second, calculate and print litres/min
   if(currentTime >= (cloopTime + 1000)){
      cloopTime = currentTime; // Updates cloopTime

      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_min = (flow_frequency  / cte);
      // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour

      flow_frequency = 0; // Reset Counter
      Liters = flow_frequency_2 / cte / 60 / 1.4 / m;
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
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  digitalWrite(FLOW_SENSOR_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flow, FALLING);
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;

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
  timer.setInterval(60000L, myTimer);
  // Timer de 1 segundo para flujo en tiempo real
  timer.setInterval(1000L, sendFlowRealtime);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Sistema listo");
}

// --- Blynk conectado ---
BLYNK_CONNECTED() {
  rtc.begin();   // sincroniza hora con internet

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Conectado con");
  lcd.setCursor(0, 1); lcd.print("exito!");
  delay(2000);
}

// --- Loop principal (núcleo 0) ---
void loop() {
  BlynkEdgent.run();
  timer.run();
}