//Audio Library
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

//Keypad
#include "Adafruit_Trellis.h"

//Sliders
#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>
#define  DEFAULT_I2C_ADDR 0x30
#define  ANALOGIN   18
#define  NEOPIXELOUT 14

//Screen
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



//Keypad
Adafruit_Trellis matrix0 = Adafruit_Trellis();


//Sliders
Adafruit_seesaw seesaw1;
Adafruit_seesaw seesaw2;
seesaw_NeoPixel pixels1 = seesaw_NeoPixel(4, NEOPIXELOUT, NEO_GRB + NEO_KHZ800);
seesaw_NeoPixel pixels2 = seesaw_NeoPixel(4, NEOPIXELOUT, NEO_GRB + NEO_KHZ800);


// GUItool: begin automatically generated code
AudioSynthWaveform       waveform;       //xy=93,160
AudioSynthWaveformSine   modulator_sin;  //xy=132,532
AudioSynthWaveformModulated waveform_fm;    //xy=465,519
AudioEffectMultiply      multiply_am;    //xy=655,422
AudioMixer4              fm_am_mixer;    //xy=823,464
AudioMixer4              apply_modulation; //xy=1025,405
AudioEffectEnvelope      envelope;       //xy=1199,445
AudioAmplifier           vol;           //xy=1358.6667556762695,457.6666965484619
AudioOutputI2S           i2s;           //xy=1579.6666564941406,458.66666412353516
AudioConnection          patchCord1(waveform, 0, multiply_am, 0);
AudioConnection          patchCord2(waveform, 0, apply_modulation, 0);
AudioConnection          patchCord3(modulator_sin, 0, multiply_am, 1);
AudioConnection          patchCord4(modulator_sin, 0, waveform_fm, 0);
AudioConnection          patchCord5(waveform_fm, 0, fm_am_mixer, 1);
AudioConnection          patchCord6(multiply_am, 0, fm_am_mixer, 0);
AudioConnection          patchCord7(fm_am_mixer, 0, apply_modulation, 1);
AudioConnection          patchCord8(apply_modulation, envelope);
AudioConnection          patchCord9(envelope, vol);
AudioConnection          patchCord10(vol, 0, i2s, 0);
AudioConnection          patchCord11(vol, 0, i2s, 1);
// GUItool: end automatically generated code

// codec
AudioControlSGTL5000 sgtl5000_1;



//Keypad
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0);





float notes[] = {220.00, 246.94, 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25, 587.33, 659.26, 698.46, 783.99, 880.00, 987.77};

int numButtonsPressed = 0;

float noteFreq = 0;

bool amfm = false; // true is fm, false is am

int current_waveform = 0;
void setup() {
  AudioMemory(120);
  Serial.begin(9600);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.25);

  // Start screen
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  startAnimation();

  // Start keypad
  trellis.begin(0x70);

  // Start sliders
  if (!seesaw1.begin(DEFAULT_I2C_ADDR) || !seesaw2.begin(DEFAULT_I2C_ADDR + 1)) {
    Serial.println(F("seesaws not found!"));
    while (1) delay(10);
  }

  if (!pixels1.begin(DEFAULT_I2C_ADDR) || !pixels2.begin(DEFAULT_I2C_ADDR + 1)) {
    Serial.println("seesaw pixels not found!");
    while (1) delay(10);
  }

  // Set up slider lights
  pixels1.setBrightness(255);  // half bright
  pixels2.setBrightness(255);  // half bright
  pixels1.show(); // Initialize all pixels to 'off'
  pixels2.show(); // Initialize all pixels to 'off'

  // Start animation for keypad
  for (uint8_t i = 0; i < 16; i++) {
    trellis.setLED(i);
    trellis.writeDisplay();
    delay(30);
    for (uint8_t j = 0; j < pixels1.numPixels(); j++) {
      pixels1.setPixelColor(j, Wheel(i*8));
      pixels2.setPixelColor(j, Wheel(i*8));
    }
    pixels1.show();
    pixels2.show();
  }
  // then turn them off
  for (uint8_t i = 0; i < 16; i++) {
    trellis.clrLED(i);
    trellis.writeDisplay();
    delay(30);
    for (uint8_t j = 0; j < pixels1.numPixels(); j++) {
      pixels1.setPixelColor(j, Wheel(i*8 + 128));
      pixels2.setPixelColor(j, Wheel(i*8 + 128));
    }
    pixels1.show();
    pixels2.show();
    
  }

  // Set up starting sounds
  envelope.attack(9.2);
  envelope.hold(2.1);
  envelope.decay(31.4);
  envelope.sustain(0.8);
  envelope.release(84.5);

  waveform.amplitude(1);
  waveform_fm.amplitude(1);
  modulator_sin.amplitude(1);


  apply_modulation.gain(0, 1);
  apply_modulation.gain(1, 0);

  switchAmFm(); 

  displayWaveform();

}

void loop() {
  delay(30); // 30ms delay is required, dont remove me!

  // If a button was just pressed or released...
  if (trellis.readSwitches()) {
    // go through every button
    for (uint8_t i = 0; i < 16; i++) {
      // if it was pressed, turn it on
      if (trellis.justPressed(i)) {
        AudioNoInterrupts();
        setFreq(notes[i]);
        envelope.noteOn();
        AudioInterrupts();
        trellis.setLED(i);
        numButtonsPressed++;
      }
      // if it was released, turn it off
      if (trellis.justReleased(i)) {
        numButtonsPressed--;
        if (numButtonsPressed == 0) {
          AudioNoInterrupts();
          envelope.noteOff();
          AudioInterrupts();
        }

        trellis.clrLED(i);
      }
    }

    // tell the trellis to set the LEDs we requested
    trellis.writeDisplay();
  }


  // read the sliders
  uint16_t slide1_val = seesaw1.analogRead(ANALOGIN); // Attack
  uint16_t slide2_val = seesaw2.analogRead(ANALOGIN); // Release

  // read the potentiometers
  uint16_t pot12_val_raw = analogRead(A12);
  float pot12_val =  (float)pot12_val_raw / 1023.0; // mod mix
//  Serial.print(pot12_val_raw);
//  Serial.print("     ");
//  
//  uint16_t pot11_val_raw = analogRead(A11);
//  float pot11_val =  (float)pot11_val_raw; // unused
//  Serial.print(pot11_val_raw);
//  Serial.print("     ");
//  
//  uint16_t pot10_val_raw = analogRead(A10);
//  float pot10_val =  (float)pot10_val_raw; // unused
//  Serial.print(pot10_val_raw);
//  Serial.print("     ");
//
//  uint16_t pot9_val_raw = analogRead(A9); // pitch mod (TODO)
//  float pot9_val =  (float)pot9_val_raw;
//  Serial.print(pot9_val_raw);
//  Serial.print("     ");
//
  uint16_t pot8_val_raw = analogRead(A8);
  float pot8_val = (float)pot8_val_raw; // waveform
//  Serial.print(pot8_val_raw);
//  Serial.print("     ");

  uint16_t pot3_val_raw = analogRead(A3);
  float pot3_val =  (float)pot3_val_raw; // mod freq
//  Serial.print(analogRead(A3));
//  Serial.print("     ");

  uint16_t pot2_val_raw = analogRead(A2);
  float pot2_val = (float)pot2_val_raw;  // vol
//  Serial.print(analogRead(A2));
//  Serial.print("     ");

  uint16_t pot1_val_raw = analogRead(A1);
  float pot1_val = (float)pot1_val_raw; // mod type
//  Serial.print(analogRead(A1));
//  Serial.print("     ");

  uint16_t pot0_val_raw = analogRead(A0);
  float pot0_val = (float)pot0_val_raw; // mod amp
//  Serial.println(analogRead(A0));

  //sets waveform type
  setWaveform(pot8_val);
  AudioNoInterrupts();

  // change attack and release
  envelope.attack(map(slide1_val, 0, 1023, 2000, 0));
  envelope.release(map(slide2_val, 0, 1023, 2000, 0));

  //sets mod amp
  modulator_sin.amplitude(amfm ? pot0_val / 2047.0 : 1);

  //mix between modulated and normal signal
  apply_modulation.gain(0, pot12_val);
  if ((1 - pot12_val) < .1) {
    apply_modulation.gain(1, 0);
  } else {
    apply_modulation.gain(1, amfm ? (1 - pot12_val) * 2.0 : 1 - pot12_val);
  }

  //sets mod freq
  modulator_sin.frequency(map(pot3_val_raw, 0, 1023, 1, 35));

  //volume
  vol.gain(pot2_val / 1023.0);

  //pitch mod (TODO)
  // if (pot9_val_raw > 711) {
  //   float modifiedPitch = (((float)pot9_val_raw - 711.0) / 1023.0) * 0.16 * noteFreq + noteFreq;
  //   setFreq(modifiedPitch);
  // } else if (pot9_val_raw < 311) {
  //   float modifiedPitch = noteFreq - (((float)pot9_val_raw) / 1023.0) * 0.16 * noteFreq;
  //   setFreq(modifiedPitch);
  // }
    // Serial.println(pot9_val_raw);

  //switches am fm
  if (pot1_val_raw > 520) {
    if (amfm) {
      amfm = false;
      switchAmFm();
    }
  } else if (pot1_val_raw < 500) {
    if (!amfm) {
      amfm = true;
      switchAmFm();
    }
  }

  AudioInterrupts();


  // sets slider colors to reflect their position
  for (uint8_t i = 0; i < pixels1.numPixels(); i++) {
    pixels1.setPixelColor(i, Wheel(slide1_val / 4));
    pixels2.setPixelColor(i, Wheel(slide2_val / 4));
  }
  pixels1.show();
  pixels2.show();

}

// Sets the frequency of all instruments
void setFreq(float freq) {
  waveform_fm.frequency(freq);
  waveform.frequency(freq);
}

// takes the sample from the pot controlling the waveform and switches between them
// 100 points of deadspace between the settings
void setWaveform(float sample) {
  float numWaveforms = 5.0;
  float division = 1023.0 / numWaveforms;
  int new_waveform = 0;
  if (sample < division - 50) {
    new_waveform = WAVEFORM_SINE;
  } else if (sample < division * 2 - 50 && sample > division + 50) {
    new_waveform = WAVEFORM_SAWTOOTH;
  } else if (sample < division * 3 - 50 && sample > division * 2 + 50) {
    new_waveform = WAVEFORM_TRIANGLE;
  } else if (sample < division * 4 - 50 && sample > division * 3 + 50) {
    new_waveform = WAVEFORM_SQUARE;
  } else if (sample > division * 4 + 50) {
    new_waveform = WAVEFORM_PULSE;
  } else new_waveform = current_waveform;
  if (new_waveform != current_waveform) {
    current_waveform = new_waveform;
    displayWaveform();
    AudioNoInterrupts();
    envelope.noteOff();
    waveform.begin(current_waveform);
    waveform_fm.begin(current_waveform);
    if (numButtonsPressed > 0) envelope.noteOn();
    AudioInterrupts();
    Serial.println(current_waveform);
  }

}

// Switches mixer to current setting of amfm global variable
void switchAmFm() {
  AudioNoInterrupts();
  if (amfm) {
    fm_am_mixer.gain(0, 0);
    fm_am_mixer.gain(1, 1);
  } else {
    fm_am_mixer.gain(0, 1);
    fm_am_mixer.gain(1, 0);
  }
  AudioInterrupts();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = (255 - WheelPos) / 2;
  if (WheelPos < 85) {
    return seesaw_NeoPixel::Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return seesaw_NeoPixel::Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return seesaw_NeoPixel::Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void startAnimation(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2 - 10; i+=3) {
    // The INVERSE color is used so rectangles alternate white/black
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, SSD1306_INVERSE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(30);
  }
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(46, 24);
  display.println("Teensy");
  display.setCursor(49, 32);
  display.print("Synth");
  display.display();
}


// Displays current waveform on the OLED and starts a scroll
void displayWaveform() {
  display.clearDisplay();
  if (current_waveform == WAVEFORM_SINE) {
    float stepsize = (3.1415926535 * 6.0) / (float)SCREEN_WIDTH;
    for (int i = 0; i < SCREEN_WIDTH; i++) {
      display.drawPixel(i, SCREEN_HEIGHT / 2 + sin(i * stepsize) * 20.0, SSD1306_WHITE);
    }
  } else if (current_waveform == WAVEFORM_SAWTOOTH) {
    display.drawLine(31, SCREEN_HEIGHT / 2 - 20, 0, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(31, SCREEN_HEIGHT / 2 + 20, 31, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);

    display.drawLine(63, SCREEN_HEIGHT / 2 - 20, 31, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(63, SCREEN_HEIGHT / 2 + 20, 63, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);

    display.drawLine(95, SCREEN_HEIGHT / 2 - 20, 63, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(95, SCREEN_HEIGHT / 2 + 20, 95, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);

    display.drawLine(127, SCREEN_HEIGHT / 2 - 20, 95, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(127, SCREEN_HEIGHT / 2 + 20, 127, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
  } else if (current_waveform == WAVEFORM_TRIANGLE) {
    display.drawLine(0, SCREEN_HEIGHT / 2 - 20, 15, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(15, SCREEN_HEIGHT / 2 + 20, 31, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
    display.drawLine(31, SCREEN_HEIGHT / 2 - 20, 47, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(47, SCREEN_HEIGHT / 2 + 20, 63, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
    display.drawLine(63, SCREEN_HEIGHT / 2 - 20, 79, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(79, SCREEN_HEIGHT / 2 + 20, 95, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
    display.drawLine(95, SCREEN_HEIGHT / 2 - 20, 111, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(111, SCREEN_HEIGHT / 2 + 20, 127, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
  } else if (current_waveform == WAVEFORM_SQUARE) {
    display.drawLine(31, SCREEN_HEIGHT / 2 - 20, 0, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
    display.drawLine(31, SCREEN_HEIGHT / 2 + 20, 31, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);

    display.drawLine(63, SCREEN_HEIGHT / 2 + 20, 31, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(63, SCREEN_HEIGHT / 2 + 20, 63, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);

    display.drawLine(95, SCREEN_HEIGHT / 2 - 20, 63, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
    display.drawLine(95, SCREEN_HEIGHT / 2 + 20, 95, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);

    display.drawLine(127, SCREEN_HEIGHT / 2 + 20, 95, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(127, SCREEN_HEIGHT / 2 + 20, 127, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
  } else if (current_waveform == WAVEFORM_PULSE) {
    display.drawLine(31, SCREEN_HEIGHT / 2 + 20, 0, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(31, SCREEN_HEIGHT / 2 - 20, 31, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);

    display.drawLine(39, SCREEN_HEIGHT / 2 - 20, 31, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
    display.drawLine(39, SCREEN_HEIGHT / 2 - 20, 39, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);

    display.drawLine(95, SCREEN_HEIGHT / 2 + 20, 39, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
    display.drawLine(95, SCREEN_HEIGHT / 2 - 20, 95, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);

    display.drawLine(103, SCREEN_HEIGHT / 2 - 20, 95, SCREEN_HEIGHT / 2 - 20, SSD1306_WHITE);
    display.drawLine(103, SCREEN_HEIGHT / 2 - 20, 103, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);

    display.drawLine(103, SCREEN_HEIGHT / 2 + 20, 127, SCREEN_HEIGHT / 2 + 20, SSD1306_WHITE);
  }
  noInterrupts();
  display.display();
  display.startscrollright(0x00, 0x0F);
  interrupts();
}
