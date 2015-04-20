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
 
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
char serverName[] = "api.openweathermap.org";
//char serverName[] = "http://api.wunderground.com";
 
// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

unsigned long lastAttemptTime = 0; // last time connected to server, in msec
boolean irrigate = true;
String location;
String request="";
int sensorPin = A0; // Input pin for moisture sensor
int sensor_value = 0; // value from moisture sensor
int soil_moisture_level = 0; // value from moisture sensor

#define W5200_CS  10
#define SDCARD_CS 4
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  pinMode(SDCARD_CS,OUTPUT);
  digitalWrite(SDCARD_CS,HIGH);//Deselect the SD card
  
  // Determine desired weather location from user
  location = getLocation();
  
  // Connect to weather server
  connectToServer(location);
}
 
void loop()
{
  // Measure soil moisture level
  soil_moisture_level = measureMoisture();
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
   lastAttemptTime = millis();
   
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
 
