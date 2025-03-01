# SI4732 (ESP32) Mini Receiver
Firmware: **ATS_MINI**

IMPORTANT - [Select Link](https://github.com/G8PTN/ATS_MINI/issues/1#issue-2889033644)

This firmware is for use on the SI4732 (ESP32-S3) Mini/Pocket Receiver.
Based on the following sources.
- Ralph Xavier: https://github.com/ralphxavier/SI4735
- PU2CLR, Ricardo: https://github.com/pu2clr/SI4735
- Goshante: https://github.com/goshante/ats20_ats_ex

G8PTN, Dave (2025)

The main changes from the Ralph Xavier firmware are summarised below.

## User Interface
- Removed the frequency scale.
- Set "Volume" as the default adjustment parameter. 
- Modified the S-Meter size and added labels. 
- All actions now use a single press of the rotary encoder button, with a 10s timeout. 
- Added status bar with indicators for SCREEN updates and EEPROM write activity. 
- Added unit labels for "Step" and "BW".

## Functions

 - Added a VFO/BFO tuning mechanism for SSB based on the Goshante ATS20_EX firmware. This allows "chuff" free tuning over a frequency span of approximately 30kHz. 
 - Modified the audio mute behaviour. The "Mute" option now toggles the volume level between 0 and previous value. Unmute using the "Mute" or "Volume" option.
 - Modified the battery meter function. Uses set voltages for 25%, 50% and 75% with a configurable hysteresis voltage. Added voltage reading to the status bar.
 - Added "Brightness" menu option. This controls a PWM output driving the display backlight. The range is 32 to 255 (full on) in steps of 32.
 - Added "Calibration" menu option. This allows offsets in the SI4732 reference clock to be compensated. The function adds an offset to the BFO configuration used by the SI4732-A10, it does not adjust the reference oscillator. The required offset will depend on the tuned frequency, and therefore it is stored per band on later versions.
 - Added Automatic Volume Control (AVC) menu option. This allows the maximum audio gain to be adjusted.
 - Settings for AGC/ATTN, SoftMute and AVC are stored in EEPROM per mode.
 - Added "Remote" serial interface for debug control and monitoring.

### YouTube Video
Please see the excellent video by Paul (OM0ET) [Select Link](https://www.youtube.com/watch?v=BzrOE9BFpyU)


### Information
The pdf document in the zip file contained in the FIRMWARE folder provides additional information.

