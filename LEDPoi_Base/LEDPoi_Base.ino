#include <SPI.h>
#include <RH_NRF24.h>

#include <ClickButton.h> */
#include <Colors.h>
#include <RGBLED.h>
#include <RGBArray.h>
#include <WS2812.h>

#include <LEDPoi.h>

// Initialize the radio
RH_NRF24 nrf24(8, 7);
bool toSend = false;
const byte DATA_SIZE = 10;
byte dataOut[DATA_SIZE];
byte dataIn[DATA_SIZE];

// Initialize RGBArray
const byte numLEDs = 9;
const byte NUM_COLORS = 16;
WS2812 ledStrip(numLEDs);
Colors::RGB colors[NUM_COLORS];
RGBArray rgbArray(numLEDs, NULL, NULL);

// Specify available modes
const byte NUM_MODES_AVAIL = 9;
RGBArray::Modes modesAvail[] = {
  RGBArray::SINGLE,
  RGBArray::BLINK,
  RGBArray::CYCLE,
  RGBArray::WAVE,
  RGBArray::WAVE_REVERSE,
  RGBArray::PONG,
  RGBArray::MERGE,
  RGBArray::MERGE_REVERSE,
  RGBArray::RANDOMINDEX
  //RGBArray::SPARKLE
};
char modeIndex = 0;

// Initialize buttons
const int nextButtonPin = 9;
ClickButton nextButton(nextButtonPin, LOW, CLICKBTN_PULLUP);
const int prevButtonPin = 10;
ClickButton prevButton(prevButtonPin, LOW, CLICKBTN_PULLUP);

// Initialize potentiometers
const byte ledPotPin = A0;
const byte intPotPin = A1;
byte oldLedPotVal, newLedPotVal, oldIntPotVal, newIntPotVal = 0;

// Set constants
const byte COLOR_MAX = 255;
const byte COLOR_DETECT_THRESHOLD = 10;
const char COLOR_THRESHOLD = 40;
const bool COLOR_SCALING_REVERSE = true;
const byte INTERVAL_MAX = 30;
const byte INTERVAL_MIN = 5;
const float MAX_BRIGHTNESS = 0.125;

/**
 * Converts a Colors::RGB struct to a cRGB
 */
cRGB RGBtoCRGB(Colors::RGB *rgbColor) {
  cRGB cRGBColor;
  cRGBColor.r = rgbColor->r * MAX_BRIGHTNESS;
  cRGBColor.g = rgbColor->g * MAX_BRIGHTNESS;
  cRGBColor.b = rgbColor->b * MAX_BRIGHTNESS;
  return cRGBColor;
}

void setup() {
  // Initialize wireless chip
  nrf24.init();
  nrf24.setChannel(1);
  nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm);

  // Initialize LED strip
  ledStrip.setColorOrderGRB();
  ledStrip.setOutput(5);
}

void loop() {
  dataOut[0] = NULL;  // Reset poi command
  nextButton.Update();
  prevButton.Update();
  rgbArray.update();
  ledStrip.sync();

  if (nextButton.clicks != 0) {
    modeIndex++;
    if (modeIndex >= NUM_MODES_AVAIL) {
      modeIndex = 0;
    }
    rgbArray.setMode(modesAvail[modeIndex]);

    // Prep data send
    toSend = true;
    dataOut[0] = LEDPoi::SET_MODE;
    dataOut[1] = modesAvail[modeIndex];
    dataOut[2] = NULL;
  }

  if (prevButton.clicks != 0) {
    modeIndex--;
    if (modeIndex < 0) {
      modeIndex = NUM_MODES_AVAIL - 1;
    }
    rgbArray.setMode(modesAvail[modeIndex]);
    
    // Prep data send
    dataOut[0] = LEDPoi::SET_MODE;
    dataOut[1] = modesAvail[modeIndex];
    dataOut[2] = NULL;
  }

  newLedPotVal = map(analogRead(ledPotPin), 0, 1024, 0, 255);
  if (abs(newLedPotVal - oldLedPotVal) > 10) {
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

  newIntPotVal = map(analogRead(intPotPin), 0, 1024, 5, 40);
  if (newIntPotVal != oldIntPotVal) {
    rgbArray.setInterval(newIntPotVal);

    oldIntPotVal = newIntPotVal;

    // Prep data send
    dataOut[0] = LEDPoi::SET_INTERVAL;
    dataOut[1] = newIntPotVal;
  }
  
  for (int i = 0; i < numLEDs; i++) {
    if (i == modeIndex) {
      ledStrip.set_crgb_at(i, RGBtoCRGB(rgbArray.getLED(i)->getColor()));
    }
    else {
      ledStrip.set_crgb_at(i, RGBtoCRGB(&Colors::BLACK));
    }
  }

  // If we've data to send, send it!
  if (dataOut[0])
    nrf24.send(dataOut, DATA_SIZE);
    //nrf24.waitPacketSent();
}
