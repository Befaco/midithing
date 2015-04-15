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

//# define ERRORCYCLES 30

//////////////////////////////////////////////
// Learn mode
// Learn Mode selected


// Initialize Learn Mode
void EnterLearnMode(void){
  
  // Set Learn mode flag
  LearnMode = ENTERLEARN;
  LearnStep = 0;
  // Init blinker
  blink.setBlink(900,100,-1);
  
  // All Notes off
  AllNotesOff();
  
  // Init timer
  LearnInitTime = millis();
  #ifdef PRINTDEBUG   
  Serial.println("Init Learn Mode");
  #endif	
}


// Learn Mode Cycle function
void DoLearnCycle(void){
 unsigned long current = millis(); 

 // 10 seconds learn time
  if( current > LearnInitTime + 10000){
    // Set normal mode
    LearnMode = NORMALMODE;
	// Turn off LED blink
	blink.setBlink(0,0,0);	
	// Store value in EEPROM
	WriteMIDIeeprom();
	
    #ifdef PRINTDEBUG   
    Serial.println("End Learn Mode");
    #endif
  }
}

//////////////////////////////
// Initialize Cal Mode
void EnterCalMode(void){
  // Set Learn mode flag
  LearnMode = ENTERCAL;
  // Init blinker
  blink.setBlink(500,500,-1);

  // All Notes off
  AllNotesOff();

  // Set mode to 4 channels
  SetModeMIDI(QUADMIDI);
  
  // Init timer
  LearnInitTime = millis();
  #ifdef PRINTDEBUG   
  Serial.println("Init Cal Mode");
  #endif	
}

// Cal Mode Cycle function
void DoCalCycle(void){
 unsigned long current = millis(); 

  // After 55 seconds without receiving a note, exit calibration
  if( current > LearnInitTime + 55000){
	#ifdef PRINTDEBUG   
	Serial.print(LearnInitTime);
	Serial.print(" msec ");
	Serial.println(current);
	Serial.println("Time expired for Calibration");
	#endif	
	EndCalMode();
  }
}

void EndCalMode(void)
{
    // Set normal mode
    LearnMode = NORMALMODE;
    // Store in EEPROM
    WriteMIDIeeprom();
	// Turn off LED blink
	blink.setBlink(0,0,0);
    #ifdef PRINTDEBUG   
    Serial.println("End Cal Mode");
    #endif
}


byte CalProcessNote(byte channel, byte pitch, byte velocity)
{
	if( channel <5){ // Channels 1-4 for DAC calibration 0-3
	  return DACConv[channel-1].Processnote(channel,pitch,velocity);
      }
	else if(channel ==5)
		// Channel 5 for setting modes
		switch( pitch){
			case 0:
			  SetModeMIDI(MONOMIDI);
			  return 1;
			  break;
			case 2:
			  SetModeMIDI(DUALMIDI);
			  return 1;
			  break;
			case 4:
			  SetModeMIDI(QUADMIDI);
			  return 1;
			  break;
			case 5:
			  SetModeMIDI(PERCTRIG);			
			  return 1;
			  break;
			/*case 6:
			  SetModeMIDI(PERCGATE);			
			  break;
                        */
		}
	return 0;
}
