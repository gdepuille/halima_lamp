#include <Arduino.h>
#include <FastLED.h>
#include <OneButton.h>

//#define DEBUG

// Input pin for the button
// Click           : Change manually the pattern / color
// Double Click    : Change patterns family
// Triple Click    : Enable / Disable automatic change
// Quadruple Click : On / Off
// Long press      : Change the brightness
#define BTN 2

#define NUM_LEDS 32

#define FRAMES_PER_SECOND 120

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

/* Color patterns */ 
void allLedsColor(CRGB color);

void white();
void blue();
void red();
void green();
void yellow();
void purple();

/* Animation patterns */
void rainbow();
void rainbowWithGlitter();
void onlyGlitter();
void confetti();
void sinelon();
void juggle();
void bpm();

void addGlitter(fract8 v);

/* Button actions */
void toggleLights();
void nextPattern();
void multiClick();
void nextPatternFamily();
void toggleAuto();
void startChangeBrightness();
void stopChangeBrightness();
void modifyBrightness();

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList animationPatterns = { rainbow, rainbowWithGlitter, onlyGlitter, confetti, sinelon, juggle, bpm };
CRGB colorPatterns[] = { 
  CRGB::White, CRGB::Aqua, CRGB::Blue, CRGB::Red, CRGB::Orange, CRGB::Green,   
  CRGB::Yellow, CRGB::Purple 
};

uint8_t currentFamilyNumber = 0; // Index of the current pattern family
uint8_t currentPatternNumber = 0; // Index number of which pattern is current inside the current family of patterns
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns

bool modeChanged = false;
bool automaticChange = false;
bool changeBrightness = false;
bool incBrightness = false;
bool enabled = true;
uint8_t brightness = 250;
CRGB leds[NUM_LEDS];

OneButton button = OneButton(BTN, true, true); // Low level + internal pullup

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Setup");

  Serial.println("Config LED BUILTIN");
#endif
  pinMode(LED_BUILTIN, OUTPUT);

#ifdef DEBUG
  Serial.println("Configuration bouton");
#endif
  button.attachClick(nextPattern);
  button.attachDoubleClick(nextPatternFamily);
  button.attachMultiClick(multiClick);
  button.attachLongPressStart(startChangeBrightness);
  button.attachLongPressStop(stopChangeBrightness);

#ifdef DEBUG
  Serial.println("Configuration bandeau LEDs");
#endif
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS);
  // set master brightness control
  FastLED.setBrightness(brightness);
}

void loop()
{
  button.tick();

  unsigned long delay = 1000 / FRAMES_PER_SECOND;
  if (enabled && modeChanged) {
    if (automaticChange) {
      allLedsColor(CRGB::Green);
    } else {
      allLedsColor(CRGB::Red);
    }
    modeChanged = false;
    delay = 5000;

  } else if (enabled && !modeChanged) {
    // Call the current pattern function once, updating the 'leds' array
    if (currentFamilyNumber == 0) {
      allLedsColor(colorPatterns[currentPatternNumber]);
    } else {
      animationPatterns[currentPatternNumber]();
    }

  } else if (!enabled) {
    fadeToBlackBy(leds, NUM_LEDS, 20);
  }

  FastLED.show(); // send the 'leds' array out to the actual LED strip
  FastLED.delay(delay); // insert a delay to keep the framerate modest

  // do some periodic updates
  EVERY_N_MILLISECONDS(20) {
    // slowly cycle the "base color" through the rainbow
    gHue++;
  }

  EVERY_N_MILLISECONDS(100) {
    // change the brightness
    if (changeBrightness) {
      modifyBrightness();
    }
  }

  EVERY_N_SECONDS(60) {
    // change patterns periodically if in automatic mode
    if (enabled && automaticChange) {
      nextPattern();
    }
  }
}

// -------------------------------------------------- //
void toggleLights() {
#ifdef DEBUG
  Serial.println("Toggle lights");
#endif

  enabled = !enabled;
}

void nextPattern() {
  if (!enabled) {
    enabled = true;
    return;
  }

#ifdef DEBUG
  Serial.println("Pattern change");
#endif

  // add one to the current pattern number, and wrap around at the end
  if (currentFamilyNumber == 0) {
    currentPatternNumber = (currentPatternNumber + 1) % ARRAY_SIZE(colorPatterns);
  } else {
    currentPatternNumber = (currentPatternNumber + 1) % ARRAY_SIZE(animationPatterns);
  }

#ifdef DEBUG
  Serial.print(" * Family number   : ");
  Serial.println(currentFamilyNumber);
  Serial.print(" * Pattern courant : ");
  Serial.println(currentPatternNumber);
#endif
}

void toggleAuto() {
  if (!enabled) {
    enabled = true;
    return;
  }

#ifdef DEBUG
  Serial.println("Pattern automatic change");
#endif
  automaticChange = !automaticChange;
  modeChanged = true;

#ifdef DEBUG
  Serial.print(" * Enchainement en mode auto : ");
  Serial.println(automaticChange);
#endif
}

void multiClick() {
  if (!enabled) {
    enabled = true;
    return;
  }

  int nbClick = button.getNumberClicks();
#ifdef DEBUG
  Serial.print("Pattern multiclick = ");
  Serial.println(nbClick);
#endif

  if (nbClick == 3) {
    toggleAuto();
  } else if (nbClick == 4) {
    toggleLights();
  } else {
#ifdef DEBUG
    Serial.println(" * Inconnu !!");
#endif
  }
}

void nextPatternFamily() {
#ifdef DEBUG
  Serial.println("Next pattern family");
#endif
  currentFamilyNumber = (currentFamilyNumber + 1) % 2;
  currentPatternNumber = 0;

#ifdef DEBUG
  Serial.print(" * Family number   : ");
  Serial.println(currentFamilyNumber);
  Serial.print(" * Pattern courant : ");
  Serial.println(currentPatternNumber);
#endif  
}

void startChangeBrightness() {
  if (!enabled) {
    return;
  }

#ifdef DEBUG
  Serial.println("Longpress start -> Brightness change");
#endif

  changeBrightness = true;
}

void stopChangeBrightness() {
  if (!enabled) {
    return;
  }
#ifdef DEBUG
  Serial.println("Longpress stop -> Brightness change");
#endif

  changeBrightness = false;
  incBrightness = !incBrightness;
}

void modifyBrightness() {
  if (!enabled) {
    return;
  }

  if (incBrightness && brightness <= 250) {
    brightness += 5;
  } else if (!incBrightness && brightness >= 10) {
    brightness -= 5;
  }

  FastLED.setBrightness(brightness);

#ifdef DEBUG
  Serial.print(" * Brightness : ");
  Serial.println(brightness);
#endif
}

// -------------------------------------------------- //
void allLedsColor(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
}


// -------------------------------------------------- //

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void onlyGlitter() {
  // glitter, with fading, only
  fadeToBlackBy(leds, NUM_LEDS, 20);
  addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter)
{
  if (random8() < chanceOfGlitter)
  {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++)
  { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle()
{
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++)
  {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


