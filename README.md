# Simple-Monophonic-Teensy-Synth
## Introduction
![Synth Removed Background](https://user-images.githubusercontent.com/71471706/235532524-13bad02f-a969-41a8-8862-1f4d5de6c4c1.png)


This is a simple teensy synth I made for the final project in my "Prototyping Intelligent Devices Class" which basically asked us to "make an embeded device."

A video of the demo for the project can be found here: https://www.youtube.com/watch?v=l1DhpedB-J8

Files included in the repo are the f3d (CAD) files for the case, and the code uploaded to the teensy 4.0.

## Software
### Synth Description
I tried to comment the code at least a little for every functional block so hopefully that helps, but here is the audio design in GUI form:

<img width="800" alt="Teensy Software GUI" src="https://user-images.githubusercontent.com/71471706/235530890-29660d80-82bf-4034-acda-173b3827bc99.png">

There are two waveforms, "waveform" and "waveform_fm". Those are the main instruments. They are both modulated by "modulator_sin". "waveform" undergoes AM and "waveform_fm" undergoes FM.

### Libraries Used
* Teensy Audio
* Adafruit Trellis
* Adafruit seesaw
* Adafruit neopixel
* Adafruit GFX
* Adafruit SSD1036 (screen)

## Components

| Component  | Link to Purchase |
| ------------- | ------------- |
| Teensy 4.0  | Generic |
| Teensy Audio Shield  | Generic |
| Class D Audio Amp  | https://www.adafruit.com/product/2130 |
| 3W Speaker  | https://www.adafruit.com/product/4445 |
| 9x 100k Panel Mount Potentiometers | Generic ~7-7.5mm diameter shaft |
| Adafruit Trellis Keypad  | https://www.adafruit.com/product/1616 and https://www.adafruit.com/product/1611 |
| 2x Adafruit I2C Sliders  | https://www.adafruit.com/product/5295 |
| Panel Mount USB C to Micro B | https://www.adafruit.com/product/4056 |
| Panel Mount Audio | https://www.adafruit.com/product/3319 |
| Monochrome OLED Display | https://www.adafruit.com/product/326 |
| Large Panel Mount Switch (unused) | https://www.adafruit.com/product/3221 |
| Small Panel Mount Switch (SD for amp) | Generic ~5.6mm diameter shaft |
| M3 Threaded Inserts | (generic but these are the dimensions for mine) https://a.co/d/aK7ZUcl |
| M3 Screws and Nuts | Generic |
| M2.5 Screws and Nuts | Generic |

There is not much to say about how to connect components but here are a few notes:
* Amp input is GND and either L or R on the output of the audio shield (since audio is MONO anyways)
  * <img width="188" alt="Screenshot 2023-05-01 at 4 54 41 PM" src="https://user-images.githubusercontent.com/71471706/235529768-840f780b-0c09-4df2-a702-69fc36244b41.png">
* All i2c devices can use the same i2c bus (pins 19 and 18 on the teensy)
* Buying Stemma QT cables to connect the i2c devices will make your life easier
* "int" pin on keypad is not used
* Connect SPST switch from SD-bar on amp and ground so you can turn off the speaker
* M3 threaded inserts go into the corners of the bottom shell; use M3 screws to fasten the top plate on the bottom shell
* M3 screws and nuts are used to fasten the speaker.
* M2.5 screws and nuts are used to fasten the screen from the top, and the sliders from the bottom.

## Future Work
### Problems to Fix
* Insane noise on analog pins when audio i2c output is initialized. No clue why that happens
  * Solution:
    * Fix the issue
    * Software smoothing

### Future Features
* More than one voice
* Arpegiator
* Tone shifting input
* More screen display
  * Shows what you are changing and to what value
* Self contained battery

## Case Design

### Bottom Shell
<img width="600" alt="Bottom CAD" src="https://user-images.githubusercontent.com/71471706/235526062-9587b856-9cdf-4215-a1da-870d0a5a56cf.png">

### Top and Bottom
<img width="600" alt="CAD" src="https://user-images.githubusercontent.com/71471706/235526236-a8d2ad15-0765-49ee-ae01-f624be04325d.png">

### Sections
<img width="600" alt="CAD Sections" src="https://user-images.githubusercontent.com/71471706/235526399-05236279-a46a-4d87-a155-6ab86c62bad7.png">

## Photos
![side](https://user-images.githubusercontent.com/71471706/235532766-ac2d7b0f-e631-47a5-a153-28592425e129.jpg)
![ports](https://user-images.githubusercontent.com/71471706/235532775-6aeafd50-085d-4d57-baf8-f9f47bf489c2.jpg)
![top](https://user-images.githubusercontent.com/71471706/235532789-6c98fb8d-4ad0-40e2-9d37-abc3a5ce4bc4.jpg)
![bottom](https://user-images.githubusercontent.com/71471706/235532800-d5924257-c379-4b34-a206-733868b1d785.jpg)

