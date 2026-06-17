// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"

#define BLYNK_FIRMWARE_VERSION "0.1.9"

#define BLYNK_PRINT Serial
// #define BLYNK_DEBUG

#define APP_DEBUG

#include <LiquidCrystal_I2C.h> // Librería para el LCD

#define rst 33

// Uncomment your board, or configure a custom board in Settings.h
// #define USE_WROVER_BOARD
// #define USE_TTGO_T7
// #define USE_ESP32C3_DEV_MODULE
// #define USE_ESP32S2_DEV_KIT

#include "BlynkEdgent.h"
// Declaring a global variable for sensor data
double Level, Vair, Vh2o, P, Vs, Vout, lon = -98.77083, lat = 30.0625;
double aux1, aux2, slope_air = 25.697, slope_h2o = 25.69635;
double tol = -0.030; // Ajusta la medida de presión
int i, rho = 997, n = 100, porcentaje, indice = 0;
double g = 9.8, total;

WidgetMap myMap(V5);

LiquidCrystal_I2C lcd(0x3f, 16, 2);

// Hasta aquí lo de la pantalla

// This function creates the timer object. It's part of Blynk library
BlynkTimer timer;

void myTimer()
{
  // This function describes what will happen with each timer tick
  // e.g. writing sensor value to datastream V0
  Blynk.virtualWrite(V0, Level);
  Blynk.virtualWrite(V1, porcentaje);
  Blynk.virtualWrite(V2, lon, lat);
}

BLYNK_WRITE(V3) // this command is listening when something is written to V3
{
  total = param.asDouble(); // assigning incoming value from pin V3 to a variable
}

BLYNK_WRITE(V4) // this command is listening when something is written to V3
{
  Vs = param.asDouble(); // assigning incoming value from pin V3 to a variable
}

void setup()
{
  Serial.begin(115200);
  pinMode(rst, OUTPUT);

  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();

  BlynkEdgent.begin();

  myMap.location(indice, lat, lon, "Cisterna");

  // Setting interval to send data to Blynk Cloud to 1000ms.
  // It means that data will be sent every second
  timer.setInterval(1000L, myTimer);
  delay(100);
}

void loop()
{
  BlynkEdgent.run();
  digitalWrite(rst, LOW);
  aux1 = 0;
  aux2 = 0;
  // aux3=0;
  for (i = 0; i < n; i++)
  {
    aux1 = aux1 + (float(analogRead(34)) * 3.3 / 4095.0); // v
    aux2 = aux2 + (float(analogRead(35)) * 3.3 / 4095.0); // v
    // aux3 = aux3 + (float(analogRead(14))*3.3/4095.0); //v
    delay(5);
  }
  Vair = aux1 / n;
  Vh2o = aux2 / n;
  // Vs = aux3/n;
  Vout = Vh2o - Vair;

  // Presión en Kpa según gráfica 4 del Datasheet
  // P = ( Vout + 0.04*Vs ) / (0.004 * Vs) + tolP; //kPa

  // Level = ((P*1000)/(rho*g));  //Medida de Nivel del tanque
  // Level = 1.081349*Level + 0.219574;

  Level = (slope_h2o * Vh2o - slope_air * Vair) / Vs + tol;

  if (Level < 0)
  {
    Level = 0;
  };

  // Serial.print("Presión:");
  // Serial.print(P);

  porcentaje = Level * 100 / total;

  // Serial.print("Vh20: ");
  // Serial.println(Vh2o);
  // Serial.print("Vair: ");
  // Serial.println(Vair);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Profundidad");
  lcd.setCursor(0, 1);
  lcd.print(Level);
  lcd.setCursor(6, 1);
  lcd.print("m");
  lcd.setCursor(12, 1);
  lcd.print(porcentaje);
  lcd.setCursor(15, 1);
  lcd.print("%");

  delay(1000);

  // Runs all Blynk stuff
  Blynk.run();

  // runs BlynkTimer
  timer.run();
  // delay(1000);
}
