#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiMulti.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "RemoteDebug.h"  //https://github.com/JoaoLopesF/RemoteDebug
#define USE_ARDUINO_INTERRUPTS true
#include <NTPClient.h>
#include "SPIFFS.h"
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// #define DEBUG 1
#define  NETWORK_DEBUG 1

    
#ifdef DEBUG
#define DEBUG_PRINTLN(x)      Serial.println (x)
#define DEBUG_PRINT(x)        Serial.print (x)
#define DEBUG_PRINTF(f,...)   Serial.printf(f,##__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(f,...)
#endif

#ifdef NETWORK_DEBUG
    #define    DEBUG_PRINTF(f,...)  Debug.printf(f,##__VA_ARGS__)
#endif

WiFiMulti   wifiMulti;          //  Create  an  instance  of  the ESP32WiFiMulti 



static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
RemoteDebug   Debug;
// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);


bool ConnectToWifi()
{

  int count ;
  wifiMulti.addAP("Sri Anjaneya", "ABCD123456");   
  //wifiMulti.addAP(ConfigData.ssid2, ConfigData.password2);    
 //wifiMulti.addAP(ConfigData.ssid3, ConfigData.password3);    
  count  = 0 ;
//  Debug.printf("Hello");
  DEBUG_PRINTF("*");


  while  (wifiMulti.run()  !=  WL_CONNECTED) 
  { 
    //  Wait  for the Wi-Fi to  connect:  scan  for Wi-Fi networks, and connect to  the strongest of  the networks  above       
    delay(1000);        
    DEBUG_PRINTF("*");    
    count++ ;
    if (count > 40)
    {
       return false ;  
    }
  }   
  delay(5000);
  WiFi.setHostname("GPS-test");

  DEBUG_PRINTF("\n");   
  DEBUG_PRINTF("Connected to  ");   
  DEBUG_PRINTF("%s\n",WiFi.SSID().c_str());         
  DEBUG_PRINTF("IP  address: ");   
  DEBUG_PRINTF("%s",WiFi.localIP().toString()); 

  WiFi.softAPdisconnect (true);   //Disable the Access point mode.
  return true ;
}



void setup()
{
  
  Serial.begin(9600);
  ConnectToWifi();
  Debug.begin("GPS-test"); // Initialize the WiFi server
  Debug.setResetCmdEnabled(true); // Enable the reset command
  Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
  Debug.showColors(true); // Colors
  ss.begin(GPSBaud);
}
void AdjustTime(int hr, int minutes, int sec, char *timeStamp)
{
  int seconds ;
  int hours ;
  int balance ;
  
  
 Serial.printf("Adjust Time-1 %d, %d ,%d \n", hr, minutes,sec);
  seconds = hr *60 * 60  + minutes * 60  + sec ;
  seconds = seconds + 19800 ; //GMT correction
  hours =  seconds/3600 ;
  balance =  seconds % 3600 ;
  minutes = balance /  60 ;
  balance = balance % 60 ;
  seconds = balance ; 
  Serial.printf("Adjust Time-2 %d %d %d\n",hours, minutes,seconds);
  sprintf(timeStamp, "%d: % d %d", hours, minutes,seconds);
  Serial.printf("Adjust Time 3 \n");
  Serial.printf("%s\n",timeStamp);
  return;
  
  
}
void loop()
{
  char stamp[15] ;
  // This sketch displays information every time a new sentence is correctly encoded.
    Debug.handle();
    yield();   
 
  if (ss.available() > 0)
  {
    byte gpsData = ss.read();
    Serial.write(gpsData);
    gps.encode(gpsData);

    if (gps.location.isUpdated())
    {
       
      DEBUG_PRINTF("L:%d D:%d T:%d-> \n",gps.location.isValid(),gps.date.isValid() ,gps.time.isValid());
      
      DEBUG_PRINTF("Lat:%f - ",gps.location.lat());
      DEBUG_PRINTF("Log:%f \n", gps.location.lng());

      DEBUG_PRINTF("%d:" ,gps.time.hour());
      DEBUG_PRINTF("%d:" ,gps.time.minute());
      DEBUG_PRINTF("%d\n" ,gps.time.second());
      int h,m ,s ;
      h = gps.time.hour();
      m = gps.time.minute();
      s = gps.time.second();
      AdjustTime(h,m,s,stamp);
      DEBUG_PRINTF("%s \n", stamp );
      //DEBUG_PRINTF("%d-%d-%d\n",gps.date.day(),gps.date.month(),gps.date.year());
      DEBUG_PRINTF("Satellites: %d  Altitude : %d \n", gps.satellites.value(),(gps.altitude.value()/100));
      
    } 
  }
}
