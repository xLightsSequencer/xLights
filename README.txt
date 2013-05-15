xLights is a program that allows you to play the sequence files from Vixen and
LOR. It has the ability to drive USB DMX controllers, E1.31 ethernet
controllers.

xLights has a extensive scheduler.

Nutcracker is a program that generates animated effects on massive RGB devices
such as megatrees, matrices, and arches. 


NUTCRACKER RELEASES
-------------------
3.0.10: May 12,2013 - Changed 32 character limit in the Text effect to be 256 characters
3.0.9: May 11,2013 - Animated Gifs added to the Pictures effect
3.0.8: Apr 25,2013 - Added export for LSP to create UserPatterns.xml
                     The actual file created will have a *.user extension. rename it to use it
				   - New Effect, Fireworks!. Dave Pitts coded up this new effect. 
3.0.7: Apr 23,2013 - Fixed Pallet 3 button. It was not picking up color when it was clicked.
                   - Fixed export of Meteors. This effected all sequencers
				   - Fixed export of Life. This effected all sequencers
				   - Fixed export of Snowstorm. This effected all sequencers
3.0.6: Mar 11,2013 - Corrected another bug in LOR lcb export.
                   - Modifed Nutcracker tab to keep a minimum size screen on
                     left window.
                   - Added color selection based on distance from center of
                     object for Spirograph.
                   - Added the ability to animate the d parameter in spirograph
                   - Moved all effects into their own source file.
                     RGBEffects.cpp is now smaller and there are 20 new cpp
                     files. RenderBars.cpp, RenderMeteors.cpp, .etc.
3.0.5: Mar 08,2013 - Corrected color swap in lcb files. Corrected bug in HLS
                     export.
3.0.4: Mar 04,2013 - Added new effects, Tree and Spirograph.
3.0.3: Mar 02,2013 - Added new effect, Twinkle.
3.0.2: Feb 28,2013 - Corrected unsigned 24bit value for HLS *.hlsnc export.
3.0.1: Feb 26,2013 - Added wxGauge to show process for exporting sequence files.
3.0.0: Feb 22,2013 - Initial Release. This is Matt Browns original porting of
                     Nutcracker into xLights.

KNOWN BUGS:
-----------
Bug#  Status  Version  Description
0001  Closed  3.0.7    Spirals,Life or Snowstorm effects are blank on any exported file
0002  Closed  3.0.7    HLS export file is jumbled
0003  Closed  3.0.4    Vixen VIR files has RED and BLUE swapped
0004  Closed  3.0.5    LCB colors swapped.
0005  Closed  3.0.7    On effect 1, Palette 3 button is not being picked up.








