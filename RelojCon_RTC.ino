#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc; 

byte Digit[12][8] =
{
  { 1, 1, 1, 1, 1, 1, 0, 0 }, // 0
  { 0, 1, 1, 0, 0, 0, 0, 0 }, // 1
  { 1, 1, 0, 1, 1, 0, 1, 0 }, // 2
  { 1, 1, 1, 1, 0, 0, 1, 0 }, // 3
  { 0, 1, 1, 0, 0, 1, 1, 0 }, // 4
  { 1, 0, 1, 1, 0, 1, 1, 0 }, // 5
  { 1, 0, 1, 1, 1, 1, 1, 0 }, // 6
  { 1, 1, 1, 0, 0, 0, 0, 0 }, // 7
  { 1, 1, 1, 1, 1, 1, 1, 0 }, // 8
  { 1, 1, 1, 0, 0, 1, 1, 0 }, // 9
  { 1, 1, 0, 0, 0, 1, 1, 0 }, // grados
  { 1, 0, 0, 1, 1, 1, 0, 0 } // celsius
};

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int sensorTemperatura = A0; // Sensor de temperatura LM35
int temperatura;
int estadoBotonCambioHora = HIGH;
int opcion = 0;
int ledSegundero = 13; //pin 13
int ledStateSeconds = LOW;  //state of seconds led in pin 13
long interval = 500; //500 ms tiempo intervalo encendido y apapgado de segundero
long previousMillis=0;
DateTime now;
 
void setup() {
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  for (int i = 2; i < 14; i++)
  {
    pinMode(i, OUTPUT); //salida para leds
  }
  pinMode(sensorTemperatura,INPUT);//pin de sensor de temperatura
  pinMode(A1,INPUT); //pin boton cambio hora/minuto
  pinMode(A2,INPUT); //pin subir conteo de hora/minuto

}

void Display(int pos, int N)
{
  digitalWrite(9 , LOW);       // Apaga todos los digitos
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);

  for (int i = 0 ; i < 8 ; i++) // Esto no cambia de la session anterior
    digitalWrite(i + 2 , Digit[N][i]) ;

  digitalWrite(pos + 9, HIGH);      // Enciende el digito pos
}

void parpadeoHora(int Num) {
  int Digit3 = Num % 10 ;
  int Digit2 = (Num % 100) / 10 ;
  int Digit1 = (Num % 1000) / 100 ;
  int Digit0 = Num  / 1000  ;

  Display(3 , Digit3);
  Display(2 , Digit2);
  Display(1 , Digit1);
  Display(0 , Digit0);
  delay(500);
  digitalWrite(9, LOW); //apagagas el primer display
  digitalWrite(10,LOW); // apagas el segundo display
  delay(500);  
}

void parpadeoMinutos(int Num) {
  int Digit3 = Num % 10 ;
  int Digit2 = (Num % 100) / 10 ;
  int Digit1 = (Num % 1000) / 100 ;
  int Digit0 = Num  / 1000  ;

  Display(3 , Digit3);
  Display(2 , Digit2);
  Display(1 , Digit1);
  Display(0 , Digit0);
  delay(500);
  digitalWrite(11, LOW); //apagagas el tercer display
  digitalWrite(12,LOW); // apagas el cuarto display
  delay(500);  
}

void displayTemperatura( int Num)
{
  int Digit0 = Num % 10 ;
  int Digit1 = Num / 10 ;

  Display(0 , Digit1);
  Display(1 , Digit0);
  Display(2 , 10);
  Display(3 , 11);
}

void displayTime(int tiempo){
  int Digit3 = tiempo % 10 ;
  int Digit2 = (tiempo % 100) / 10 ;
  int Digit1 = (tiempo % 1000) / 100 ;
  int Digit0 = tiempo  / 1000  ;

  Display(3 , Digit3);
  Display(2 , Digit2);
  Display(1 , Digit1);
  Display(0 , Digit0);
}

void loop() {
 
  now = rtc.now();
  int horas=now.hour();
  int minutos =now.minute();
  int segundos=now.second();
  int tiempo = (horas*100)+minutos; //horas apasado a formato decimal hhmm
  
  if (digitalRead(A1) == LOW) {  // si se presiono el boton de cambio de hora
    estadoBotonCambioHora = LOW;
    opcion++;
    opcion = opcion % 3;
    while (digitalRead(A1) == LOW);
  }

  if (estadoBotonCambioHora == LOW) { 
      switch (opcion) {
        
        case 1: //parpadear horas
                parpadeoHora(tiempo);
                if (digitalRead(A2) == LOW) {
                   horas++;
                    rtc.adjust(DateTime(F(__DATE__), horas,now.minute(),now.second()));
                    while (digitalRead(A2) == LOW);
                }
                break;
                
        case 2: //parpadear minutero
                parpadeoMinutos(tiempo);
                if (digitalRead(A2) == LOW) {
                 rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute()+1, now.second()));
                  while (digitalRead(A2) == LOW);
                }
                break;

        case 0:
                estadoBotonCambioHora = HIGH;
                break;
    }

  } else { // si no hay ningun boton pulsado

    if (segundos >= 30 && segundos <= 35) {
        // Se mostrara la temperatura en el display en el segundo 30
        temperatura = (5.0 * analogRead(sensorTemperatura) * 100.0) / 1024.0;
        displayTemperatura(temperatura);  // función para visualizar el número guardado en la variable "temperatura"
    } else {

      displayTime(tiempo);
      unsigned long currentMillis = millis();    // Se toma el tiempo actual

        // se comprueba si el tiempo actual menos el tiempo en que el LED cambió
        // de estado por última vez es mayor que el intervalo.
        if (currentMillis - previousMillis > interval) {

          // Si se cumple la condición se guarda el nuevo tiempo
          // en el que el LED cambia de estado
          previousMillis = currentMillis;

          // Y ahora cambiamos de estado el LED, si está encendido a
          // apagado o viceversa.
          if (ledStateSeconds== LOW) {
            ledStateSeconds = HIGH;
          } else {
            ledStateSeconds = LOW;
          }
          // Hacemos que el contenido de la variable llegue al LED
          digitalWrite(ledSegundero, ledStateSeconds);
        }
    }
  }
}
