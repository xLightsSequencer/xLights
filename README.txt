xLights is a program that allows you to play the sequence files from Vixen and
LOR. It has the ability to drive USB DMX controllers, E1.31 ethernet
controllers.

xLights has a extensive scheduler.

Nutcracker is a program that generates animated effects on massive RGB devices
such as megatrees, matrices, and arches. 


NUTCRACKER RELEASES
-------------------
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
0006  Open    3.0.10   If using Effect 1 and you select on Effect 2, it changes Effect 1.
  








