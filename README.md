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

