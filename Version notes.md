# MIDI Thing
Firmware for the [Befaco](http://befaco.org) Eurorack MIDI module (MIDI Thing).

- Version V2 Beta based on Polyphonic mode

# Changes in V2 Beta

- Bug fixes:
	- Learn mode min note play on next high note
	
	- Can't cancel Learn mode 
	
	- Learn mode in same channel for same min note can be done (incorrect keyboard split)

- Changes and improvements
	- Added: lightshow at power up for Voice leds, Voice gates, clock and ST/SP check (leds and signals)
	
	- Added: visual feedback when save, cancel or error action is done:
		- When save, LEARN and MODE leds blinks 
		- When cancel or error CLOCK and ST/SP leds Blinks
		
	- Added: visual feedback on Learn mode, led of active channel to be configured remains 
		on until correct midi in configuration.
		
	- Added: cancel Learn mode (undo changes) by pressing button or wait 10 seconds with no midi in
	
	- Changed: in LEARN or MENU mode, leds remain on until end mode.
	
	- Changed: in LEARN mode, only data saved if all channels are configured
	
	- Added: in LEARN mode MIDI keyboard split can be done on alternate voices but only from higher note 
		to lowest note Example:
		- LearnStep 1: Voice 1: MIDI Channel 1 from C3 to avobe
		- LearnStep 2: Voice 2: MIDI Channel 2 from C3 to avobe
		- LearnStep 3: Voice 3: MIDI Channel 1 from E2 to C3
		- LearnStep 4: Voice 4: MIDI Channel 1 from A1 to E2
 
		In this example:
		- Voice 1 can be any key
		- Voice 2 can be in any key because channel not equal to Channel of voice 1.
		- Voice 3 Can be any key lower than voice 1 because is in the same channel
		- Voice 4 Can be any key lower than Voice 3 because is in the same Channel and Voice 3 is lower than Voice 1.
		
	- Changed: in MENU mode, if another configuration is selected (From duo to poly for example) 
		ends mode if correct midi input is made.
		
	- Changed: in MENU mode, Calibration is now possible on:
		- 11 for voice 1
		- 12 for voice 2
		- 13 for voice 3
		- 14 for voice 4
		
	- Changed: in MENU mode, configuration (MIDI Mode) is now possible on every octave higher than 0 
		and can be done in every single note(depending on number of modes) on channels from 1 to 10
		
	- Changed: in MENU mode, configuration (MIDI Mode) can be done in this MIDI notes:
#			_________________________________________________
#			|Root Note 		|				|			   	|
#			|(MID note nr.)	|Musical note	|Mode selected	|
#			|_______________|_______________|_______________|
#			|			0	|		C		|	MONOMIDI	|
#			|			1	|		C#		|	DUALMIDI	|
#			|			2	|		D		|	QUADMIDI	|
#			|			3	|		D#		|	PERCTRIG	|
#			|			4	|		E		|	POLYFIRST	|
#			|			5	| 		F		|	POLYLAST	|
#			|			6	|		F#		|	POLYHIGH	|
#			|			7	|		G		|	POLYLOW		|
#			|_______________|_______________|_______________|
		

