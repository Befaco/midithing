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
typedef byte BitField128[16]; // 128 bits

static inline void setbit128(BitField128 field, size_t idx)
{
  field[idx / CHAR_BIT] |= 1u << (idx % CHAR_BIT);
}

static inline void unsetbit128(BitField128 field, size_t idx)
{
  field[idx / CHAR_BIT] &= ~(1u << (idx % CHAR_BIT));
}

static inline void togglebit128(BitField128 field, size_t idx)
{
  field[idx / CHAR_BIT] ^= 1u << (idx % CHAR_BIT);
}

static inline bool isbitset128(BitField128 field, size_t idx)
{
  return field[idx / CHAR_BIT] & (1u << (idx % CHAR_BIT));
}

static inline void clearbitset128(BitField128 field)
{
  memset(field, 0, sizeof(BitField128));
}

#define MAXNOTES 8

struct NoteEvent {
public:
  byte pitch;
  byte velocity;
};

struct NoteEventInfo {
  bool isPlaying(byte pitch) {
    return isbitset128(noteField_, (size_t)pitch);
  }

  byte setPlaying(byte pitch, byte velocity) {
    if (velocity) {
      setbit128(noteField_, (size_t)pitch);
      events_[count_++] = { pitch, velocity };
    }
    else {
      unsetbit128(noteField_, (size_t)pitch);
      // Remove note from the list and play the last active note
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
    return count_;
  }

  byte getCount() { return count_; }

  bool getEvent(byte idx, byte* pitch, byte* velocity) {
    if (!count_) {
      *pitch = 0;
      *velocity = 0;
      return false;
    }
    *pitch = events_[idx].pitch;
    *velocity = events_[idx].velocity;
    return true;
  }

  bool getLastEvent(byte* pitch, byte* velocity) {
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


class MIDICV
{
//////////////////////////////////////////////
//Variables
public:
  // Var MIDI
  byte midiChannel = 1; // Initial channel
  byte pinGATE = 2;
  NoteEventInfo notes;
  // Var MIDI-DAC
  class MultiPointConv *PitchDAC, *VelDAC, *BendDAC, *ModulDAC;

//////////////////////////////////////////////
// Function declaration
  MIDICV()
  {
  }
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
};
