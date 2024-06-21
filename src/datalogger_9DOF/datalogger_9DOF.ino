/*
  ESP32 Data Logger with Adafruit 9DOF sensor, by Till Häussner

  SOURCES for the code parts:
  - Google Sheets Client: Rui Santos, https://RandomNerdTutorials.com/esp32-datalogging-google-sheets/
                  Adapted from the examples of the Library Google Sheet Client Library for Arduino devices: https://github.com/mobizt/ESP-Google-Sheet-Client
  - Adafruit 9DOF Sensor Library: https://github.com/adafruit/Adafruit_9DOF
  - I2C Scanning and Debugging: Rui Santos, https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/
  - Adafruit RTC DS1307 Library: https://github.com/adafruit/RTClib
  - microSD integration: Rui Santos, https://randomnerdtutorials.com/esp32-microsd-card-arduino/

  REQUIRED LIBRARIES (install by searching for name in Arduino IDE -> Library Manager):
  - SD (1.2.4) by Arduino, SparkFun: https://www.arduino.cc/reference/en/libraries/sd/
                  for SD card support
  - Adafruit 9DOF (1.1.4) by Adafruit: https://github.com/adafruit/Adafruit_9DOF
                  for the 9DOF breakout board by Adafruit
  - Adafruit L3GD20 U (2.0.3) by Adafruit: https://github.com/adafruit/Adafruit_L3GD20_U
                  Driver for the gyro on the 9DOF breakout board by Adafruit
  - Adafruit LSM303DLHC (1.0.4) by Adafruit: https://github.com/adafruit/Adafruit_LSM303DLHC
                  Old driver for 9DOF breakout board (see README on GitHub for newer libraries), but still works.
  - Adafruit BusIO (1.16.1) by Adafruit: https://github.com/adafruit/Adafruit_BusIO
                  for SPI and I2C abstraction
  - Adafruit Unified Sensor (1.1.14) by Adafruit: https://github.com/adafruit/Adafruit_Sensor
                  For sensor compatibility and abstraction
  - RTClib (2.1.4) by Adafruit: https://github.com/adafruit/RTClib
                  For RTC DS1307, also support others
  - ESP-Google-Sheet-Client (1.4.4) by Mobizt: https://github.com/mobizt/ESP-Google-Sheet-Client
                  For Google Sheets Connection
  
  REQUIRED BOARD PACKAGES (install by searching for name in Arduino IDE -> Boards Manager):
  - esp32 by Espressif: https://github.com/espressif/arduino-esp32
                  For ESP32 support in Arduino IDE
  
*/

// _______ LIBRARIES ________________________________________________________
// Arduino:
#include <Arduino.h>
//
// For ESP32 wifi connection:
#include <WiFi.h>
//
// For Adafruit 9DOF sensor, from example: Adafruit 9DOF -> pitchrollheading, see https://github.com/adafruit/Adafruit_9DOF
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>
//
// For NTP timestamp
// commented out, as th time stamp was taken from the RTC.
// if you want to use the NTP server for the time stamp (which requires internet connection),
// uncomment
// #include "time.h"
//
// For RTC DS1307
#include "RTClib.h"
//
// For Google Sheets Connection; see https://github.com/mobizt/ESP-Google-Sheet-Client
#include <ESP_Google_Sheet_Client.h>
//
// For SD/SD_MMC mounting helper
#include <GS_SDHelper.h>
//
// For microSD Card
#include "FS.h"
#include "SD.h"
#include <SPI.h>
// __________________________________________________________________________



// ______ WIFI CONNECTION______________
// access to the ETH IOT wifi network
#define WIFI_SSID "eth-iot"
#define WIFI_PASSWORD "QGJ3mavt-ghe"
// ____________________________________



// _______ GOOGLE SHEETS CONNECTION _______________________________________
// Google Project ID
#define PROJECT_ID "esp32-datalogger-tests"
// 
// Service Account's client email
#define CLIENT_EMAIL "esp32-datalogger-tests@esp32-datalogger-tests.iam.gserviceaccount.com"
//
// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDDFhc83qbGS+jD\n8j15bxRIc19QIkRqTCVdOu4qIXCy+LaRMojmvRz0QgWoDNQyvPBsX1ZgwT3hQ+Ya\nRaJYvh5FtF5DNKHvn/3XMgPOGEQP3BdUD0R/mp8OPRlxeBTh41unJLzkdDkN5FgH\ntefyhX3tdDeWSLXUwjzwApI38M8OOciVjZCV4NO/85xQt51XgjRgXY9QvLsKNNUb\nu2l9opV1ywXbflNebFFYwS3dLIvRbFse/z1duvEgMmyK3ZHzNbOJfRwR/KCFzXqN\n+DwJmUmmgsegyDUK/TPMUE7wYFcR7d6C23AxSQkN3HGRBUJrdAERtGXwE8kP8pJw\nClgI4wMPAgMBAAECggEADn3im1vyyZeJTUepDLnqqM3L48OtOgmOsP6YAzsuswIf\naxPKRacAsp2rlpCKvCYlj0yvDvWWki2dYeWQVMVIBxI0pQtmMssQwTPu7RgHs6Hm\n0xC7RuYGhg6V3ZmiUNFh1T25ERjCcDy+fqbNpHS0JmJ+tUQ7daLkLJ6+8TbgkAQr\niyniOU7nUapE0c1YoxFsje7q0H6cDH9pZ7VhFGWgRlkI9Msdd+cLoKCRg1kuHJeq\nwk4Nbq4f3jWnHMPXyv8rY3aLj+WqWhM34mAB5Uoyk4//9D6vXt8HDaU4Ung3XcBl\nG4903RwsrkQDPDRWMy2pITdy7BCheqCfYmfZ0ShfSQKBgQDx0e7JYyLnYtI6xaMh\nfzc3qkh8RgYhDk/8EEXmKi5x1dxbHkxdMe8kN7SWFTqjS/aVqttFDbUyy51xHntg\n+DxLELNoZMzZDwRORYSGZc+kuyfkujCrPFdXWjAyCM3NlIQHl2hHzcDsMPsTZpDS\nL2xNJoehS8MzqA99spzaq5VaqwKBgQDOhp3zkdNuuftNPel2wef48QFH3itUHFl8\nYlchUd/DJpFmlKqGTr9IUKnC4yo4SWt/sVdt3v0SvxLe3YxaO5XYTcbuWvebBqmo\nyjyfHi64FyC3za+OAJUbq3sjh2/OAm4orH5n+BBjaItCopS0RQMKp6SnbiiStd1m\nt9Lle1Y5LQKBgEadqX+BxbWQNBgZktO7VKKaxWQVBsEsbssK7X6THtD7RMgPBvnI\nFS+cXM3fESqSikWWiUWXBSz6LNXVsu3UT7cT/3Aiz3crXXSF9HdSz9opFTND8+Zt\nTowhuCVEUOUR06Rr4HXnbckLXKaDsxLN6nB1KfI+L5isISnxuMBajtjVAoGBAJdV\nj3KGve7aO2prXqMWDbyeJXkbNJbKu0vo573Tt+rRXSRU9eu60nHUIcFcInjiW/JS\n6iTpjHprJxHGOG16UiARRh8s61le9X1ozCkwvb1JvUDSaLzecmS3MGFlgijsIPqP\nA4V+AcxGNB2c8uhx1Xu+Qmopz3NJ7rWxJ8neqdz1AoGBAN5U9OpVZ78IFKWE7hQJ\nQ016035UBhz4X7T0+XGmZ/wWI/uBunCZOrGvFDSNwNFhT1mAF6Qbtoh5QMjfPrYB\n6yz5+u9d8yPJbKp29mYfM9HlxanMzGSbqd8PpJKqAzY0KGnsHl9E8JEL/zHWZXQ8\ny0+6YT4YIy1dWs/K/w4KZolj\n-----END PRIVATE KEY-----\n";
//
// The ID of the spreadsheet where you'll publish the data
const char spreadsheetId[] = "1Dp0jl5t3F-D2OPdoOo2naJbEfygzyLq-sfOtOqr3_y4";
//
// Token Callback function
void tokenStatusCallback(TokenInfo info);
// ____________________________________________________________________________


/*
// _________ NTP TIME SERVER CONNECTION ___________
// NTP server to request epoch time
 const char* ntpServer = "pool.ntp.org";
//
// Variable to save current epoch time
unsigned long epochTime; 
//
// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
// ________________________________________________
*/


// ________ REAL TIME CLOCK Adafruit RTC DS1307 __________________
// I2C Address: (Found using the I2C Scanner by Rui Santos (https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/))
// rtc:   0x69
//
// Setting up I2C Connection:
// Define pins:
#define SDA_2 33
#define SCL_2 32
// create wire instane
// TwoWire I2Ctwo = TwoWire(1);
//
// assign ID
RTC_DS1307 rtc;

// define weekdays
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// ________________________________________________________________



// ________ MICRO SD CARD ________________________________________
// String for data entry
String dataMessage;
// Initialize SD card
void initSDCard(){
   if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}
// Write to the SD card
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}
// Append data to the SD card
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
// _______________________________________________________________



// _________ Adafruit 9DOF Sensor ____________________
// I2C Addresses: (Found using the I2C Scanner by Rui Santos (https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/) and the Adafruit Documentation: https://learn.adafruit.com/i2c-addresses/the-list)
// accel: 0x19
// mag:   0x1E
// dof:   0x69
// 
// Assign a unique ID to the sensors
Adafruit_9DOF                dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
//
// Setting up I2C Connection:
// Define pins:
// #define SDA_1 21
// #define SCL_1 22
// Create wire instance
// TwoWire I2C_9dof = TwoWire(0);

// Initialize Sensors
void initSensors()
{
  if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("Ooops, no LSM303 detected ... Check your wiring!"));
    while(1);
  }
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
}
// ___________________________________________________



void setup(){

    Serial.begin(115200);
    Serial.println();
    Serial.println("Setting up hopefully.......");
    Serial.println();

    initSDCard();

    // I2C Setup
    // I2C_9dof.begin(SDA_1, SCL_1, 100000); 
    // I2C_rtc.begin(SDA_2, SCL_2, 100000);
    Wire.begin();
    Wire1.begin(SDA_2, SCL_2);
    bool status_rtc = rtc.begin(&Wire1);  
    if (!status_rtc) {
      Serial.println("Could not find RTC at 0x68");
      while (1);
    }
    
    // bool status1 = dof.begin(0x69, &I2C_9dof);
    // bool status2 = mag.begin(0x1E, &I2C_9dof); 
    // bool status3 = accel.begin(0x19, &I2C_9dof);
    

    // RTC DS1307 Setup
    if (! rtc.begin(&Wire1)) {
      Serial.println("Couldn't find RTC");
      Serial.flush();
      while (1) delay(10);
    }
    if (! rtc.isrunning()) {
      Serial.println("RTC is NOT running, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    // SD Card: _______________________________________________
    // If the data.txt file doesn't exist
    // Create a file on the SD card and write the data labels
    File file = SD.open("/data.txt");
    if(!file) {
      Serial.println("File doesn't exist");
      Serial.println("Creating file...");
      writeFile(SD, "/data.txt", "Timestamp, Pitch, Roll, Heading \r\n");
    }
    else {
      Serial.println("File already exists");  
    }
    file.close();
    // ________________________________________________________



    //Configure NTP time
    //configTime(0, 0, ntpServer);

    /* Initialise the sensors */
    initSensors();

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

    // Connect to Wi-Fi
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Set the callback for Google API access token generation status (for debug only)
    GSheet.setTokenCallback(tokenStatusCallback);

    // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
    GSheet.setPrerefreshSeconds(10 * 60);

    // Begin the access token generation for Google API authentication
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}


// time variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 1;

void loop(){
    // Call ready() repeatedly in loop for authentication checking and processing
    bool ready = GSheet.ready();

    // get time from RTC
    DateTime now = rtc.now();

    if (ready && now.unixtime() - lastTime > timerDelay){
        lastTime = now.unixtime();

        FirebaseJson response;

        Serial.println("\nAppend spreadsheet values...");
        Serial.println("----------------------------");

        FirebaseJson valueRange;

        


        // Get NTP timestamp
        // epochTime = getTime();

        // Create RTC timestamp
        String timestamp = "";
        timestamp += daysOfTheWeek[now.dayOfTheWeek()];
        timestamp += ", ";
        timestamp += now.year();
        timestamp += '/';
        timestamp += now.month();
        timestamp += '/';
        timestamp += now.day();
        timestamp += ' ';
        timestamp += now.hour();
        timestamp += ':';
        timestamp += now.minute();
        timestamp += ':';
        timestamp += now.second();

        // Get sensor readings
        float dof_pitch = 0;
        float dof_roll = 0;
        float dof_heading = 0;

        sensors_event_t accel_event;
        sensors_event_t mag_event;
        sensors_vec_t   orientation;
        accel.getEvent(&accel_event);
        if (dof.accelGetOrientation(&accel_event, &orientation))
        {
          dof_pitch = orientation.pitch;
          dof_roll = orientation.roll;
        }
        mag.getEvent(&mag_event);
        if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation))
        {
          dof_heading = orientation.heading;
        }

        // write data to SD Card ____________________________
        dataMessage = timestamp + "," + String(dof_pitch) + "," + String(dof_roll) + "," + String(dof_heading)+ "\r\n";
        Serial.print("Saving data to microSD: ");
        Serial.println(dataMessage);
        //Append the data to file
        appendFile(SD, "/data.txt", dataMessage.c_str());
        // _____________________________________________________________



        // Write data to Google Sheets ___________________________________________________
        // create entry array
        valueRange.add("majorDimension", "COLUMNS");
        // add timestamp to array
        valueRange.set("values/[0]/[0]", timestamp);
        valueRange.set("values/[1]/[0]", dof_pitch);
        valueRange.set("values/[2]/[0]", dof_roll);
        valueRange.set("values/[3]/[0]", dof_heading);

        // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/append
        // Append values to the spreadsheet
        bool success = GSheet.values.append(&response /* returned response */, spreadsheetId /* spreadsheet Id to append */, "Sheet1!A1" /* range to append */, &valueRange /* data range to append */);
        if (success){
            response.toString(Serial, true);
            valueRange.clear();
        }
        else{
            Serial.println(GSheet.errorReason());
        }
        Serial.println();
        Serial.println(ESP.getFreeHeap());
        // ___________________________________________________________________________
    }
}

void tokenStatusCallback(TokenInfo info){
    if (info.status == token_status_error){
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    }
    else{
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}