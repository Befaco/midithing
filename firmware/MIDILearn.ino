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

  // All Notes off
  AllNotesOff();

  // Init blinker
  Blink.setBlink(100, 0, -1, PINLED);
  Blink.setBlink(100, 0, -1, PINGATE);

  // Init timer
  LearnInitTime = millis();
#ifdef PRINTDEBUG
  Serial.println("Init Learn Mode");
#endif
}

// Learn Mode Cycle function
void DoLearnCycle(void)
{
  Blink.setBlink(100, 0, -1, PINLED);
#ifdef PRINTDEBUG
  Serial.println("End Learn Mode");
#endif
}

//////////////////////////////
// Initialize Cal Mode
/////////////////////////////
void EnterCalMode(void)
{
  // Set Learn mode flag
  LearnMode = ENTERCAL;

  // All Notes off
  AllNotesOff();

  // Init blinker
  Blink.setBlink(100, 0, -1, PINLED2);

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
  Blink.setBlink(100, 0, -1, PINLED2);
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
  BlinkSaving();
  // Set normal mode
  LearnMode = NORMALMODE;
  //Reset Process calibration flag if setted
  exitCalProc();
  // Store in EEPROM
  WriteMIDIeeprom();
  // Turn off LED blink
  ResetBlink();
#ifdef PRINTDEBUG
  Serial.println("End Cal Mode");
#endif
}

//////////////////////////////
// Menu mode handle
/////////////////////////////
byte MenuModeHandle(byte channel, byte pitch, byte velocity)
{
  byte lv_return = 1;
  switch (checkMenuMode(channel)) {
    case CALMODE:
      // Channels 1-4 for DAC calibration 0-3
      lv_return = (DACConv[channel - 1].Processnote(channel, pitch, velocity));
      break;
    case CHANGEMODE:
      //Selection of voice MODE
      lv_return = selectVoiceMode(pitch);
      break;
    case CHANGEOPTIONS:
      //Options Selection
      lv_return = selectOptions(pitch);
      break;
    default:
      lv_return = 0;
      break;
  }

  ResetBlink();
  switch (lv_return) {
    case 1:
      if ( checkMenuMode(channel) != CALMODE) {
        EndCalMode();
      } else {
        BlinkOK();
      }
      break;
    case 0:
      if ( checkMenuMode(channel) != CALMODE) {
        BlinkKO();
      }
      break;
    default:
      //do nothing
      break;
  }
  return (lv_return);
}

byte getElementalPitch(byte pitch)
{
  byte lv_pitch = pitch;
  while (lv_pitch >= 12) {
    lv_pitch = lv_pitch - 12;
  }
  return (lv_pitch);
}

void InitLearnMode(void) {
  for (int i = 0; i < 4; i++) {
    Voice[i].InitLearn();
  }
}

void ConfirmLearnMode(void)
{
  BlinkSaving();
  for (int i = 0; i < 4; i++) {
    Voice[i].ResetOldLearn();
  }

  if ( IsPoly2Mode() ){
    //IF DUOMODE, only Voice 0 is configured And has to be replicated to Voice[2] (CV3)
    Voice[1].SetLearn( Voice[0].midiChannel, Voice[0].pitchDAC->minInput );
  }
  
  if ( IsPoly4Mode() ){
    //IF POLY, only Voice 0 is configured and has to be replicated voice to all (CV2,3 &4)
    Voice[1].SetLearn( Voice[0].midiChannel, Voice[0].pitchDAC->minInput );
    Voice[2].SetLearn( Voice[0].midiChannel, Voice[0].pitchDAC->minInput );
    Voice[3].SetLearn( Voice[0].midiChannel, Voice[0].pitchDAC->minInput );
  }
  
  EndLearnMode();
}

void EndLearnMode() {
  // Set normal mode
  LearnMode = NORMALMODE;
  LearnStep = 0;
  // Store value in EEPROM
  WriteMIDIeeprom();
  ResetBlink();
  return;
}

void CancelLearnMode(void)
{
  ResetBlink();
  Blink.setBlink(100, 0, -1, PINCLOCK);
  BlinkKO();
  for (int i = 0; i < 4; i++) {
    Voice[i].ResetLearn();
    Voice[i].ResetOldLearn();
  }
  EndLearnMode();
}

byte selectOptions(byte pitch)
{
  byte lv_pitch = pitch;
  lv_pitch = getElementalPitch(pitch);
  switch (lv_pitch) {
    case 0: // C
      SetOverlap(true);
      return 1;
      break;
    case 1: // C#
      ppqnCLOCK = 24;
      trigCLOCK = ( ppqnCLOCK * clockFactor );
      return 1;
      break;
    case 2: //D
      SetOverlap(false);
      return 1;
      break;
    case 3: //D#
      ppqnCLOCK = 48;
      trigCLOCK = ( ppqnCLOCK * clockFactor );
      return 1;
      break;
    case 6: // F#
      ppqnCLOCK = 12;
      trigCLOCK = ( ppqnCLOCK * clockFactor );
      return 1;
      break;
    case 8: // G#
      ppqnCLOCK = 6;
      trigCLOCK = ( ppqnCLOCK * clockFactor );
      return 1;
      break;
    case 10: // A#
      ppqnCLOCK = 3;
      trigCLOCK = ( ppqnCLOCK * clockFactor );
      return 1;
      break;
    case 11: //B
      //Set Process calibration flag if setted
      enterCalProc( );
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

byte selectVoiceMode(byte pitch)
{
  byte lv_pitch = pitch;

  lv_pitch = getElementalPitch(pitch);

  switch (lv_pitch) {
    case 0: // C
      SetVoiceMode(MONOMIDI);
      return 1;
      break;
    case 1: // C#
      SetVoiceMode(DUALMIDI);
      return 1;
      break;
    case 2: // D
      SetVoiceMode(QUADMIDI);
      return 1;
      break;
    case 3: // D#
      SetVoiceMode(PERCTRIG);
      return 1;
      break;
    case 4: // E
      SetVoiceMode(DUOFIRST);
      return 1;
      break;
    case 5: // F
      SetVoiceMode(DUOLAST);
      return 1;
      break;
    case 6: // F#
      SetVoiceMode(DUOHIGH);
      return 1;
      break;
    case 7: // G
      SetVoiceMode(DUOLOW);
      return 1;
      break;
    case 8: // G#
      SetVoiceMode(POLYFIRST);
      return 1;
      break;
    case 9: // A
      SetVoiceMode(POLYLAST);
      return 1;
      break;
    case 10: // A#
      SetVoiceMode(POLYHIGH);
      return 1;
      break;
    case 11: // B
      SetVoiceMode(POLYLOW);
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

byte checkMenuMode( byte channel )
{
  if (calProcEnabled) {
    return CALMODE;
  } else if (channel == 1) {
    return CHANGEMODE;
  } else if (channel == 2) {
    return CHANGEOPTIONS;
  }
}

void enterCalProc(void)
{
  for (int i = 0; i < 10; i++) {
    Blink.setBlink(100, 1, 1, PINCLOCK);
    delay(100);
    Blink.setBlink(0, 0, 0, PINCLOCK);
    delay(100);
    Blink.setBlink(100, 1, 1, PINSTARTSTOP);
    delay(100);
    Blink.setBlink(0, 0, 0, PINSTARTSTOP);
    delay(100);
  }

  calProcEnabled = true;
  InitLearnMode();
  for (int i = 0; i < 4; i++) {
    Voice[i].SetLearn((i + 1), 0);
  }
  WriteMIDIeeprom();
}


void exitCalProc(void)
{
  if (calProcEnabled == true ) {
    calProcEnabled = false;
    for (int i = 0; i < 4; i++) {
      Voice[i].ResetLearn();
      Voice[i].ResetOldLearn();
    }
    Blink.setBlink(0, 0, 0, PINCLOCK);
    Blink.setBlink(0, 0, 0, PINSTARTSTOP);
  }
}

