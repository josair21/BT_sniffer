#include "BLEDevice.h"
#include "BLEAddress.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <string.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);   // ESP32 Thing, HW I2C with pin remapping

String devices[] = {"default", "default", "default", "default", "default"};
String addresses[] = {"default", "default", "default", "default", "default"};
String device_toConnect = "default";
static BLEAddress MAC("00:00:00:00:00:00");
static BLEUUID serviceUUID("4aeab230-9aa6-4ca8-9329-1154c1d88d51");
static BLEUUID    charUUID_read("4aeab330-9aa6-4ca8-9329-1154c1d88d51"), charUUID_write("4aeab331-9aa6-4ca8-9329-1154c1d88d51");
static BLERemoteCharacteristic* pRemoteCharacteristic_read;
static BLERemoteCharacteristic* pRemoteCharacteristic_write;
int index_device = 0;
bool duplicate = false, doConnect = false, connected;
char Rx;
unsigned int estado = 0;
static BLEAdvertisedDevice* myDevice;

/**
 * Made with Marlin Bitmap Converter
 * https://marlinfw.org/tools/u8glib/converter.html
 *
 * This bitmap from the file 'Output.bmp'
 */




static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    /*Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");*/
    //Serial.println((char*)pData);
    if(pData[0] == 0x00 || pData[0] == 0x02){
      for (int i = 0; i < length; i++){
        if(i == 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
        }
        else if(i >= 7){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
        }
        else if(i%2 == 1){
          Serial.print(pData[i]*0x100 + pData[i+1], DEC);
          Serial.print(" ");
        }
      }
    }
    else if(pData[0] == 0x01){
      for (int i = 0; i < length; i++){
        if(i == 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
        }
        else if(i == length - 1 &&  i%2 != 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
        }
        else if(i%2 == 1){
          Serial.print(pData[i]*0x100 + pData[i+1], DEC);
          Serial.print(" ");
        }
      }
    }
    else{
      for (int i = 0; i < length; i++){
        Serial.print(pData[i], DEC);
        Serial.print(" ");
      }
    }
    Serial.println("");
    //S
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Desconectado");
  }
};

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {  
    devices[index_device] = advertisedDevice.getName().c_str();
    if(estado == 0){
      devices[index_device] = advertisedDevice.getName().c_str();
      addresses[index_device] = advertisedDevice.getAddress().toString().c_str();
      
      if(devices[index_device][0] == 'C' && devices[index_device][1] == 'C'){
          if(index_device == 0){
            //Serial.println(devices[index_device]);  
            index_device++;
          }
          else{
            duplicate = false;
            for (int i = 0; i < index_device; i++){
              if(devices[i][9] == devices[index_device][9] && devices[i][10] == devices[index_device][10]){
                duplicate = true;
                //Serial.println("Duplicado");
                break;
              }
            }
            if(duplicate == false && index_device <=3){
              //Serial.println(devices[index_device]);  
              index_device++;  
            }    
          }
       }
    } // onResult
    if(estado == 2){
      //Serial.print(devices[index_device]);
      //Serial.println(device_toConnect);
      if(devices[index_device] == device_toConnect){
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
      }
    }
  }
}; // MyAdvertisedDeviceCallbacks

void setup() {
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  Serial.begin(115200);
  u8g2.begin();  
  Serial.println("Iniciando...");
  start_screen();
  delay(2000);
  BLEDevice::init("");
}
int keyPressed, key_index = 0;
void loop() {
  keyPressed = read_buttons();
  if(Serial.available() > 0){
    Rx = Serial.read();
    if(Rx == 'R') estado = 1;
    else if(Rx == 'C') estado = 2;
    else if(Rx == 'D') estado = 3;
    else if(Rx == 'X') estado = 0;
  }
  if(estado == 0){
    zero_screen(true);
    scan_devices();
    devices_found();
    found_screen();
    estado = 1;
  }
  if(estado == 1){
    select_screen();
  }
  else if(estado == 2){
    select_device();
    //connect_device();
  }
  else if(estado == 3){
    send_command();
  }
  Rx = ' ';
}
void select_screen(){
  if(keyPressed == 1){
    estado = 0;
  }
  else if(keyPressed == 2){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_7x14_tr);
        u8g2.drawStr(2,16*(key_index+2)-1,">");
        key_index++;
        if(key_index >= index_device) key_index = 0;
    } while ( u8g2.nextPage() );
  }
}
int read_buttons(){
  if(digitalRead(12) == 0){
    return 1;
  }
  else if(digitalRead(13) == 0){
    return 2;
  }
  else if(digitalRead(14) == 0){
    return 3;
  }
  else return 0;
}
void found_screen(){
  char aux[20];
  char aux2[22];
  u8g2.firstPage();
  do {
    Serial.println(index_device);
    u8g2.setFont(u8g2_font_profont22_tf);
    u8g2.drawStr(0,15,"Devices:  ");      
    for(int i=0; i<index_device; i++){
      devices[i].toCharArray(aux2, 22);
      u8g2.setFont(u8g2_font_7x14_tr);
      sprintf(aux, "%d. %s", i+1, aux2);
      Serial.println(aux2);
      u8g2.drawStr(20,16*(i+2)-1,aux);   
    }
  } while ( u8g2.nextPage() );
}
void zero_screen(bool i){
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_profont22_tf);
    u8g2.drawStr(0,15,"Buscando");
    if(i){
      u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
      u8g2.drawStr(128-16,15,"J");
    }
    else{
      u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t);
      u8g2.drawStr(128-16,15," ");
    }
  } while ( u8g2.nextPage() );
}

void send_command(){
  if(Rx == 'D'){
    pRemoteCharacteristic_write->writeValue({0x80,0x31,0x0b,0xcc});
    //pRemoteCharacteristic_write->writeValue({0x01,0x00});
    pRemoteCharacteristic_write->writeValue({0x80,0x31,0x0a,0xcc});
    Serial.println("Comandos enviados");
  }
  
}
void scan_devices(){
  Serial.println("Iniciando Busqueda..");
  index_device = 0;
  String devices[] = {"", "", "", ""};
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1350);
  pBLEScan->setWindow(1000);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void devices_found(){
  Serial.println("Dispositivos encontrados:");
  for(int i = 0; i < index_device; i++){
    Serial.print(i+1);
    Serial.print(".  ");
    Serial.print(devices[i]);
    Serial.print("   ");
    Serial.println(addresses[i]);
  }
  Serial.println("Busqueda terminada.");
  estado = 0;
}
void select_device(){
  if(Rx == 'C') Serial.println("Elegir dispositivo 1~9");
  if(0 < Rx - '0' && Rx - '0' <= index_device ){
    int i = Rx - '0' - 1;
    Serial.print("Conectando a: ");
    Serial.print(devices[i]);
    Serial.print("  ");
    Serial.println(addresses[i]);
    device_toConnect = devices[i];
    connect_device();
    estado = 0;
  }
}
void connect_device(){
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1250);
  pBLEScan->setWindow(1200);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  if(doConnect){
    connectToServer();
    doConnect = false;
  }
  else Serial.println("Conexion interrumpida");
}
bool connectToServer() {
  //Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Cliente Creado");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Conectado correctamente");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Servicio encontrado");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic_read = pRemoteService->getCharacteristic(charUUID_read);
  if (pRemoteCharacteristic_read == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_read.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Caracteristica 1 encontrada");


  pRemoteCharacteristic_write = pRemoteService->getCharacteristic(charUUID_write);
  if (pRemoteCharacteristic_write == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_write.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Caracteristica 2 encontrada");

  // Read the value of the characteristic.
  if(pRemoteCharacteristic_read->canRead()) {
    std::string value = pRemoteCharacteristic_read->readValue();
    Serial.print("Valor: ");
    Serial.println(value.c_str());
  }

  if(pRemoteCharacteristic_read->canNotify())
    pRemoteCharacteristic_read->registerForNotify(notifyCallback);
  connected = true;
  return true;
}
