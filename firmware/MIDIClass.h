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

#define MAXNOTES 8

struct NoteEvent {
public:
  byte pitch;
  byte velocity;
};

struct NoteRamp {
public:
  bool onoff = false;
  float goal = 0;
  float current = 0;
};

class MIDICV
{
//////////////////////////////////////////////
//Variables
public:
  // Var MIDI
  byte midiChannel = 1; // Initial channel
  BitField128 playingNotes = {0}; // more efficient than iterating list
  NoteEvent NotesOn[MAXNOTES];
  byte nNotesOn = 0; // Number of notes on
  byte pinGATE = 2;
  NoteRamp ramp;
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
  void ProcessPortaOnOff(byte value);
#ifdef PRINTDEBUG
  void PrintNotes(void);
#endif
  byte CheckRepeat(byte pitch);
  void playNote(byte note, byte plvelocity);
  void playNoteOff(void);
  void LearnThis(byte channel, byte pitch, byte velocity);
  void Run();
};
