#include <Wire.h>

#define i2c_addr  0x20
#define Tsample   100
#define constante_distancia 350

byte payload[4];

long counter = 0;  //This variable will increase or decrease depending on the rotation of encoder
float counter_now, counter_last;
unsigned long Tnow, Tlast;
float distancia;
float rev_seg; //2pi=rev---->2000p

void setup() {
  pinMode(2, INPUT);           // set pin to input
  pinMode(3, INPUT);           // set pin to input
  digitalWrite(2, HIGH);       // turn on pullup resistors
  digitalWrite(3, HIGH);       // turn on pullup resistors
  attachInterrupt(0, ai0, RISING);
  attachInterrupt(1, ai1, RISING);
  Wire.begin(0x20);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin (115200);
}


void loop() {

  Tnow=millis();
  if(Tnow-Tlast>=Tsample){
    counter_now=counter;
    distancia=counter/constante_distancia;
    rev_seg=((counter_now-counter_last)/2000.0)*10.0;
    counter_last=counter_now;
    Tlast=Tnow;
  }


  Serial.print(distancia);
  Serial.print("::");
  Serial.print(rev_seg);
  Serial.print("::");
  Serial.println(counter_now);
  
  
  delay(1000);
}


void ai0() {
  if(digitalRead(3)==LOW) {
    counter++;
  }else{
    counter--;
  }
}

void ai1() {
  // ai0 is activated if DigitalPin nr 3 is going from LOW to HIGH
  // Check with pin 2 to determine the direction
  if(digitalRead(2)==LOW) {
    counter--;
  }else{
    counter++;
  }
}

void receiveEvent(int howMany){
  while(Wire.available()){
    if(Wire.read() =='S'){
    //byte data=Wire.read();
    counter=0;
    counter_last=0;
  }}
}

void requestEvent(){
   union Bytefloat{
    char byteformat[4];
    float floatformat;
  }vall;
  vall.floatformat=rev_seg;
  Wire.write(vall.byteformat,4);
  
 /* for (int i = 0; i < payload.length(); i++) {
    Wire.write(payload.charAt(i));
  }*/
}
