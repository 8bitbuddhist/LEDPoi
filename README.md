# LEDPoi
This is a set of Arduino sketches for remotely controlling a set of LED poi. The sketch works for any system using 5V Arduinos, NRF24L01+ wireless chips, and WS2812 RGB LEDs.

The LEDPoi_Base folder contains Arduino sketches for the transmitter. The LEDPoi_Poi folder contains Arduiono sketches for the poi. The STL folder contains blueprints for the poi body.

# Components
## Base
The base consists of an Arduino, an NRF24L01+ wireless chip, two potentiometers, two pushbuttons, one 10-LED WS2812 strip (more or less depending on the number of modes you want to have available), and a rechargable battery with USB output.

## Poi
Each poi consists of a custom 3D-printed frame, an Anker 2nd Generation Astro Mini battery, an Arduino Pro Mini, an NRF24L01+ wireless chip, an LD1117V33 voltage regulator (for stepping down the 5V battery output to the 3.3V required by the radio), and four 8-LED WS2812 strips. The WS2812 strips are controlled by a single output from the Arduino, although a more advanced setup could have four individually-controlled strips.

# Use
Download and extract the LEDPoi folder to your Arduino IDE's library folder. You will also need to download and extract the RGBArray, RadioHead, ClickButton, and light_WS2812 libraries using the following links:
https://github.com/Anewman2/RGBArray
http://www.airspayce.com/mikem/arduino/RadioHead/
https://code.google.com/p/clickbutton/
https://github.com/cpldcpu/light_ws2812/tree/master/light_ws2812_Arduino/light_WS2812

Upload the LEDPoi_Base.ino sketch to the Arduino controlling your base unit, and upload the LEDPoi_Poi.ino sketch to your poi units. After powering on the base and the poi, your poi will automatically start receiving commands.

Use the pushbuttons to select the previous or next display mode. The potentiometer on pin A0 changes the color scheme, while the potentiometer on pin A1 changes the speed of the animation.

# Configuration
By default, the base and poi sketches are configured to communicate over wireless channel 1. You might want to choose a different channel to reduce the chance of interference. You can change the default channel by changing the CHANNEL constant in both sketches.

The default brightness is set to 0.1 to prevent the LEDs from drawing too much current. You can change this by setting the MAX_BRIGHTNESS value to any value between 0.0 (off) and 1.0 (full brightness). Assuming each LED draws 60 mA at full brightness, 60 mA * 0.1 * 32 LEDs = 192 mA per poi, just for the LEDs.
