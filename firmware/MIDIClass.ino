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

MIDICV Voice[4];
GENERALSETTINGS GS;
byte LastVel = 0;
byte LastNote = 0;

// Actions to perform for Note On
void MIDICV::processNoteOn(byte pitch, byte velocity)
{
#ifdef PRINTDEBUG
  Serial.print(pitch);
  Serial.print(" noteon / v ");
  Serial.println(velocity);
  printNotes();
#endif
/*if (pitch < pitchDAC->minInput){
  return; 
}*/
  if (notes.getCount() >= MAXNOTES) return; // as fast as possible

  // Add a new note to the note on list
  // First check if already in the list (should not happen, but...)
  // If not repeated, increase pressed key counter and store it
  if (!checkRepeat(pitch)) {
    notes.setPlaying(pitch, velocity);
    playNote(pitch, velocity);
  }
#ifdef PRINTDEBUG
  else {
    Serial.print("repeat");
  }

  Serial.print(" rep / Notes On: ");
  Serial.println(notes.getCount());
  printNotes();
#endif
}

// Actions to perform for Note Off
void MIDICV::processNoteOff(byte pitch)
{
#ifdef PRINTDEBUG
  Serial.print(pitch);
  Serial.print(" noteoff / v ");
  printNotes();
#endif

  if (!notes.getCount()) return; // as fast as possible

  // First check if the note off is in the list
  int repeat;
  repeat = checkRepeat(pitch);
  if (!repeat) {
#ifdef PRINTDEBUG
    Serial.println("Note on for received note off not found");
#endif
    return; // Note on for received note off not found
  }
  if (!notes.setPlaying(pitch, 0)) {
    playNoteOff(); //Leaving Pitch and velocity value and Gate down
#ifdef PRINTDEBUG
    Serial.print(" Last Note Off: ");
    Serial.println(pitch);
#endif
  }
  else {
    byte lastPitch, lastVelocity;
    if (notes.getLastEvent(&lastPitch, &lastVelocity)) {
      playNote(lastPitch, lastVelocity);
    }
#ifdef PRINTDEBUG
    Serial.print(repeat);
    Serial.print(" rep / Notes On: ");
    Serial.println(notes.getCount());
    printNotes();
#endif
  }
}

// Actions to perform for Pitch Bend
void MIDICV::processBend(int bend)
{
  unsigned int voltage = 0;

  if (bendDAC == NULL) {
    return;
  }

  voltage = bendDAC->linealConvert(bend);
  if (voltage > 4095) {
    voltage = 0;
  }

#ifdef PRINTDEBUG
  Serial.print("pitch bend: ");
  Serial.print(bend);
  Serial.print(" volt: ");
  Serial.println(voltage);
#endif
  SendvaltoDAC(bendDAC->DACnum, voltage);
}

// Actions to perform for Modulation change
void MIDICV::processMod(byte value)
{
  unsigned int voltage = 0;
  if (modDAC == NULL) {
    return;
  }
  // Convert to voltage
  voltage = modDAC->linealConvert(value);
  if (voltage > 4095) {
    voltage = 0;
  }

#ifdef PRINTDEBUG
  Serial.print("Modulation: ");
  Serial.print(value);
  Serial.print(" volt: ");
  Serial.println(voltage);
#endif
  SendvaltoDAC(modDAC->DACnum, voltage);
}

#ifdef PRINTDEBUG
void MIDICV::printNotes(void)
{
  for (int i = 0; i < notes.getCount(); i++) {
    byte pitch, velocity;
    notes.getEvent(i, &pitch, &velocity);
    Serial.print(pitch);
    Serial.print("/");
    Serial.print(velocity);
    Serial.print(" ");
  }
  Serial.println(" ");
}
#endif

// Check if the note is in the list and return position
byte MIDICV::checkRepeat(byte pitch)
{
  if (notes.getCount() < 1) {
    return (0);
  }

  if (notes.isPlaying(pitch)) {
    return (1);
  }
  return (0);
}

// Play Notes to DAC and pin outputs
void MIDICV::playNote(byte note, byte plvelocity)
{
  unsigned int voltage = 0;
  unsigned int voltage2 = 0;

  if (pitchDAC != NULL) {
    voltage = pitchDAC->intervalConvert(note);
    if (voltage > 4095) {
      voltage = 4095;
    }
    SendvaltoDAC(pitchDAC->DACnum, voltage);
  }
/*if (voltage == 0) {
  return; 
}*/
  if (velDAC != NULL) {
    voltage2 = velDAC->linealConvert(plvelocity);
    if (voltage2 > 4095) {
      voltage2 = 4095;
    }
    SendvaltoDAC(velDAC->DACnum, voltage2);
  }

  if (LearnMode == NORMALMODE) {
    //Retrig handle only in normalmode
    if (plvelocity == 0) { //NOTE OFF
      if ( !GS.VoiceOverlap ) {
        Retrig[gatePin].NoteOffTrig = true;
      }
      playNoteOff();
    } else if (gatePin != -1) {
      if ( !GS.VoiceOverlap && !IsPolyMode(GS.VoiceMode) && plvelocity > 0 && LastVel > 0 && note != LastNote && !Retrig[gatePin].DoRetrig) {
        Retrig[gatePin].DoRetrig = true;
      } else {
        digitalWrite(gatePin, HIGH);
      }
    }
  } else {
    if (plvelocity == 0) { //NOTE OFF
      playNoteOff();
    } else if (gatePin != -1) {
      digitalWrite(gatePin, HIGH);
    }
  }

  LastNote = note;
  LastVel = plvelocity;

}

// All Notes off
void MIDICV::playNoteOff(void)
{
  if (gatePin != -1) {
    if (LearnMode == NORMALMODE) {
      Retrig[gatePin].NoteOffTrig = true;
      //RetrigProcess();
    }
    digitalWrite(gatePin, LOW);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//Learn Cycles
/////////////////////////////////////////////////////////////////////////////////////////////
// Note on received in Learn Mode
// Capture channel and new minimum note
void MIDICV::LearnThis(byte channel, byte pitch, byte velocity)
{
  bool channelExists = false;
  bool endLearn      = false;
  if (pitchDAC == NULL) {
    return;
  }
#ifdef PRINTDEBUG
  Serial.print(channel);
  Serial.print(" Channel. New min Input: ");
  Serial.println(pitch);
  Serial.print(LearnStep);
  Serial.println(" Step, End Learn Mode");
#endif
  LearnInitTime = millis();
  notes.clear();
  LearnStep++;
  if (ChannelExists(channel, pitch, LearnStep)) {
    //Bad learning --> same step until OK
    BlinkKO();
    LearnStep--;
    channelExists = true;
  }
  Blink.setBlink(0, 0, 0);
  switch (LearnStep) {
    case 1:
      if (!channelExists) {
        //Ok blink
        delay(200);
        Blink.setBlink(100, 1, 1, PINGATE);
        delay(200);
        Blink.setBlink(0, 0, 0);
      }
    
      if (GS.VoiceMode == MONOMIDI || IsPolyMode(GS.VoiceMode)) {
        endLearn = true;  //Only one voice, one channel --> end 
      } else {
        Blink.setBlink(100, 0, -1, PINGATE2); //Next channel to learn
      }

      break;
    case 2:
      if (!channelExists) {
        //Ok blink
        delay(200);
        Blink.setBlink(100, 1, 1, PINGATE2);
        delay(200);
        Blink.setBlink(0, 0, 0);
      }
      if (GS.VoiceMode == DUALMIDI) {
        endLearn = true; //Dual voice = 2 channels --> end 
      } else {
        Blink.setBlink(100, 0, -1, PINGATE3); //Next Channel to learn
      }
      break;
    case 3:
      if (!channelExists) {
        //OK blink
        delay(200);
        Blink.setBlink(100, 1, 1, PINGATE3);
        delay(200);
        Blink.setBlink(0, 0, 0);
      }
      Blink.setBlink(100, 0, -1, PINGATE4); //Next Channel to learn
      break;
    case 4:

      // Turn off LED
      delay(200);
      Blink.setBlink(100, 1, 1, PINGATE4);
      delay(200);
      Blink.setBlink(0, 0, 0);
      Blink.setBlink(0, 0, 0, PINLED);
      if (GS.VoiceMode == QUADMIDI) {
        endLearn = true;  //Quad mode = 4 channels --> end
      }
      break;
  }
  if (channelExists) {
    return;
  }

  SetLearn(channel, pitch);

  if (endLearn == true) {
    ConfirmLearnMode();
  }
}

bool MIDICV::ChannelExists(byte channel, byte mininput, byte learnstep) {
  int i;

  for (int i = (learnstep - 2); i > -1; i--) {
    if (Voice[i].midiChannel == channel && Voice[i].pitchDAC->minInput <= mininput) {
      //new min notes in same channel have to be lower than old ones
      return true;
    }
  }
  return false;
}

//Backup learn
void MIDICV::SetBckLearn(byte channel, byte mininput) {
  midiChannel_bck = channel;
  minInput_bck    = mininput;
}
//Current Learn for recovery
void MIDICV::SetCurrentLearn(byte channel, byte mininput) {
  midiChannel_current = channel;
  minInput_current    = mininput;
}
//Set active learn
void MIDICV::SetLearn(byte channel, byte mininput) {
  midiChannel           = channel;
  pitchDAC->minInput    = mininput;
}
//Reset Current learn
void MIDICV::ReSetCurrentLearn() {
  SetCurrentLearn(0, 0);
}
//Backup learn
void MIDICV::BckUpLearn() {
  SetBckLearn(midiChannel, pitchDAC->minInput);
}
//Recovery Backup learn 
void MIDICV::RecoveryLearn() {
  SetLearn(midiChannel_bck, minInput_bck);
}
//Recover active learn
void MIDICV::ResetLearn() {        
  SetLearn(midiChannel_current, minInput_current);
}

void MIDICV::InitLearn() {
  SetCurrentLearn(midiChannel, pitchDAC->minInput);
}

///////////////////////////////////
// Support functions for MIDI class

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

// Turn off all notes
void AllNotesOff(void)
{
  for (int i = 0; i < GS.NumVoices; i++) {
    Voice[i].playNoteOff();
    Voice[i].notes.clear();
  }
}

// Read MIDI properties from eeprom
int ReadMIDIeeprom(void)
{
  //return -1;
  MIDICV tempMIDI;
  MultiPointConv tempConv;
  GENERALSETTINGS lGS;
  int i, offset = 0;
  
  eeprom_read_block((void *)&lGS, (void *)offset, sizeof(lGS));
  offset = offset + sizeof(lGS);
  
#ifdef PRINTDEBUG
  Serial.print("Read EEPROM Num Voices");
  Serial.print(numVoices);
  Serial.print(" / Voice Mode: ");
  Serial.println(voiceMode);
#endif
  if (lGS.NumVoices < 1 || lGS.NumVoices > 4) {
    return (-1);
  }
  if (lGS.VoiceMode <= MIDIMODE_INVALID || lGS.VoiceMode >= MIDIMODE_LAST) {
    return (-1);
  }

  for (i = 0; i < 4; i++) {
    eeprom_read_block((void *)&tempConv, (void *)offset, sizeof(tempConv));
    offset = offset + sizeof(tempConv);
    // stored in Pitch Only if EEPROM values for rangeDAC are correct
    if (i == 0) {
      if (tempConv.rangeDAC == 4093) {
        memcpy(&DACConv[i], &tempConv, sizeof(DACConv[i]));
      } else {
        return (-1);
      }
    } else {
      memcpy(&DACConv[i], &tempConv, sizeof(DACConv[i]));
    }
    eeprom_read_block((void *)&tempMIDI, (void *)offset, sizeof(tempMIDI));
    offset = offset + sizeof(tempMIDI);
    memcpy(&Voice[i], &tempMIDI, sizeof(Voice[i]));
    
    /*   #ifdef PRINTDEBUG
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
  
  GS.NumVoices = lGS.NumVoices;
  GS.VoiceMode = lGS.VoiceMode;
  SetOverlap(lGS.VoiceOverlap);
  SetPpqnClock(lGS.PpqnCLOCK);
  SetClockMode(lGS.ClockMode);
  SetSTSPMode(lGS.StSpMode);
  SetRoundRobin(lGS.RoundRobin);
  return (GS.NumVoices);
}

// Write MIDI properties from eeprom
void WriteMIDIeeprom(void)
{
  //return;
  int i, offset = 0;
#ifdef PRINTDEBUG
  Serial.println("Write MIDI");
  Serial.print("Num Voices: ");
  Serial.print(GS.NumVoices);
  Serial.print(" / Voice Mode: ");
  Serial.println(GS.VoiceMode);
#endif

  // Store in EEPROM number of channels and MIDI mode
  eeprom_write_block((void *)&GS, (void *)offset, sizeof(GS));

  offset = offset + sizeof(GS);
  
  // Store DAC channels and MIDI config
  for (i = 0; i < 4; i++) {
    if (i == 0) {
      DACConv[i].rangeDAC = 4093;
    }
    //offset =  offset + i * (sizeof(DACConv[i]) + sizeof(Voice[i]));
    eeprom_write_block((const void *)&DACConv[i], (void *)offset, sizeof(DACConv[i]));

    //offset = offset + sizeof(DACConv[i]) + i * (sizeof(Voice[i]) + sizeof(DACConv[i]));
    offset = offset + sizeof(DACConv[i]);
    eeprom_write_block((const void *)&Voice[i], (void *)offset, sizeof(Voice[i]));

    offset = offset + sizeof(Voice[i]);
    /*
       #ifdef PRINTDEBUG
       Serial.print("DACConv addr: ");
       Serial.print((sizeof(GS.NumVoices)*2+i*(sizeof(DACConv[i])+sizeof(Voice[i]))));
       Serial.print(" /2: ");
       Serial.println((sizeof(GS.NumVoices)*2+sizeof(DACConv[i])+i*(sizeof(Voice[i])+sizeof(DACConv[i]))));
       Serial.print("DACConv 1: ");
       Serial.print(DACConv[i].DACPoints[1]);
       Serial.print(" /2: ");
       Serial.println(DACConv[i].DACPoints[2]);
       #endif
    */
  }
}

//Retrig => Overlap=false
void SetOverlap(bool overlap) {
  GS.VoiceOverlap = overlap;
}

//Set clock resolution
void SetPpqnClock(int ppqnClock){
  GS.PpqnCLOCK = ppqnClock;
  trigCLOCK = ( GS.PpqnCLOCK * clockFactor );
}
//Set Clock mode
void SetClockMode(int mode){
  GS.ClockMode = mode;
}
//Set Start/Stop Mode
void SetSTSPMode(int mode){
  GS.StSpMode = mode;
}

//Save current VoiceMode
void SetCurrentVoiceMode()
{
  GS.CurrentVoiceMode = GS.VoiceMode;
}

void ResetToCurrentVoiceMode(void)
{
  SetVoiceMode(GS.CurrentVoiceMode);
}

// Set default MIDI properties manually
void SetVoiceMode(int mode)
{
  if (GS.VoiceMode >= MIDIMODE_LAST) return;

  GS.VoiceMode = mode;
  // Reset pointers
  for (int i = 0; i < 4; i++) {
    Voice[i].pitchDAC = NULL;
    Voice[i].velDAC = NULL;
    Voice[i].bendDAC = NULL;
    Voice[i].modDAC = NULL;
    Voice[i].gatePin = -1;
    Voice[i].midiChannel = 0;
  }
  Selector.clear();

  switch (mode) {
    case MONOMIDI:
      GS.NumVoices = 1;
      Voice[0].pitchDAC = &DACConv[0];
      Voice[0].velDAC = &DACConv[1];
      Voice[0].bendDAC = &DACConv[2];
      Voice[0].modDAC = &DACConv[3];
      Voice[0].gatePin = PINGATE;
      Voice[0].midiChannel = 1;
      // Bend hava a range of -8192 (min bend) to 8191 (max bend) and output 0.5 V
      Voice[0].bendDAC->rangeInput = 16383;
      Voice[0].bendDAC->minInput = -8192;
      Voice[0].bendDAC->rangeDAC = 68;
      Voice[0].modDAC->rangeInput = 127;
      break;
    case DUALMIDI:
      GS.NumVoices = 2;
      Voice[0].pitchDAC = &DACConv[0];
      Voice[0].velDAC = &DACConv[1];
      Voice[0].gatePin = PINGATE;
      Voice[0].midiChannel = 1;
      Voice[1].pitchDAC = &DACConv[2];
      Voice[1].velDAC = &DACConv[3];
      Voice[1].gatePin = PINGATE3;
      Voice[1].midiChannel = 2;
      break;
    case QUADMIDI:
      GS.NumVoices = 4;
      Voice[0].pitchDAC = &DACConv[0];
      Voice[0].gatePin = PINGATE;
      Voice[0].midiChannel = 1;
      Voice[1].pitchDAC = &DACConv[1];
      Voice[1].gatePin = PINGATE2;
      Voice[1].midiChannel = 2;
      Voice[2].pitchDAC = &DACConv[2];
      Voice[2].gatePin = PINGATE3;
      Voice[2].midiChannel = 3;
      Voice[3].pitchDAC = &DACConv[3];
      Voice[3].gatePin = PINGATE4;
      Voice[3].midiChannel = 4;
      break;
    case DUOFIRST:
    case DUOLAST:
    case DUOHIGH:
    case DUOLOW:
      GS.NumVoices = 2;
      Voice[0].pitchDAC = &DACConv[0];
      Voice[0].velDAC = &DACConv[1];
      Voice[0].gatePin = PINGATE;
      Voice[0].midiChannel = 1;
      Voice[1].pitchDAC = &DACConv[2];
      Voice[1].velDAC = &DACConv[3];
      Voice[1].gatePin = PINGATE3;
      Voice[1].midiChannel = 1;
      break;
    case POLYFIRST:
    case POLYLAST:
    case POLYHIGH:
    case POLYLOW:
      GS.NumVoices = 4;
      Voice[0].pitchDAC = &DACConv[0];
      Voice[0].gatePin = PINGATE;
      Voice[0].midiChannel = 1;
      Voice[1].pitchDAC = &DACConv[1];
      Voice[1].gatePin = PINGATE2;
      Voice[1].midiChannel = 1;
      Voice[2].pitchDAC = &DACConv[2];
      Voice[2].gatePin = PINGATE3;
      Voice[2].midiChannel = 1;
      Voice[3].pitchDAC = &DACConv[3];
      Voice[3].gatePin = PINGATE4;
      Voice[3].midiChannel = 1;
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
}

void SetRoundRobin(bool roundRobin) {
  GS.RoundRobin = roundRobin;
}
