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

// Time on, time off, number of times to blink
// times = -1 continuous
void Blinker::setBlink(unsigned long pon, unsigned long poff, int times, int newpin)
{
  if (newpin > 0) {
    pinLED = newpin;
  }

  countBlinks = times;
  // Turn off blinker
  if (times == 0) {
    if (pinLED > 0) {
      if (pinLED < 128) {
        digitalWrite(pinLED, LOW);
      } else {
        SendvaltoDAC(pinLED - 128, 0);
      }
    }
    status = LOW;
    return;
  }

  periodon = pon;
  periodoff = poff;
  initblink = millis();
  status = HIGH;
  if (pinLED > 0) {
    if (pinLED < 128) {
      digitalWrite(pinLED, HIGH);
    } else {
      SendvaltoDAC(pinLED - 128, 4095);
    }
  }
}

// Cyclic run of blink
void Blinker::playBlink(void)
{
  // No blink
  if (countBlinks == 0) {
    return;
  }

  // Get current time
  unsigned long current = millis();
  // Check end of period on
  if (status == HIGH) {
    if (current > initblink + periodon) {
      // End of period on
      initblink = current;
      status = LOW;
      if (pinLED > 0) {
        if (pinLED < 128) {
          digitalWrite(pinLED, LOW);
        } else {
          SendvaltoDAC(pinLED - 128, 0);
        }
      }
    }
  }
  // Check end of period off
  else if (current > initblink + periodoff) {
    // End of period off
    initblink = current;
    // Decrease counter
    if (countBlinks > 0) {
      countBlinks--;
    }
    // If not zero, init next cycle
    if (countBlinks != 0) {
      status = HIGH;
      if (pinLED > 0) {
        if (pinLED < 128) {
          digitalWrite(pinLED, HIGH);
        } else {
          SendvaltoDAC(pinLED - 128, 4095);
        }
      }
    }
  }
}
