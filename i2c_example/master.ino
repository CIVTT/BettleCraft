#include <Wire.h>
byte  nodepayload[4];
//char cc='0';
//String nodePayload="";
  
  float d=0.0f;
int env_cod=LOW;
char caracter;
void setup() {
  // put your setup code here, to run once:
Wire.begin();
Serial.begin(115200);
}

void loop() {
  float vel=0.0f;
      // put your main code here, to run repeatedly:
    if (Serial.available()){
      caracter=Serial.read();
      switch (caracter) {
      case 'S':  //star
        env_cod=HIGH;
        break;
      case 'D':  //stop
        //running = false;
        break;
    }}
if (env_cod==HIGH){    
Wire.beginTransmission(0x20);
Wire.write('S');
Wire.endTransmission();
env_cod=LOW;
}

Wire.requestFrom(0x20,4);
if(Wire.available()){
  for(int i=0;i<4;i++)nodepayload[i] = Wire.read();
  for (int j = 0; j < 4; j++) Serial.println(nodepayload[j]);
   //for (int z=0; z<4; z++) Serial.print(nodepayload[z],HEX);

}
Serial.println(getfloat(nodepayload,0));
Wire.endTransmission();

//memcpy(&vel, nodepayload,4);

//Serial.println(vel);
delay(1000);
}
unsigned long bitetofloat(byte pack[], byte i){
  unsigned long value=0;
  value=(value * 256) + pack[i];
  value=(value * 256) + pack[i+1];
  value=(value * 256) + pack[i+2];
  value=(value * 256) + pack[i+3];
  return value;
}
float getfloat(byte pack[], byte i){
  union u_tag{
    byte bin[4];
    float num;
  } u;
  u.bin[0]=pack[i];
  u.bin[1]=pack[i+1];
  u.bin[2]=pack[i+2];
  u.bin[3]=pack[i+3];
  return u.num;
}
