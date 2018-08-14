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

// Make conversion interpolating between points
unsigned int MultiPointConv::intervalConvert(int inp)
{
  int val = inp - minInput; // Adjust learnt zero value
  if (val < 0) {
    val = 0;
  }
  if (val > 119) {
    val = 119;  // max 10 oct. = 120 notes
  }

  // Look for interval
  int interv = val / 6;
  if (interv > 19) {
    interv = 19;
  }
  int A = DACPoints[interv];   //low output interpolation point
  int B = DACPoints[interv + 1]; //high output interpolation point

  // get value
  unsigned int outp = A + (val - interv * 6) * (B - A) / 6;
  if (outp < 0) {
    outp = 0;
  } else if (outp > 4095) {
    outp = 4095;
  }
#ifdef PRINTDEBUG
  Serial.print("Int: ");
  Serial.print(interv);
  Serial.print(" Low Int: ");
  Serial.print(A);
  Serial.print(" High Int: ");
  Serial.println(B);

  Serial.print("Note Input: ");
  Serial.print(val);
  Serial.print("/");
  Serial.print(inp);
  Serial.print(" DAC Output: ");
  Serial.println(outp);
#endif
  return (outp);
}

byte MultiPointConv::Processnote(byte channel, byte pitch, byte velocity)
{
  LearnInitTime = millis(); // Reset calibration counter
  int val = pitch - minInput; // Adjust learnt zero value
  if (val < 0) {
    return (0);
  }
  
  if (val > 119) {
    val = 119;  // max 10 oct. = 120 notes
  }

  if (val == 1) {
    return (0);
  }

  // Look for interval
  int interv = val / 6;
  if (interv > 19) {
    return (0);
  }

  if (val == (interv + 1) * 6 - 1) { //decrease
    DACPoints[interv + 1]--;
  } else if (val == interv * 6 + 1) { //increase
    DACPoints[interv]++;
  } else {
    return (0);
  }

#ifdef PRINTDEBUG
  Serial.print("Note: ");
  Serial.print(val);
  Serial.print("Int: ");
  Serial.print(interv);
  Serial.print(" Note: ");
  Serial.print(pitch);
  Serial.print(" DAC New value: ");
  Serial.println(DACPoints[interv]);
#endif
  return (1);
}
