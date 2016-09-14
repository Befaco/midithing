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
// bitfield implementation adapted from
// http://stackoverflow.com/questions/1590893/error-trying-to-define-a-1-024-bit-128-byte-bit-field

#include <limits.h>
#include "firmware.h"
#include "MultiPointConv.h"
#include "BitField128.h"

void AllNotesOff(void);
int ReadMIDIeeprom(void);
void WriteMIDIeeprom(void);
void SetModeMIDI(int mode);
// Learn mode
void DoLearnCycle(void);
void EnterLearnMode(void);
// Calibration mode
void DoCalCycle(void);
void EnterCalMode(void);
void EndCalMode(void);
byte CalProcessNote(byte channel, byte pitch, byte velocity);
int PercussionNoteGate(byte pitch);

//MIDI
class MIDICV;
extern MIDICV ChanMIDI[4]; // Define up to four MIDI channels
extern int MAXNumMIDI; // Number of MIDI channels in use
extern int MIDImode; // MIDI mode Set to quad mode by default

#define MAXNOTES 8

static bool IsPolyMode() {
  return (MIDImode == POLYFIRST
          || MIDImode == POLYLAST
          || MIDImode == POLYHIGH
          || MIDImode == POLYLOW);
}

static bool IsPercMode() {
  return (MIDImode == PERCTRIG || MIDImode == PERCGATE);
}

struct NoteEvent {
public:
  byte pitch;
  byte velocity;
};

class NoteEventInfo {
public:
  bool isPlaying(byte pitch) {
    return isbitset128(noteField_, (size_t)pitch);
  }

  byte setPlaying(byte pitch, byte velocity) {
    if (velocity) {
      if (!isbitset128(noteField_, (size_t)pitch)) {
        setbit128(noteField_, (size_t)pitch);
        events_[count_++] = { pitch, velocity };
      }
    }
    else {
      if (isbitset128(noteField_, (size_t)pitch)) {
        unsetbit128(noteField_, (size_t)pitch);
        // Remove note from the list
        for (int i = 0; i < count_; i++) {
          if (events_[i].pitch == pitch) {
            while (i < count_) {
              events_[i] = events_[i + 1];
              i++;
            }
            break;
          }
        }
        count_--;
      }
    }
    return count_;
  }

  byte setPlayingFront(byte pitch, byte velocity) {
    if (velocity) {
      if (!isbitset128(noteField_, (size_t)pitch)) {
        setbit128(noteField_, (size_t)pitch);
        for (int i = count_ - 1; i >= 0; i--) {
          events_[i + 1] = events_[i];
        }
        events_[0] = { pitch, velocity };
        count_++;
      }
      return count_;
    }
    else return setPlaying(pitch, velocity);
  }

  byte getCount() { return count_; }

  bool getEvent(byte idx, byte* pitch, byte* velocity = nullptr) {
    if (!count_ || idx >= count_) {
      *pitch = 0;
      if (velocity) *velocity = 0;
      return false;
    }
    *pitch = events_[idx].pitch;
    if (velocity) *velocity = events_[idx].velocity;
    return true;
  }

  bool getLastEvent(byte* pitch, byte* velocity = nullptr) {
    return getEvent(count_ - 1, pitch, velocity);
  }

  void clear() {
    count_ = 0;
    clearbitset128(noteField_);
  }

private:
  byte count_ = 0;
  BitField128 noteField_ = {0}; // more efficient than iterating list
  NoteEvent events_[MAXNOTES] = {};
};

class MIDICV {

//////////////////////////////////////////////
//Variables
public:
  // Var MIDI
  byte midiChannel = 1; // Initial channel
  byte pinGATE = PINGATE;
  NoteEventInfo notes;
  // Var MIDI-DAC
  class MultiPointConv *PitchDAC, *VelDAC, *BendDAC, *ModulDAC;

//////////////////////////////////////////////
// Function declaration
  MIDICV() {}

  void ProcessNoteOn(byte pitch, byte velocity);
  void ProcessNoteOff(byte pitch, byte velocity);
  void ProcessBend(int bend);
  void ProcessModul(byte value);
#ifdef PRINTDEBUG
  void PrintNotes(void);
#endif
  byte CheckRepeat(byte pitch);
  void playNote(byte note, byte plvelocity);
  void playNoteOff(void);
  void LearnThis(byte channel, byte pitch, byte velocity);
  bool isPlayingNote(byte pitch) { return notes.isPlaying(pitch); }
};

class VoiceSelector {
public:

  void addToPlaying(byte pitch)
  {
    playing_.setPlaying(pitch, 64);
  }

  void removeFromPlaying(byte pitch)
  {
    playing_.setPlaying(pitch, 0);
  }

  void clear()
  {
    playing_.clear();
    pool_.clear();
  }

  bool popNextNoteFromPool(byte* pitch, byte* velocity = nullptr)
  {
    int poolCount = pool_.getCount();
    byte nextNote = 0;

    *pitch = 0;
    if (velocity) *velocity = 0;

    if (!poolCount) return false;
    switch (MIDImode) {
      case POLYFIRST:
        pool_.getEvent(0, pitch);
        removeFromPool(*pitch);
        return true;
        break;
      case POLYLAST:
        pool_.getLastEvent(pitch);
        removeFromPool(*pitch);
        return true;
        break;
      case POLYHIGH:
        nextNote = 0;
        for (int i = 0; i < poolCount; i++) {
          byte testNote;
          pool_.getEvent(i, &testNote);
          if (testNote > nextNote) {
            nextNote = testNote;
          }
        }
        *pitch = nextNote;
        removeFromPool(*pitch);
        return true;
        break;
      case POLYLOW:
        nextNote = 127;
        for (int i = 0; i < poolCount; i++) {
          byte testNote;
          pool_.getEvent(i, &testNote);
          if (testNote < nextNote) {
            nextNote = testNote;
          }
        }
        *pitch = nextNote;
        removeFromPool(*pitch);
        return true;
        break;
      default:
        return false;
    }
    return false;
  }

  // Check if received channel is any active MIDI
  int getTargetChannel(byte channel, byte pitch = 0, byte velocity = 0)
  {
    // If in percussion mode, check the note and return the associated gate
    if (IsPercMode() && channel == 10) {
      return (PercussionNoteGate(pitch));
    }

    if (IsPolyMode()) {
      return getPolyTargetChannel(channel, pitch, velocity);
    }

    // In other modes, check if received one is an active channel
    for (int i = 0; i < MAXNumMIDI; i++) {
      if (ChanMIDI[i].midiChannel == channel && ChanMIDI[i].PitchDAC->minInput < pitch) {
        return (i);
      }
    }
    return (-1);
  }

private:

  void addToPool(byte pitch)
  {
    pool_.setPlaying(pitch, 64);
  }

  void addToPoolFront(byte pitch)
  {
    pool_.setPlayingFront(pitch, 64);
  }

  void removeFromPool(byte pitch)
  {
    pool_.setPlaying(pitch, 0);
  }

  int getPolyLowTarget(byte channel, byte pitch, byte velocity)
  {
    byte highestPitch = 0;
    byte highestChannel = 0;

    for (int i = 0; i < MAXNumMIDI; i++) {
      if (ChanMIDI[i].notes.getCount() && ChanMIDI[i].midiChannel == channel) {
        byte testPitch;
        ChanMIDI[i].notes.getLastEvent(&testPitch); // should only be one
        if (testPitch > highestPitch) {
          highestPitch = testPitch;
          highestChannel = i;
        }
      }
    }

    if (pitch > highestPitch) {
      addToPool(pitch);
      return -1;
    }

    if (pitch < highestPitch) {
      ChanMIDI[highestChannel].ProcessNoteOff(highestPitch, 0); // TODO: verify that there's only one note per channel in this mode
      addToPool(highestPitch);
      playing_.setPlaying(highestPitch, 0);
      return highestChannel;
    }

    addToPool(pitch);
    return -1; // something went wrong
  }

  int getPolyHighTarget(byte channel, byte pitch, byte velocity)
  {
    byte lowestPitch = 127;
    byte lowestChannel = 0;

    for (int i = 0; i < MAXNumMIDI; i++) {
      if (ChanMIDI[i].notes.getCount() && ChanMIDI[i].midiChannel == channel) {
        byte testPitch;
        ChanMIDI[i].notes.getLastEvent(&testPitch); // should only be one
        if (testPitch < lowestPitch) {
          lowestPitch = testPitch;
          lowestChannel = i;
        }
      }
    }

    if (pitch < lowestPitch) {
      addToPool(pitch);
      return -1;
    }

    if (pitch > lowestPitch) {
      ChanMIDI[lowestChannel].ProcessNoteOff(lowestPitch, 0); // TODO: verify that there's only one note per channel in this mode
      addToPool(lowestPitch);
      playing_.setPlaying(lowestPitch, 0);
      return lowestChannel;
    }

    addToPool(pitch);
    return -1; // something went wrong
  }

  int getPolyLastTarget(byte channel, byte pitch, byte velocity)
  {
    // get playing_[0]
    // find the channel playing that pitch
    // send a noteoff to that channel
    // move playing_[0] to pool_ (it will be the newest note in pool_)
    // shift playing_ down
    // return the channel
    byte oldestPitch;
    byte oldestChannel = -1;

    if (!playing_.getEvent(0, &oldestPitch)) {
      // something went wrong, but let's add it to the pool
      addToPool(pitch);
      return -1;
    }

    for (int i = 0; i < MAXNumMIDI; i++) {
      if (ChanMIDI[i].midiChannel == channel) {
        byte testPitch;
        ChanMIDI[i].notes.getLastEvent(&testPitch); // should only be one
        if (testPitch == oldestPitch) {
          oldestChannel = i;
          break;
        }
      }
    }

    if (oldestChannel >= 0) {
      ChanMIDI[oldestChannel].ProcessNoteOff(oldestPitch, 0); // TODO: verify that there's only one note per channel in this mode
      addToPoolFront(oldestPitch);
      playing_.setPlaying(oldestPitch, 0);
      return oldestChannel;
    }

    addToPool(pitch);
    return -1; // something went wrong
  }

  int getPolyTargetChannel(byte channel, byte pitch, byte velocity)
  {
    // first check all channels for the note
    for (int i = 0; i < MAXNumMIDI; i++) {
      if (ChanMIDI[i].midiChannel == channel && ChanMIDI[i].isPlayingNote(pitch)) {
        return i;
      }
    }

    // then look for an empty channel
    for (int i = 0; i < MAXNumMIDI; i++) {
      if (ChanMIDI[i].midiChannel == channel && !ChanMIDI[i].notes.getCount()) {
        return i;
      }
    }

    if (!velocity) {
      removeFromPool(pitch); //cleanup
      return -1;
    }

    switch (MIDImode) {
      case POLYFIRST: // the last-played note is added to the pool, available only if a channel is freed
        addToPool(pitch);
        return -1;
        break;
      case POLYLAST: // the last-played note replaces the oldest playing note
        return getPolyLastTarget(channel, pitch, velocity);
        break;
      case POLYHIGH:
        return getPolyHighTarget(channel, pitch, velocity);
        break;
      case POLYLOW:
        return getPolyLowTarget(channel, pitch, velocity);
        break;
      default: return -1;
    }
    // something went wrong
    return -1;
  }

  NoteEventInfo playing_;
  NoteEventInfo pool_;
};

extern VoiceSelector Selector;

