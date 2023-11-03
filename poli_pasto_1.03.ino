#include <ModbusRTUMaster.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define alpha 0.2  //Alpha
#define pi 3.1416
#define rxPin 8
#define txPin 9
#define dePin 7
#define btn_1 2
#define btn_2 3
#define sensorPin A0
int frecc[12] = { 0, 598, 665, 733, 800, 868, 935, 1003, 1070, 1138, 1205, 1273 };
int con_fre = 0;
int cont = 0;
int adc_filtrado = 0;
int adc_raw = 0;
int pulsado_1 = HIGH;
int pulsado_2 = HIGH;
int estado_1 = HIGH;
int estado_2 = HIGH;
unsigned long fin_1, ini_1, ini_2, fin_2;
const unsigned long periodo=1000;
unsigned long perante=0;
char caracter;
float SETFREC = 0;
String dat_1 = "";
boolean running = false;
SoftwareSerial puerta(rxPin, txPin);
ModbusRTUMaster modbus(puerta, dePin);
LiquidCrystal_I2C lcd_1(0x27, 20, 4);
float valor_galga = 0;
int valor_in = 0;
float vel = 0;
float rpm = 0;
File my_File;
String onoff = "OFF";

void star() {
  //Serial.println(F("mandando"));

  //Serial.println(running);
  uint16_t value = 1;
  if (modbus.writeSingleHoldingRegister(01, 8192, value)) {
    //Serial.println(F("ENCENDIDO"));
  }  //else processError();
  running = true;
  onoff = "ON";
}
void stop() {
  //Serial.println(F("parando..."));

  //Serial.println(running);
  uint16_t value = 7;
  if (modbus.writeSingleHoldingRegister(01, 8192, value)) {
  }  // else processError();
  running = false;
  onoff = "OFF";
}
void envio(uint16_t value) {
  //uint16_t vall = value;
  
  if (modbus.writeSingleHoldingRegister(01, 8193, value)) {
    //valor_in = value;
  }
  //else processError();
}
void ini_sd() {

  Serial.println("SD inicializada.");
  if (!SD.exists("data.txt")) {
    //Serial.println("sd inicializada");
    my_File = SD.open("data.txt", FILE_WRITE);
    if (my_File) {
      //Serial.println("Archivo nuevo, Escribiendo encabezado(fila 1)");
      my_File.println("seg;rpm;mm/s;kg");
      my_File.close();
    } else {
      //Serial.println("Error creando el archivo data.csv");
    }
  } //else {  //Serial.println("ya existe"); }
}
void setup() {
  pinMode(btn_1, INPUT_PULLUP);
  pinMode(btn_2, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.setTimeout(100);
  while (!Serial) { ; }
  // library data rs 485 and modbus vdf
  modbus.begin(9600);  // modbus baud rate, config (optional)
  //read_actual();
  if (!SD.begin(10)) {
    //Serial.println("initialization failed!");
    lcd_1.init();
    lcd_1.backlight();
    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("NO SD");
    //delay(500);
    while (1)
      ;
  }
  ini_sd();
  //ETCG Notes - Start LCD
  lcd_1.init();
  lcd_1.backlight();
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Iniciando espere...");
  delay(1000);
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("------UCSP------");
  lcd_1.setCursor(0, 1);
  lcd_1.print("PROYEC POLIPASTO");
  delay(1000);
}
void loop() {

  calculate();
  display();

  if (Serial.available()) {
    caracter = Serial.read();
    switch (caracter) {
      case 'a':  //star
        running = false;
        star();
        break;
      case 'b':  //stop
        running = false;
        stop();
        break;
      case 'c':
        lectura();
        break;
      case 'd':
        borrado();
        break;

      case '+':
        con_fre=con_fre+1;
        if(con_fre>11) con_fre=0;
        envio((uint16_t)frecc[con_fre]);
        break;
      case '-':
        con_fre=con_fre-1;
        if(con_fre<0) con_fre=11;
        envio((uint16_t)frecc[con_fre]);
        break;
    }
  }

  estado_1 = digitalRead(btn_1);
  if (estado_1 != pulsado_1) {
    if (estado_1 == LOW && pulsado_1 == HIGH) {
      ini_1 = millis();
      pulsado_1 = estado_1;
      delay(30);
    }
    if (estado_1 == HIGH && pulsado_1 == LOW) {
      fin_1 = millis() - ini_1;
      pulsado_1 = estado_1;
      delay(30);
      if (fin_1 > 2000) {
        star();
      }
      if (fin_1 > 60 && fin_1 < 500) { 
        con_fre=con_fre+1;
        if(con_fre>11) con_fre=0;
        envio((uint16_t)frecc[con_fre]);
      }
      pulsado_1 = estado_1;
    }
  }

  //boton menos y stop
  estado_2 = digitalRead(btn_2);
  if (estado_2 != pulsado_2) {
    //cont=0;
    if (estado_2 == LOW && pulsado_2 == HIGH) {
      ini_2 = millis();
      //count++;
      pulsado_2 = estado_2;
      delay(30);
    }
    if (estado_2 == HIGH && pulsado_2 == LOW) {
      fin_2 = millis() - ini_2;
      delay(30);
      if (fin_2 > 2000) {
        stop();
      }
      if (fin_2 > 60 && fin_2 < 500) {
        con_fre=con_fre-1;
        if(con_fre<0) con_fre=11;
        envio((uint16_t)frecc[con_fre]);
      }
      pulsado_2 = estado_2;
    }
  }

  if (running) {
    unsigned long peri_act=millis();
    if(peri_act-perante>=periodo){
    cont = cont + 1;
    //while (millis() % 1000 != 0) {}
    sd_card();
    perante=peri_act;
    }
  } else {
    cont = 0;
    //my_File.close();
  }
}
void display() {
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("RPM:");
  lcd_1.setCursor(4, 0);
  lcd_1.print(rpm);
  lcd_1.setCursor(9, 0);
  lcd_1.print("Vo:");
  lcd_1.setCursor(12, 0);
  lcd_1.print(vel);
  // segunda linea
  lcd_1.setCursor(0, 1);
  lcd_1.print("Ts: ");
  lcd_1.setCursor(3, 1);
  lcd_1.print(valor_galga);
  lcd_1.setCursor(9, 1);
  lcd_1.print("PWR:");
  lcd_1.setCursor(13, 1);
  lcd_1.print(onoff);
  while (millis() % 300 != 0) {}
}
void calculate() {
  //velocidad en mm/s

  SETFREC = float(frecc[con_fre]) / 100;
  //SETFREC = SETFREC / 100; //posible cambio si falla por interferencia en variador y serial, comentar y descomenrtar anterior linea
  rpm = (SETFREC * 0.741) - 1.43;
  if (rpm <= 0) {
    rpm = 0;
    vel = 0;
  }
  if (rpm > 0) {
    vel = rpm * pi * 80 / 30;
  }
  //peso
  adc_raw = analogRead(sensorPin);
  adc_filtrado = (alpha * adc_raw) + ((1 - alpha) * adc_filtrado);
  valor_galga = (500 * adc_filtrado) / 1023;
  //dat_1 = String(cont) + ";" + String(rpm, 2) + ";" + String(vel, 2) + ";" + String(valor_galga, 2);
}

void lectura() {
  //Serial.println("leendo..");

  my_File = SD.open("data.txt");  //, FILE_WRITE);
  if (my_File) {

    while (my_File.available()) {
      Serial.write(my_File.read());  // En un caso real se realizarían las acciones oportunas
    }
    my_File.close();
    //Serial.println("enviado por serial");
  } else {
    //Serial.println("Error al abrir el archivo");
  }
}
void borrado() {
  //Serial.println("Removiendo data.csv...");
  SD.remove("data.txt");
  ini_sd();
  // Serial.println("Removido data.csv");
}

void sd_card() {
  //Serial.println("escribiendo data... ");
  //dat_1=String(cont)+";"+String(rpm)+";"+String(vel)+";"+String(valor_galga);
  if (SD.exists("data.txt")) {
    my_File = SD.open("data.txt", FILE_WRITE);  //abrimos  el archivo
    //Serial.println("escribiendo data... ");
    if (my_File) {

    my_File.print(cont);
    my_File.print(";");
    my_File.print(rpm);
    my_File.print(";");
    my_File.print(vel);
    my_File.print(";");
    my_File.println(valor_galga);
     // my_File.println(dat_1);
      //Serial.println("Escribiendo SD: ");
      //delay(10);
      my_File.close();  //cerramos el archivo
      //delay(10);
    } else {
      //estado = "OFF";
      //Serial.println("error al abrir archivo...");
    }
  }
}
