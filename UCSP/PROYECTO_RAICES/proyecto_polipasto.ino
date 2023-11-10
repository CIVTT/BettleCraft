#include <SoftwareSerial.h>
#include <ModbusRTUMaster.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

#define alpha 0.2  //Alpha
#define pi 3.1416
//unsigned long time=millis();
float vel=0;
float rpm=0;
File myFile;
//inicializador de toma de datos en sdcard
boolean running = false;
int adc_filtrado = 0;
int adc_raw = 0;
int sensorPin = A0;
uint16_t SETFREC=0;
String estado="off";
const uint8_t rxPin = 9;
const uint8_t txPin = 10;
const uint8_t dePin = 5;
int analogPin= A0;
float valor_galga = 0;
int valor_in=0;


SoftwareSerial mySerial(rxPin, txPin);
ModbusRTUMaster modbus(mySerial, dePin);
LiquidCrystal_I2C lcd(0x27,20,4);

long requestUserInput() {
  //Serial.print("> ");
  while (!Serial.available()) {}
  String userInput = Serial.readStringUntil('\n');
  userInput.trim();
  //Serial.println(userInput);
  return userInput.toInt();
}

long selectValue(long minValue, long maxValue) {
  long value;
  while (1) {
    value = requestUserInput();
    if (value < minValue or value > maxValue) ;//Serial.println(F("Entrada invalida, de nuevo por favor"));
    else return value;
  }
}
void processError() {
  if (modbus.getTimeoutFlag()) {
    Serial.println(F("Conexion fuera de tiempo"));
    modbus.clearTimeoutFlag();
  }
  else if (modbus.getExceptionResponse() != 0) {
    Serial.print(F("Received exception response: "));
    Serial.print(modbus.getExceptionResponse());
    modbus.clearExceptionResponse();
  }
  else {
    Serial.println("Ocurrio un error");
  }
}

void star(){
  //Serial.println(F("Valid entries: 0 - 5000"));
running=true;
  uint16_t value = 1;
  if (modbus.writeSingleHoldingRegister(01, 8192, value)) {
    //Serial.println(F("ENCENDIDO"));
    estado="on";
  }
  
  //else {
    //processError();
    //estado="off";
   // }
}
void stop(){
//Serial.println(F("Valid entries: 0 - 5000"));
running=false;
  uint16_t value = 7;
  if (modbus.writeSingleHoldingRegister(01, 8192, value)) {
    //Serial.println(F("MOTOR EN STOP"));
    estado="off";
  }
  //else processError();
}
void frec_in(uint16_t value){
  //uint16_t value = selectValue(0, 5000);
  if (modbus.writeSingleHoldingRegister(01, 8193, value)) {
    /*Serial.print(F("Frec deseada: "));
    Serial.print(value);*/
    valor_in = value;
  }
  //else processError();
}
void frec_set(){
  uint16_t value = 0;
  if (modbus.readHoldingRegisters(01,8451 , &value, 1)) {
    
    SETFREC= value;
  }
  //else //processError();
}
void readValue() {
  //Serial.println(F("Ingrese el valor deseado"));
  //Serial.println(F("Entrada validas: 0 - 5000"));
  uint16_t in = selectValue(0, 5000);
  frec_in(in);
}
void read_actual(){
  uint16_t value = 0;
  if (modbus.readHoldingRegisters(01,8450 , &value, 1)) {
    /*Serial.print(F("Frec set: "));
    Serial.print(value);*/
    valor_in= value;
}}
void setup() {
  
Serial.begin(9600);
 Serial.setTimeout(100);
  while(!Serial) {;}
// library data rs 485 and modbus vdf
modbus.begin(9600); // modbus baud rate, config (optional)
  read_actual();
// Creando en la SD un csv
 
if (!SD.begin(4)) {
    //Serial.println("initialization failed!");
    //while (1);
  }
     if(!SD.exists("data.csv")){
 myFile = SD.open("data.csv", FILE_WRITE);
      if (myFile) {
        //Serial.println("Archivo nuevo, Escribiendo encabezado(fila 1)");
        myFile.println("time,RPM,Vo,TS");
        myFile.close();
      } else {

        //Serial.println("Error creando el archivo datalog.csv");
      }
  }

//ETCG Notes - Start LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Iniciando espere...");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("------UCSP------");
  lcd.setCursor(0,1);
  lcd.print("PROYEC POLIPASTO");
  
  //delay(1000);
}

void loop(){
  frec_set();
 control_serial();
 peso();
  // controlado por serial
  /*Serial.println(F("Selecciona una accion"));
  Serial.println(F("a - INICIO"));
  Serial.println(F("b - STOP"));
  Serial.println(F("c - ESCRIBE LA FRECUENCIA DESEADA"));*/
  char caracter=Serial.read();
 // char point;
 // caracter.toCharArray(point, 1);
  switch (caracter) {
    case 'a'://star
      star();
      break;
    case 'b'://stop
      stop();
      break;
    case 'c'://valor frecuencia asignado
      readValue();
      //Serial.print("123");
      break;
  }
//delay(500);
velos();
  sd_card();
  Serial.println();
 //delay(10000);
  // codigo para ldc display
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("RPM:");
  lcd.setCursor(4, 0);
  lcd.print(rpm);

  lcd.setCursor(9,0);
  lcd.print("Vo:");
  lcd.setCursor(12,0);
  lcd.print(vel);
  // segunda linea
  lcd.setCursor(0,1);
  lcd.print("Ts: ");
  lcd.setCursor(3,1);
  lcd.print(valor_galga);
  lcd.setCursor(9,1);
  lcd.print("PWR:");
  lcd.setCursor(13,1);
  lcd.print(estado);
  //
}
void velos(){
//velocidad en mm/s
SETFREC=SETFREC/100;
if(SETFREC<6){
  rpm=0; 
  vel=0;
}else {
  rpm=SETFREC*0.741 - 1.43;
  vel=rpm*pi*80/30;
  }
}
void peso(){
  // read the value from the sensor:
  adc_raw = analogRead(sensorPin);
  int data;
  adc_filtrado = (alpha*adc_raw) + ((1-alpha)*adc_filtrado);
valor_galga=(500*adc_filtrado)/1023;
}
void control_serial(){
//display serial
  Serial.print(F("SET FREC:"));
  Serial.println(rpm);
  Serial.print(F("OUT FREC:"));
  Serial.println(vel);
  Serial.print(F("TENSION:"));
  Serial.println(valor_galga);
  Serial.print(F("ESTADO:"));
  Serial.println(estado);

}
void sd_card(){

  if (running){
myFile = SD.open("data.csv", FILE_WRITE);//abrimos  el archivo
  
  if (myFile) { 
        //Serial.print("Escribiendo SD: ");
        myFile.print("time");
        myFile.print(",");
        myFile.print(rpm);
        myFile.print(",");
        myFile.print(vel);
        myFile.print(",");
        myFile.println(valor_galga);
               
        myFile.close(); //cerramos el archivo

  } else {
  	// if the file didn't open, print an error:
    
  }
  //delay(100);
}
}
