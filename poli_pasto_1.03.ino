#include <SoftwareSerial.h>
#include <ModbusRTUMaster.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
const int alpha= 0.2;  //Alpha
const int pi =3.1416;
//pines usados
const int sensorPin= A0;
const uint8_t rxPin = 9;
const uint8_t txPin = 10;
const uint8_t dePin = 5;
const uint8_t btn_1 = 2;
const uint8_t btn_2 = 3;
//variables usadas
uint16_t frecc[] = { 0, 598, 665, 733, 800, 868, 935, 1003, 1070, 1138, 1205, 1273 };
int i = 0;
int adc_filtrado = 0;
int adc_raw = 0;

int seg = 0;
uint16_t SETFREC = 0;
float valor_galga = 0;
int valor_in = 0;
float vel = 0;
float rpm = 0;
unsigned long even_time = 0;
const unsigned long Time_d = 1000UL;  //
//variables no numericas
int pulsado_1 = HIGH;
int pulsado_2 = HIGH;
int estado_1 = HIGH;
int estado_2 = HIGH;
/*unsigned long timeer;
unsigned long inicio;
unsigned long fin;
unsigned long duracion;*/
File my_File;
char estado = "OFF";
boolean running = false;
char caracter;
//variables de librerias
SoftwareSerial mySerial(rxPin, txPin);
ModbusRTUMaster modbus(mySerial, dePin);
LiquidCrystal_I2C lcd_1(0x27, 16, 2);

void processError() {
  if (modbus.getTimeoutFlag()) {
    Serial.println(F("Conexion fuera de tiempo"));
    modbus.clearTimeoutFlag();
  } else if (modbus.getExceptionResponse() != 0) {
    Serial.print(F("Received exception response: "));
    Serial.print(modbus.getExceptionResponse());
    modbus.clearExceptionResponse();
  } else {
    Serial.println("Ocurrio un error");
  }
}
void star() {
  //Serial.println(F("mandando"));
  running = true;
  //Serial.println(running);
  uint16_t value = 1;
  if (modbus.writeSingleHoldingRegister(01, 8192, value)) {
    //Serial.println(F("ENCENDIDO"));
  } else processError();
}
void stop() {
  //Serial.println(F("parando..."));
  running = false;
  //Serial.println(running);
  uint16_t value = 7;
  if (modbus.writeSingleHoldingRegister(01, 8192, value)) {
  } else processError();
}
void frec_in(uint16_t value) {
  //uint16_t value = selectValue(0, 5000);
  if (modbus.writeSingleHoldingRegister(01, 8193, value)) {
    valor_in = value;
  }
  //else processError();
}
void frec_set() {
  uint16_t value = 0;
  if (modbus.readHoldingRegisters(01, 8451, &value, 1)) {
    SETFREC = value;
  }
  //else //processError();
}

void read_actual() {
  uint16_t value = 0;
  if (modbus.readHoldingRegisters(01, 8450, &value, 1)) {
    valor_in = value;
  }
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
  // Creando en la SD un csv

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("SD lista.");
  /* if(!SD.exists("data.csv")){
      Serial.println("sd inicializada");
   myFile = SD.open("data.csv", FILE_WRITE);
      if (myFile) {
        Serial.println("Archivo nuevo, Escribiendo encabezado(fila 1)");
        myFile.println("seg;rpm;mm/s;kg");
        myFile.close();
      } else {

        Serial.println("Error creando el archivo data.csv");
      }
  }else{Serial.println("ya existe");}*/
  //read_actual();
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
}
void loop() {
  unsigned long cont_1;
  unsigned long cont_2;
  //long cont_1;
  //long cont_2;
  //inicio=0;
  //calculate();
  //display();
  sd_card();
   if (Serial.available()) {
    caracter = Serial.read();
    switch (caracter) {
      case 'a':  //star
        lectura();
        //star();
        //Serial.println("iniciando");  //star();
        break;
      case 'b':  //stop
        borrado();
        //stop();
        //Serial.println("stop");
        break;
        //default: break;
    }
  }
  //boton mas y star
  
  estado_1 = digitalRead(btn_1);
  if (estado_1 == 0) {
    cont_1++;
    delay(20);
  }
  if (pulsado_1 == 0 && estado_1 == 1) {

    if (cont_1 > 300) {
      running = true;
      Serial.println("encendido");
    } else {
      Serial.print("frec:");
      Serial.println(frecc[i]);
      i++;
      if (i > 11) {
        i = 0;
      }
    }
    Serial.println(cont_1);
    cont_1 = 0;
  }
  pulsado_1 = estado_1;
//boton menos y stop
  estado_2 = digitalRead(btn_2);
  if (estado_2 == 0) {
    cont_2++;
    delay(20);
  }
  if (pulsado_2 == 0 && estado_2 == 1) {
    if (cont_2 > 300) {
      running = false;
      Serial.println("apagado");
    } else {
      Serial.print("frec:");
      Serial.println(frecc[i]);
      i--;
      if (i < 0) {
        i = 11;
      }
    }
    Serial.println(cont_2);
    cont_2 = 0;
  }
  pulsado_2 = estado_2;
  /*
  timeer = millis();
  duracion = millis() - timeer;
  seg = duracion / 1000;
  if (timeer > even_time) {
    //running=true;
    display();
    even_time += Time_d;
  }
*/
}
void calculate() {
  //velocidad en mm/s
  SETFREC = frecc[i] / 100;
  //SETFREC = SETFREC / 100; //posible cambio si falla por interferencia en variador y serial, comentar y descomenrtar anterior linea
  rpm = (SETFREC * 0.741) - 1.43;

  if (rpm <= 0) {
    rpm = 0;
    vel = 0;
  } else {
    vel = rpm * pi * 80 / 30;
  }
  //peso
  adc_raw = analogRead(sensorPin);
  adc_filtrado = (alpha * adc_raw) + ((1 - alpha) * adc_filtrado);
  valor_galga = (500 * adc_filtrado) / 1023;
}

void lectura() {
  /*if (!SD.begin(4)) {
    Serial.println(F("Error al iniciar"));
    return;
  }*/
  //Serial.println(F("Iniciado correctamente"));
  my_File = SD.open("data.txt");
  if (my_File) {
    //String dataLine;
    while (my_File.available()) {
      //dataLine = my_File.read();
      Serial.write(my_File.read());  // En un caso real se realizarían las acciones oportunas
    }
    my_File.close();
  } else {
    Serial.println("Error al abrir el archivo");
  }
}
void borrado() {
  if (!SD.begin(4)) {
    //Serial.println("initialization failed!");
    //while (1);
  }
  Serial.println("Removiendo data.csv");
  SD.remove("data.txt");

  if (!SD.exists("data.txt")) {
    my_File = SD.open("data.txt", FILE_WRITE);
    if (my_File) {
      Serial.println("reimprimiendo archivo");
      my_File.println("seg;rpm;mm/s;kg");
      my_File.close();
    }
    //Serial.println("example.txt exists.");
  } else {
    Serial.println("ya existe txt.");
  }
  Serial.println("terminado");
  /*if (!SD.exists("data.csv")) {
    myFile = SD.open("data.csv", FILE_WRITE);
    if (myFile) {
      //Serial.println("Archivo nuevo, Escribiendo encabezado(fila 1)");
      myFile.println("TIME(seg);RPM(rpm);Vo(mm/s);Ts(kg)");
      myFile.close();
    } else {
      Serial.println("Error creando el archivo datalog.csv");
    }}*/
}
void sd_card(){
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  if (running) {
    //estado = "ON";
    my_File = SD.open("data.txt", FILE_WRITE);  //abrimos  el archivo
    //Serial.println("escribiendo data... ");
    if(my_File){
      //
      my_File.print("seg");
      my_File.print(";");
      my_File.print("rpm");
      my_File.print(";");
      my_File.print("vel");
      my_File.print(";");
      my_File.println("valor_galga");
      Serial.println("Escribiendo SD: ");
      my_File.close();  //cerramos el archivo
    } else {
      //estado = "OFF";
      Serial.println("error al abrir archivo...");
    }
  }
}
void display() {
  // codigo para ldc display
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
  lcd_1.print(estado);
  //codigo display serial
  /*Serial.print(F("SET FREC:"));
  Serial.println(rpm);
  Serial.print(F("OUT FREC:"));
  Serial.println(vel);
  Serial.print(F("TENSION:"));
  Serial.println(valor_galga);
  Serial.print(F("ESTADO:"));
  Serial.println(estado);*/
  //codigo display sd
  //delay(500);
  
}
