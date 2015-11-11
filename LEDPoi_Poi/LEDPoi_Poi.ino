#include <SPI.h>
#include <RH_NRF24.h>

#include <Colors.h>
#include <RGBLED.h>
#include <RGBArray.h>
#include <WS2812.h>

#include <LEDPoi.h>

// Initialize RGBArrays
const byte NUM_LEDS = 8;
const byte NUM_COLORS = 16;
const byte NUM_STRIPS = 4;
Colors::RGB colors[NUM_COLORS];
WS2812 ledStrips[] = {
  WS2812(NUM_LEDS),
  WS2812(NUM_LEDS),
  WS2812(NUM_LEDS),
  WS2812(NUM_LEDS)
};
RGBArray rgbArrays[] = {
  RGBArray(NUM_LEDS, NULL, NULL),
  RGBArray(NUM_LEDS, NULL, NULL),
  RGBArray(NUM_LEDS, NULL, NULL),
  RGBArray(NUM_LEDS, NULL, NULL)
};

// Initialize wireless radio
RH_NRF24 nrf24(8, 7);
const byte DATA_SIZE = 10;
byte dataIn[DATA_SIZE];

// Define variables used in script
Colors::RGB color;

const float MAX_BRIGHTNESS = 0.1;
const char COLOR_THRESHOLD = 40;
const bool COLOR_SCALING_REVERSE = true;

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
  // Initialize LED strips
  for (int i = 0; i < NUM_STRIPS; i++) {
    ledStrips[i].setColorOrderGRB();
    ledStrips[i].setOutput(i + 2);
  }

  nrf24.init();
  nrf24.setChannel(1);
  nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm);

  updateLEDs();
}

void loop() {
  // Update strips
  for (int i = 0; i < NUM_STRIPS; i++) {
    rgbArrays[i].update();
    ledStrips[i].sync();
  }

  if (nrf24.available()) {
    updateLEDs();
  }

  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      ledStrips[i].set_crgb_at(j, RGBtoCRGB(rgbArrays[i].getLED(j)->getColor()));
    }
  }
}

void updateLEDs() {
  uint8_t len = DATA_SIZE;
  if (nrf24.recv(dataIn, &len)) {
    // Prep any actions
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

    // Apply actions
    for (int i = 0; i < NUM_STRIPS; i++) {
      switch ((LEDPoi::Actions)dataIn[0]) {
        case LEDPoi::CHANGE_COLOR:
          rgbArrays[i].setColor(dataIn[1], &color);
          break;
        case LEDPoi::GENERATE_COLOR_ARRAY:
          rgbArrays[i].setColors(colors, dataIn[1]);
          break;
        case LEDPoi::SET_INTERVAL:
          rgbArrays[i].setInterval(dataIn[1]);
          break;
        case LEDPoi::SET_MODE:
          rgbArrays[i].setMode((RGBArray::Modes)dataIn[1], dataIn[2]);
      }
    }
  }
}

