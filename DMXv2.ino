/*

 Base macro code 


*/

//  include librayrys 
#include <ESP8266WiFi.h>  
#include <ESP8266mDNS.h>  
#include <WiFiUdp.h>  
#include <ArduinoOTA.h>  

#include <FastLED.h>
#include <DMXSerial.h>

// define constant

const char* ssid = "Champagne";       // SSID name
const char* password = "1cafe2cafe";  // wifi password
#define LED_PIN     12    /GPIO12
#define NUM_LEDS    26
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
//#define UPDATES_PER_SECOND  100
CRGB leds[NUM_LEDS];
#define DMXSTART 81  //First DMX Channel
#define DMXLENGTH 88  // number of DMX channels used


// Setup()

// Setup OTA

// Setup DMX



