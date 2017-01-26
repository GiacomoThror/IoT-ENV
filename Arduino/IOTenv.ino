/*
 IoTenv Software
 By: Eva Vivas & Giacomo Napoli
 Sapienza University of Rome
 Date: October 12th, 2016
 License: This code is public domain. 
 
 This code reads all the various sensors (wind speed, direction, rain gauge, humidty, pressure, light)
 and reports it over the serial comm port. This can be easily routed to an datalogger (such as OpenLog) or
 a wireless transmitter (such as Electric Imp).
 Measurements are reported once a second but windspeed and rain gauge are tied to interrupts that are
 calculated at each report.
 This example code assumes the GPS module and battery level were not used.
 
 The circuit:
  * SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila
 ** CS - depends on your SD card shield or module.
    Pin 4 used here for consistency with other Arduino examples
 */

#include <Wire.h> //I2C needed for sensors


//#include <SPI.h> //Needed for SD card
//#include <SD.h> //Needed for SD card

#include "SparkFunMPL3115A2.h" //Pressure sensor - Search "SparkFun MPL3115" and install from Library Manager
#include "SparkFunHTU21D.h" //Humidity sensor - Search "SparkFun HTU21D" and install from Library Manager
#include "RTClib.h" // RTClib real time clock - Search "RTClib" and install from Library Manager
#include <Adafruit_SleepyDog.h> //needed for FONA
#include <SoftwareSerial.h> //needed for FONA
#include "Adafruit_FONA.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_FONA.h"

#define halt(s) { Serial.println(F( s )); while(1);  }

MPL3115A2 myPressure; //Create an instance of the pressure sensor
HTU21D myHumidity; //Create an instance of the humidity sensor
RTC_PCF8523 rtc; // Create an instance of the real time clock

//Hardware pin definitions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// digital I/O pins
const byte WSPEED = 3;
const byte RAIN = 2;
const byte STAT1 = 7;
const byte STAT2 = 8;

// analog I/O pins
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2; // unused
const byte WDIR = A0;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Global Variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define PERIOD 10000
long lastPeriod = 0;

long lastWindCheck = 0;
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;

//These are all the weather values that wunderground expects:
int winddir = 0; // [0-360 instantaneous wind direction]
float windspeed = 0; // [Kmh instantaneous wind speed]
float humidity = 0; // [%]
float tempc = 0; // [temperature C]
float rainin = 0;
float pressure = 0;
float light_lvl = 455; //[analog value from 0 to 1023]

// volatiles are subject to modification by IRQs
volatile unsigned long raintime, rainlast, raininterval;
volatile int rainClicks = 0;

// set up variables using the SD utility library functions:
//Sd2Card card;
//SdVolume volume;
//SdFile root;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
//const int chipSelect = 10;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*************************** FONA Pins ***********************************/


#define FONA_RX 11
#define FONA_TX 10
#define FONA_RST 9
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

/************************* APN CONFIG *********************************/

  // Optionally configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
#define FONA_APN       "internet.wind"
#define FONA_USERNAME  ""
#define FONA_PASSWORD  ""

/************************* MQTT server config *********************************/

#define AIO_SERVER      "m2m.eclipse.org"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    ""
#define AIO_KEY         ""

/************ Global State (you don't need to change this!) ******************/

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the FONA MQTT class by passing in the FONA class and MQTT server and login details.
Adafruit_MQTT_FONA mqtt(&fona, MQTT_SERVER, AIO_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

// FONAconnect is a helper function that sets up the FONA and connects to
// the GPRS network. See the fonahelper.cpp tab above for the source!
boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
const char WEATHERSTATION_1_FEED[] PROGMEM = AIO_USERNAME "iotenv/1";
Adafruit_MQTT_Publish weather_station = Adafruit_MQTT_Publish(&mqtt, WEATHERSTATION_1_FEED);

/*************************** Sketch Code ************************************/

// How many transmission failures in a row we're willing to be ok with before reset
uint8_t txfailures = 0;
#define MAXTXFAILURES 3

//Interrupt routines (these are called by the hardware interrupts, not by the main code)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void rainIRQ()
// Count rain gauge bucket tips as they occur
// Activated by the magnet and reed switch in the rain gauge, attached to input D2
{
  raintime = millis(); // grab current time
  raininterval = raintime - rainlast; // calculate interval between this and last event

  if (raininterval > 10) // ignore switch-bounce glitches less than 10mS after initial edge
  {
    rainClicks++;
    //rainin += 0.2794; // (mm) += 0.011; Each dump is 0.011" of water
    rainlast = raintime; // set up for next event
  }
}

void wspeedIRQ()
// Activated by the magnet in the anemometer (2 ticks per rotation), attached to input D3
{
  if (millis() - lastWindIRQ > 10) // Ignore switch-bounce glitches less than 10ms (228,52KmH max reading) after the reed switch closes
  {
    lastWindIRQ = millis(); //Grab the current time
    windClicks++; //There is 1.492MPH for each click per second.
  }
}


void setup()
{

   // Watchdog is optional!
  //Watchdog.enable(8000);
  Watchdog.reset();
  
  Serial.begin(115200);
  Serial.println("IoTenv Software");

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  Watchdog.reset();

  Watchdog.reset();
  
  pinMode(STAT1, OUTPUT); //Status LED Blue
  pinMode(STAT2, OUTPUT); //Status LED Green

  pinMode(WSPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
  pinMode(RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor

  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);

  //Configure the pressure sensor
  myPressure.begin(); // Get sensor online
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags

  //Configure the humidity sensor
  myHumidity.begin();

  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();
  
  // Initialise the FONA module
  while (! FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD))) {
    Serial.println("Retrying FONA");
  }

  Serial.println(F("Connected to Cellular!"));

  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();

  lastPeriod = millis();

  // attach external interrupt pins to IRQ functions
  attachInterrupt(0, rainIRQ, FALLING);
  attachInterrupt(1, wspeedIRQ, FALLING);

  // turn on interrupts
  interrupts();

  Serial.println("Weather Shield online!");
  Watchdog.reset();

}

void loop()
{
  
  //Keep track of which minute it is
  if(millis() - lastPeriod >= PERIOD)
  {
 
    digitalWrite(STAT1, HIGH); //Blink stat LED

    Watchdog.reset();
    
    // Ensure the connection to the MQTT server is alive (this will make the first
    // connection and automatically reconnect when disconnected).  See the MQTT_connect
    // function definition further below.
    MQTT_connect();
    
    Watchdog.reset();
    
    lastPeriod += PERIOD;

    //Report all readings every 10 seconds
    readWeather();

    if (! weather_station.publish(printWeather().c_str())) {
      Serial.println(F("Failed"));
      txfailures++;
    } else {
      Serial.println(F("OK!"));
      txfailures = 0;
    }


    Serial.println(printWeather());

    digitalWrite(STAT1, LOW); //Turn off stat LED
  }

  delay(100);
}

//Calculates each of the variables that wunderground is expecting
void readWeather()
{
  //Calc winddir
  winddir = get_wind_direction();

  //Calc windspeed
  windspeed = get_wind_speed();
  
  //Calc humidity
  humidity = myHumidity.readHumidity();    // I^2C register
  //float temp_h = myHumidity.readTemperature();
  //Serial.print(" TempH:");
  //Serial.print(temp_h, 2);

  //Calc tempc from pressure sensor
  tempc = myPressure.readTemp(); // Temperature in Celsius
  //Serial.print(" TempP:");
  //Serial.print(tempc, 2);

  //Total rainfall for the day is calculated within the interrupt
  rainin = get_rain_falling();

  //Calc pressure
  pressure = myPressure.readPressure();

  //Calc light level
  light_lvl = get_light_level();
}

//Returns the voltage of the light sensor based on the 3.3V rail
//This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
float get_light_level()
{
  float operatingVoltage = analogRead(REFERENCE_3V3);

  float lightSensor = analogRead(LIGHT);

  operatingVoltage = 3.3 / operatingVoltage; //The reference voltage is 3.3V

  lightSensor = operatingVoltage * lightSensor;

  return(lightSensor);
}

//Returns the instataneous wind speed
float get_wind_speed()
{
  float deltaTime = millis() - lastWindCheck; //750ms

  deltaTime /= 1000.0; //Covert to seconds

  float windSpeed = (float)windClicks / deltaTime; //3 / 0.750s = 4 click/s

  windClicks = 0; //Reset and start watching for new wind
  lastWindCheck = millis();

  windSpeed *= 2.4; // *=1.492; 4 * 1.492 = 5.968MPH --->KmH
    
  return(windSpeed);
}

float get_rain_falling()
{
  float deltaTime = millis() - rainlast; //750ms

  deltaTime /= 1000.0; //Convert to seconds

  float rain = (float)rainClicks / deltaTime; //clicks/s

  rainClicks = 0;
  rainlast = millis();

  rain *= 0.2794*60; // mm/min
    
  return(rain);
}

//Read the wind direction sensor, return heading in degrees
int get_wind_direction()
{
  unsigned int adc;

  adc = analogRead(WDIR); // get the current reading from the sensor

  // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
  // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
  // Note that these are not in compass degree order! See Weather Meters datasheet for more information.

  if (adc < 380) return (113);
  if (adc < 393) return (68);
  if (adc < 414) return (90);
  if (adc < 456) return (158);
  if (adc < 508) return (135);
  if (adc < 551) return (203);
  if (adc < 615) return (180);
  if (adc < 680) return (23);
  if (adc < 746) return (45);
  if (adc < 801) return (248);
  if (adc < 833) return (225);
  if (adc < 878) return (338);
  if (adc < 913) return (0);
  if (adc < 940) return (293);
  if (adc < 967) return (315);
  if (adc < 990) return (270);
  return (-1); // error, disconnected?
}


//Prints the various variables directly to the port
//I don't like the way this function is written but Arduino doesn't support floats under sprintf
String printWeather()
{
  String ws_status= "{";
  ws_status += "sensors";
  ws_status += ": ";
  ws_status += "[";
  ws_status += winddir;
  ws_status += ", ";
  ws_status += windspeed;
  ws_status += ", ";
  ws_status += rainin;
  ws_status += ", ";
  ws_status += humidity;
  ws_status += ", ";
  ws_status += tempc;
  ws_status += ", ";
  ws_status += pressure;
  ws_status += ", ";
  ws_status += light_lvl;
  ws_status += "], ";
  ws_status += printTime();
  
  return ws_status;
}

String printTime()
{
    DateTime right_now = rtc.now();
    String timestamp = "datetime";
    timestamp += ": ";
    timestamp += right_now.year();
    timestamp += "/";
    timestamp += right_now.month();
    timestamp += "/";
    timestamp += right_now.day();
    timestamp += " ";
    timestamp += right_now.hour();
    timestamp += ":";
    timestamp += right_now.minute();
    timestamp += ":";
    timestamp += right_now.second();
    timestamp += "}";
    return timestamp;
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password) {
  Watchdog.reset();

  Serial.println(F("Initializing FONA....(May take 3 seconds)"));
  
  fonaSS.begin(4800); // if you're using software serial
  
  if (! fona.begin(fonaSS)) {           // can also try fona.begin(Serial1) 
    Serial.println(F("Couldn't find FONA"));
    return false;
  }
  fonaSS.println("AT+CMEE=2");
  Serial.println(F("FONA is OK"));
  Watchdog.reset();
  Serial.println(F("Checking for network..."));
  while (fona.getNetworkStatus() != 1) {
   delay(500);
  }

  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();
  
  fona.setGPRSNetworkSettings(apn, username, password);

  Serial.println(F("Disabling GPRS"));
  fona.enableGPRS(false);
  
  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();

  Serial.println(F("Enabling GPRS"));
  if (!fona.enableGPRS(true)) {
    Serial.println(F("Failed to turn GPRS on"));  
    return false;
  }
  Watchdog.reset();

  return true;
}

