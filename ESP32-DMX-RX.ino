/*

DMX receiver 8 channels ( future to have two modes with extended 16 channels..)    
      
      
      Channel 1  Brightness for smart RGB strip
      Channel 2  Brightness for analog RGB leds
      Channel 3  Speed of animation
      Channel 4  Hue control 
      Channel 5  Hue2 - addtional control for section 2
      Channel 6  Hue3 - addtional control for section 3
      Channel 7  relay control  00, 01, 10, 11
      Channel 8   Show selection, 26 modes (round dmx value / 10) 0.0 to 25.5

   

LED STRIP = 7.5M long for 75 lEDS 2811
Section 1.. 250cm = 25 leds,   is above fixed RGB using PWM output1
Section 2.. 250cm = 25 leds  is above fixed RGB using PWM output2
Section 3...250cm = 25 leds    is above fixed RGB using PWM output3


*/


#include <dmx.h>
#include <FastLED.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


/*
 * 
 * ----------CONSTANT
 * 
 */
const char* ssid = "********";
const char* password = "*********";
const uint8_t  BrightAdjust[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

#define COLOR_ORDER BRG
#define LED_TYPE    WS2811
#define NUM_LEDS 75

#define END_LEDS1 24   // LED ID of end of section 1
#define END_LEDS2 49    // LED IDof end of section 2
#define MID_LEDS1 12    // LED ID of middle of section 1
#define MID_LEDS2 37    // LED ID of middle of section 2
#define MID_LEDS3 62    // LED ID of middle of section 3
#define DMXSTART 1   //the base address start


//Pin Definitions for FastLED
#define DEBUG 1
#define BUILT_IN 2  //Builtin LED GPIO
#define LED_PIN 4   //strip GPIO Pin
#define RELAY1_PIN 25 // Relay trigger pin    
#define RELAY2_PIN 33 //
#define R1 0        //RGB PWM Channels and PINS 3 x RGB
#define R1_PIN 12
#define G1 1
#define G1_PIN 13
#define B1 2
#define B1_PIN 14
#define R2 3
#define R2_PIN 23
#define G2 4
#define G2_PIN 21
#define B2 5
#define B2_PIN 22
#define R3 6
#define R3_PIN 19
#define G3 7
#define G3_PIN 18
#define B3 8
#define B3_PIN 5


#define NUM_STARS 5 //Number of stars in the random starts show

//FastLED smart LED config
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
CRGB matrix[NUM_LEDS];

/*
 * 
 *  -----  VARIABLES
 * 
 * 
 */

bool relay1 = 0;    // Status of Relay 1
bool relay2 = 0;     // Status of relay 2
int readcycle = 0;
uint8_t BRIGHT1 = 0; // Brightness level for smart leds strip
uint8_t BRIGHT2 = 0;  // Brightness level for analog RGB
uint8_t SPEED = 0;   // Speed of animation 
uint8_t HUE = 0;
uint8_t HUE2 = 0;     //when applicable, hue for section 2
uint8_t HUE3 = 0;     // when applicable hue for section 3
uint8_t DMX_rel = 0;   //DMX value from the Relay channe;
uint8_t DMX_show = 0;   //DMX value for the show select Channel
uint8_t ShowMode = 0;   // Show number currently playing
uint8_t StepMax = 0;    //Total # of steps in the show
uint8_t Step = 0;  // step counter for show
bool DirUp = true;  // true, step increase, false decrease
uint8_t Stars[NUM_STARS];   // array for the randomstars show

bool NewShow = true;
bool NewHue = false;
bool NewBright = false;
bool HasChanged = 0;
/*
 * 
 * ---------------- SETUP
 * 
 */


void setup() {
  if (DEBUG) {Serial.begin(115200);}

  /*

     ------------ OTA setup

  */
  if (DEBUG) {Serial.println("Booting");}
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    if (DEBUG) {Serial.println("Connection Failed! Rebooting...");}
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    if (DEBUG) {Serial.println("Start updating " + type);}
  })
  .onEnd([]() {
    if (DEBUG) {Serial.println("\nEnd");}
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    if (DEBUG) {Serial.printf("Progress: %u%%\r", (progress / (total / 100)));}
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  /*
      -------------------  Outputs FASTLED and DMX SETUP

  */
  pinMode(BUILT_IN,OUTPUT);
  pinMode(RELAY1_PIN,OUTPUT);
  pinMode(RELAY2_PIN,OUTPUT);
  ledcAttachPin(R1_PIN,R1);  ledcSetup(R1,4000,8);
  ledcAttachPin(G1_PIN,G1);  ledcSetup(G1,4000,8);
  ledcAttachPin(B1_PIN,B1);  ledcSetup(B1,4000,8);
  ledcAttachPin(R2_PIN,R2);  ledcSetup(R2,4000,8);
  ledcAttachPin(G2_PIN,G2);  ledcSetup(G2,4000,8);
  ledcAttachPin(B2_PIN,B2);  ledcSetup(B2,4000,8);
  ledcAttachPin(R3_PIN,R3);  ledcSetup(R3,4000,8);
  ledcAttachPin(G3_PIN,G3);  ledcSetup(G3,4000,8);
  ledcAttachPin(B3_PIN,B3);  ledcSetup(B3,4000,8);
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(matrix, NUM_LEDS); //.setCorrection( TypicalLEDStrip );
  DMX::Initialize();
}
/*

   -----------------------LOOP------------------------
*/
void loop()
{
  // OTA Check
  ArduinoOTA.handle();

/*  
 *   
 *   ------------ DEBUG SECTION DMX CHECKING
 *   
 */
  if (DMX::IsHealthy() == 0)
  {
      if(DEBUG) {Serial.println("DMX fail");}
      Pulse(3);
      delay(200);
  }
 
  if ((millis() - readcycle > 10000) and DEBUG)
  {
    readcycle = millis();
    Serial.print(readcycle);
    if (DMX::IsHealthy())
    {
      Serial.print(": ok - ");
    }
    else
    {
      Serial.print(": fail - ");
    }
    Serial.print(DMX::Read(DMXSTART+0));
    Serial.print(" - ");
    Serial.print(DMX::Read(DMXSTART+1));
    Serial.print(" - ");
    Serial.print(DMX::Read(DMXSTART+2));
    Serial.print(" - ");
    Serial.print(DMX::Read(DMXSTART+3));
    Serial.print(" - ");
    Serial.println(DMX::Read(DMXSTART+4));
  }

 if (DEBUG and DMX::IsHealthy() and DMX::Read(DMXSTART+2) == 0 and DMX::Read(DMXSTART+3) == 0 and DMX::Read(DMXSTART+7) == 0 and DMX::Read(4) == 0 and DMX::Read(5) == 0) 
    { Serial.print(millis());
      Serial.print(" - 0-0-0-0 - ");
      do { delay(5);} while 
      (DEBUG and DMX::IsHealthy() and DMX::Read(1) == 0 and DMX::Read(2) == 0 and DMX::Read(3) == 0 and DMX::Read(4) == 0 and DMX::Read(5) == 0);
     Serial.println(millis());
     }  
 
/*
 * 
 * 
 *    DMX UPDATES 
 * 
 */
// Channel 1 BRIGHT for smart RGB LED strip
  if (DMX::Read(DMXSTART + 0) != BRIGHT1)
  {
    BRIGHT1 = DMX::Read(DMXSTART + 0);
    FastLED.setBrightness (BRIGHT1);
    if (DEBUG) {Serial.print("Bright1 : ");}
    if (DEBUG) {Serial.println(BRIGHT1);}
    NewBright = true;
  }
// Channel 2 BRIGHT2 for fixed analog RGB LED
  if (DMX::Read(DMXSTART + 1) != BRIGHT2)
  {
    BRIGHT2 = DMX::Read(DMXSTART + 1);
    if (DEBUG) {Serial.print("Bright2 : ");}
    if (DEBUG) {Serial.println(BRIGHT2);}
    NewBright = true;
  }
//  Channel 3 SPEED
  if (DMX::Read(DMXSTART + 2) != SPEED)
  {
    SPEED = DMX::Read(DMXSTART + 2);
    if (DEBUG) {Serial.print("SPEED: ");}
    if (DEBUG) {Serial.println(SPEED);}
  }
// Channel 4 HUE
  if (DMX::Read(DMXSTART + 3) != HUE) 
  {
    HUE = DMX::Read(DMXSTART + 3);
    if (DEBUG) {Serial.print("HUE: ");}
    if (DEBUG) {Serial.println(HUE);}
    NewHue=true;
  }
// Channel 5 HUE2 for section 2
  if (DMX::Read(DMXSTART + 4) != HUE2) 
  {
    HUE2 = DMX::Read(DMXSTART + 4);
    if (DEBUG) {Serial.print("HUE2: ");}
    if (DEBUG) {Serial.println(HUE2);}
    NewHue = true;
  }
// Channel 6 HUE3 for section 3
  if (DMX::Read(DMXSTART + 5) != HUE3) 
  {
    HUE3 = DMX::Read(DMXSTART + 5);
    if (DEBUG) {Serial.print("HUE3: ");}
    if (DEBUG) {Serial.println(HUE3);}
    NewHue = true;
  }
// Channel 7 is Relays output control
  if (DMX::Read(DMXSTART + 6) != DMX_rel)  // DMX value has changed
    {
      DMX_rel = DMX::Read(DMXSTART + 6);
      UpdateRelays();
    }
//  Channel 8 SHOW SELECT
  if (DMX::Read(DMXSTART + 7) != DMX_show)
  {
    DMX_show = DMX::Read(DMXSTART + 7);
    ChangeShow(DMX_show);
  }
/*

 ----------------- SHOW TIME ------------

*/
switch(ShowMode)
  {
  case 0:  // -------------- NightRider
    if (NewShow)
    {
     StepMax = NUM_LEDS / 3;    // dividing the #of leds in three segments
     DirUp = true;            // we are starting forward     
     NewShow = false;
    }
    NightRider3(Step,false);
    Step = Step + 1;
    if (Step >= StepMax){
      Step = 0;
      if (DirUp == true) {DirUp = false;} else {DirUp = true;}
      }
    FastLED.show();
    FastLED.delay(SPEED+75);
    break;
    
  case 1:  // ---------------- RandomStars
    if (NewShow)
    {
     StepMax = NUM_STARS; //*16; Each Stars goes through 1evels of brightness
     FastLED.clear(); 
     NewShow = false;
    }
    RandomStars(Step,0);
    Step += 1;
    if (Step >= StepMax){Step =0;}
    FastLED.delay(SPEED);
    break;
    
  case 2:  // ------------------- Burst single color
    if (NewShow)
    {
      if (DEBUG) {Serial.println("--Burst--");}
      StepMax =round( NUM_LEDS / 6); // number of steps equal to each third
      DirUp = true;
      NewShow = false;
      Step = 1;
    }
    Burst(Step,DirUp, false);   // 
    Step = Step +1;
    if (Step > StepMax) 
      {
        Step=1;
        if (DirUp == true) {
            DirUp = false;
            } else 
            {DirUp = true;}
      }
    break;   

  case 3: // -------------- Ocean Colors
    if (NewShow)
    {
      currentPalette = OceanColors_p;
      currentBlending = LINEARBLEND;
      Serial.println("OceanColors_p");
      NewShow = false;
    }
    PalettePlay(Step);
    Step += 1;
    FastLED.delay(SPEED);
    break;
    
  case 4:
    if (NewShow)
    {
      currentPalette = RainbowStripeColors_p;
      currentBlending = LINEARBLEND;
      Serial.println("RainbowStripColors_p"); 
      NewShow = false;
    }
    PalettePlay(Step);
    Step += 1;
    FastLED.delay(50+SPEED);
    break;
    
  case 5:
    if (NewShow)
    {
      currentPalette = PartyColors_p;
      currentBlending = LINEARBLEND;
      Serial.println("PartyColors_p");
      NewShow = false;
    } 
    PalettePlay(Step);
    Step += 1;
    FastLED.delay(SPEED);   
    break;
    
  case 6:
    if (NewShow)
    {
      currentPalette = RainbowColors_p;
      currentBlending = LINEARBLEND;  
      Serial.println("RainbowColors_p");    
      NewShow = false;
    }
    PalettePlay(Step);
    Step += 1;
    FastLED.delay(SPEED);
    break; 

  case 10:  // -------------- NightRider
    if (NewShow)
    {
     StepMax = NUM_LEDS / 3;    // dividing the #of leds in three segments
     DirUp = true;            // we are starting forward     
     NewShow = false;
    }
    NightRider3(Step,true);
    Step = Step + 1;
    if (Step >= StepMax){
      Step = 0;
      if (DirUp == true) {DirUp = false;} else {DirUp = true;}
      }
    FastLED.show();
    FastLED.delay(SPEED+75);
    break;
    
  case 11:  // ----------------- Random starst 3sections
    if (NewShow)
    {
      StepMax = NUM_STARS; //*16; Each Stars goes through 1evels of brightness
      NewShow = false;
      FastLED.clear();
    }
    RandomStars(Step,1);
    Step += 1;
    if (Step >= StepMax){Step =0;}
    FastLED.delay(SPEED);
    break;
    
  case 12:  // ------------------- Burst three color
    if (NewShow)
    {
      if (DEBUG) {Serial.println("--Burst--");}
      StepMax =round( NUM_LEDS / 6); // number of steps equal to each third
      DirUp = true;
      NewShow = false;
      Step = 1;
    }
    Burst(Step,DirUp, true);   // 
    Step = Step +1;
    if (Step > StepMax) 
      {
        Step=1;
        if (DirUp == true) {
            DirUp = false;
            } else 
            {DirUp = true;}
      }
    break; 
      
  case 22:  // Fixed color, 2 independant sections, 3rd is off
    if (NewShow or  NewHue or NewBright){
    FixedColor(false,true);
    NewShow = false;
    NewHue = false;
    NewBright = false;
    }
    break;
    
  case 23:  // Fixed color, 2 independant sections, 3rd is off
    if (NewShow or  NewHue or NewBright){
    FixedColor(false,true);
    NewShow = false;
    NewHue = false;
    NewBright = false;
    }
    break;
       
  case 24:  // Fixed color, 3 independant sections
    if (NewShow or  NewHue or NewBright){
    FixedColor(true,false);
    NewShow = false;
    NewHue = false;
    NewBright = false;
    }
    break;
    
  case 25:  // Fixed color,no movement, Hue control of analog rgb
    if (NewShow or NewHue or NewBright) {
    FixedColor(false,false);
    NewShow = false;
    NewHue = false;
    NewBright = false;
    }
    break;
      
  default:
    if (NewShow)
    {
    Serial.print("Undefined ShowMode - ");
    Serial.println(ShowMode);
    NewShow = false;
    }
    break;
   }
}

//   -------------------- FUNCTIONS --------------------------------------

/*
 *   ------------ CHANGE SHOW MODE 
 *    Change the show number based on the DMX value passed via channelVal  
   the channelvalue from DMX is between 0 255 but ignoring last digit
   So 26 channels when looking at 10's
     00-09 =mode0, 10-19= mode 1, 110-119 = mode 11, 240-249 
 * 
 */
void ChangeShow(uint8_t channelval) 
{
  uint8_t ChannelMode = round(channelval/10); // divide by 10
  if (ChannelMode != ShowMode)                // are we changing mode ?
  {
    ShowMode = ChannelMode;         // Updating the show number
    NewShow = true;
    NewHue = false;
    NewBright = false;
    Step = 0;                       //Reset the step counter
    if (DEBUG) {Serial.print("switching to mode - ");}
    if (DEBUG) {Serial.println(ShowMode);}
  }
}
/*
 * ------------------------------------------------------------------------
 *  Fill all the led matrix from the current palette starting at the 
 *   colorindex position  
 * 
 */
void PalettePlay( uint8_t colorIndex)
{
  for ( int i = 0; i < NUM_LEDS; i++) {
    matrix[i] = ColorFromPalette( currentPalette, colorIndex, 255, currentBlending);
    colorIndex += 3;
    }
    // Also matching the analog LEDS to the middle leds of each section
    AnalogRGB(1,matrix[MID_LEDS1],BRIGHT2);
    AnalogRGB(2,matrix[MID_LEDS2],BRIGHT2);
    AnalogRGB(3,matrix[MID_LEDS3],BRIGHT2);

    FastLED.show();
}

/*
 * ----------------------- ANALOG RGB LEDS -------------
 *   Functions to turn on 3 RGB LEDS OUTPUTS  usiung RGB argument
   1st argument poitns to which led, 1,2 or 3 to select (choose 4 for all 3)
   2nd arg is the CRGB value
   3rd is brightness. Could be passed via an HSV conversion but does not work with Palettes hence this arg
                      when using an HSV covnersion in the 2nd arg, set V at 255 and brightness via the 3rd arg
 */

void AnalogRGB( uint8_t led, const CRGB& rgb, uint8_t br)
{
 
  uint8_t BrightAd = BrightAdjust[br]; // this uses the template at the end to adjust brightness
  if (led==1 or led==4)
  {
  ledcWrite(R1,scale8(rgb.r,BrightAd));
  ledcWrite(G1,scale8(rgb.g,BrightAd));
  ledcWrite(B1,scale8(rgb.b,BrightAd));
  }
  if (led==2 or led==4)
  {
  ledcWrite(R2,scale8(rgb.r,BrightAd));
  ledcWrite(G2,scale8(rgb.g,BrightAd));
  ledcWrite(B2,scale8(rgb.b,BrightAd));
  }
  if (led==3 or led==4)
  {
  ledcWrite(R3,scale8(rgb.r,BrightAd));
  ledcWrite(G3,scale8(rgb.g,BrightAd));
  ledcWrite(B3,scale8(rgb.b,BrightAd));
  }
}


/*
 * -----------------DEV SMART LEDS fixed color ----------
 *  DEV -- Similar to AnalogRGB for fixed SmartRGB
 * section 1,2 or 3 . 4 means all sections
 * 
 */
void SmartRGB(uint8_t section, const CRGB& rgb,uint8_t br)
{
  if (section == 1 or section == 4)
    {
    for ( int x = 0; x < END_LEDS1; x++)
    {matrix[x] = CHSV(rgb,255,br);}
    }
  if (section == 2 or section == 4)
    {
    for ( int x = END_LEDS1; x < END_LEDS2; x++)
    {matrix[x] = CHSV(rgb,255,br);}
    }
  if (section == 3 or section == 4)
    {
   for ( int x = END_LEDS2; x < NUM_LEDS; x++)
   {matrix[x] = CHSV(rgb,255,br);}
    }
 }

 
/*
 * ---------------- RELAY CONTROL
 * DMX 0 is all off, DMX 255 is both on
 * DMX lower half is 1 only and upper half is relay 2 only
 */
void UpdateRelays()
{
  if (relay1 == 0 and ((DMX_rel >= 1 and DMX_rel <= 120) or DMX_rel == 255))
        {
          relay1 = 1;
          digitalWrite(RELAY1_PIN,LOW);   // inverted relay!
          if (DEBUG) {Serial.println("Relay 1 ON");}
        }   
      if (relay1 == 1 and ((DMX_rel <= 254 and DMX_rel >= 130) or DMX_rel == 0))
        {
          relay1 = 0;
          digitalWrite(RELAY1_PIN,HIGH); //iunverted RELAY
          if (DEBUG) {Serial.println("Relay 1 OFF");}
        }    
      if (relay2 == 0 and DMX_rel >= 130)
        {
          relay2 = 1;
          digitalWrite(RELAY2_PIN,HIGH);
          if (DEBUG) {Serial.println("Relay 2 ON");}
        }   
      if (relay2 == 1 and DMX_rel <= 120)
        {
          relay2 = 0;
          digitalWrite(RELAY2_PIN,LOW);
          if (DEBUG) {Serial.println("Relay 2 OFF");}
        }        
}

/*
 * ---------------------------- NIGHT RIDER
 * 
 */
void NightRider3(uint8_t i, bool three) //Pos point to the step in the sequence
{
  int h2 = HUE;
  int h3 = HUE;
  if (three)
    { h2 = HUE2;
      h3 = HUE3;
     } 
  FastLED.clear();

    if (DirUp == true) // going forward
    {
    matrix[i] = CHSV(HUE,255,BRIGHT1);
    matrix[END_LEDS2 -1 -i] =CHSV(h2,255,BRIGHT1);
    matrix[END_LEDS2 +i ] = CHSV(h3,255,BRIGHT1);
    if (i>0) { //adding a tail only after leave the end at half the brightness
      matrix[i-1] = CHSV(HUE,255,BRIGHT1/2); // tail
      matrix[END_LEDS2 -1 -i +1] =CHSV(h2,255,BRIGHT1/2);
      matrix[END_LEDS2 +i -1] = CHSV(h3,255,BRIGHT1/2); // tail
        }
    }
    else   // going backward
    {
    matrix[END_LEDS1 -1 -i] = CHSV(HUE,255,BRIGHT1);
    matrix[END_LEDS1 +i] = CHSV(h2,255,BRIGHT1);
    matrix[NUM_LEDS -i] = CHSV(h3,255,BRIGHT1);
    if (i>0) { //adding a tail only after leaving the end
      matrix[END_LEDS1 -1 -i +1] = CHSV(HUE,255,BRIGHT1/2); // tail
      matrix[END_LEDS1 +i -1] =CHSV(h2,255,BRIGHT1/2);
      matrix[NUM_LEDS -i +1] = CHSV(h3,255,BRIGHT1/2); // tail
        }
    } 
    if (three)
    {
      AnalogRGB(1,CHSV(HUE,255,255),BRIGHT2);
      AnalogRGB(2,CHSV(HUE2,255,255),BRIGHT2);
      AnalogRGB(3,CHSV(HUE3,255,255),BRIGHT2);
    }
    else
    {
    AnalogRGB(4,CHSV(HUE,255,255),BRIGHT2);
    }
}

/*
 * ---------------------- RANDOM STARS  ---------------------
 * random 5 starst that appears and disappers radnon
 * 
 */
void RandomStars(int i, bool three)
{
  randomSeed(analogRead(0));
  bool Found = false;
  int newstar = 0;
  int col = HUE;   // color of the newstar, use HUE for same all or diff for three
  do
    { // check to see if newstar is in the array already, if so need to pick another one
      Found = false;
      newstar = random(0,NUM_LEDS);
      for ( int x = 0; x < NUM_STARS; x++)
      {
        if (Stars[x] == newstar) {Found = true;}
      }
    } while (Found == true);
   // Serial.println(newstar);
   
  for (int x=1; x<255; x++)   //fade the previous start before the new one
    {
      matrix[Stars[i]].fadeToBlackBy(x);
      FastLED.show();
    }
  if (three) 
    {
      if (newstar <= END_LEDS1){col = HUE;}
      if (newstar > END_LEDS1 and newstar <= END_LEDS2) {col = HUE2;}
      if (newstar > END_LEDS2) {col = HUE3;}
      AnalogRGB(1,CHSV(HUE,255,255),BRIGHT2);
      AnalogRGB(2,CHSV(HUE2,255,255),BRIGHT2);
      AnalogRGB(3,CHSV(HUE3,255,255),BRIGHT2);
    }
  else
    {
      AnalogRGB(4,CHSV(HUE,255,255),BRIGHT2);
    }
  for (int x=1; x<BRIGHT1; x++)
    {
      matrix[newstar] = CHSV(col,255,x); // fade in
          FastLED.show();
     }
  Stars[i]=newstar;   
}
/*
 *    --------------------BURST --------------------
 *  effect:3 parts. starts from the middle of each and expand both ways to the end
 *  if three is true use differnt HUE
 */
void Burst(int i, bool dir, bool three) 
{
  FastLED.clear();
//  if (DEBUG) {Serial.print(i);Serial.print(" - ");}
  for( int x = MID_LEDS1 - i; x < MID_LEDS1 + i;x++)
  {matrix[x] = CHSV(HUE,225,255);
//  if (DEBUG) {Serial.println(x);}
  }
  for( int x = MID_LEDS2 - i; x < MID_LEDS2 + i;x++)
  {
    if (three)
    {matrix[x] = CHSV(HUE2,225,255);}
    else
    {matrix[x] = CHSV(HUE,225,255);}
  }
  for (int x = MID_LEDS3-i; x<MID_LEDS3+i; x++)
  {
    if (three)
      {matrix[x] = CHSV(HUE3,225,255);}
    else
      {matrix[x] = CHSV(HUE,225,255);}     
  }
  if (three)
  {
          AnalogRGB(1,CHSV(HUE,255,255),BRIGHT2);
          AnalogRGB(2,CHSV(HUE2,255,255),BRIGHT2);
          AnalogRGB(3,CHSV(HUE3,255,255),BRIGHT2);
  }
  else
  {
     AnalogRGB(4,CHSV(HUE,255,255),BRIGHT2);
  }
  FastLED.show();
  FastLED.delay(100+SPEED);
}
/*
 * ----------------- FIXED COLORS ----------------------
 *    Both smart and anaolog RGB
 *     either all the same or 3 independants
 *     if two then only the fisrt two series are on
 */
void FixedColor(bool three, bool two)
{
  FastLED.clear();
  if(three)
  {
    for ( int x = 0; x < END_LEDS1; x++)
    {matrix[x] = CHSV(HUE,255,BRIGHT1);}
        for ( int x = END_LEDS1; x < END_LEDS2; x++)
    {matrix[x] = CHSV(HUE2,255,BRIGHT1);}
        for ( int x = END_LEDS2; x < NUM_LEDS; x++)
    {matrix[x] = CHSV(HUE3,255,BRIGHT1);}
    AnalogRGB(1,CHSV(HUE,255,255),BRIGHT2);
    AnalogRGB(2,CHSV(HUE2,255,255),BRIGHT2);
    AnalogRGB(3,CHSV(HUE3,255,255),BRIGHT2);
  }
  else
    {
    if (two)
    {
      for ( int x = 0; x < END_LEDS1; x++)
      {matrix[x] = CHSV(HUE,255,BRIGHT1);}
      for ( int x = END_LEDS1; x < END_LEDS2; x++)
      {matrix[x] = CHSV(HUE2,255,BRIGHT1);}
      AnalogRGB(1,CHSV(HUE,255,255),BRIGHT2);
      AnalogRGB(2,CHSV(HUE2,255,255),BRIGHT2);
      AnalogRGB(3,CHSV(HUE3,255,255),0);
    }
    else
    {
    for ( int x = 0; x < NUM_LEDS; x++)
    {matrix[x] = CHSV(HUE,255,BRIGHT1);}
    AnalogRGB(4,CHSV(HUE,255,255),BRIGHT2);
    }
    }
  FastLED.show();
}
/*
 *      - STATUS LED PULSE
 * 
 */
void Pulse(int i)
{
  for (int x=0; x<i; x++)
  {
    digitalWrite(BUILT_IN,HIGH);
    delay(100);
    digitalWrite(BUILT_IN,LOW);
    delay(100);
  }
}
