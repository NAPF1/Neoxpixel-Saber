#include <FastLED.h> // Library for controlling LED strips with Arduino
#include <EEPROM.h> // For saving state of LEDs after turning off

#define BUTTON_GPIO 13 // Pin that button is attached to
#define LED_GPIO 12 // Pin that LEDs are attached to
#define NUM_LEDS 171 // Number of LEDs
#define LED_TYPE WS2812 // TYpe of LEDs
#define COLOR_ORDER GRB
#define BRIGHTNESS 100 // 100 okay for 5200mah battery with 5v converter

CRGB leds[NUM_LEDS]; // Array Leds for adding/changing colors
bool on = false; // Track whether saber is on or off
int currentColorIndex = 0; // Tracks current color saber is set to

int colors[5][3] = { // Colors to cycle through when double button press is activated
  {255, 0, 0}, // Red
  {50, 255, 0}, // Neon Green
  {0, 0, 255}, // Blue
  {120, 0, 180}, // Purple
  {255, 255, 255} // White 
};

// One-time setup code before looping
void setup() {
  FastLED.addLeds<LED_TYPE, LED_GPIO, COLOR_ORDER>(leds, NUM_LEDS); // Setup LEDs before looping through program
  FastLED.setBrightness(BRIGHTNESS); // Change the brightness to decrease power draw + better diffusion
  pinMode(BUTTON_GPIO, OUTPUT);
}

// Code that continually runs
void loop() {
  static int buttonState = HIGH;       // Current state of the button
  static int lastButtonState = HIGH;   // Previous state of the button
  static unsigned long lastDebounceTime = 0;  // Last time the button state changed
  static int clicks = 0;               // Number of button clicks
  static unsigned long lastClickTime = 0;  // Last time a click was detected

  int reading = digitalRead(BUTTON_GPIO);  // Read the button state

  if (reading != lastButtonState) { // Debounce the button state
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > 50) { // Time is greater than debounce
    if (reading != buttonState) { 
      buttonState = reading; // Read as an active press
      if (buttonState == LOW) { // Button was pressed
        clicks++; // Increment clicks
        lastClickTime = millis(); // Save time last click occured
      } 
    }
  }

  if ((millis() - lastClickTime) > 500) { // Time limit for detecting double click has passed
    if (clicks == 1) { // Single click
      if (on) { // Saber already on
        TurnSaberOff(); // Turn saber off!
      } else { // Otherwise turn saber on
        TurnSaberOn();
      }
    } 
    else if (clicks == 2) { // Double click
      ChangeColor(); // Change to a different color
    }
    clicks = 0; // Reset the click count
  }

  lastButtonState = reading; // Save the current button state for comparison
}

// Function for turning saber on
void TurnSaberOn() {
  currentColorIndex = EEPROM.read(0); // Read the current color index from EEPROM

  for (int i = 0; i <= NUM_LEDS / 2; i++) { // For loop cycling through LEDs starting from ends and going to the middle
    for (int j = 0; j <= i; j++) {
      leds[i] = CRGB(colors[currentColorIndex][0], colors[currentColorIndex][1], colors[currentColorIndex][2]);
      leds[NUM_LEDS - 1 - i] = CRGB(colors[currentColorIndex][0], colors[currentColorIndex][1], colors[currentColorIndex][2]);  
    }
    FastLED.show();
  }
  on = true;
}

// Function for turning saber off
void TurnSaberOff() {
  for (int i = 0; i <= NUM_LEDS / 2; i++) { // For loop cycling through LEDs starting from ends and going to the middle 
    for (int j = 0; j <= i; j++) {
      leds[NUM_LEDS/2 - j] = CRGB(0,0,0);
      leds[NUM_LEDS/2 + j] = CRGB(0,0,0);
    }
    FastLED.show();
  }
  on = false;
}

// Function for changing color of saber
void ChangeColor() {
  currentColorIndex = (currentColorIndex + 1) % 5; // Cycle through colors and loop back once end is reached
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(colors[currentColorIndex][0], colors[currentColorIndex][1], colors[currentColorIndex][2]);
  }
  FastLED.show();
  EEPROM.write(0, currentColorIndex); // Save the current color index to EEPROM
}
// End of program