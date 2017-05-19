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

//Class helper to blink led
class Blinker
{
  public:
    byte status = 0;
    unsigned long initblink = 0;
    unsigned long periodon, periodoff;
    int countBlinks = 0;
    byte pinLED = 0;

  public:
    Blinker(byte port)
    {
      pinLED = port;
    }
    Blinker() {}
    void setBlink(unsigned long periodon, unsigned long periodoff, int times, int newpin = -1);
    void playBlink(void);
};

extern Blinker Gates[10]; // Gates triggers

/*
   // Blink a double color two	pin LED
   class DualBlinker: public Blinker{
   public:
   byte pinLED1=0;
   byte pinLED2=0;
   public:
   DualBlinker(byte port1, byte port2) { pinLED = port1; pinLED1 = port1; pinLED2 = port2;}
   void setBlink( unsigned long periodon, unsigned long periodoff, int times, byte color);
   };
*/

