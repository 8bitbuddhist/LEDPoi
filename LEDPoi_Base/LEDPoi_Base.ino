// Libraries related to the NRF24L01+ chip.
#include <SPI.h>
#include <RH_NRF24.h>

// Library for interpreting button presses.
#include <ClickButton.h>

// Libraries related to controlling the LEDs.
#include <WS2812.h>
#include <Colors.h>
#include <RGBArray.h>

// Main control library.
#include <LEDPoi.h>

/*
 * Initialize the wireless radio.
 * 
 * CHANNEL:   Sets the channel to communicate on. This should match the channel set for your poi.
 * DATA_SIZE: Sets the number of bytes to send with the command.
 * toSend:    Determines whether we need to send a command on this loop.
 * dataOut:   Packet containing the actual update command.
 */
RH_NRF24 nrf24(8, 7);
const byte CHANNEL = 1;
const byte DATA_SIZE = 10;
bool toSend = false;
byte dataOut[DATA_SIZE];

/*
 * Initialize the LEDs.
 * 
 * LED_STRIP_PIN: Output pin for the led strip.
 * NUM_COLORS:  The max number of colors stored in each strip.
 * NUM_LEDS:    The max number of LEDs in each strip.
 * colors:      Temporary storage for colors shown in the array.
 * ledStrip:    Output for the WS2812 strip displayed on the base unit.
 * rgbArrays:   Handles color changes, animations, etc. for ledStrip.
 */
const byte LED_STRIP_PIN = 5;
const byte NUM_COLORS = 16;
const byte NUM_LEDS = 10;
Colors::RGB colors[NUM_COLORS];
WS2812 ledStrip = WS2812(NUM_LEDS);
RGBArray rgbArray = RGBArray(NUM_LEDS, NULL, NULL);

/*
 * Define the available display modes.
 * TODO: Add support for PATTERN mode.
 * 
 * NUM_MODES:       The number of display modes available.
 * modeIndex:       The current selected mode.
 */
const byte NUM_MODES = 9;
RGBArray::Modes modesAvail[] = {
  RGBArray::SINGLE,
  RGBArray::BLINK,
  RGBArray::CYCLE,
  RGBArray::WAVE,
  RGBArray::WAVE_REVERSE,
  RGBArray::PONG,
  RGBArray::MERGE,
  RGBArray::MERGE_REVERSE,
  RGBArray::SPARKLE
  //RGBArray::PATTERN
};
char modeIndex = 0;

/*
 * Define the available patterns.
 * 
 * NUM_PATTERNS:    The number of available patterns.
 * patternIndex:    The current selected pattern.
 */
/*const byte NUM_PATTERNS = 5;
byte patterns[][8] = {
  {0x82, 0x41, 0x82, 0x41, 0x82, 0x41, 0x82, 0x41}, // Checkered
  {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55}, // Horizontal stripes
  {0x99, 0x24, 0x42, 0x99, 0x99, 0x42, 0x24, 0x99}, // Round pattern
  {0x42, 0xC3, 0x24, 0x18, 0x18, 0x24, 0xC3, 0x42}, // X pattern
  {0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}  // Vertical stripes
};
char patternIndex = 0;*/

/*
 * Initialize the pushbuttons.
 * 
 * nextButton:    Button listener for changing to the next mode.
 * prevButton:    Button listener for changing to the previous mode.
 */
const int nextButtonPin = 9;
ClickButton nextButton(nextButtonPin, LOW, CLICKBTN_PULLUP);
const int prevButtonPin = 10;
ClickButton prevButton(prevButtonPin, LOW, CLICKBTN_PULLUP);

/*
 * Initialize the potentiometers.
 * 
 * ledPotPin: Changes the color scheme for the current mode.
 * intPotPin: Changes the speed of the current mode. When in PATTERN mode, changes the current pattern.
 */
const byte ledPotPin = A0;
const byte intPotPin = A1;
byte oldLedPotVal, newLedPotVal, oldIntPotVal, newIntPotVal = 0;

/*
 * Set constants.
 * 
 * COLOR_DETECT_THRESHOLD:  Sets the minimum change in ledPotPin before a new color scheme is generated.
 * COLOR_THRESHOLD:         Sets the distance between colors when generating a new color scheme.
 * COLOR_SCALING_REVERSE:   Determines whether a newly generated color scheme fades back to the original color, or cuts off at the last color.
 * INTERVAL_MAX:            Determines the maximum delay that can be set by intPot.
 * INTERVAL_MINL            Determines the minimum delay that can be set by intPin.
 * MAX_BRIGHTNESS:          Sets the maximum brightness of the LEDs.
 */
const byte COLOR_DETECT_THRESHOLD = 10;
const char COLOR_THRESHOLD = 40;
const bool COLOR_SCALING_REVERSE = true;
const byte INTERVAL_MAX = 30;
const byte INTERVAL_MIN = 5;
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
}

void loop() {
  /*
   * Set dataOut to NULL to prevent the radio from sending any commands.
   * If any changes occur, dataOut will be updated, triggering the radio to send commands to the poi.
   */
  dataOut[0] = NULL;

  // Check for button presses.
  nextButton.Update();
  prevButton.Update();

  // Update the LED strips.
  rgbArray.update();
  ledStrip.sync();

  /* Check the buttons for changes.
   * If nextButton is pressed, switch to the next mode. 
   * If prevButton is pressed, switch to the previous mode.
   */
  if (nextButton.clicks == 1) {
    modeIndex++;
    if (modeIndex >= NUM_MODES) {
      modeIndex = 0;
    }
    rgbArray.setMode(modesAvail[modeIndex]);

    // Prep data send
    toSend = true;
    dataOut[0] = LEDPoi::SET_MODE;
    dataOut[1] = modesAvail[modeIndex];
    dataOut[2] = NULL;
  }

  if (prevButton.clicks == 1) {
    modeIndex--;
    if (modeIndex < 0) {
      modeIndex = NUM_MODES - 1;
    }
    rgbArray.setMode(modesAvail[modeIndex]);
    
    // Prep data send
    dataOut[0] = LEDPoi::SET_MODE;
    dataOut[1] = modesAvail[modeIndex];
    dataOut[2] = NULL;
  }

  // If we're in SPARKLE mode, disable fading. Otherwise, enable it.
  if (nextButton.clicks == 1 || prevButton.clicks == 1) {
    if (modesAvail[modeIndex] == RGBArray::SPARKLE || !rgbArray.getFade()) {
      rgbArray.toggleFade();
    }
  }

  // Check ledPotPin for changes. If the changes surpass our threshold, generate a new color scheme.
  newLedPotVal = map(analogRead(ledPotPin), 0, 1024, 0, 255);
  if (abs(newLedPotVal - oldLedPotVal) > COLOR_DETECT_THRESHOLD) {
    Colors::RGB currentColor = Colors::HSVtoRGB(newLedPotVal, 255, 255);      
    Colors::generateScalingColorArray(colors, currentColor, NUM_COLORS, COLOR_THRESHOLD, COLOR_SCALING_REVERSE);
    rgbArray.setColors(colors, NUM_COLORS);
    
    oldLedPotVal = newLedPotVal;

    // Prep data send
    dataOut[0] = LEDPoi::GENERATE_COLOR_ARRAY;
    dataOut[1] = NUM_COLORS;
    dataOut[2] = currentColor.r;
    dataOut[3] = currentColor.g;
    dataOut[4] = currentColor.b;
  }

  // Check intPotPin for changes.
  newIntPotVal = map(analogRead(intPotPin), 0, 1024, INTERVAL_MIN, INTERVAL_MAX);
  if (newIntPotVal != oldIntPotVal) {
    rgbArray.setInterval(newIntPotVal);

    oldIntPotVal = newIntPotVal;

    // Prep data send
    dataOut[0] = LEDPoi::SET_INTERVAL;
    dataOut[1] = newIntPotVal;
  }

  // Update the LED strip. Since we only want to light the LED corresponding to the current mode, we turn off all other LEDs.
  for (byte i = 0; i < NUM_LEDS; i++) {
    if (i == modeIndex) {
      ledStrip.set_crgb_at(i, RGBtoCRGB(rgbArray.getLED(i)->getColor()));
    }
    else {
      ledStrip.set_crgb_at(i, RGBtoCRGB(&Colors::BLACK));
    }
  }

  // Broadcast any updates.
  if (dataOut[0])
    nrf24.send(dataOut, DATA_SIZE);
}
