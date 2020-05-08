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

  // Initialize microSD card
  while (!sd.begin(SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(50)))) {
    Serial.println("microSD initialization failed");
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

    String check_checksum = reconstruct_msg(fields);

    // If checksum matches, send ACK
    if (verify_checksum(check_checksum, fields)) {
      xBeeSerial.println("ACK: " + String(fields[0]) + "!");
    }

    received = "";
  }
  delay(10);
}

void log_data() {
  // Open file for writing
  dataFile = sd.open("fullscale_1_ground_station_log_2-22-20.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println(received);
    dataFile.close();
  } else {
    Serial.println("Couldn't write to file.");
  }
}

void receive_data() {
  temp = "";

  // Read chars building received string until ! delimiter
  while (1) {
    temp = xBeeSerial.read();
    if (temp == "!") {
      break;
    } else {
      received = received + temp;
    }
    delay(2);
  }

  // Flush buffer
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

  // Store tokens into fields array
  while (ptr != NULL) {
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
    message += fields[i];
  }

  return message;
}

bool verify_checksum(String msg, String fields[]) {
  int checksum = 0;

  // Perform checksum calculation by summing ASCII values
  for (int i = 0; i < msg.length(); i++) {
    if (msg[i] != ',') {
      checksum += msg[i];
    }
  }

  checksum %= 256;

  // Check calculation against checksum
  if (checksum == fields[8].toInt()) {
    return true;
  }
  else {
    return false;
  }
}
