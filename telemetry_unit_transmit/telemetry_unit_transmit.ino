//*********************************
// Avionics Telemetry Unit Firmware
//*********************************

#include <Adafruit_MPL3115A2.h>
#include <TinyGPS++.h>
#include "SdFat.h"
SdFs sd;
TinyGPSPlus gps;

#define gpsPort Serial1
#define GPSECHO  true
#define xBeeSerial Serial3

#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"

const int SD_CS_PIN = SDCARD_SS_PIN;
FsFile dataFile;

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();
bool usingInterrupt = false;

//Accelerometer
const int xInput = A7;
const int yInput = A8;
const int zInput = A9;

// Raw Ranges:
// initialize to mid-range and allow calibration to
// find the minimum and maximum for each axis
int xRawMin = 509;
int xRawMax = 518;

int yRawMin = 507;
int yRawMax = 514;

int zRawMin = 509;
int zRawMax = 516;

int xRaw, yRaw, zRaw;
long xScaled, yScaled, zScaled;
float xAccel, yAccel, zAccel;

String message = "";
String temp = "";
String received = "";
char buf[32];
int enumerate = 0;
char *fields[20];
char *ptr = NULL;

void setup() {
  Serial.begin(9600);

  pinMode(24, INPUT);
  pinMode(25, INPUT);
  pinMode(26, INPUT);
  pinMode(27, INPUT);

  xBeeSerial.begin(9600);
  gpsPort.begin(9600);

  // GPS 10 Hz update rate
  gpsPort.println(PMTK_SET_NMEA_UPDATE_10HZ);

  delay(1);

  while (!sd.begin(SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(50)))) {
    Serial.println("microSD Initialization Failed");
    delay(1000);
  }
}

void loop() {
  if (! baro.begin()) {
    Serial.println("Couldn't find barometeric pressure sensor.");
    return;
  }

  // Packet number
  enumerate += 1;
  message = String(enumerate);

  float altm = baro.getAltitude();

  // Convert from meters to feet
  float altmImperial = altm * 3.28084;

  message = String(message + ",");
  message = message + String(altmImperial);

  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (gpsPort.available()) {
      char c = gpsPort.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  unsigned long age;
  float flat, flon;

  // Datetime
  temp = get_datetime(gps);
  message = String(message + temp);

  // Latitude
  message = String(message + ",");
  temp = String(gps.location.lat(), 6);
  message = String(message + temp);

  // Longitude
  message = String(message + ",");
  temp = String(gps.location.lng(), 6);
  message = String(message + temp);

  readAccel();

  // X-axis Acceleration
  message = String(message + ",");
  temp = String(xAccel);
  message = String(message + temp);

  // Y-axis Acceleration
  message = String(message + ",");
  temp = String(yAccel);
  message = String(message + temp);

  // Z-axis Acceleration
  message = String(message + ",");
  temp = String(zAccel);
  message = String(message + temp);

  // Checksum
  message = String(message + ",");
  temp = String(calc_checksum(message));
  message = String(message + temp);

  // ! packet deliieter
  message = String(message + "!\n");

  log_data();

  // Print packet
  Serial.println(message);
  
  // Send packet
  xBeeSerial.println(message);

  // Print ACK if received
  if (xBeeSerial.available()) {
    receive_data();
    Serial.println(received);
    received = "";
  }
}

void readAccel() {
  xRaw = ReadAxis(xInput);
  yRaw = ReadAxis(yInput);
  zRaw = ReadAxis(zInput);

  // Convert raw values to 'milli-Gs"
  xScaled = map(xRaw, xRawMin, xRawMax, -1000, 1000);
  yScaled = map(yRaw, yRawMin, yRawMax, -1000, 1000);
  zScaled = map(zRaw, zRawMin, zRawMax, -1000, 1000);

  // Re-scale to fractional Gs
  xAccel = xScaled / 1000.0;
  yAccel = yScaled / 1000.0;
  zAccel = zScaled / 1000.0;
}

int ReadAxis(int axisPin) {
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < 10; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading / 10;
}

void log_data() {
  dataFile = sd.open("fullscale_1_telemetry_unit_2-22-20.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(message);
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
    delay(1);
  }

  // Flush buffer
  while (xBeeSerial.available()) {
    temp = xBeeSerial.read();
    delay(1);
  }
  temp = "";
}

// The checksum calculation is the sum of all ASCII chars in a string
int calc_checksum(String msg) {
  int checksum = 0;

  for (int i = 0; i < msg.length(); i++) {
    if (msg[i] != ',') {
      checksum += msg[i];
    }
  }

  return (checksum %= 256);
}


// Extract datetime fields and format the string
String get_datetime(TinyGPSPlus gps) {
  String datetime;
  uint16_t year;
  uint8_t month, day, hour, minute, second, hundredth;

  year = gps.date.year();
  month = gps.date.month();
  day = gps.date.day();
  hour = gps.time.hour();
  minute = gps.time.minute();
  second = gps.time.second();
  hundredth = gps.time.centisecond();

  datetime += String(",");
  datetime += String(year);
  datetime += String("/") + String(month);
  datetime += String("/") + String(day);
  datetime += String(" ") + String(hour);
  datetime += String(":") + String(minute);
  datetime += String(":") + String(second);
  datetime += String(".") + String(hundredth);

  return datetime;
}
