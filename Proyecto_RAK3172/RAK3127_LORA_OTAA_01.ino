/**********LIBRERIAS**********/
#include <math.h>
/**********PINOUTs************/
#define pin_temperatura     PA10
#define pin_bateria         PA11
#define pin_desgaste        PA13

/**********CONSTANTES*********/
#define OTAA_PERIOD   (20000)               //Send uplink every 
#define OTAA_BAND     (RAK_REGION_AU915)    //RAK_REGION_EU868 RAK_REGION_US915
#define OTAA_DEVEUI   {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x18, 0x8C}
#define OTAA_APPEUI   {0x0E, 0x0D, 0x0D, 0x01, 0xEE, 0x01, 0x02, 0xEE}
#define OTAA_APPKEY   {0xB3, 0xD8, 0x33, 0xDD, 0x4C, 0x17, 0x00, 0xD2, 0xD4, 0x3E, 0x81, 0x37, 0x7A, 0x37, 0x86, 0x55}
#define resistencia_25      100000.0  // 10k ohmios a 25°C
#define temperatura_25      298.15
#define numero_e            2.71828182
#define constante_betha     3977
#define DEBUG 0
float R2 = 98790;      //Valor medido de R2, no exactamente 10K, cambialo por el valor que uses tu.
float A = -14.05920035E-3; //¡¡¡Datos calculados para un caso concreto...
float B = 21.56699139E-4;  //...debes cambiarlos por los que calcules tu...
float C = -48.50532932E-7;  //...de tu propio termistor NTC!!!
/** Packet buffer for sending */
uint8_t collected_data[64] = { 0 };

void recvCallback(SERVICE_LORA_RECEIVE_T * data)
{
    if (data->BufferSize > 0) {
        Serial.println("Something received!");
        for (int i = 0; i < data->BufferSize; i++) {
            Serial.printf("%x", data->Buffer[i]);
        }
        Serial.print("\r\n");
    }
}

void joinCallback(int32_t status)
{
    Serial.printf("Join status: %d\r\n", status);
}

void sendCallback(int32_t status)
{
    if (status == 0) {
        Serial.println("Successfully sent");
    } else {
        Serial.println("Sending failed");
    }
}

/**********SECUENCIA DE INICIO**********/
void setup(){
    Serial.begin(115200, RAK_AT_MODE);
    #ifdef DEBUG
        Serial.println("RAKwireless LoRaWan OTAA Example");
        Serial.println("------------------------------------------------------");
    #endif
    if(api.lorawan.nwm.get() != 1){
        int result=api.lorawan.nwm.set(1);
        #ifdef DEBUG
            Serial.printf("Set Node device work mode %s\r\n",result ? "Success" : "Fail");
        #endif
        api.system.reboot();
    }

    uint8_t node_device_eui[8] = OTAA_DEVEUI;   // OTAA Device EUI MSB first
    uint8_t node_app_eui[8] = OTAA_APPEUI;      // OTAA Application EUI MSB first
    uint8_t node_app_key[16] = OTAA_APPKEY;     // OTAA Application Key MSB first
  
    if (!api.lorawan.appeui.set(node_app_eui, 8)) {
        Serial.printf("LoRaWan OTAA - set application EUI is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.appkey.set(node_app_key, 16)) {
        Serial.printf("LoRaWan OTAA - set application key is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.deui.set(node_device_eui, 8)) {
        Serial.printf("LoRaWan OTAA - set device EUI is incorrect! \r\n");
        return;
    }
  
    if (!api.lorawan.band.set(OTAA_BAND)) {
        Serial.printf("LoRaWan OTAA - set band is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.deviceClass.set(RAK_LORA_CLASS_A)) {
        Serial.printf("LoRaWan OTAA - set device class is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.njm.set(RAK_LORA_OTAA)){
        Serial.printf("LoRaWan OTAA - set network join mode is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.join()){
        Serial.printf("LoRaWan OTAA - join fail! \r\n");
        return;
    }
    /** Wait for Join success */
    while (api.lorawan.njs.get() == 0) {
        Serial.print("Wait for LoRaWAN join...");
        api.lorawan.join();
        delay(10000);
    }
  
    if (!api.lorawan.adr.set(true)) {
        Serial.printf("LoRaWan OTAA - set adaptive data rate is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.rety.set(1)) {
        Serial.printf("LoRaWan OTAA - set retry times is incorrect! \r\n");
        return;
    }
    if (!api.lorawan.cfm.set(1)) {
        Serial.printf("LoRaWan OTAA - set confirm mode is incorrect! \r\n");
        return;
    }
    /** Check LoRaWan Status*/
    Serial.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get()? "ON" : "OFF");	// Check Duty Cycle status
    Serial.printf("Packet is %s\r\n", api.lorawan.cfm.get()? "CONFIRMED" : "UNCONFIRMED");	// Check Confirm status
    uint8_t assigned_dev_addr[4] = { 0 };
    api.lorawan.daddr.get(assigned_dev_addr, 4);
    Serial.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]);	// Check Device Address
    Serial.printf("Uplink period is %ums\r\n", OTAA_PERIOD);
    Serial.println("");
    api.lorawan.registerRecvCallback(recvCallback);
    api.lorawan.registerJoinCallback(joinCallback);
    api.lorawan.registerSendCallback(sendCallback);
}

void uplink_routine(){
    //int valor_temperatura = analogRead(pin_temperatura);
    //float temperatura_escalada=valor_temperatura/10;
    float temp_1;
    temp_1 = termistor(analogRead(pin_temperatura));    
    String temp_string=String(temp_1);
    Serial.print("Temperatura: ");

    Serial.println(temp_1,1);       
    uint8_t data_len = 0;
    for(int i=0;i<temp_string.length();i++){
      data_len++;
      collected_data[i]=temp_string.charAt(i);
    }    
    /** Payload of Uplink */
    //uint8_t data_len = 0;
    // collected_data[data_len++] = (uint8_t) '3';
    // collected_data[data_len++] = (uint8_t) '2';
    // collected_data[data_len++] = (uint8_t) '.';
    // collected_data[data_len++] = (uint8_t) '4';
    // collected_data[data_len++] = (uint8_t) '5';
  
    Serial.println("Data Packet:");
    for (int i = 0; i < data_len; i++) {
        Serial.printf("0x%02X ", collected_data[i]);
    }
    Serial.println("");
  
    /** Send the data package */
    if (api.lorawan.send(data_len, (uint8_t *) & collected_data, 2, true, 1)) {
        Serial.println("Sending is requested");
    } else {
        Serial.println("Sending failed");
    }
}

/**********SECUENCIA PRINCIPAL**********/
void loop(){
    static uint64_t last = 0;
    static uint64_t elapsed;
       
    if((elapsed = millis() - last) > OTAA_PERIOD) {
        uplink_routine();
        last = millis();
    }
    //Serial.printf("Try sleep %ums..", OTAA_PERIOD);
    api.system.sleep.all(OTAA_PERIOD);
    //Serial.println("Wakeup..");
}

/**********SECUENCIAS SECUNDARIAS**********/
float termistor(int RawADC) {
  
  long resistencia;  
  
  float temp;

  resistencia = R2*((1024.0 / RawADC) - 1); //Calculamos R1 mediante la lectura analogica
  
  temp = log(resistencia);
  
  temp = 1 / (A + (B * temp) + (C * temp * temp * temp));
  
  temp = temp - 273.15;  // Kelvin a grados centigrados                      

  return temp;
  
}
