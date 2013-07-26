xLights is a program that allows you to play the sequence files from Vixen and
LOR. It has the ability to drive USB DMX controllers, E1.31 ethernet
controllers.

xLights has a extensive scheduler.

Nutcracker is a program that generates animated effects on massive RGB devices
such as megatrees, matrices, and arches. 


NUTCRACKER RELEASES
-------------------
3.0.22: Jul 26,2013 - Enhancement: New button to randomly create effects for every row in your sequence
                    - Enhancement: Brightness slider now goes to 200% (Old value was 100%). This will allow improving movies
					- Bug: If Tree effect was selected for Effect2 and slider was all the way to left, clights crashed. fixed
					- Bug: Removed the gauge that was next to Export button. The gauge library does not work in Unix.
3.0.21: Jul 25,2013 - Enhancement: In the Model setup screen, you can now set teh starting channel for each string
                    - Enhancement: In the Model setup screen, you can now choose how the strings wrap. 3 more choices are now available
					  bottom left, upper left, upper right, bottom right
					- Enhancement: Meteor Effect class has 3 new effects
					  1) Swirl1. As meteors fall they are moved in a sine wave
					  2) Swirl2. Same as Swirl 1 excep odd strands move one way, even strands move the other
					  3) Strarfield. Meteor effect starts in center of your display and moves outwards
3.0.20: Jul 19,2013 - Enhancement: Highlight cell you are editing in so you know where "update" will apply 
                    - Bug: Opening a music file twice, the second time will not prompt for a new file name. Instead, it overwrites the first file. Fixed
					- Enhancement: Ability to read an Audacity or a XML file when creating a blank sequence from a music file
					- Enhancement: 1 is Unmask, 2 is Unmask now keeps the brightness value of the mask. This is big! Use a picture as a mask on a colorwash.
					  The new picture will be a mono tone version. Use text as a mask, the edges will feather but in a solid color. 
					  This fixes the antio-aliasing issue of text. To fix use text as effect 1, color wash as effect2. set effect 2 color what you want your text
					  to look like
					- Bug: Deleting some models caused crash, fixed.
					- Bug: If you had multiple models attached to a sequence, deleting anything other than 1st model caused crash. fixed
3.0.19: Jul 17,2013 - Bug Fix: Crash on exit fixed
                    - Bug fix: Empty export files (LOR, hls,vixen,conductor) fixed
					- Enhancement: When reading in your sequence, missing attributes will be added if your file is from pre ver 18.
					- Enhancement: Changed rotation slider in Spirals so it does ten times more rotations.
					- Bug fix: If you have two lines of text but only one color selected, both lines will be same color
	NOTE!! Please make a backup of all your xml files. This release will be fixing your sequences, protect yourself.
3.0.18: Jul 06,2013 - Added new Brightness and Comtrast sliders. Located next to Sparkle slider it allows dimming the overall effect.
                      Contrast will make brights brighter and sim colors go towards black.
3.0.17: Jul 01,2013 - Fixed text in Effect2. Added Twinkle, Tree, Spirograph and Fireworks to Effect2
3.0.16: Jun 28,2013 - New option added to the Text effect, Countdown timer.
3.0.15: Jun 27,2013 - Added check to make it so empty picture selection does not cause hundreds of pop up windows
                    - fixed LOR lms and lcb export
					- added check to make sure model's have at least 1 pixel wide and high
					-- new feature in Meteors effect. Meteors can now fall up
3.0.14: Jun 13,2013 - New movie files added to Picture Effect
                      Use web page to change your movies into jpg files. http://image.online-convert.com/convert-to-jpg
                    - Strobe in Twinkle Effect is now random lights
3.0.13: May 30,2013 - New Strobe Effect now available in the Twinkle Effect Class
                    - Twinkle Effect Class now allows different steps of twinkle.
3.0.12: May 26,2013 - New type of vertical text where letters are not rotated but stacked on
                      top of each other. Finally we have Line 1 one color, line 2 second color.
					  Each line of text can have a different font, a different direction.
					  There is a position bar for each line of text.
3.0.11: May 15,2013 - Text can now be rotated 0, 90, 180 or 270 degrees
3.0.10: May 12,2013 - Changed 32 character limit in the Text effect to be 256 characters
3.0.9:  May 11,2013 - Animated Gifs added to the Pictures effect
3.0.8:  Apr 25,2013 - Added export for LSP to create UserPatterns.xml
                      The actual file created will have a *.user extension. rename it to use it
		   		    - New Effect, Fireworks!. Dave Pitts coded up this new effect. 
3.0.7:  Apr 23,2013 - Fixed Pallet 3 button. It was not picking up color when it was clicked.
                    - Fixed export of Meteors. This effected all sequencers
			 	    - Fixed export of Life. This effected all sequencers
				    - Fixed export of Snowstorm. This effected all sequencers
3.0.6:  Mar 11,2013 - Corrected another bug in LOR lcb export.
                    - Modifed Nutcracker tab to keep a minimum size screen on
                      left window.
                    - Added color selection based on distance from center of
                      object for Spirograph.
                    - Added the ability to animate the d parameter in spirograph
                    - Moved all effects into their own source file.
                      RGBEffects.cpp is now smaller and there are 20 new cpp
                      files. RenderBars.cpp, RenderMeteors.cpp, .etc.
3.0.5:  Mar 08,2013 - Corrected color swap in lcb files. Corrected bug in HLS
                     export.
3.0.4:  Mar 04,2013 - Added new effects, Tree and Spirograph.
3.0.3:  Mar 02,2013 - Added new effect, Twinkle.
3.0.2:  Feb 28,2013 - Corrected unsigned 24bit value for HLS *.hlsnc export.
3.0.1:  Feb 26,2013 - Added wxGauge to show process for exporting sequence files.
3.0.0:  Feb 22,2013 - Initial Release. This is Matt Browns original porting of
                      Nutcracker into xLights.

KNOWN BUGS:
-----------
Bug#  Status  Version  Description
0001  Closed  3.0.7    Spirals,Life or Snowstorm effects are blank on any exported file
0002  Closed  3.0.7    HLS export file is jumbled
0003  Closed  3.0.4    Vixen VIR files has RED and BLUE swapped
0004  Closed  3.0.5    LCB colors swapped.
0005  Closed  3.0.7    On effect 1, Palette 3 button is not being picked up.
0006  Closed  3.0.10   If using Effect 1 and you select on Effect 2, it changes Effect 1.
  








