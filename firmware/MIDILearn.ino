// Copyright 2014 Sergio Retamero.
//
// Author: Sergio Retamero (sergio.retamero@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------

//# define ERRORCYCLES 30

//////////////////////////////////////////////
// Learn mode
// Learn Mode selected

// Initialize Learn Mode
void EnterLearnMode(void)
{
  InitLearnMode();
  // Set Learn mode flag
  LearnMode = ENTERLEARN;
  LearnStep = 0;
  // Init blinker
  Blink.setBlink(100, 0, -1, PINLED);
  Blink.setBlink(100, 0, -1, PINGATE);
  // All Notes off
  AllNotesOff();

  // Init timer
  LearnInitTime = millis();
#ifdef PRINTDEBUG
  Serial.println("Init Learn Mode");
#endif
}

// Learn Mode Cycle function
void DoLearnCycle(void)
{
  unsigned long current = millis();

  /* // 10 seconds learn time  "Only cancel with button
    if (current > LearnInitTime + 10000) {
     CancelLearnMode();
       // Set normal mode
       LearnMode = NORMALMODE;
       // Turn off LED blink
       Blink.setBlink(0, 0, 0);
       Blink.setBlink(0, 0, 0, PINLED);
       // Store value in EEPROM
       WriteMIDIeeprom();*/

#ifdef PRINTDEBUG
  Serial.println("End Learn Mode");
#endif
  //}
}

//////////////////////////////
// Initialize Cal Mode
void EnterCalMode(void)
{
  // Set Learn mode flag
  LearnMode = ENTERCAL;
  // Init blinker
  //Blink.setBlink(500, 500, -1, PINLED2);
  Blink.setBlink(100, 0, -1, PINLED2);

  // All Notes off
  AllNotesOff();

  // Set mode to 4 channels
  SetVoiceMode(QUADMIDI);

  // Init timer
  LearnInitTime = millis();
#ifdef PRINTDEBUG
  Serial.println("Init Cal Mode");
#endif
}

// Cal Mode Cycle function
void DoCalCycle(void)
{
  unsigned long current = millis();

  // After 55 seconds without receiving a note, exit calibration
  if (current > LearnInitTime + 55000) {
#ifdef PRINTDEBUG
    Serial.print(LearnInitTime);
    Serial.print(" msec ");
    Serial.println(current);
    Serial.println("Time expired for Calibration");
#endif
    EndCalMode();
  }
}

void EndCalMode(void)
{

  // Set normal mode
  LearnMode = NORMALMODE;
  // Store in EEPROM
  WriteMIDIeeprom();
  // Turn off LED blink
  Blink.setBlink(0, 0, 0);
  Blink.setBlink(0, 0, 0, PINLED2);

#ifdef PRINTDEBUG
  Serial.println("End Cal Mode");
#endif
}

byte CalProcessNote(byte channel, byte pitch, byte velocity)
{
  byte lv_return = 1;

  if (channel == 11 || channel == 12 || channel == 13 || channel == 14) { // Channels 1-4 for DAC calibration 0-3
    channel = channel - 10;
    lv_return = (DACConv[channel - 1].Processnote(channel, pitch, velocity));
  } else if (channel >= 1 && channel <= 10) {

    pitch = getElementalPitch(pitch);

    switch (pitch) {
      case 0: // C
        SetVoiceMode(MONOMIDI);
        lv_return = 1;
        break;
      case 1: // C#
        SetVoiceMode(DUALMIDI);
        lv_return = 1;
        break;
      case 2: // D
        SetVoiceMode(QUADMIDI);
        lv_return = 1;
        break;
      case 3: // D#
        SetVoiceMode(PERCTRIG);
        lv_return = 1;
        break;
      case 4: // E
        SetVoiceMode(DUOFIRST);
        lv_return = 1;
        break;
      case 5: // F
        SetVoiceMode(DUOLAST);
        lv_return = 1;
        break;
      case 6: // F#
        SetVoiceMode(DUOHIGH);
        lv_return = 1;
        break;
      case 7: // G
        SetVoiceMode(DUOLOW);
        lv_return = 1;
        break;
      case 8: // G#
        SetVoiceMode(POLYFIRST);
        lv_return = 1;
        break;
      case 9: // A
        SetVoiceMode(POLYLAST);
        lv_return = 1;
        break;
      case 10: // A#
        SetVoiceMode(POLYHIGH);
        lv_return = 1;
        break;
      case 11: // B
        SetVoiceMode(POLYLOW);
        lv_return = 1;
        break;
      default:
        lv_return = 0;
        break;
    }
  } else if (channel == 15) {

    pitch = getElementalPitch(pitch);
    switch (pitch) {
      case 0: // C
        SetOverlap(true);
        lv_return = 1;
        break;
      case 1: // C#
        SetOverlap(false);
        lv_return = 1;
        break;
      case 2: //D
        ppqnCLOCK = 24;
        lv_return = 1;
        break;
      case 3: //D#
        ppqnCLOCK = 12;
        lv_return = 1;
        break;
      case 4: // E
        ppqnCLOCK = 6;
        lv_return = 1;
        break;
      case 5: // F
        ppqnCLOCK = 4;
        lv_return = 1;
        break;
      case 6: // F#
        ppqnCLOCK = 2;
        lv_return = 1;
        break;
      case 7: // G
        ppqnCLOCK = 1;
        lv_return = 1;
        break;
      case 8: // G#
        ppqnCLOCK = 36;
        lv_return = 1;
        break;
      case 9: // A
        ppqnCLOCK = 48;
        lv_return = 1;
        break;
      case 10: // A#
        ppqnCLOCK = 52;
        lv_return = 1;
        break;
      case 11: // B
        ppqnCLOCK = 96;
        lv_return = 1;
        break;
      default:
        lv_return = 0;
        break;
    }
  } else {
    lv_return = 0;
  }
  Blink.setBlink(0, 0, 0);
  switch (lv_return) {
    case 1:
      BlinkOK();
      if ( (channel >= 1 && channel <= 10) || (channel == 15) ){
        EndCalMode();
      }
      break;
    case 0:
      BlinkKO();
      Blink.setBlink(0, 0, 0);
      Blink.setBlink(100, 0, -1, PINLED2);
      break;
    default:
      //do nothing
      break;
  }

  return (lv_return);
}

byte getElementalPitch(byte pitch)
{
  while (pitch >= 12) {
    pitch = pitch - 12;
  }
  return (pitch);
}

void InitLearnMode(void) {
  for (int i = 0; i < 4; i++) {
    Voice[i].InitLearn();
  }
}

void ConfirmLearnMode(void)
{

  for (int i = 0; i < 4; i++) {
    Voice[i].ResetOldLearn();
  }
  BlinkOK();
  EndLearnMode();
}

void EndLearnMode() {
  // Set normal mode
  LearnMode = NORMALMODE;
  LearnStep = 0;
  // Store value in EEPROM
  WriteMIDIeeprom();
  return;
}

void CancelLearnMode(void)
{
  for (int i = 0; i < 4; i++) {
    Voice[i].ResetLearn();
    Voice[i].ResetOldLearn();
  }
  BlinkKO();
  EndLearnMode();
}
