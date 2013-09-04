xLights is a program that allows you to play the sequence files from Vixen and
LOR. It has the ability to drive USB DMX controllers, E1.31 Ethernet
controllers. xLights has a extensive scheduler. Nutcracker is a program that generates animated effects on massive RGB devices
such as megatrees, matrices, and arches. 

XLIGHTS/NUTCRACKER RELEASE NOTES: http://nutcracker123.com/nutcracker/releases/

3.1.4:  Sep 03,2013 - Enhancement: (Matt). Custom Model now uses pixel count 1..n. Models can be put on preview. For example, you can 
						define a candy cane out of rgb pixels and then place candycane in the preview window. Very Nice!
                    - Enhancement: (Matt) You can now copy models.
					- Enhancement: (Matt) You can now rotate single strand models in the preview window.
					- Enhancement: (Matt) There is a Sequence open button on the Preview tab. If you use this to open a sequence there is
						an option to set the "Part of my Display" checkboxes to match the models that just got opened.
3.1.3:  Aug 27,2013 - Enhancement: (Matt). New Effect Curtain.
					- Enhancement: (Matt)  New tab PREVIEW. There is a new tab that will allow you to see all of your models displaying effects.
					- Bug (Frank) Fixed channel numbering for single string and Window models
3.1.2:  Aug 17,2013 - Enhancement: (Frankr). Circle effect now has new option called plasma.
					- Enhancement: (Matt Brown).  bars compress & expand so that the two halves mirror each other.
					- Bug: (Frankr).  found and fixed the issues with fire and meteors that were being discussed at DIYC.
					- Enhancement (Dan Kulp). New import from HLS now available on CONVERT tab
					- Enhancement (Frankr). New option in spirals to allow the thickness to grow or shrink
3.1.1:  Aug 12,2013 - Bug Fix: (Matt Brown). Matt has fixed the Meteor effect. He has also enhanced it so meteors 
						can fall to the left or right.
						We have	renamed	Starfiled to be called "explode" and "implode"
					- Enhancement: (Matt Brown)  I made some improvements to countdown & skip blank cells logic. I also changed a couple of method names to make them more clear. Also, the timer now skips if the last interval takes more than 50ms to process. Before it was blocking until the last interval finished (so if every interval took more than 50ms you had a stackup of waiting timer calls to process).
3.1.0:  Aug 7,2013 	- Enhancement: (frankr) new single BACKUP option now.
						Goto to "File,Backup" and every xml file you have will be created in the new Backup sub-directory under show show directory.
						How big are xLight sequences? The xseq file for 12K channels is around 30mbytes, the xml of your sequence is around 30kbytes. As comparison the XML file for LOR for the same song is 800mbytes, for LSP 1.2gbytes. Because xlight sequence files are so small (normally < 50kbytes), we can back them all up quickly.
						How long to back up over 100 sequences? < 1 second.
						How much space does 100 sequences take? < 10 mbytes
						How many times can you do this per day? 1000's. 
						How to use this feature? Just do a "File, Backup" before getting new versions.
						I would do a copy if you plan on doing lots of edits.  Basically get yourself protected.

						So after upgrading to 3.1.0, go and make a copy before opening any sequences

					- Enhancement: (Matt Brown) re-engineered the underlying architecture. This is like replacing the galvanized pipe with copper pipe on your home. Takes lots of effort, when you are done the house still looks the same. This re-architecture was needed to allows us to offer new fit to time frame option. With this option effects will be enhanced to do things like start and end exactly to the timing marks on a row. Wait for future releases to see how this is going to help. Trust me, you will like the new things coming.

					- Enhancement: (Matt Brown) has enhanced the preview screen so it is multiple times brighter than before. So much nicer now!!

					- Enhancement: (frankr) made it so that when you select none, it will not overwrite existing data. Why do we care? This enhancement means a LSP person could sequence the first 30 seconds of a megatree (mark those rows as none,none in nutcracker). Now fill the rest of the sequence with nutcracker effects. The result? LSP effects will appear for the first 30 seconds, Nutcracker effects will appear for the remainder.

					- Enhancement:(Frankr) enhanced the right click process for protecting a cell. Before you had to get focus on the cell by left clicking (turned it yellow), then you could right click. Now just ricght click any cell, you dont need left click first

					- Enhancement: (Matt Brown) made it so effects are now listed alphabetically except for effects current under development (like the piano keyboard)

					- Enhancement: (sean) Sean added Radial 3d Button. Click this button besides the Radial button to get new radial effects

					- Bug Fix: (Sean, Matt, frankkr) . Found more division by zero errors, fixed them.

					- Enhancement: (Sean), For any  person who is pulling code and compiling it (Like MaterDaddy for Linux compiles). I have renamed repository nutcracker_c to be called xLights. Nutcracker_c was just a temp name I was using. I want everyone to remember we started with the xLights code base from Matt Brown in January of this  year. Nutcracker is just one tab on xllights. People get confused is it xLights? Is it Nutcracker? All of nutcracker has been put inside of xLights so the repository should be called the same. Again thanks to Matt for making this all possible. So here is new repository: https://github.com/smeighan/xLights

					- Enhancement: Please start to enter bugs into github so we can track them there. If you want to raise a bug go to https://github.com/smeighan/xLights/issues

					- Bug fix: (Matt) Meteors has been fixed by Matt's rewrite of the preview window. This means the normal Meteor effect is now working again. 
						StarField still does not display on megatrees, it does work on other models.

3.0.24: Aug 4,2013 	- Enhancement: (frankr) New fade in and fade out boxes on every effect. Enter the number of seconds to either fade in/out.
					- Enhancement: (sean) Started development of Piano Keyboard. This is not complete. Will eventually use an input file from
                           Audacity that will identify the polyphonic notes in a song.
					- Enhancement: (matt) Many changes to text. Ghosting is now fixed. You no longer have to use the temporary fix of using
						   "1 is unmask, 2 is color wash". Text now has 45 degrees rotation. Countdown timer can show seconds counting down or
						   it can show "xx d yy h zz m aa s" display. countdown text is now centered correctly.
					- Enhancement: (frankr) New effect for circles. Circles can bounce, collide. Choose radial to make a new effect
					- Bug Fix: (sean) fixed crash when creating random effects. The new effect classes (piano,circles) were causing problem. fixed
					- Bug Fix: (sean) When randomly creating Spirograph effects, the animate button was not being selected. fixed.
					- Enhancement: (frankr) Cells in your effect grid can now be protected/un-protected. Protect a cell and
						   press the "create random effects" and all cells will get a new effect unless the cell is protected.
						   Under Development: Piano Effect. This is NOT done yet. when completed it will animate a piano
						   keyboard.
					- Enhancement: (frankr) Slider next to Layer choice. This slider allows you to pick how much of effect 1 and effect 2 show
						   through.
					
					- UNDER DEVELOPMENT: (sean) Piano Keyboard. Not functional yet. Will be pulled forward into release 25
3.0.23: Jul 28,2013 - Enhancement: (sean) Fire effect now allows hue shift. You can now have green, blue fire. 
                    - Enhancement: (sean) Fire effect now has a "Grow Fire?". If you click yes, the height of the fire will grow over time
					- Enhancement: (frankr) Before if you changed or created a sequence and then clicked exit, program would just exit and you would lose your
					  change. Now, you will be prompted if you want to save before exit.
					- Enhancement: (frankr) Random effect generation. When creating a Spirograph, the Animate button will always be set and the little radius will be
					  set to its minimum value. By setting r to zero and setiing animate to true, random spirographs will now always produce an animation.
					- Enhancement: (frankr) New choice in model generation. You can now select custom. We are not quite done with this but it will support having
					  coro snowflakes, stars, strings wrapped around a car, fire sticks, candles,. Basically any model. Watch this space for a future tutorial.
3.0.22: Jul 26,2013 - Enhancement: (frankr) New button to randomly create effects for every row in your sequence
                    - Enhancement: (sean) Brightness slider now goes to 400% (Old value was 100%). This will allow improving movies
					- Bug Fix: (sean) If Tree effect was selected for Effect2 and slider was all the way to left, xlights crashed. fixed
					- Bug Fix: (sean) Removed the gauge that was next to Export button. The gauge library does not work in Unix.
3.0.21: Jul 25,2013 - Enhancement: (frankr) In the Model setup screen, you can now set the starting channel for each string
                    - Enhancement: (frankr) In the Model setup screen, you can now choose how the strings wrap. 3 more choices are now available
					  bottom left, upper left, upper right, bottom right
					- Enhancement: (sean) Meteor Effect class has 3 new effects
					  1) Swirl1. As meteors fall they are moved in a sine wave
					  2) Swirl2. Same as Swirl 1 excep odd strands move one way, even strands move the other
					  3) Strarfield. Meteor effect starts in center of your display and moves outwards
3.0.20: Jul 19,2013 - Enhancement: (frankr) Highlight cell you are editing in so you know where "update" will apply 
                    - Bug Fix: (frankr) Opening a music file twice, the second time will not prompt for a new file name. Instead, it overwrites the first file. Fixed
					- Enhancement: (frankr) Ability to read an Audacity or a XML file when creating a blank sequence from a music file
					- Enhancement: (frankr) 1 is Unmask, 2 is Unmask now keeps the brightness value of the mask. This is big! Use a picture as a mask on a colorwash.
					  The new picture will be a mono tone version. Use text as a mask, the edges will feather but in a solid color. 
					  This fixes the antio-aliasing issue of text. To fix use text as effect 1, color wash as effect2. set effect 2 color what you want your text
					  to look like
					- Bug Fix: (frankr) Deleting some models caused crash, fixed.
					- Bug Fix: (frankr) If you had multiple models attached to a sequence, deleting anything other than 1st model caused crash. fixed
3.0.19: Jul 17,2013 - Bug Fix: (matt) Crash on exit fixed
                    - Bug fix: (matt) Empty export files (LOR, hls,vixen,conductor) fixed
					- Enhancement: (sean) When reading in your sequence, missing attributes will be added if your file is from pre ver 18.
					- Enhancement: (sean) Changed rotation slider in Spirals so it does ten times more rotations.
					- Bug fix: (matt) If you have two lines of text but only one color selected, both lines will be same color
	NOTE!! Please make a backup of all your xml files. This release will be fixing your sequences, protect yourself.
3.0.18: Jul 06,2013 - (sean) Added new Brightness and Comtrast sliders. Located next to Sparkle slider it allows dimming the overall effect.
                      Contrast will make brights brighter and sim colors go towards black.
3.0.17: Jul 01,2013 - (sean) Fixed text in Effect2. Added Twinkle, Tree, Spirograph and Fireworks to Effect2
3.0.16: Jun 28,2013 - (sean) New option added to the Text effect, Countdown timer.
3.0.15: Jun 27,2013 - (sean) Added check to make it so empty picture selection does not cause hundreds of pop up windows
                    - (sean) fixed LOR lms and lcb export
					- (sean) added check to make sure model's have at least 1 pixel wide and high
					- (sean) new feature in Meteors effect. Meteors can now fall up
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
  








