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

#ifndef __MULTIPOINTCONV_H__
#define __MULTIPOINTCONV_H__

// Utility class to convert MIDI to CV Range
// Linear scale input to output (defined as minimun/range)
class RangeConv
{
  public:
    byte DACnum = 0;
    int minInput, rangeInput;
    unsigned int minDAC, rangeDAC;
    RangeConv()
    {
      // Default to 12 bits in the CV for 127 MIDI values;
      minInput = 0;
      rangeInput = 127;
      minDAC = 0;
      rangeDAC = 4095;
    }
    // Make conversion
    unsigned int linealConvert(int inp)
    {
      return (minDAC + ((long)(inp - minInput) * rangeDAC) / rangeInput);
    }
};

// Utility class to convert MIDI to CV Range
// Multi-Linear scale input to output (defined as 20 fix points and output as interpolation between each pair of fix points)
class MultiPointConv : public RangeConv
{
  public:
    int DACPoints[21];
    MultiPointConv()
    {
      // Default to 12 bits in the CV for 120 MIDI values;
      minInput = 0;
      rangeInput = 120;
      minDAC = 0;
      rangeDAC = 4095;
      for (int i = 0; i < 21; i++) {
        DACPoints[i] = i * 204.75;  //4095/20;
      }
    }
    // Make conversion
    unsigned int intervalConvert(int inp);
    byte Processnote(byte channel, byte pitch, byte velocity);
};

#endif // __MULTIPOINTCONV_H__
