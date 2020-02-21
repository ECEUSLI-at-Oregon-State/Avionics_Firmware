//*********************************
// Avionics Telemetry Unit Firmware
//*********************************

#include <Adafruit_MPL3115A2.h>
#include <TinyGPS.h>
#include "SdFat.h"
SdFs sd;
TinyGPS gps;

#define gpsPort Serial1
#define GPSECHO  true
#define xBeeSerial Serial3

#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"

const int SD_CS_PIN = SDCARD_SS_PIN;
FsFile dataFile;

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();
bool usingInterrupt = false;

//Accelerometer
const int xInput = A20;
const int yInput = A8;
const int zInput = A9;
//const int buttonPin = 2;

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
  // 10 Hz update rate
  gpsPort.println(PMTK_SET_NMEA_UPDATE_10HZ);

  delay(1);

    while (!sd.begin(SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(50)))) {
      Serial.println("Initialization Failed");
      delay(1000);
    }
}

void loop() {
  if (! baro.begin()) {
    Serial.println("Couldnt find sensor");
    return;
  }

  enumerate += 1;
  message = String(enumerate);
 
  float altm = baro.getAltitude();
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

  gps.f_get_position(&flat, &flon, &age);
  temp = get_datetime(gps, age);
  message = String(message + temp);

  // Latitude
  message = String(message + ",");
  temp = String(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
  message = String(message + temp);

  // Longitude
  message = String(message + ",");
  temp = String(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
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

  message = String(message + "!\n");
  log_data();

  Serial.println(message);
  xBeeSerial.println(message);

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

  // re-scale to fractional Gs
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
  dataFile = sd.open("AllBlockCheckoff.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(message);
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
    delay(1);
  }
  while (xBeeSerial.available()) {
    temp = xBeeSerial.read();
    //    delay(1);
  }
  temp = "";
}

int calc_checksum(String msg) {
  int checksum = 0;
  for (int i = 0; i < msg.length(); i++) {
    if (msg[i] != ',') {
      checksum += msg[i];
    }
  }
  //  Serial.println(checksum %= 256);
  return (checksum %= 256);
}

String get_datetime(TinyGPS gps, unsigned long age) {
  String datetime;
  int year;
  uint8_t month, day, hour, minute, second, hundredth;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredth, &age);
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
