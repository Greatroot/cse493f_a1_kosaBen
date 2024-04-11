/*
 * This example cross fades the colors of the RGB LED
 * 
 * By Ben Kosa
 * bkosa2@uw.edu
 * Code heavily inspired from
 *  https://makeabilitylab.github.io/physcomp
 *
 */

 #include "src/RGBConverter/RGBConverter.h"

// I use a common cathode RGB LED in my design
const boolean COMMON_ANODE = false;
// Because of my design, you need to press
// kind of hard for the button to press, so
// we need a longer debouncing window.
const int DEBOUNCE_WINDOW = 500; // 30ms delay to debounce our mode switch button

// Struct so we have a datatype to represent
// RGB values for any color in a clean way.
struct ColorRGB {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

// Traffic Light Timing
const int POT_INPUT_PIN = A3;
const int GREEN_DELAY = 5000;  // Duration of green light
const int YELLOW_DELAY = 2500;  // Yellow light wait time
const int RED_DELAY = 5000;  // Duration of red light
const int TOTAL_LOOP_TIME = GREEN_DELAY + (2 * YELLOW_DELAY) + RED_DELAY;

// PIN for mode switch digital button
const int MODE_SWITCH_PIN = 3;
int _mode;  // Program supports modes 1, 2, and 3

// HSL values for traffic light RED, YELLOW, and GREEN.
// GREEN: RGB (R: 45, G: 201, B: 55) or HSL (H: 124, S: 63.4, L: 48.2)
// YELLOW: RGB (R: 231, G: 180, B: 22) or HSL (H: 45, S: 82.6, L: 49.6)
// RED: RGB (R: 204, G: 50, B: 50) or HSL (H: 0, S: 60.6, L: 49.8)
const ColorRGB GREEN = {0, 255, 0};
const ColorRGB YELLOW = {122, 10, 0};
const ColorRGB RED = {255, 0, 0};

// After testing, I found that variable resistor typically
// ranges between 80V and 250V
const int LOFI_RES_PIN = A5;  // The pin that our lo-fi variable sensor is at
const int MIN_SHOW_GREEN = 80;
const int MAX_SHOW_GREEN = 132;
const int MIN_SHOW_YELLOW = 133;
const int MAX_SHOW_YELLOW = 189;
const int MIN_SHOW_RED = 190;
const int MAX_SHOW_RED = 246;

const int PHOTOCELL_INPUT_PIN = A0;
const int MIN_PHOTOCELL_VAL = 0;  // min darkness level, try different thresholds
const int MAX_PHOTOCELL_VAL = 800;  // max darkness level

const int RGB_RED_PIN = 11;
const int RGB_GREEN_PIN  = 9;
const int RGB_BLUE_PIN  = 10;
const int DELAY_MS = 10; // delay in ms between each time we increment the hue

float _hue = 0;  // hue varies between 0 and 1
float _step = 0.001f;

RGBConverter _rgbConverter;

void setup() {
  // Set the RGB pins to output
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);

  // Set the photo-sensitive resistor
  pinMode(PHOTOCELL_INPUT_PIN, INPUT);

  // Set the mode switch input pin
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);

  // Set pin 7 to HIGH 5v for powering the
  // pot for mode 3
  pinMode(7, HIGH);

  _mode = 1;  // Program supports modes 1, 2, and 3

  // Turn on Serial so we can verify expected colors via Serial Monitor
  Serial.begin(9600);
}

void loop() {

  // Check if the button has been pressed. If so,
  // then switch modes.
  // buttonState == 1 if not pressed
  // buttonState == 0 if pressed
  const int buttonState = digitalRead(MODE_SWITCH_PIN);
  if (buttonState == 0) {
    // If button is pressed, then increment to the next mode
    if (_mode == 3) {
      _mode = 1;
    } else {
      _mode++;
    }

    delay(DEBOUNCE_WINDOW);
  }

  Serial.print("mode: ");
  Serial.println(_mode);

  // MODE 1
  if (_mode == 1) {
    // Set the lightness based on the amount of light the photo-sensitive
    // resistor is currently exposed to.
    int photocellVal = analogRead(PHOTOCELL_INPUT_PIN);
    Serial.print("photocellVal: ");
    Serial.println(photocellVal);
    
    // Map the photo-sensitive resistor output to the lightness range.
    // IMPORTANT, we dont' want brightness to ever be 1 because then 
    // the light just becomes white, so let's have it range between
    // 0.05 and 0.9
    // In arduino, there's a map() function takes the following args:
    //    map(): (old_in, in_min, in_max, out_min, out_max)
    float lightness = map(photocellVal, MIN_PHOTOCELL_VAL, MAX_PHOTOCELL_VAL, 5, 70);
    // Serial.print("lightness before constraint: ");
    // Serial.println(lightness);
    lightness = constrain(lightness, 5, 70) / 100;  // Clamp our converted lightness val between 0 and 1
    // Serial.print("lightness after constraint: ");
    // Serial.println(lightness);

    // Serial.print("lightness: ");
    // Serial.print(lightness);

    byte rgb[3];
    // Args for hslToRGB: (hue, saturation, lightness, variable to store hsl converted to rgb)
    _rgbConverter.hslToRgb(_hue, 1, lightness, rgb);

    Serial.print("hue=");
    Serial.print(_hue);
    Serial.print(" r=");
    Serial.print(rgb[0]);
    Serial.print(" g=");
    Serial.print(rgb[1]);
    Serial.print(" b=");
    Serial.println(rgb[2]);

    setColor(rgb[0], rgb[1], rgb[2]);

    // increment the hue by our step size
    _hue += _step;

    // hue ranges between 0 and 1, so if hue is over 1, then reset to 0
    // to keep clamped between 0 and 1.
    if (_hue > 1.0) {
      _hue = 0;
    }

    delay(DELAY_MS);
  }

  // MODE 2
  if (_mode == 2) {
    // Depending on what range of voltage drop our
    // variable resistor is slid to (I'm using a lofi slider),
    // set our traffic RGB LED to either GREEN, YELLOW, or
    // RED. The range is dependent on how far away the car is from the light.
    // GREEN for if the car is far away (lowest voltage), YELLOW for if the
    // car is nearing the traffic light (mid-range voltage), and RED
    // for if the car is closest to the light (highest voltage)
    int potVal = analogRead(LOFI_RES_PIN);
    Serial.print("potVal before constraint: ");
    Serial.println(potVal);
    potVal = constrain(potVal, MIN_SHOW_GREEN, MAX_SHOW_RED);  // Clamp our potVal between our highest and lowest voltages
    Serial.print("potVal after constraint: ");
    Serial.println(potVal);
    if (potVal <= MAX_SHOW_GREEN) {
      // If in the GREEN range, set color to traffic light green
      setColor(GREEN.red, GREEN.green, GREEN.blue);
    } else if (potVal <= MAX_SHOW_YELLOW) {
      // If in the YELLOW range, set color to traffic light green
      setColor(YELLOW.red, YELLOW.green, YELLOW.blue);
    } else {
      // If in the RED range, set color to traffic light green
      setColor(RED.red, RED.green, RED.blue);
    }

    delay(DELAY_MS);

  }

  // MODE 3
  if (_mode == 3) {
    // int potVal = analogRead(POT_INPUT_PIN); // Returns num between 0 and 1023
    // Serial.print("potVal before constraint: ");
    // Serial.println(potVal);

    // store current time
    unsigned long currentMillis = millis();
    unsigned long millis_in_loop = currentMillis % TOTAL_LOOP_TIME;

    if (millis_in_loop < GREEN_DELAY) {
      // If we're in the first GREEN_DELAY ms of the traffic loop, then
      // the light should be green.
      setColor(GREEN.red, GREEN.green, GREEN.blue);
    } else if (millis_in_loop < (GREEN_DELAY + YELLOW_DELAY)) {
      // If we're within YELLOW_DELAY ms right after GREEN_DELAY in the current 
      // traffic loop, then the light should be yellow.
      setColor(YELLOW.red, YELLOW.green, YELLOW.blue);
    } else if (millis_in_loop < (GREEN_DELAY + YELLOW_DELAY + RED_DELAY)) {
      // If we're within RED_DELAY ms right after YELLOW_DELAY in the current 
      // traffic loop, then the light should be red.
      setColor(RED.red, RED.green, RED.blue);
    } else {
      setColor(YELLOW.red, YELLOW.green, YELLOW.blue);
    }

    // Silly way to do traffic light change using
    // delay
    // setColor(GREEN.red, GREEN.green, GREEN.blue);
    // delay(GREEN_DELAY);
    // setColor(YELLOW.red, YELLOW.green, YELLOW.blue);
    // delay(YELLOW_DELAY);
    // setColor(RED.red, RED.green, RED.blue);
    // delay(RED_DELAY);
    // setColor(YELLOW.red, YELLOW.green, YELLOW.blue);
    // delay(YELLOW_DELAY);
  }
}

/**
 * setColor takes in values between 0 - 255 for the amount of red, green, and blue, respectively
 * where 255 is the maximum amount of that color and 0 is none of that color. You can illuminate
 * all colors by intermixing different combinations of red, green, and blue
 *
 * Assumes that we're using a COMMON CATHODE RGB LED
 * 
 * This function is based on 
 * https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade.html
 */
void setColor(int red, int green, int blue)
{
  // Serial.print(red);
  // Serial.print(", ");
  // Serial.print(green);
  // Serial.print(", ");
  // Serial.println(blue);

  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);
}
