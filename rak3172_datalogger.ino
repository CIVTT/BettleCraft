/*****LIBRERIAS*****/
#include <SD.h>
#include <SPI.h>

/*****CONSTANTES*****/
#define pin     PA13
#define pinCS   PA4
#define tsample 1000

/*****VARIABLES*****/
int segundos=0, minutos=0, horas=0;
unsigned long tiempoActual, tiempoAnterior;
bool estadoLed;
File dataFile;

/*****SECUENCIA DE INICIO*****/
void setup() {
  pinMode(pin,OUTPUT);
  Serial.begin(115200);
  for(int i=0;i<5;i++){
    estadoLed=!estadoLed;
    digitalWrite(pin,estadoLed);
    delay(100);
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(pinCS)) {
    Serial.println("Card failed, or not present");
  }
  else{
    dataFile = SD.open("data.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.println("*************REINICIO***********");
      dataFile.close();
      Serial.println("uSd initialized");
    } else {
      Serial.println("Error al abrir el archivo en la tarjeta microSD.");
    }
  }
  tiempoAnterior=millis();
}

/*****SECUENCIA PRINCIPAL*****/
void loop() {
  tiempoActual=millis();
  if(tiempoActual-tiempoAnterior>=tsample){
    tiempoAnterior=tiempoActual;    
    segundos=segundos+1;
    if (segundos>=60){
      segundos=0;
      minutos=minutos+1;
      if(minutos>=60){
        minutos=0;
        horas=horas+1;
      }
    }
    digitalWrite(pin,estadoLed);
    estadoLed=!estadoLed;
    if (SD.begin()) {
      dataFile = SD.open("data.txt", FILE_WRITE);
      if (dataFile) {
        dataFile.println(mostrar());
        dataFile.close();
        Serial.println("Datos guardados en la tarjeta microSD.");
        
      } 
      else {
        Serial.println("Error al abrir el archivo en la tarjeta microSD.");
      }
      SD.end();
    } 
    else {
      Serial.println("Tarjeta microSD no encontrada.");
    }
  }
}

String mostrar(){
  String timeStamp;
  if(horas<10){
    timeStamp.concat(0);
  }
  timeStamp.concat(horas);
  timeStamp.concat(":");
  if(minutos<10){
    timeStamp.concat(0);
  }
  timeStamp.concat(minutos);
  timeStamp.concat(":");
  if(segundos<10){
    timeStamp.concat(0);
  }
  timeStamp.concat(segundos);
  Serial.println(timeStamp);
  return timeStamp;  
}