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
//

// Actions to perform for Note On
void MIDICV::ProcessNoteOn(byte pitch, byte velocity)
{
#ifdef PRINTDEBUG
  Serial.print(pitch);
  Serial.print(" noteon / v ");
  Serial.println(velocity);
  PrintNotes();
#endif

  if (nNotesOn >= MAXNOTES) {
    return;                         // as fast as possible
  }
  // Add a new note to the note on list
  // First check if already in the list (should not happen, but...)
  // If not repeated, increase pressed key counter and store it
  int repeat = CheckRepeat(pitch);
  char newnote = 0;
  if (!repeat) {
    setbit128(playingNotes, pitch);
    NotesOn[nNotesOn] = { pitch, velocity };
    nNotesOn++;
    newnote = 1;
  }
#ifdef PRINTDEBUG
  Serial.print(repeat);
  Serial.print(" rep / Notes On: ");
  Serial.println(nNotesOn);
  PrintNotes();
#endif
  // If not repeated Play the note
  if (newnote) {
    playNote(pitch, velocity);
  }
}

// Actions to perform for Note Off
void MIDICV::ProcessNoteOff(byte pitch, byte velocity)
{
#ifdef PRINTDEBUG
  Serial.print(pitch);
  Serial.print(" noteoff / v ");
  Serial.println(velocity);
  PrintNotes();
#endif

  if (!nNotesOn) {
    return;              // as fast as possible
  }
  // First check if the note off is in the list
  int repeat;
  repeat = CheckRepeat(pitch);
  if (!repeat) {
#ifdef PRINTDEBUG
    Serial.println("Note on for received note off not found");
#endif
    return; // Note on for received note off not found
  }
  unsetbit128(playingNotes, pitch);
  if (nNotesOn == 1) {
    // Last note, play note off
    nNotesOn = 0;
    playNoteOff(); //Leaving Pitch and velocity value and Gate down
#ifdef PRINTDEBUG
    Serial.print(" Last Note Off: ");
    Serial.println(pitch);
#endif
  } else {
    // Remove note from the list and play the last active note
    for (int i = 0; i < nNotesOn; i++) {
      if (NotesOn[i].pitch == pitch) {
        while (i < nNotesOn) {
          NotesOn[i] = NotesOn[i + 1];
          i++;
        }
        break;
      }
    }
    nNotesOn--; // Decrease # of active notes
    playNote(NotesOn[nNotesOn - 1].pitch, NotesOn[nNotesOn - 1].velocity);
#ifdef PRINTDEBUG
    Serial.print(repeat);
    Serial.print(" rep / Notes On: ");
    Serial.println(nNotesOn);
    PrintNotes();
#endif
  }
}

// Actions to perform for Pitch Bend
void MIDICV::ProcessBend(int bend)
{
  unsigned int voltage = 0;

  if (BendDAC == NULL) {
    return;
  }

  voltage = BendDAC->linealConvert(bend);
  if (voltage > 4095) {
    voltage = 0;
  }

#ifdef PRINTDEBUG
  Serial.print("pitch bend: ");
  Serial.print(bend);
  Serial.print(" volt: ");
  Serial.println(voltage);
#endif
  sendvaltoDAC(BendDAC->DACnum, voltage);
}

// Actions to perform for Modulation change
void MIDICV::ProcessModul(byte value)
{
  unsigned int voltage = 0;
  if (ModulDAC == NULL) {
    return;
  }
  // Convert to voltage
  voltage = ModulDAC->linealConvert(value);
  if (voltage > 4095) {
    voltage = 0;
  }

#ifdef PRINTDEBUG
  Serial.print("Modulation: ");
  Serial.print(value);
  Serial.print(" volt: ");
  Serial.println(voltage);
#endif
  sendvaltoDAC(ModulDAC->DACnum, voltage);
}

void MIDICV::ProcessPortaOnOff(byte value)
{
  ramp.onoff = (value >= 64) ? true : false;
}

#ifdef PRINTDEBUG
void MIDICV::PrintNotes(void)
{
  for (int i = 0; i < nNotesOn; i++) {
    Serial.print(NotesOn[i].pitch);
    Serial.print("/");
    Serial.print(NotesOn[i].velocity);
    Serial.print(" ");
  }
  Serial.println(" ");
}
#endif

// Check if the note is in the list and return position
byte MIDICV::CheckRepeat(byte pitch)
{
  if (nNotesOn < 1) {
    return (0);
  }

  if (isbitset128(playingNotes, pitch)) {
    return (1);
  }
  return (0);
}

// Play Notes to DAC and pin outputs
void MIDICV::playNote(byte note, byte plvelocity)
{
  unsigned int voltage = 0;
  unsigned int voltage2 = 0;

  if (PitchDAC != NULL) {
    ramp.goal = (float)note;
    Run(); // handle ramps and voltage output
  }

  if (VelDAC != NULL) {
    voltage2 = VelDAC->linealConvert(plvelocity);
    if (voltage2 > 4095) {
      voltage2 = 4095;
    }
    sendvaltoDAC(VelDAC->DACnum, voltage2);
  }

  if (plvelocity == 0) { //NOTE OFF
    playNoteOff();
  } else if (pinGATE != -1) {
    digitalWrite(pinGATE, HIGH);
  }
}

// All Notes off
void MIDICV::playNoteOff(void)
{
  if (pinGATE != -1) {
    digitalWrite(pinGATE, LOW);
  }
}

// Note on received in Learn Mode
// Capture channel and new minimum note
void MIDICV::LearnThis(byte channel, byte pitch, byte velocity)
{
  if (PitchDAC == NULL) {
    return;
  }
  // Learn channel and clock
  midiChannel = channel;
  // Set new minimum pitch
  PitchDAC->minInput = pitch;
#ifdef PRINTDEBUG
  Serial.print(channel);
  Serial.print(" Channel. New min Input: ");
  Serial.println(pitch);
  Serial.print(LearnStep);
  Serial.println(" Step, End Learn Mode");
#endif
  LearnInitTime = millis();
  nNotesOn = 0;

  // Go to next channel learn step.
  if (LearnStep < MAXNumMIDI - 1 && LearnStep < 3) {
    LearnStep++;
  } else {
    // Set normal mode
    LearnMode = NORMALMODE;
    LearnStep = 0;
    // Turn off LED
    blink.setBlink(0, 0, 0);
    // Store value in EEPROM
    WriteMIDIeeprom();
  }
}

void MIDICV::Run()
{
  static float rampFactor = 0.99;
  static float oneMinusRampFactor = 1.0 - rampFactor;

  if (LearnMode == NORMALMODE) {
    if (ramp.current == ramp.goal) {
      return;
    }

    if (ramp.onoff) {
      ramp.current = (ramp.goal * oneMinusRampFactor) + (ramp.current * rampFactor);
      if (fabs(ramp.current - ramp.goal) < 0.01) {
        ramp.current = ramp.goal;
      }
    } else {
      ramp.current = ramp.goal;
    }
  } else {
    ramp.current = ramp.goal;
  }

  int voltage = PitchDAC->intervalConvertF(ramp.current);
  if (voltage > 4095) {
    voltage = 4095;
  }
  sendvaltoDAC(PitchDAC->DACnum, voltage);
}

///////////////////////////////////
// Support functions for MIDI class

// Check if received channel is any active MIDI
int CheckActiveMIDI(byte channel, byte pitch)
{
  // If in percussion mode, check the note and return the associated gate
  if ((MIDImode == PERCTRIG || MIDImode == PERCGATE) && channel == 10) {
    return (PercussionNoteGate(pitch));
  }
  // In other modes, check if received one is an active channel
  for (int i = 0; i < MAXNumMIDI; i++) {
    if (ChanMIDI[i].midiChannel == channel && ChanMIDI[i].PitchDAC->minInput < pitch) {
      return (i);
    }
  }
  return (-1);
}

// Check percussion notes and return associated gate
int PercussionNoteGate(byte pitch)
{
  switch (pitch) {
  case 36:
    return (0);
  case 38:
    return (1);
  case 42:
    return (2);
  case 45:
    return (3);
  case 47:
    return (5);
  case 48:
    return (6);
  case 49:
    return (7);
  case 51:
    return (8);
  }
  return (-1);
}

// Check if received channel is any active MIDI
void AllNotesOff(void)
{
  for (int i = 0; i < MAXNumMIDI; i++) {
    ChanMIDI[i].playNoteOff();
    ChanMIDI[i].nNotesOn = 0;
  }
}

// Read MIDI properties from eeprom
int ReadMIDIeeprom(void)
{
  //return -1;
  MIDICV TempMIDI;
  MultiPointConv TempConv;
  int numMIDI, modeMIDI;
  int i;

  eeprom_read_block((void *)&numMIDI, (void *)0, sizeof(numMIDI));
  eeprom_read_block((void *)&modeMIDI, (void *)sizeof(int), sizeof(modeMIDI));
#ifdef PRINTDEBUG
  Serial.print("Read EEPROM Num MIDI");
  Serial.print(numMIDI);
  Serial.print(" / Mode MIDI: ");
  Serial.println(modeMIDI);
#endif
  if (numMIDI < 1 || numMIDI > 4) {
    return (-1);
  }
  if (modeMIDI < MONOMIDI || modeMIDI > PERCGATE) {
    return (-1);
  }

  for (i = 0; i < 4; i++) {
    eeprom_read_block((void *)&TempConv,
                      (void *)(sizeof(int) * 2 + i * (sizeof(TempConv) + sizeof(TempMIDI))),
                      sizeof(TempConv));
    // stored in Pitch Only if EEPROM values for rangeDAC are correct
    if (i == 0) {
      if (TempConv.rangeDAC == 4093) {
        memcpy(&DACConv[i], &TempConv, sizeof(DACConv[i]));
      } else {
        return (-1);
      }
    } else {
      memcpy(&DACConv[i], &TempConv, sizeof(DACConv[i]));
    }

    eeprom_read_block((void *)&TempMIDI,
                      (void *)(sizeof(int) * 2 + sizeof(TempConv) + i *
                               (sizeof(TempConv) + sizeof(TempMIDI))),
                      sizeof(TempMIDI));
    memcpy(&ChanMIDI[i], &TempMIDI, sizeof(ChanMIDI[i]));
    /*
       #ifdef PRINTDEBUG
       Serial.print("DACConv addr: ");
       Serial.print((sizeof(int)*2+i*(sizeof(TempConv)+sizeof(TempMIDI))));
       Serial.print(" /2: ");
       Serial.println((sizeof(int)*2+sizeof(TempConv)+i*(sizeof(TempConv)+sizeof(TempMIDI))));
       Serial.print("DACConv 1: ");
       Serial.print(DACConv[i].DACPoints[1]);
       Serial.print(" /2: ");
       Serial.println(DACConv[i].DACPoints[2]);
       #endif
     */
  }
  MAXNumMIDI = numMIDI;
  MIDImode = modeMIDI;

  return (MAXNumMIDI);
}

// Write MIDI properties from eeprom
void WriteMIDIeeprom(void)
{
  //return;
  int i;
#ifdef PRINTDEBUG
  Serial.println("Write MIDI");
  Serial.print("Num MIDI: ");
  Serial.print(MAXNumMIDI);
  Serial.print(" / Mode MIDI: ");
  Serial.println(MIDImode);
#endif

  // Store in EEPROM number of channels and MIDI mode
  eeprom_write_block((void *)&MAXNumMIDI, (void *)0, sizeof(MAXNumMIDI));
  eeprom_write_block((void *)&MIDImode, (void *)sizeof(int), sizeof(MIDImode));
  // Store DAC channels and MIDI config
  for (i = 0; i < 4; i++) {
    if (i == 0) {
      DACConv[i].rangeDAC = 4093;
    }
    eeprom_write_block((const void *)&DACConv[i],
                       (void *)(sizeof(MAXNumMIDI) * 2 + i *
                                (sizeof(DACConv[i]) + sizeof(ChanMIDI[i]))),
                       sizeof(DACConv[i]));
    eeprom_write_block((const void *)&ChanMIDI[i],
                       (void *)(sizeof(MAXNumMIDI) * 2 + sizeof(DACConv[i]) + i *
                                (sizeof(ChanMIDI[i]) + sizeof(DACConv[i]))), sizeof(ChanMIDI[i]));
    /*
       #ifdef PRINTDEBUG
       Serial.print("DACConv addr: ");
       Serial.print((sizeof(MAXNumMIDI)*2+i*(sizeof(DACConv[i])+sizeof(ChanMIDI[i]))));
       Serial.print(" /2: ");
       Serial.println((sizeof(MAXNumMIDI)*2+sizeof(DACConv[i])+i*(sizeof(ChanMIDI[i])+sizeof(DACConv[i]))));
       Serial.print("DACConv 1: ");
       Serial.print(DACConv[i].DACPoints[1]);
       Serial.print(" /2: ");
       Serial.println(DACConv[i].DACPoints[2]);
       #endif
     */
  }
}

// Set default MIDI properties manually
void SetModeMIDI(int mode)
{
  MIDImode = mode;
  // Reset pointers
  for (int i = 0; i < 4; i++) {
    ChanMIDI[i].PitchDAC = NULL;
    ChanMIDI[i].VelDAC = NULL;
    ChanMIDI[i].BendDAC = NULL;
    ChanMIDI[i].ModulDAC = NULL;
    ChanMIDI[i].pinGATE = -1;
    ChanMIDI[i].midiChannel = 0;
  }
  switch (mode) {
  case MONOMIDI:
    MAXNumMIDI = 1;
    ChanMIDI[0].PitchDAC = &DACConv[0];
    ChanMIDI[0].VelDAC = &DACConv[1];
    ChanMIDI[0].BendDAC = &DACConv[2];
    ChanMIDI[0].ModulDAC = &DACConv[3];
    ChanMIDI[0].pinGATE = PINGATE;
    ChanMIDI[0].midiChannel = 1;
    // Bend hava a range of -8192 (min bend) to 8191 (max bend) and output 0.5 V
    ChanMIDI[0].BendDAC->rangeInput = 16383;
    ChanMIDI[0].BendDAC->minInput = -8192;
    ChanMIDI[0].BendDAC->rangeDAC = 68;
    ChanMIDI[0].ModulDAC->rangeInput = 127;
    break;
  case DUALMIDI:
    MAXNumMIDI = 2;
    ChanMIDI[0].PitchDAC = &DACConv[0];
    ChanMIDI[0].VelDAC = &DACConv[1];
    ChanMIDI[0].pinGATE = PINGATE;
    ChanMIDI[0].midiChannel = 1;
    ChanMIDI[1].PitchDAC = &DACConv[2];
    ChanMIDI[1].VelDAC = &DACConv[3];
    ChanMIDI[1].pinGATE = PINGATE3;
    ChanMIDI[1].midiChannel = 2;
    break;
  case QUADMIDI:
    MAXNumMIDI = 4;
    ChanMIDI[0].PitchDAC = &DACConv[0];
    ChanMIDI[0].pinGATE = PINGATE;
    ChanMIDI[0].midiChannel = 1;
    ChanMIDI[1].PitchDAC = &DACConv[1];
    ChanMIDI[1].pinGATE = PINGATE2;
    ChanMIDI[1].midiChannel = 2;
    ChanMIDI[2].PitchDAC = &DACConv[2];
    ChanMIDI[2].pinGATE = PINGATE3;
    ChanMIDI[2].midiChannel = 3;
    ChanMIDI[3].PitchDAC = &DACConv[3];
    ChanMIDI[3].pinGATE = PINGATE4;
    ChanMIDI[3].midiChannel = 4;
    break;
  case PERCTRIG:
    break;
  case PERCGATE:
    break;
  default:
    return;
  }
#ifdef PRINTDEBUG
  Serial.print("New Mode: ");
  Serial.println(mode);
#endif

  WriteMIDIeeprom();
}
