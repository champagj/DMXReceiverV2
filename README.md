# DMXReceiverV2
Upgrade hardware version of DMX v1 with support for 2811, fixed RGB leds, relays control with ESP32 and OTA.
This is essentially a custom RGB led fixture controlled by external DMX controller using 8 DMX channels

New hardware version upgrade from DMX v1.

DMX Input using MAX485 module in receive mode to Rx input using DMXSerial library and examples
Output drivers to
1 x 2811 smart leds using FastLED library and examples
3 x RGB output (9 PWM) using MOSFET FQP30N06L
2 x 12v relay outputs (controls fixed AC lights, laser) using NPN transitors


LED STRIP = 7.5M long for 75 LEDS 
Segment 1.. 250cm = 25 leds,   is above fixed RGB1 using PWM output1
Segment 2.. 250cm = 25 leds    is above fixed RGB2 using PWM output2
Segment 3...250cm = 25 leds    is above fixed RGB3 using PWM output3

DMX Channel Mapping - 8Channels
DMX receiver 8 channels ( future to have two modes with extended 16 channels..)    
      
      
      Channel 1  Brightness for smart RGB strip
      Channel 2  Brightness for analog RGB leds
      Channel 3  Speed of animation (for effects with movement)
      Channel 4  Hue control 
      Channel 5  Hue2 - addtional control for section 2 (for different effect on each segment
      Channel 6  Hue3 - addtional control for section 3
      Channel 7  relay control  00, 01, 10, 11 (controls 2 relays)
      Channel 8   Show selection, 26 modes (round dmx value / 10) 0.0 to 25.5


Next steps..

Cleanup spaghetti code.. maybe split into .h and .cpp 
I found a great effect library for WS2812 from https://github.com/kitesurfer1404/WS2812FX I would like to incorporate.
I also found the great WLED project https://github.com/Aircoookie/WLED but it does not appear to support plain DMX input and trying to make it work with a combo smart and fixed leds maybe too mcuh















