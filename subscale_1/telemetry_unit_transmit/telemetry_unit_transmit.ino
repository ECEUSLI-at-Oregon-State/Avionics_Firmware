#include <Adafruit_MPL3115A2.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include "SdFat.h"
SdFs sd;
TinyGPS gps;

#define xBeeSerial Serial1
#define GPSECHO  true
#define gpsPort Serial2

const int SD_CS_PIN = SDCARD_SS_PIN;
FsFile dataFile;

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();
bool usingInterrupt = false;
String message;
String temp;
char buf[32];
void setup() {
  Serial.begin(9600);

  pinMode(24, INPUT);
  pinMode(25, INPUT);
  pinMode(26, INPUT);
  pinMode(27, INPUT);
  
  Serial1.begin(9600);
  gpsPort.begin(9600);

  delay(1000);

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

  float pascals = baro.getPressure();
  // Our weather page presents pressure in Inches (Hg)
  // Use http://www.onlineconversion.com/pressure.htm for other units
  float inHg = pascals / 3377;

  float altm = baro.getAltitude();
  float altmImperial = altm * 3.28084;

  message = "\0";
  message = "Alt: ";
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

  int year;
  uint8_t month, day, hour, minutes, second, hundredths;
  unsigned long age;
  float flat, flon;
  
  gps.f_get_position(&flat, &flon, &age);
  gps.crack_datetime(&year, &month, &day, &hour, &minutes, &second, &hundredths, &age);

  message = String(message + ",Time: ");

  temp = String(year);
  temp = String(temp + "/");
  message = String(message + temp);

  temp = String(month);
  temp = String(temp + "/");
  message = String(message + temp);

  temp = String(day);
  temp = String(temp + " ");
  message = String(message + temp);

  temp = String(hour);
  temp = String(temp + ":");
  message = String(message + temp);

  temp = String(minutes);
  temp = String(temp + ":");
  message = String(message + temp);

  temp = String(second);
  temp = String(temp + ".");
  message = String(message + temp);

  temp = String(hundredths);
  message = String(message + temp);

  message = String(message + ",Lat: ");
  temp = String(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);//Latitude
  message = String(message + temp);
  
  message = String(message + ",Lon: ");
  temp = String(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);//Longitude
  message = String(message + temp);

  log_data();
  message = String(message + "!\n");
  
  Serial.println(message);
  Serial1.println(message);
}

void log_data() {
  dataFile = sd.open("data2.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(message);
    dataFile.close();
  } else {
    Serial.println("Couldn't Write to File.");
  }
}
