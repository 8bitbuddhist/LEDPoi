// Libraries related to the NRF24L01+ chip.
#include <SPI.h>
#include <RH_NRF24.h>

// Libraries related to controllign the LEDs.
#include <WS2812.h>
#include <Colors.h>
#include <RGBArray.h>

// Main control library.
#include <LEDPoi.h>

/*
 * Initialize the wireless radio.
 * 
 * CHANNEL:   Sets the channel to listen on. This should match the channel set for your base unit.
 * DATA_SIZE: Sets the number of bytes to receive with each command.
 * dataIn:    Packet containing the actual update command.
 */
RH_NRF24 nrf24(8, 7);
const byte CHANNEL = 1;
const byte DATA_SIZE = 10;
byte dataIn[DATA_SIZE];

/* Initialize the LEDs.
 * Since each strip shows the same animations, we'll only use one array and one output.
 * 
 * LED_STRIP_PIN: Output pin for the led strip.
 * NUM_COLORS:    The max number of colors stored in each strip.
 * NUM_LEDs:      The max number of LEDs in each strip.
 * color:         Temporary storage for color transmitted by the base unit.
 * colors:        Temporary storage for colors shown in the array.
 * ledStrip:      Output for the WS2812 strip displayed on the poi.
 * rgbArrays:     Handles color changes, animations, etc. for ledStrip.
 */
const byte LED_STRIP_PIN = 5;
const byte NUM_COLORS = 16;
const byte NUM_LEDS = 8;
Colors::RGB color;
Colors::RGB colors[NUM_COLORS];
WS2812 ledStrip = WS2812(NUM_LEDS);
RGBArray rgbArray = RGBArray(NUM_LEDS, NULL, NULL);

/*
 * Set constants.
 * 
 * COLOR_SCALING_REVERSE:   Determines whether a newly generated color scheme fades back to the original color, or cuts off at the last color.
 * COLOR_THRESHOLD:         Sets the distance between colors when generating a new color scheme.
 * MAX_BRIGHTNESS:          Sets the maximum brightness of the LEDs.
 */
const bool COLOR_SCALING_REVERSE = true;
const char COLOR_THRESHOLD = 40;
const float MAX_BRIGHTNESS = 0.1;

/*
 * Converts a Colors::RGB struct to a cRGB. Automatically adjusts based on MAX_BRIGHTNESS.
 * 
 * @param *rgbColor Colors::RGB to convert.
 * @return cRGB representation of rgbColor.
 */
cRGB RGBtoCRGB(Colors::RGB *rgbColor) {
  cRGB cRGBColor;
  cRGBColor.r = rgbColor->r * MAX_BRIGHTNESS;
  cRGBColor.g = rgbColor->g * MAX_BRIGHTNESS;
  cRGBColor.b = rgbColor->b * MAX_BRIGHTNESS;
  return cRGBColor;
}

void setup() {
  // Initialize the radio.
  nrf24.init();
  nrf24.setChannel(CHANNEL);
  nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm);
  
  /*
   * Initialize the LED strips.
   * Because of the way RGBArray stores colors, we need to output the final color as GRB instead of RGB.
   * Make sure you set the correct output pin.
   */
  ledStrip.setColorOrderGRB();
  ledStrip.setOutput(LED_STRIP_PIN);

  // Prime the LEDs by resetting them to their defualt display mode.
  updateLEDs();
}

void loop() {
  // Update the LED strips.
  rgbArray.update();
  ledStrip.sync();

  // If the radio is available, check for any commands and update rgbArray.
  if (nrf24.available()) {
    updateLEDs();
  }

  // Update the LED strip.
  for (int i = 0; i < NUM_LEDS; i++) {
    ledStrip.set_crgb_at(i, RGBtoCRGB(rgbArray.getLED(i)->getColor()));
  }
}

void updateLEDs() {
  // Set the size of our packet.
  uint8_t len = DATA_SIZE;

  // Check for incoming data.
  if (nrf24.recv(dataIn, &len)) {
    /*
     * If we have incoming data, we need to translate the raw bytes into actual commands.
     * We start by prepping any commands that require additional work, like generating a color array.
     * Then, we update rgbArray based on the initial command.
     */
    // Prep any commands that require additional work.
    switch ((LEDPoi::Actions)dataIn[0]) {
      case LEDPoi::CHANGE_COLOR:
        color = {dataIn[2], dataIn[3], dataIn[4]};
        break;
      case LEDPoi::GENERATE_COLOR_ARRAY:
      {
        Colors::RGB baseColor = {dataIn[2], dataIn[3], dataIn[4]};
        Colors::generateScalingColorArray(colors, baseColor, NUM_COLORS, COLOR_THRESHOLD, COLOR_SCALING_REVERSE);
        break;
      }
    }

    // Apply the actions.
    switch ((LEDPoi::Actions)dataIn[0]) {
      case LEDPoi::CHANGE_COLOR:
        rgbArray.setColor(dataIn[1], &color);
        break;
      case LEDPoi::GENERATE_COLOR_ARRAY:
        rgbArray.setColors(colors, dataIn[1]);
        break;
      case LEDPoi::SET_INTERVAL:
        rgbArray.setInterval(dataIn[1]);
        break;
      case LEDPoi::SET_MODE:
        rgbArray.setMode((RGBArray::Modes)dataIn[1], dataIn[2]);
    }
  }
}

