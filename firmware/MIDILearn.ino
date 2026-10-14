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
// V2 - 2018
// Author: Alberto Navarro (albertonafu@gmail.com) 
// Enhacements, new functions, new modes, usability, user interface and bug fixings.
// -----------------------------------------------------------------------------
//# define ERRORCYCLES 30

/////////////////////////////////////////////////////////////////////////////////////////////
// Initialize Learn Mode
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
// Initialize Cal Mode
/////////////////////////////////////////////////////////////////////////////////////////////
void EnterCalMode(void)
{
  // Set Learn mode flag
  LearnMode = ENTERCAL;
  // All Notes off
  AllNotesOff();
  // Init blinker
  Blink.setBlink(100, 0, -1, PINLED2);
  //For recovery pourpose
  SetCurrentVoiceMode();
  InitLearnMode();
  // Set mode to 4 channels
  SetVoiceMode(QUADMIDI);
  // Init timer
  LearnInitTime = millis();
#ifdef PRINTDEBUG
  Serial.println("Init Cal Mode");
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
// Cal Cycle
/////////////////////////////////////////////////////////////////////////////////////////////
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
    EndCalMode(false);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Cal ending
/////////////////////////////////////////////////////////////////////////////////////////////
void EndCalMode(bool cancel)
{
  // Not idempotent otherwise, and this does get called more than once: the
  // button, MenuModeHandle() and the 55 second timeout can all reach it. The
  // first call restores the learned channels and ReSetCurrentLearn() then
  // clears the backups; a second call would run RecoverVoice() again, which
  // re-applies the voice mode - resetting the channels to the mode defaults -
  // and can no longer restore anything, silently losing the learn.
  if (LearnMode == NORMALMODE) {
    return;
  }


  BlinkSaving();
  // Set normal mode
  LearnMode = NORMALMODE;
  // The learned channels have to come back on the way out whichever path got
  // us here. EnterCalMode() replaced them with the mode defaults, and the
  // restore used to hang off calProcEnabled inside exitCalProc(), so the 55
  // second timeout taken before the procedure was ever started wrote those
  // defaults into EEPROM and the learn was gone.
  // RecoverVoice() must run exactly once: it re-applies the voice mode first
  // and only then restores the channels, so calling it twice would leave the
  // defaults in place.
  if (!cancel) {
    calProcEnabled = false; // end of the calibration procedure, if it was running
  }
  RecoverVoice();
  // Store in EEPROM
  WriteMIDIeeprom();
  // Turn off LED blink
  ResetBlink( );
#ifdef PRINTDEBUG
  Serial.println("End Cal Mode");
#endif
}

//Set Current Active voice
void RecoverVoice(void) {
  ResetToCurrentVoiceMode( );
  for (int i = 0; i < GS.NumVoices; i++) {
    Voice[i].ResetLearn();
    Voice[i].ReSetCurrentLearn();
  }
  Blink.setBlink(0, 0, 0, PINCLOCK);
  Blink.setBlink(0, 0, 0, PINSTARTSTOP);
#ifdef PRINTDEBUG
  Serial.println("End Menu Mode");
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Menu mode handle
/////////////////////////////////////////////////////////////////////////////////////////////
byte MenuModeHandle(byte channel, byte pitch, byte velocity)
{
  byte lv_return = 1;
  switch (checkMenuMode(channel)) {
    case CALMODE:
      // Channels 1-4 for DAC calibration 0-3
      if (channel < 1 || channel > GS.NumVoices) {
        // DACConv holds one entry per voice: any other channel would index
        // past the end of the array and Processnote() writes through it.
        lv_return = 0;
        break;
      }
      lv_return = (DACConv[channel - 1].Processnote(channel, pitch, velocity));
      break;
    case CHANGEMODE:
      //Selection of voice MODE
      lv_return = selectVoiceMode(pitch);
      break;
    case CHANGEOPTIONS:
      //Options Selection
      lv_return = selectOptions(pitch);
      if (!calProcEnabled && lv_return == 1) {
        ResetToCurrentVoiceMode();
      }
      break;
    default:
      lv_return = 0;
      break;
  }

  ResetBlink( );
  switch (lv_return) {
    case 1:
      if ( checkMenuMode(channel) != CALMODE) {
        EndCalMode(false);
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

/////////////////////////////////////////////////////////////////////////////////////////////
//Options management in menu mode
/////////////////////////////////////////////////////////////////////////////////////////////
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
      SetPpqnClock(24);
      return 1;
      break;
    case 2: //D
      SetOverlap(false);
      return 1;
      break;
    case 3: //D#
      SetPpqnClock(48);
      return 1;
      break;
    case 4: //E
      SetClockMode(NORMAL_CLOCK);
      return 1;
      break;
    case 5: //F
      SetClockMode(RUNNING_CLOCK);
      return 1;
      break;
    case 6: // F#
      SetPpqnClock(12);
      return 1;
      break;
    case 7: //G
      SetSTSPMode(NORMAL_STSP);
      return 1;
      break;
    case 8: // G#
      SetPpqnClock(6);
      return 1;
      break;
    case 9: //A
      SetSTSPMode(UPDOWN_STSP);
      return 1;
      break;
    case 10: // A#
      SetPpqnClock(3);
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
/////////////////////////////////////////////////////////////////////////////////////////////
//Voice mode selection
/////////////////////////////////////////////////////////////////////////////////////////////
byte selectVoiceMode(byte pitch)
{
  byte lv_pitch = pitch;
  byte lv_return = 1;
  int lv_newVoiceMode;

  lv_pitch = getElementalPitch(pitch);

  switch (lv_pitch) {
    case 0: // C
      lv_newVoiceMode = MONOMIDI;
      lv_return = 1;
      //return 1;
      break;
    case 1: // C#
      lv_newVoiceMode = DUALMIDI;
      lv_return = 1;
      //return 1;
      break;
    case 2: // D
      lv_newVoiceMode = QUADMIDI;
      lv_return = 1;
      //return 1;
      break;
    case 3: // D#
      lv_newVoiceMode = PERCTRIG;
      lv_return = 1;
      //return 1;
      break;
    case 4: // E
      lv_newVoiceMode = DUOFIRST;
      lv_return = 1;
      //return 1;
      break;
    case 5: // F
      lv_newVoiceMode = DUOLAST;
      lv_return = 1;
      //return 1;
      break;
    case 6: // F#
      lv_newVoiceMode = DUOHIGH;
      lv_return = 1;
      //return 1;
      break;
    case 7: // G
      lv_newVoiceMode = DUOLOW;
      lv_return = 1;
      //return 1;
      break;
    case 8: // G#
      lv_newVoiceMode = POLYFIRST;
      lv_return = 1;
      //return 1;
      break;
    case 9: // A
      lv_newVoiceMode = POLYLAST;
      lv_return = 1;
      //return 1;
      break;
    case 10: // A#
      lv_newVoiceMode = POLYHIGH;
      lv_return = 1;
      //return 1;
      break;
    case 11: // B
      lv_newVoiceMode = POLYLOW;
      lv_return = 1;
      //return 1;
      break;
    default:
      lv_return = 0;
      //return 0;
      break;
  }
  if (lv_return == 1) {
    SetVoiceMode(lv_newVoiceMode);
    AllRecoveryLearn();
    if (IsPolyMode(lv_newVoiceMode)) {
      PolyModeReplicateVoices(lv_newVoiceMode);
    }
    // EndCalMode() -> RecoverVoice() restores GS.CurrentVoiceMode and the
    // midiChannel_current backups taken on entering the menu. Make the new
    // mode and its channels the ones it restores, or the change is undone.
    SetCurrentVoiceMode();
    InitLearnMode();
  }
  return (lv_return);

}
/////////////////////////////////////////////////////////////////////////////////////////////
///Secondary Procedures
/////////////////////////////////////////////////////////////////////////////////////////////
byte getElementalPitch(byte pitch)
{
  byte lv_pitch = pitch;
  while (lv_pitch >= 12) {
    lv_pitch = lv_pitch - 12;
  }
  return (lv_pitch);
}

void InitLearnMode(void) {
  for (int i = 0; i < GS.NumVoices; i++) {
    Voice[i].InitLearn();
  }
}

void BckUpAllLearn(void) {
  if (IsPolyMode(GS.VoiceMode)) {
    Voice[0].BckUpLearn();
  } else {
    for (int i = 0; i < GS.NumVoices; i++) {
      Voice[i].BckUpLearn();
    }
  }
}

void AllRecoveryLearn(void) {
  for (int i = 0; i < GS.NumVoices; i++) {
    Voice[i].RecoveryLearn();
  }
}

void PolyModeReplicateVoices(int mode) {
  if ( IsPoly2Mode(mode) ) {
    //IF DUOMODE, only Voice 0 is configured And has to be replicated to Voice[1] (CV3)
    Voice[1].SetLearn( Voice[0].midiChannel, Voice[0].pitchDAC->minInput );
  }
  if ( IsPoly4Mode(mode) ) {
    //IF POLY, only Voice 0 is configured and has to be replicated voice to all (CV2,3 &4)
    Voice[1].SetLearn( Voice[0].midiChannel, Voice[0].pitchDAC->minInput );
    Voice[2].SetLearn( Voice[0].midiChannel, Voice[0].pitchDAC->minInput );
    Voice[3].SetLearn( Voice[0].midiChannel, Voice[0].pitchDAC->minInput );
  }
}

void ConfirmLearnMode(void)
{
  BlinkSaving();
  PolyModeReplicateVoices(GS.VoiceMode);
  BckUpAllLearn();
  EndLearnMode();
}

void EndLearnMode() {
  // Set normal mode
  LearnMode = NORMALMODE;
  LearnStep = 0;
  // Store value in EEPROM
  WriteMIDIeeprom();
  ResetBlink( );
  return;
}

void CancelLearnMode(void)
{
  ResetBlink( );
  Blink.setBlink(100, 0, -1, PINCLOCK);
  BlinkKO();
  for (int i = 0; i < GS.NumVoices; i++) {
    Voice[i].ResetLearn();
  }
  EndLearnMode();
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
  return 0; // no menu on this channel; without this the function fell off the
            // end and callers acted on whatever happened to be in the register
}

//Warning lightshow when entering in calc procedure
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
  // No InitLearnMode() here. EnterCalMode() has already backed the learned
  // channels up into midiChannel_current - and has already replaced
  // midiChannel with the QUADMIDI defaults via SetVoiceMode(). Taking the
  // backup a second time at this point captures those defaults instead, so
  // RecoverVoice() faithfully restores 1-4 on the way out and the learn is
  // gone. The loop below is what calibration actually needs: channels 1-4
  // mapped to outputs 1-4, with minInput at 0 so val == pitch.
  for (int i = 0; i < GS.NumVoices; i++) {
    Voice[i].SetLearn((i + 1), 0);
  }
}


void exitCalProc(void)
{
  if (calProcEnabled == true ) {
    calProcEnabled = false;
    RecoverVoice();
  }
}

void DoLearnCycle(void)
{
  Blink.setBlink(100, 0, -1, PINLED);
#ifdef PRINTDEBUG
  Serial.println("End Learn Mode");
#endif
}

