#include <stdio.h>
#include <string.h>
#include <SPI.h>

String received = "";
String temp = "";

void setup() {
  Serial.begin(115200);
  while(!Serial){};
  Serial2.begin(9600);
  while(!Serial2){};
  Serial.println("Initialized");
}

void loop() {
  if(Serial2.available()){
    receive_data();
    Serial.println(received);
    //log_data();
    received = "";
  }
  delay(10);
}

void receive_data(){
  temp = "";
  while(1){
      temp = Serial2.read();
      if(temp == "!"){
        break;
      }else{
        received = received + temp;
      }
      delay(2);
    }
    while(Serial2.available()){
      temp = Serial2.read();
      delay(2);
    }
    temp = "";
}
