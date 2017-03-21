# MIDI Thing
Firmware for the [Befaco](http://befaco.org) Eurorack MIDI module (MIDI Thing).

- Version V2 Beta based on Polyphonic mode

# Changes in PolyXp Duo version (v2)

- Bug fixes:
	- Learn mode min note play on next high note
	
	- Can't cancel Learn mode 
	
	- Learn mode in same channel for same min note can be done (incorrect keyboard split)

- Changes and improvements
	- Added: lightshow at power up for Voice leds, Voice gates and clock check (leds and signals)
	
	- Added: visual feedback when save, cancel or error action is done:
		- When save, LEARN and MODE leds blinks 
		- When cancel or error CLOCK
		
	- Added: visual feedback on Learn mode, led of active channel to be configured remains 
		on until correct midi in configuration.
		
	- Added: cancel Learn mode (undo changes) by pressing button.
	
	- Changed: in LEARN or MENU mode, leds remain on until end mode.
	
	- Changed: in LEARN mode, only data saved if all util channels (depending on mode) are configured
	
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
	
	- Added: duophonic modes (listed in next point), where:
		- Voice 1
			Pitch 	 -> Out CV1
			Gate	 -> Out Gate1
			Velocity ->	Out CV2

		- Voice 2
			Pitch	 -> Out CV3
			Gate	 -> Out Gate3
			Velocity ->	Out CV4

		-Unused: 
			Out Gate2
			Out Gate4
		
	- Changed: in MENU mode, configuration (MIDI Mode) can be done with this MIDI notes:
.			_________________________________________________
.			|Root Note 		|				|			   	|
.			|(MID note nr.)	|Musical note	|Mode selected	|
.			|_______________|_______________|_______________|
.			|			0	|		C		|	MONOMIDI	|
.			|			1	|		C#		|	DUALMIDI	|
.			|			2	|		D		|	QUADMIDI	|
.			|			3	|		D#		|	PERCTRIG	|
.			|			4	|		E		|	DUOFIRST	|
.			|			5	| 		F		|	DUOLAST		|
.			|			6	|		F#		|	DUOHIGH		|
.			|			7	|		G		|	DUOLOW		|
.			|			8	|		G#		|	POLYFIRST	|
.			|			9	| 		A		|	POLYLAST	|
.			|			10	|		A#		|	POLYHIGH	|
.			|			11	|		B		|	POLYLOW		|
.			|_______________|_______________|_______________|

	- Added: in MENU mode, channel Overlap and non-Overlap mode (only in monophonic modes), this modes are stored in memory: 
. 		- MIDI Channel 15
.			_________________________________________________
.			|Root Note 		|				|			   	|
.			|(MID note nr.)	|Musical note	|Mode selected	|
.			|_______________|_______________|_______________|
.			|			0	|		C		|	Overlap		|
.			|			1	|		C#		|	Non- Overlap|
.			|_______________|_______________|_______________|

		- Overlap active 		- Gate is not retriggered when one note is hold and insert another note.
		- Non-Overlap active 	- Gate retriggered when one note is hold and insert another note.
	
		Overlap active
			Notes  	|note 1         |note2
			Gate   	|------------------------

		Non-Overlap active
			Notes  	|note 1         |note2
			Gate	|---------------|--------
		
		(-)gate time
		(|)Gate trig		
 
	- Added: Clock resolution (ppqn) change, this mode are stored in memory:
. 		- MIDI Channel 15
.			_____________________________________________________
.			|Root Note 		|				|			   		|
.			|(MID note nr.)	|Musical note	|Clock Res. selected|
.			|_______________|_______________|___________________|
.			|			2	|		D		|	24				|
.			|			3	|		D#		|	12				|
.			|			4	|		E		|	6				|
.			|			5	| 		F		|	4				|
.			|			6	|		F#		|	2				|
.			|			7	|		G		|	1				|
.			|			8	|		G#		|	36				|
.			|			9	| 		A		|	48				|
.			|			10	|		A#		|	52				|
.			|			11	|		B		|	96				|
.			|_______________|_______________|___________________|
	
		

