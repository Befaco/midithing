# MIDI Thing v2.2
Firmware for the [Befaco](http://befaco.org) Eurorack MIDI module (MIDI Thing).

## Requirements
You need Arduino >= 1.6.3. 

Additionally you will need the following libraries installed (see the [Arduino Guide](https://www.arduino.cc/en/Guide/Libraries) on how to do this):

* [MIDI](https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/5.0.2) 
* [Bounce2](https://github.com/thomasfredericks/Bounce2) 
* [mcp4728](https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/neuroelec/mcp4728_library_v1.3_for_Arduino_1.0.zip)

## bug fix

- Fixed Retrigger issues
- Fixed Perc trigger note error.


## Uploading the firmware


1. Get an ICSP Programmer 
We normally use an USBasp programmer like this one https://www.ebay.com/itm/USBASP-USB-ISP-Programmer-for-Atmel-AVR-ATMega328-ATMega32U4-Arduino-/322662323277

2. Download Arduino IDE
Go to the official Arduino website https://www.arduino.cc/en/Main/Software and download the latest version of Arduino IDE for your operating system and install it.

3. Download the firmware 
Go to https://github.com/Befaco/midithing and press "clone or download" to download the repository to your computer. Extract the zip file and copy "firmware" folder to your sketchbook folder. 

4. Install the libraries
Download the libraries from Library manager (Bounceand MIDI) and install them following the instructions from this link https://www.arduino.cc/en/Guide/Libraries
Keep in mind MCP library must be installed from the following link. Do not use library manager one:
 https://code.google.com/archive/p/neuroelec/downloads 
 
5. Connecting the module
Connect the programmer to the ICSP conector in the back of the module. Pay special attention to the pinout when you plug the module. You can check this link for more information about ICSP https://www.arduino.cc/en/Tutorial/ArduinoISP

6. Upgrading
Press "Open" and search the file "firmware.ino" located on the "firmware" folder. Select "Tools > Board > Arduino/Genuino Uno". Go to "Sketch" and hit "Upload using programmer".

If everything goes well you should see a "Done uploading" message in a few seconds. If something happens during the upgrade, check your Arduino settings following the instructions of this link https://www.arduino.cc/en/Guide/Troubleshooting

## Credits

MIDI Thing firmware v1 was coded with love by Sergio Retamero:
sergio.retamero@gmail.com

Performance boost, bug fixing and extra awesomeness by Jeremy Bernstein:
jeremy.d.bernstein@googlemail.com

Firmware v2 coding:

Quad Polyphonic modes were implemented by Jeremy Bernstein.

New configuration system, duophonic modes implementation, clock modes, a touch of beauty to the interface and a gazillion little details no living person could ever imagine by Alberto "Numeríco" Navarro:
albertonafu@gmail.com 

Calibration method inspired on Mutable instruments CVpal.

Thanks to everyone that took the time to finger point bugs, provide feedback and made loads of  suggestions. Special thanks to Jonatan "Jessica Fletcher" Bernabeu.


