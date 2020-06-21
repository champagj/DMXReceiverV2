# DMXReceiverV2
Upgrade hardware version of DMX v1 with support for 2811, fixed RGB leds, relays control with ESP32 and OTA

New hardware version upgrade from DMX v1.

DMX receiver using ESP32 with OTA support
Input using MAX485 module in receive mode 
to Rx input using DMXSerial library and examples
Output drivers to
1 x 2811 smart leds using FastLED library and examples
3 x RGB output (9 PWM) using MOSFET FQP30N06L
3 x 12v relay outputs (controls fixed AC lights, laser) using NPN transitors

DMX Channel Mapping - 24Channels
BANK 1 is for controlling 3 groups of dumb RGB lights (controlling 9 PWM outputs)
1- Master Brightness
2- Mode selection (fixed one color, fixed 3 colors, animations etc
3- 
4-
5-
6- Hue group 1
7- Hue group 2
8- Hue group 3
BANK2 is sort of an extension for the other banks, addtionsl options.. TBD
9-
10-
11-
12-
13-
14-
15-
16-
BANK3 is for controlling the smart LEDs 2811
17- Master Brightness
18- Mode selection (FastLED palettes options, fixed three sections of leds)
19- Speed of animation
20-
21-
22- Hue1 section 1
23-  Hue2  section 2
24-  Hue3  section 3













