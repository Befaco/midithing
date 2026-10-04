# Changelog — Community firmware

Changes on the `community` branch, which collects community contributions on
top of the official Befaco firmware (`master`, V2.2).

Every commit to `community` adds an entry under **Unreleased**, in one of:

- **New**: a feature or option that did not exist before.
- **Changed**: existing behaviour that works differently now. Users may notice.
- **Fixed**: a bug fix with no intended change in behaviour.

On release, rename **Unreleased** to the version and date, and copy the section
into the release notes. Each entry names its commit and source (PR number and
author), so it can be traced back.

## [Unreleased]

### Changed

- **MIDI Stop now releases all held notes.** A paused sequencer never sends the
  Note Offs for the notes it was playing. Each pause left them stuck in the
  voice, and after a few pauses that output went silent until power cycle.
  Note that a note held on a keyboard is now also released when a Stop arrives.
  (`bd23727`, PR #19, @robmurru)
- **The MIDI library is now bundled with the firmware** in `firmware/src/MIDI`
  (FortySevenEffects MIDI Library 5.0.2, MIT, with the fix below). It no longer
  needs to be installed to build the firmware. Bounce2 and mcp4728 are still
  required. (`e21d560`, PR #19, @robmurru)

### Fixed

- **MIDI input locking up until power cycle.** After a truncated SysEx message
  (an F0 with no F7, e.g. from a Beatstep Pro merging USB onto DIN, or one
  corrupted byte), clock kept working but notes, CC and panic were ignored on
  all outputs. The parser now drops the broken message and resyncs.
  (`d9e03f3`, PR #19, @robmurru)
- **MIDI panic (CC 123 All Notes Off) did nothing when every voice was busy.**
  It is now handled before the voice lookup. (`c64d90b`, PR #19, @robmurru)
- **Panic and mode changes left the poly voice allocator out of sync**, which
  could later make POLYLAST/DUOLAST address an invalid voice.
  (`c5b07b5`, PR #19, @robmurru)
- **Learned MIDI channels lost after running the calibration procedure.**
  Entering calibration, starting the procedure (B on channel 2) and exiting
  reset the channels to 1-4 and saved that to EEPROM.
  (`2b81faf`, PR #19, @robmurru)
- **An output could stop responding, or CV come out of the wrong jack, after a
  firmware update.** Internal memory addresses were restored from EEPROM. Only
  the real settings are restored now. Existing EEPROM data stays compatible.
  (`3245bb8`, PR #19, @robmurru)
- **Notes on channels 3 and 4 in calibration menu mode could change settings.**
  (`0a140db`, PR #19, @robmurru)
- **Out-of-bounds write in calibration** for channels above the voice count.
  (`5f42e18`, PR #19, @robmurru)
- **POLYLAST/DUOLAST could address a non-existent voice** (unsigned comparison
  that was always true). (`f0f31ab`, PR #19, @robmurru)
- **Out-of-bounds read when removing a note from a full note list.**
  (`711868b`, PR #19, @robmurru)
- **Voice mode validation checked the current mode instead of the requested
  one.** (`46f0e01`, PR #19, @robmurru)
