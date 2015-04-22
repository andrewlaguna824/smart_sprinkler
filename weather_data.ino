// Andrew Palmer ajp294@cornell.edu
// 6 April 2015
// To Do: 
// Clean up code and turn most things into functions (like twitter example)
// add user input for location - Done
// might need some sort of Serial menu (do at end of project) to navigate program
// make Connect to Server take in location input as argument
// ADC code for moisture sensor

#include <SPI.h>
#include <EthernetV2_0.h>
#include <Wire.h>
#include "RTClib.h"
 
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
char serverName[] = "api.openweathermap.org";
//char serverName[] = "http://api.wunderground.com";
 
// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
// Initialize RTC_DS1307 Time Module
RTC_DS1307 rtc;

int state = 0; // Program state
boolean irrigate = true; // boolean to irrigate or not
String location; // user inputed location
String request=""; // url request string
int sensorPin = A0; // Input pin for moisture sensor
int sensor_value = 0; // value from moisture sensor
int soil_moisture_level = 0; // value from moisture sensor
int m, d, y; // Month, day, and year global variables
int s, mn, hr; // second, minute, and hour global variables
int check_time, check_s, check_mn, check_hr; // time to check moisture

#define W5200_CS  10
#define SDCARD_CS 4
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  
  pinMode(SDCARD_CS,OUTPUT);
  digitalWrite(SDCARD_CS,HIGH);//Deselect the SD card
  
  // Make sure RTC is running
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");  
  }
  // Determine desired weather location from user
  //location = getLocation();
  
  // Connect to weather server
  //connectToServer(location);
} // end setup
 
void loop()
{
  
  switch (state) {
    case 0: {
      // Program Menu to set location, time, or force check moisture
      // only go into Menu when menu button pressed
      // if button is pressed, check moisture level
      // if another button is pressed, force irrigation
      // Serial.println("Do you need to set the current date?");
      // Serial.println("Do you need to set the current time?");
      getDate(); // get current date
      getCurrentTime(); // get current time
      // Set the RTC Date and Time
      rtc.adjust(DateTime(y, m, d, hr, mn, s)); // year, month, day, hour, minute, second
      delay(1000);
      setCheckTime(); // set time to check moisture
      // Set desired location;
      location = getLocation();
      state = 1;
    }
      break;
    case 1: {
      // Continually check time
      Serial.println("REAL TIME CLOCK CASE STATEMENT");
      DateTime now = rtc.now();
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(' ');
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.print(now.second(), DEC);
      Serial.println();
      state = 2;
    }
      break;
    case 2: {
      // Measure Moisture
      soil_moisture_level = measureMoisture();
      state = 3;
    }
      break;
    case 3: {
      // Download weather data from user's location
      connectToServer(location);
      while(client.available()) {
        if (client.find("number=\"5")) {
          Serial.println("Rain in the weather forecast"); 
          Serial.println("Irrigation system will not turn on");
          // irrigate = false;
        } // end if
        else if (!client.find("number=\"5") && irrigate) {
          Serial.println("No rain in the weather forecast, but soil is dry");
          Serial.println("Irrigation system will turn on");
          // irrigate = true;
        } // end else if
      } // end while
 
      // if the server's disconnected, stop the client:
      if (!client.connected()) {
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();
 
        // do nothing forevermore:
        while(true);
      } // end it
    } //end case 3
      state = 1;
      break;
  } // end switch statement
  
} //end loop function
  // location = getLocation();
/*  connectToServer(location);
  // Measure soil moisture level
  //soil_moisture_level = measureMoisture();
  // parse weather data for precipitation forecast
  while(client.available()) {
    //Serial.println("client available");
     //char c = client.read();
     //Serial.print(c);
     if (client.find("number=\"5")) {
       Serial.println("Rain in the weather forecast"); 
       Serial.println("Irrigation system will not turn on");
       // irrigate = false;
     }
     else if (!client.find("number=\"5") && irrigate) {
       Serial.println("No rain in the weather forecast, but soil is dry");
       Serial.println("Irrigation system will turn on");
       // irrigate = true;
     }
  }
 
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
 
    // do nothing forevermore:
    while(true);
  }
}
 */
 void connectToServer(String location_name) {
   // attempt DHCP connection
   if (!Ethernet.begin(mac)) {
     Serial.println("failed to get an IP address using DHCP"); 
   }
   delay(1000);
   // attempt to connect to server and wait a millisecond
   // CAN USE THIS METHOD to ask user for city name also
   Serial.println("connecting to server...");
   if (client.connect(serverName, 80)) {
     Serial.println("connected!");
     Serial.println("making HTTP request...");
     // make the HTTP GET request to weather api
     request = "GET /data/2.5/forecast/daily?q=" + location_name + "&mode=xml&units=metric&cnt=1 HTTP/1.0";
     Serial.println(request);
     client.println(request);
     client.println();
   }// end if
   
   else {
    // if you didn't get a connection to the server:
    // maybe retry a couple times in this method
    Serial.println("connection failed");
   }
   
   // note the time of this connect attempt
   //lastAttemptTime = millis();
   
 } // end connectToServer method
 
 String getLocation() {
   String city_name;
   Serial.println("Please enter desired city: ");
     while (!Serial.available());  // wait for the user to input data
     while (Serial.available()) {
       delay(10);
       if (Serial.available() > 0) {
         char c = Serial.read();
         city_name += c; 
       } // end if
     } // end while 
     Serial.println("LOCATION: ");
     Serial.println(city_name);
     return city_name;
 } // end get location method
 
 String getDate() {
   String date;
   Serial.println("Please enter current date in mmddyyyy format");
   while (!Serial.available()); // wait for user to input data
   while (Serial.available()) { // read user input
     delay(10);
     if (Serial.available() > 0) {
       char c = Serial.read();
       date += c;
     } // end if
   } // end while
   m = date.substring(0,2).toInt();
   d = date.substring(2,4).toInt();
   y = date.substring(4).toInt();
  // m.toInt();
   //d.toInt();
   //y.toInt();
   Serial.println("DATE: " + String(m) + "/" + String(d) + "/" + String(y));
   return date;
 } // End get date method
 
 String getCurrentTime() {
   String time;
   Serial.println("Please enter current time in hhmmss format");
   while (!Serial.available()); // wait for user to input data
   while (Serial.available()) { // read user input
     delay(10);
     if (Serial.available() > 0) {
       char c = Serial.read();
       time += c;
     } // end if
   } // end while
   hr = time.substring(0,2).toInt();
   mn = time.substring(2,4).toInt();
   s = time.substring(4).toInt();
   //hr.toInt();
   //mn.toInt();
   //s.toInt();
   Serial.println("TIME: " + String(hr) + ":" + String(mn) + ":" + String(s));
   return time; 
 }
 
 String setCheckTime() {
   String check_time;
   Serial.println("Please enter desired moisture check time in hhmmss format");
   while (!Serial.available()); // wait for user to input data
   while (Serial.available()) { // read user input
     delay(10);
     if (Serial.available() > 0) {
       char c = Serial.read();
       check_time += c;
     } // end if
   } // end while
   check_hr = check_time.substring(0,2).toInt();
   check_mn = check_time.substring(2,4).toInt();
   check_s = check_time.substring(4).toInt();
   //check_hr.toInt();
   //check_mn.toInt();
   //check_s.toInt();
   Serial.println("Check Time: " + String(check_hr) + ":" + String(check_mn) + ":" + String(check_s));
   return check_time; 
 }
 
 int measureMoisture() {
   // read value from sensor
   sensor_value = analogRead(sensorPin);
   
   if (sensor_value <= 430) {
     irrigate = true; 
   }
   else {
     irrigate = false; 
   }
   // print sensor value to UART
   Serial.println("Moisture Value: " + sensor_value);
 } // end measure Moisture method
 
