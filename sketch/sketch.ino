#include "screen.h"
#include "functions.h"
#define enter 1
#define select 2
int keyPressed = 0, estado = 0, subEstado = 0, place = 0;
unsigned long lastMillis = 0;
void setup() {
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  Serial.begin(115200);
  begin_screen();  
  begin_ble();
}

void loop() {
  keyPressed = get_input();
  if(estado == 0){
    logo_screen();
    delay(5000);
    estado = 1;
  }
  else if(estado == 1){
    search_screen();
    clear_ble();
    if(keyPressed == enter) estado = 2;
  }
  else if(estado == 2){
    if(subEstado == 0){
      for(int i=0; i<12; i++){
        found_screen(i, 255, 0);
        search_ble();
      }
      if(iDevice == 0){
        found_screen(0, 0, 0);
        subEstado = 1;
        lastMillis = millis();
      }
      else{
        subEstado = 2;
        found_screen(0, iDevice, 0);
        lastMillis = millis();
      }
    }
    
    if(subEstado == 1){
      if(millis() - lastMillis > 5000 || keyPressed == 1){
        estado = 1;
        subEstado = 0;
      }
    }
    if(subEstado == 2){
      if(keyPressed == select){
        place++;
        lastMillis = millis();
        if(place > iDevice) place = 0;
        Serial.println(place);
        found_screen(0, iDevice, place);
      }
      if((keyPressed == enter && place == 0) || millis() - lastMillis >= 20000){
        estado = 1;
        subEstado = 0;
      }
    }
  }
  else if(estado == 3){
    
  }
}

int get_input(){
  if(digitalRead(12) == 0){
    delay(300);
    while(digitalRead(12) == 0);  
    return 1;
  }
  else if(digitalRead(13) == 0){
    delay(300);
    while(digitalRead(13) == 0);  
    return 2;
  }
  else return 0;
}
