//************************
// Ground Station Firmware
//************************

#include <stdio.h>
#include <string.h>
//#include <SPI.h>

String received = "";
String temp = "";
char *ptr = NULL;

void setup() {
  // Initialize USB serial
  Serial.begin(9600);
  while (!Serial) {};

  // Initialize XBee UART
  Serial2.begin(9600);
  while (!Serial2) {};

  Serial.println("Initialized");
}

void loop() {
  String fields[50];

  if (Serial2.available()) {
    receive_data();
    Serial.println(received);
    
    tokenize(received, fields);

//    // Print fields
//    for (int n = 0; n < 6; n++) {
//      Serial.println(fields[n]);
//    }
    
    String check_checksum = reconstruct_msg(fields);
    Serial.println(check_checksum);
    if (verify_checksum(check_checksum, fields)) {
      Serial.println("ACK: " + String(fields[0]) + "!");
      Serial2.println("ACK: " + String(fields[0]) + "!");
    }

    //log_data();

    received = "";
  }
  delay(10);
}

void receive_data() {
  temp = "";
  while (1) {
    temp = Serial2.read();
    if (temp == "!") {
      break;
    } else {
      received = received + temp;
    }
    delay(2);
  }
  while (Serial2.available()) {
    temp = Serial2.read();
    delay(2);
  }
  temp = "";
}

void tokenize(String received, String fields[]) {
  int i = 0;
  char mssg[received.length()];
  received.toCharArray(mssg, received.length() + 1);
  ptr = strtok(mssg, ",");

  while (ptr != NULL) {
    //    Serial.println(ptr);
    fields[i] = String(ptr);
    i++;
    ptr = strtok(NULL, "!,");
  }
}

String reconstruct_msg(String fields[]) {
  int i = 0;
  String message = "";

  // Rebuild message string to calculate checksum with
  for (i = 0; i < 5; i++) {
    Serial.println(fields[i]);
    message += fields[i];
  }
  return message;
}

bool verify_checksum(String msg, String fields[]) {
  int checksum = 0;
  for (int i = 0; i < msg.length(); i++) {
    if (msg[i] != ',') {
      checksum += msg[i];
    }
  }
  checksum %= 256;
  Serial.println("Calculated " + String(checksum));
  Serial.println("Comparing with " + fields[5]);
  if (checksum == fields[5].toInt()) {
    return true;
  }
  else {
    return false;
  }
}
