// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPLaEzS0d_s"
#define BLYNK_DEVICE_NAME "Medidor de profundidad"  

#define BLYNK_FIRMWARE_VERSION        "0.3.3"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

#include <LiquidCrystal_I2C.h> // Librería para el LCD

#define boton 25
#define wifi 26
#define flowsensor 27
#define rst 33
#define aire 34
#define agua 35

//const int rojoPin = 4;
//const int greenPin = 5;
//const int bluePin = 14;

// setting PWM properties
const int freq = 5000;
const int ledChanne2 = 8;
const int ledChanne3 = 8;
//const int ledChanne4 = 2;
const int resolution = 8;

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7
//#define USE_ESP32C3_DEV_MODULE
//#define USE_ESP32S2_DEV_KIT

#include "BlynkEdgent.h"
// Declaring a global variable for sensor data
double Level,Vair, Vh2o,P,Vs=4.97, Vout, lon = -98.77083, lat = 20.0625;
double aux1=0, aux2=0, slope_air = 25.697, slope_h2o = 25.69635;
double tol=-0.0995; // Ajusta la medida de presión
int i, rho = 997, n=0, porcentaje, indice=0;
double g=9.8, total=1.2;
bool modo=0;

volatile int flow_frequency; // Measures flow sensor pulses
volatile int flow_frequency_2; // Measures flow sensor pulses
float l_hour;
unsigned long currentTime;
unsigned long cloopTime;
float Liters, cte = 9.86;

int ReCnctFlag;  // Reconnection Flag
int ReCnctCount = 0;  // Reconnection counter

WidgetMap myMap(V5);

LiquidCrystal_I2C lcd(0x3f, 16, 2);   //Las direcciones para la pantalla LCD son 0x27 o 0x3f o 0x20 o 0x38
//Hasta aquí lo de la pantalla

// This function creates the timer object. It's part of Blynk library 
BlynkTimer timer; 
BlynkTimer timer1;
BlynkTimer timer2;

void myTimer() 
{
  // This function describes what will happen with each timer tick
  // e.g. writing sensor value to datastream V0
  Blynk.virtualWrite(V0, Level);
  Blynk.virtualWrite(V1, porcentaje);
  Blynk.virtualWrite(V2, lon, lat);
  Blynk.virtualWrite(V5, l_hour);
  Blynk.virtualWrite(V6, Liters); 
}

void Medicion()
{
    aux1 = aux1 + (float(analogRead(aire))*3.3/4095.0); //v
    aux2 = aux2 + (float(analogRead(agua))*3.3/4095.0); //v
    n++;

    if(n==200){
      Vair = aux1/n;
      Vh2o = aux2/n;
      //Vs = aux3/n;
      Vout = Vh2o - Vair;

      //Serial.println(Vh2o);

      Level = (slope_h2o * Vh2o - slope_air * Vair)/Vs + tol;

      if (Level<0){
        Level = 0; 
      };  

      porcentaje = Level * 100/total;

      aux1=0;
      aux2=0;
      n=0; 
    }
}

void Pantalla(){
  //Aquí se decide qué se va a mostrar en la pantalla
  if (digitalRead(boton) == 1){
    modo = !modo; 
  }
  if (modo == 0){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Profundidad");
    lcd.setCursor(0,1);
    lcd.print(Level);
    lcd.setCursor(6,1);
    lcd.print("m");
    lcd.setCursor(12,1);
    lcd.print(porcentaje);
    lcd.setCursor(15,1);
    lcd.print("%");
  }
  else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(l_hour);
    //lcd.setCursor(5, 0);
    lcd.print(" L/min");
    lcd.setCursor(0,1);
    lcd.print(Liters);
    //lcd.setCursor(5,1);
    lcd.print(" L");
    lcd.setCursor(12,1);
    lcd.print(porcentaje);
    lcd.setCursor(15,1);
    lcd.print("%");
  }
}

BLYNK_WRITE(V3) // this command is listening when something is written to V3
{
  total = param.asDouble(); // assigning incoming value from pin V3 to a variable
}

BLYNK_WRITE(V4) // this command is listening when something is written to V4
{
  Vs = param.asDouble(); // assigning incoming value from pin V4 to a variable
}

BLYNK_WRITE(V7) // this command is listening when something is written to V7
{
  tol = param.asDouble(); // assigning incoming value from pin V7 to a variable
}

BLYNK_WRITE(V8) // this command is listening when something is written to V8
{
  cte = param.asDouble(); // assigning incoming value from pin V8 to a variable
}

BLYNK_WRITE(V9) // this command is listening when something is written to V9
{
  lon = param.asDouble(); // assigning incoming value from pin V9 to a variable
}

BLYNK_WRITE(V10) // this command is listening when something is written to V10
{
  lat = param.asDouble(); // assigning incoming value from pin V10 to a variable
}

void flow() // Interrupt function
{
flow_frequency++;
flow_frequency_2++;
}

void setup()
{
  Serial.begin(115200);
  pinMode(boton, INPUT_PULLDOWN);
  pinMode(rst, INPUT_PULLUP);
  pinMode(wifi, INPUT_PULLDOWN);
  pinMode(agua, INPUT_PULLDOWN);

  pinMode(flowsensor, INPUT_PULLUP);
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  attachInterrupt(digitalPinToInterrupt(flowsensor), flow, FALLING);
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;

  // initialize LCD
  lcd.init();                      
  lcd2.init();
  lcd3.init();
  // turn on LCD backlight
  lcd.backlight();                      
  lcd2.backlight();
  lcd3.backlight();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Iniciando...");

  // configure LED PWM functionalitites
  //ledcSetup(ledChanne2, freq, resolution);
  //ledcSetup(ledChanne3, freq, resolution);
  //ledcSetup(ledChanne3, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  //ledcAttachPin(rojoPin, ledChanne2);
  //ledcAttachPin(greenPin, ledChanne3);
  //ledcAttachPin(bluePin, ledChanne3);

  BlynkEdgent.begin();

  myMap.location(indice, lat, lon, "Tinaco");

  // Setting interval to send data to Blynk Cloud to 1000ms. 
  // It means that data will be sent every second
  timer.setInterval(1000L, myTimer);
  timer1.setInterval(10L, Medicion);
  timer2.setInterval(1000L, Pantalla);
  delay(100); 
}

BLYNK_CONNECTED() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Conectado con");
  lcd.setCursor(0,1);
  lcd.print("exito!");
  delay(2000);
}

void loop() {
  // Si no hay conexion a internet se activa el botón para que el programa funcione normalmente
  if(digitalRead(wifi)==0){BlynkEdgent.run();}
  while (digitalRead(rst) == 0){
    lcd.setCursor(0, 0);
    lcd.print("Pulse el boton");
    lcd.setCursor(0,1);
    lcd.print("por ");
    for(int i=10;i>=0;i--){
      if(digitalRead(rst)==1){break;}
      lcd.setCursor(4,1);
      lcd.print(i);
      lcd.print(" segundos ");
      delay(1000);
    }
  }
   //Medición de flujo
   currentTime = millis();
   // Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000)){
   cloopTime = currentTime; // Updates cloopTime

   // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
   l_hour = (flow_frequency  / cte);
   // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour

   flow_frequency = 0; // Reset Counter
   Liters = flow_frequency_2*cte/3600;
   }

  // Runs all Blynk stuff
  Blynk.run();

  // runs BlynkTimer
  timer.run();
  timer1.run();
  timer2.run();
}


