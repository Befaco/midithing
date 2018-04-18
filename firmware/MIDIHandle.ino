// Copyright 2014 Sergio Retamero.
//
// Author: Sergio Retamero (sergio.retamero@gmail.com)
//
// Enhancemnt and Bug fixing: Jeremy Bernstein
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
// Polyphonic mode base code by Jeremy Bernstein.
// -----------------------------------------------------------------------------

//////////////////////////////////////////////
// MIDI function definitions
// Play a NOTE to the MCU outputs/DAC
// Do whatever you want when you receive a Note On.

VoiceSelector Selector;

static int countCLOCK = 1;

unsigned long trigCLOCK = ( GS.PpqnCLOCK * clockFactor );

static byte MIDIRun = 0; // Set to 0 to init in stop condition

void HandleNoteOn(byte channel, byte pitch, byte velocity)
{
  int MIDIactive = -1;
  // If in learn mode, catch info
  if (LearnMode == ENTERLEARN && velocity > 0) {
    Voice[LearnStep].LearnThis(channel, pitch, velocity);
    return;
  }
  // If in cal mode, adjust notes
  if (   LearnMode == ENTERCAL
         && velocity > 0
         && checkMenuMode(channel) != 0
         && MenuModeHandle(channel, pitch, velocity)) {
    //Calibration
    return;  // do not play note if calibration key pressed
  }

  // Check if received channel is any active MIDI
  MIDIactive = Selector.getTargetChannel(channel, pitch, velocity);
  if (MIDIactive == -1) { // no channels available
#ifdef PRINTDEBUG
    Serial.println("Not active MIDI");
#endif
    return; // received channel not any active MIDI
  }
  if (GS.VoiceMode == PERCTRIG && channel == PERCCHANNEL && velocity > 0) {
    // Play percussion
    Gates[MIDIactive].setBlink(TRIGPERCUSSION, 1, 1); // Play trigger
    Blink.setBlink(100, 1, 1, PINLED);  // Blink once every Note ON (not in CAL/LEARN mode)
    return;
  }

  if (velocity == 0) {
    // This acts like a NoteOff.
    HandleNoteOff(channel, pitch, velocity);
    return;
  }

  if (LearnMode != ENTERCAL) {
    Blink.setBlink(100, 1, 1, PINLED);  // Blink once every Note ON (not in CAL/LEARN mode)
  }
  Selector.noteOn(MIDIactive, pitch, velocity);
}

// Do whatever you want when you receive a Note Off.
void HandleNoteOff(byte channel, byte pitch, byte velocity)
{
  int MIDIactive = -1;

  if (LearnMode == ENTERLEARN) {
    return;  // Do not process while in Learn Mode
  }

  // Check if received channel is any active MIDI
  MIDIactive = Selector.getTargetChannel(channel, pitch);
  if (MIDIactive == -1) {
#ifdef PRINTDEBUG
    Serial.println("Not active MIDI");
#endif
    return; // received channel not any active MIDI
  }
  // Do nothing in percussion mode
  if (GS.VoiceMode == PERCTRIG && channel == 10) {
    return;
  }

  Selector.noteOff(MIDIactive, pitch);
}

// Do whatever you want when you receive a Note On.
void HandlePitchBend(byte channel, int bend)
{
  int MIDIactive = -1;

  if (LearnMode == ENTERLEARN) {
    return;  // Do not process while in Learn Mode
  }

  // Check if received channel is any active MIDI
  MIDIactive = Selector.getTargetChannel(channel);
  if (MIDIactive == -1) {
    return;  // received channel not any active MIDI
  }

  Voice[MIDIactive].processBend(bend);
}

// Do whatever you want when you receive a Control Change
void HandleControlChange(byte channel, byte number, byte value)
{
  int MIDIactive = -1;

  if (LearnMode == ENTERLEARN) {
    return;  // Do not process while in Learn Mode
  }
  // Check if received channel is any active MIDI
  MIDIactive = Selector.getTargetChannel(channel);
  if (MIDIactive == -1) {
    return;  // received channel not any active MIDI
  }

  switch (number) {
    case 1:
      Voice[MIDIactive].processMod(value);
      break; // Handle only CC #1 = Modulation
    case 123:
      for (int i = 0; i < GS.NumVoices; i++) {
        Voice[i].playNoteOff(); // All notes off received
        Voice[i].notes.clear(); // 0 notes on
      }
#ifdef PRINTDEBUG
      Serial.println("All Notes Off");
#endif
    default:
      return;
  }
}

#ifdef STARTSTOPCONT
// Handle MIDI Start/Stop/Continue
void HandleStart(void)
{
  MIDIRun = 1;
  countCLOCK = GS.PpqnCLOCK;
  switch (GS.StSpMode) {
    case NORMAL_STSP:
      //Once in Start (Reset)
      Gates[9].setBlink(TRIGSTART, 1, 1);
      break;
    case UPDOWN_STSP:
      //Gate Up in start
      Blink.setBlink(100, 0, -1, PINSTARTSTOP);
      break;
  }
#ifdef PRINTDEBUG
  Serial.println("MIDI Start");
#endif
}

void HandleContinue(void)
{
  MIDIRun = 1;
  countCLOCK = 0;
  switch (GS.StSpMode) {
    case NORMAL_STSP:
      //Once in Continue (Reset)
      Gates[9].setBlink(TRIGSTART, 1, 1);
      break;
    case UPDOWN_STSP:
      //Gate Up in continue
      Blink.setBlink(100, 0, -1, PINSTARTSTOP);
      break;
  }
#ifdef PRINTDEBUG
  Serial.println("MIDI Continue");
#endif
}

void HandleStop(void)
{
  MIDIRun = 0;
  countCLOCK = 0;
  ResetBlink( );
#ifdef PRINTDEBUG
  Serial.println("MIDI Stop");
#endif
}

void HandleClock(void)
{
  //Clock out only when sequencer is playing (start/continue)
  if (GS.ClockMode == RUNNING_CLOCK && !MIDIRun) return;
  //Clock out everytime receives clock in normal mode
  if (countCLOCK < GS.PpqnCLOCK) {
    countCLOCK++;
  } else {
    countCLOCK = 1;
    // Send trigger to CLOCK port
    Gates[4].setBlink(trigCLOCK, 1, 1);
  }
}
#endif  //STARTSTOPCONT

