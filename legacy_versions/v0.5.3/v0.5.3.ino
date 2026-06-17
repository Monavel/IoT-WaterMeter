// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"

#define BLYNK_FIRMWARE_VERSION "0.5.3"

#define BLYNK_PRINT Serial
// #define BLYNK_DEBUG

#define APP_DEBUG

#include "BlynkEdgent.h"

#include <LiquidCrystal_I2C.h> // Librería para el LCD

// Aquí se asigna la funcion de los pines del ESP32
#define boton 25      // A este pin se conecta el boton que cambia la medicion que se visualiza en el LCD
#define wifi 26       // A este pin se conecta el interruptor que activa y desactiva el wifi
#define flowsensor 27 // A este pin se conecta el sensor de flujo
#define rst 33        // A este pin se conecta el boton de reset
#define aire 34       // A este pin se conecta el sensor de presión que está adentro de la caja
#define agua 35       // A este pin se conecta el sensor de presión que va al agua

const int rPin = 4;    // A este pin se conecta la terminal R del led RGB indicador de nivel
const int gPin = 5;    // A este pin se conecta la terminal G del led RGB indicador de nivel
const int zPin = 2;    // A este pin se conecta el zumbador
const int brillo = 32; // Define el brillo maximo del led RGB indicador de nivel (0 ~ 255)

// setting PWM properties
const int freq = 5000;     // Frecuencia de la señal PWM para el led RGB y el zumbador
const int ledChannel2 = 4; // Asignacion de canal para el pin R del LED RGB
const int ledChannel3 = 5; // Asignacion de canal para el pin G del LED RGB
const int ledChannel4 = 6; // Asignacion de canal para el zumbador
const int resolution = 8;  // Resolucion de la señal PWM (8 bits, de 0 a 255 en decimal)

// Declaracion de variables para la medicion de profundidad
double Level;                // Aqui se guarda el valor de profundidad
int porcentaje;              // Aqui se guarda el valor de profundidad en porcentaje
double total = 1.77;         // Esta variable es el valor maximo de profundidad del tanque donde se va a utilizar el sensor de presion
double Vair;                 // Aqui se guarda el valor de voltaje medido del sensor de presion que esta dentro de la caja
double Vh2o;                 // Aqui se guarda el valor de voltaje medido del sensor de presion que va al agua
double Vout;                 // Aqui se almacena la diferencia de voltajes medidos por los sensores
double Vs = 4.96;            // Aqui se guarda el voltaje de alimentacion de los sensores
double aux1 = 0;             // Esta variable va acumulando las mediciones de Vair para despues sacar un promedio
double aux2 = 0;             // Esta variable va acumulando las mediciones de Vh20 para despues sacar un promedio
int n = 0;                   // Este es el numero de muestras que se van a tomar para calcular el promedio
double slope_air = 25.697;   // Calibracion para el voltaje Vair
double slope_h2o = 25.69635; // Calibracion para el voltaje Vh20
double tol = 0.02;           // Calibracion para la intercepcion con el eje y
double slope = 1.0106;
double intercept = 0.0841;

// Declaracion de variables para el sensor de flujo
volatile int flow_frequency;   // Measures flow sensor pulses
volatile int flow_frequency_2; // Measures flow sensor pulses
float l_min;
unsigned long currentTime;
unsigned long cloopTime;
float Liters, cte = 9.9;

// Declaracion de la variable para la función de la pantalla
bool modo = 0; // Cuando esta variable cambia de estado se cambia la medicion que se muestra en la pantalla

// Declaracion de las variables para la ubicacion del dispositivo
double lon = 0; // Longitud en valor decimal
double lat = 0; // Latitud en valor decimal
int indice = 0; // Se utiliza como marcador para la ubicacion

WidgetMap myMap(V5); // Declaracion del objeto que envía la ubicación al mapa de la aplicación

LiquidCrystal_I2C lcd(0x27, 16, 2); // Declaracion del objeto para la pantalla. Las direcciones para la pantalla pueden ser 0x27 o 0x3f o 0x20 o 0x38 (depende del fabricante)

BlynkTimer timer;  // Esta funcion crea el objeto timer para enviar datos al servidor. Es parte de la librería Blynk
BlynkTimer timer1; // Esta funcion crea el objeto timer1 para tomar datos del sensor de presión.
BlynkTimer timer2; // Esta funcion crea el objeto timer2 para mostrar los datos en la pantalla con cada tick.

void myTimer()
{
  // Esta funcion describe qué pasará con cada tick del timer (Cada segundo)
  Blynk.virtualWrite(V0, Level);      // Escribe el valor de profundidad en el datastream V0
  Blynk.virtualWrite(V1, porcentaje); // Escribe el valor de porcentaje en el datastream V1
  Blynk.virtualWrite(V2, lon, lat);   // Escribe el valor de longitud y latitud en el datastream V2
  Blynk.virtualWrite(V5, l_min);      // Escribe el valor de flujo en el datastream V5
  Blynk.virtualWrite(V6, Liters);     // Escribe el valor de litros en el datastream V6
  Blynk.virtualWrite(V11, Vair);      // Escribe el valor de Vair en el datastream V11
  Blynk.virtualWrite(V12, Vh2o);      // Escribe el valor de Vh2o en el datastream V12
}

void Medicion()
{
  // Esta funcion describe qué pasará con cada tick del timer1 (Cada 10 ms)

  aux1 = aux1 + (float(analogRead(aire)) * 3.3 / 4095.0); // Toma la medición del sensor interno (V) y lo va sumando al valor anterior
  aux2 = aux2 + (float(analogRead(agua)) * 3.3 / 4095.0); // Toma la medición del sensor de agua (V) y lo va sumando al valor anterior
  n++;                                                    // Aumenta n en 1 cada vez que se toma una medición para llevar la cuenta de cuántas mediciones se han hecho

  // Cuando la cuenta de mediciones llega a 200 se calcula el promedio para hacer los cálculos posteriores
  if (n == 200)
  {
    Vair = aux1 / n;
    Vh2o = aux2 / n;
    // Vs = aux3/n;
    Vout = Vh2o - Vair;

    Level = slope * ((slope_h2o * Vh2o - slope_air * Vair) / Vs + tol) + intercept; // Cálculo de la profundidad con los valores medidos y las constantes de calibración

    // Si el cálculo resulta ser menor que 0 se establece en 0 la variable para evitar mediciones negativas.
    if (Level < 0)
    {
      Level = 0;
    };

    // Se calcula el porcentaje de capacidad con el valor calculado de profundidad y el valor definido para la profundidad maxima
    porcentaje = Level * 100 / total;

    // El color del LED indicador de nivel dependerá del porcentaje calculado.
    ledcWrite(ledChannel2, brillo * (1 - porcentaje / 100)); // Nivel de intensidad para el color rojo (0 cuando es 100% y brillo max cuando es 0%)
    ledcWrite(ledChannel3, brillo * porcentaje / 100);       // Nivel de intensidad paral el nivel verde (brillo max cuando es 100% y 0 cuando es 0%)

    // Si el nivel de agua es menor del 10% se envía un 'evento' a la plataforma para enviar una notificación al usuario
    if (porcentaje < 10)
    {
      Blynk.logEvent("nivel_bajo");
    }

    // Si el nivel de agua es mayor del 100% se envía un 'evento' a la plataforma para enviar una notificación al usuario
    if (porcentaje > 100)
    {
      Blynk.logEvent("nivel_alto");
    }

    // Se resetean las variables para volver a tomar mediciones y realizar el siguiente cálculo
    aux1 = 0;
    aux2 = 0;
    n = 0;
  }
}

void Pantalla()
{
  // Esta funcion describe qué pasará con cada tick del timer2 (Cada segundo)

  // Aquí se decide qué se va a mostrar en la pantalla

  if (digitalRead(boton) == 1)
  { // Verifica si el botón se ha pulsado y cambia el estado lógico de la variable 'modo'
    modo = !modo;
  }

  // Cuando la variable 'modo' vale 0 se muestra la medicion de la profundidad
  if (modo == 0)
  {
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
  }
  // Cuando la variable 'modo' vale 1 se muestra la medicion del flujo
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(l_min);
    lcd.print(" L/min");
    lcd.setCursor(0, 1);
    lcd.print(Liters);
    lcd.print(" L");
    lcd.setCursor(12, 1);
    lcd.print(porcentaje);
    lcd.setCursor(15, 1);
    lcd.print("%");
  }
}

BLYNK_WRITE(V3) // Este comando esta escuchando cuando algo se envíe desde la aplicacion al datastream V3
{
  total = param.asDouble(); // Asignacion del valor entrante del datastream V3 a la variable total
}

BLYNK_WRITE(V4) // Este comando esta escuchando cuando algo se envíe desde la aplicacion al datastream V4
{
  Vs = param.asDouble(); // Asignacion del valor entrante del datastream V4 a la variable Vs
}

BLYNK_WRITE(V7) // Este comando esta escuchando cuando algo se envíe desde la aplicacion al datastream V7
{
  tol = param.asDouble(); // Asignacion del valor entrante del datastream V3 a la variable tol
}

BLYNK_WRITE(V8) // Este comando esta escuchando cuando algo se envíe desde la aplicacion al datastream V8
{
  cte = param.asDouble(); // Asignacion del valor entrante del datastream V3 a la variable cte
}

BLYNK_WRITE(V9) // Este comando esta escuchando cuando algo se envíe desde la aplicacion al datastream V9
{
  lon = param.asDouble(); // Asignacion del valor entrante del datastream V3 a la variable lon
}

BLYNK_WRITE(V10) // Este comando esta escuchando cuando algo se envíe desde la aplicacion al datastream V10
{
  lat = param.asDouble(); // Asignacion del valor entrante del datastream V3 a la variable lat
}

void flow() // Interrupt function
{
  flow_frequency++;
  flow_frequency_2++;
}

void setup()
{
  Serial.begin(115200); // Se define la velocidad de la comunicación serial del ESP32

  // Inicializa el LCD (todos controlan el mismo LCD pero realizan diferentes funciones)
  lcd.init();  // Este corresponde al codigo principal donde se muestran las mediciones
  lcd2.init(); // Este corresponde a los mensajes de estado (librería 'CongifMode.h')
  lcd3.init(); // Este corresponde a los mensajes de error (librería 'ConfigStore.h')
  // turn on LCD backlight
  lcd.backlight();
  lcd2.backlight();
  lcd3.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando..."); // Mensaje de bienvenida

  // Se fuerza a estar en estado bajo los siguientes pines para evitar comportamientos no deseados (e. g. Que cambie lo que se muestre en la pantalla
  // sin haber pulsado el boton, que se desconecte del wifi, que se resetee o que se muestren mediciones aleatorias del sensor de agua)
  pinMode(boton, INPUT_PULLDOWN);
  pinMode(rst, INPUT_PULLUP); // Este pin funciona con lógica inversa (se pone en estado bajo cuando se aprieta el boton)
  pinMode(wifi, INPUT_PULLDOWN);
  pinMode(agua, INPUT_PULLDOWN);

  pinMode(flowsensor, INPUT_PULLUP);
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  attachInterrupt(digitalPinToInterrupt(flowsensor), flow, FALLING);
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;

  // Aquí se configuran las funciones para el PWM del LED indicador de nivel y el zumbador
  ledcSetup(ledChannel2, freq, resolution);
  ledcSetup(ledChannel3, freq, resolution);
  ledcSetup(ledChannel4, freq, resolution);

  // Asocia los pines del LED indicador de nivel y el zumbador a los canales que los van a controlar
  ledcAttachPin(rPin, ledChannel2);
  ledcAttachPin(gPin, ledChannel3);
  ledcAttachPin(zPin, ledChannel4);

  BlynkEdgent.begin(); // Corre las instrucciones de las liberías de Blynk para conectarse al servidor

  myMap.location(indice, lat, lon, "Tinaco"); // Envía los parámetros de la ubicación al servidor de Blynk para mostralos en el Dashboard

  // Se definen los intervalos de tiempo para que se ejecuten las instrucciones de cada timer
  timer.setInterval(1000L, myTimer);   // Intervalo de 1000 ms para mandar datos a la nube de Blynk Cloud. Los datos serán enviados cada segundo.
  timer1.setInterval(10L, Medicion);   // Cada 10 ms se ejecuta la función que lee los valores de voltaje de los sensores.
  timer2.setInterval(1000L, Pantalla); // Cada 1000 ms (1 segundo) se actualiza la pantalla para mostrar las mediciones.
}

// Cuando el dispositivo logra conectarse a la nube de Blynk se muestra el siguiente mensaje
BLYNK_CONNECTED()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectado con");
  lcd.setCursor(0, 1);
  lcd.print("exito!");
  delay(2000);
}

void loop()
{
  // Si no hay conexion a internet se desactiva el switch de wifi para que el programa funcione normalmente
  if (digitalRead(wifi) == 0)
  {
    BlynkEdgent.run();
  }
  else
  {
  }

  // Cuando se presiona el botón de reset se muestra el siguiente mensaje en la pantalla
  // con un contador de 10 segundos para saber cuándo hay que dejar de pulsar el botón
  while (digitalRead(rst) == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("Pulse el boton");
    lcd.setCursor(0, 1);
    lcd.print("por ");
    for (int i = 10; i >= 0; i--)
    {
      if (digitalRead(rst) == 1)
      {
        break;
      } // Si se deja de pulsar el botón se interrumpe el contador
      lcd.setCursor(4, 1);
      lcd.print(i);
      lcd.print(" segundos ");
      delay(1000);
    }
  }

  // Medición de flujo
  currentTime = millis();
  // Every second, calculate and print litres/min
  if (currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime; // Updates cloopTime

    // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
    l_min = (flow_frequency / cte);
    // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour

    flow_frequency = 0; // Reset Counter
    Liters = flow_frequency_2 * cte / 3600;
  }

  // Runs all Blynk stuff
  Blynk.run();

  // runs Timers
  timer.run();
  timer1.run();
  timer2.run();
}
