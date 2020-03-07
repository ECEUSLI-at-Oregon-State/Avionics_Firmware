//************************
// Ground Station Firmware
//************************

#include <stdio.h>
#include <string.h>
#include "SdFat.h"
SdFs sd;

#define xBeeSerial Serial3

const int SD_CS_PIN = SDCARD_SS_PIN;
FsFile dataFile;

String received = "";
String temp = "";
char *ptr = NULL;

void setup() {
  // Initialize USB serial
  Serial.begin(9600);
  while (!Serial) {};

  // Initialize XBee UART
  xBeeSerial.begin(9600);
  while (!xBeeSerial) {};

  while (!sd.begin(SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(50)))) {
    Serial.println("microSD Initialization Failed");
    delay(1000);
  }

  Serial.println("Initialized");
}

void loop() {
  String fields[50];

  if (xBeeSerial.available()) {
    receive_data();
    Serial.println(received);
    log_data();

    tokenize(received, fields);

    //    // Print fields
    //    for (int n = 0; n < 6; n++) {
    //      Serial.println(fields[n]);
    //    }

    String check_checksum = reconstruct_msg(fields);
    //    Serial.println(check_checksum);
    if (verify_checksum(check_checksum, fields)) {
//      Serial.println("ACK: " + String(fields[0]) + "!\n");
      xBeeSerial.println("ACK: " + String(fields[0]) + "!");
    }

    //log_data();

    received = "";
  }
  delay(10);
}

void log_data() {
  dataFile = sd.open("Fullscale2-3-8-20-gs.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(received);
    dataFile.close();
  } else {
    Serial.println("Couldn't Write to File.");
  }
}

void receive_data() {
  temp = "";
  while (1) {
    temp = xBeeSerial.read();
    if (temp == "!") {
      break;
    } else {
      received = received + temp;
    }
    delay(2);
  }
  while (xBeeSerial.available()) {
    temp = xBeeSerial.read();
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
  for (i = 0; i < 8; i++) {
    //    Serial.println(fields[i]);
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
//  Serial.println("Calculated " + String(checksum));
//  Serial.println("Comparing with " + fields[8]);
  if (checksum == fields[8].toInt()) {
    return true;
  }
  else {
    return false;
  }
}
