xLights is a program that allows you to play the sequence files from Vixen and
LOR. It has the ability to drive USB DMX controllers, E1.31 Ethernet controllers. xLights has a extensive scheduler. 
Nutcracker is a program that generates animated effects on massive RGB devices such as megatrees, matrices, and arches. 

Latest releases are found at http://nutcracker123.com/nutcracker/releases/

XLIGHTS/NUTCRACKER RELEASE NOTES: 
=================================
3.4.13	Sep 19, 2014
	-ENH (sean) The scale slider in the PREVIEW tab now goes to 300. before range was 1-100, now 1-300
	-bug (djulien) Avoid wx assert error during save/load pgo settings (bad grid row#), default to first group after loading settings (however, this can lead to mismatches), don't show "suspicious" message for missing voice data, list all models (even if not in Preview)
	-bug (djulien) Allow pgo parser to tolerate blank voices
3.4.12
        -ENH (dkulp) HLS import - now uses wxYield to display progress during load
        -ENH (dkulp) HLS import - if an HLS universe has less channels than configured in xLights, use the xLights value.
                     If the HLS universe has more, display a warning.
3.4.11	Aug 19, 2014
	-ENH (sean) 4th Open option of Nuctacrker, "New Animation Sequence", now allows fractional times. Before
		only values like "10,11,15" were allowed. Now "10.123,11.32123,15.2"
3.4.11	Aug 15, 2014
	-ENH (sean) Single Strand Effect now can use "Fit to Time". 
		If this is checked, one cycle of an arch will be done. 
		It will exactly fit the time on the current row of the grid.
	-Bug (sean) Two places in the code would cause a crash. Both have been fixed.
3.4.10	Aug 14, 2014
	-ENH (sean) complete rewrite of Single Strand effect. New sliders: Number of Chases, 
		color (Rainbow,palette)
3.4.9	Aug 7, 2014
	-Bug (djulien) wxDateTime is broken again; force RFC822 format to try to get it to work; 
	for example: "Thu, 25 Dec 2014 00:00:00 MST" or "Thu, 25 Dec 2014 00:00:00 -0700"
	can be used on line 1 of the text effect, select different display options from the countdown	
	-Bug (sean) "Create Random Effects" would create a preponderance of BARS and BUTTERFLY effects. Effects are now random.
	-Bug (sean) xLights icon (Yellow Bulb), was not associated with the exe file. Shortcuts showed default icon. This now fixed.
3.4.8   Aug 3,2014
	-Enh (sean) Changed Playback counter on NUTCRACKER tab. It now updated 20 times per second instead of once per second.
	also changed the format to include fractional seconds. Finally added two displays, one has minutes, the second is
	just total seconds.
3.4.7   Jul 29,2014
	-Enh (sean) Added Model brightness adjustment into Model screen. Not functional yet, this is a pre-release 
	-Bug (sean) Fixed export for single channel strings for LSP
3.4.6   Jul 23,2014
	-Enh (sean) Increased maximum time for a empty sequence from 300 seconds to 3600 seconds
3.4.5   Jul 15, 2014
        -Enh (dkulp) If an empty LMS channel is part of an RGB group, don’t consider it empty.
        -Enh (dkulp) Add a checkbox to TabConvert to go ahead and map the empty channels
        -Bug fix (mbrown) Fix Custom RGB Models that have multiple nodes with the same number
        -Bug fix (djulien) fix segv due to incorrect compiler code in Piano effect
        -Bug fix (mbrown) Custom models now work with traditional strings
3.4.4	Jul 2, 2014
	-Bug (dkulp) Fix LOR Import
	The old XML parser, on the start element event, you could query if the element was empty or not.  
	For the "channel" element, if it was empty, we skipped it and didn't map it to a channel.
	The new parser doesn't have that functionality.   I have to record some information at the start tag, 
	determine if there are any effect child elements, and if not, undo some stuff at the end element.   
	The detection and undo wasn't working properly.  Thus, any empty channels in the LMS were getting mapped.   
	Why is this a problem?   Well, LMS has a habit of using extra channels for things.   
	For example, the CCR's which have 50 pixels would normally just be 150 channels.  
	However, in the LMS file, they are 157 channels.  I have no idea what the LR, MM, ME, CM, CS, and CI 
	channels that it puts for each CCR are used for.
	In any case, those extra 7 channels (and any other empty channels) would cause large shifts.    
	This release has changes to fix that.
3.4.3	Jun 30, 2014  
	-Enh (dkulp) Reduce memory usage of LMS import
        -Enh (dkulp) Change XML parser to spxml to avoid loading entire file into memory
3.4.2	Jun 26, 2014
        -Bug fix (dkulp) Problems loading large LMS files causing crashes/hangs
        -Enh (dkulp) Add additional status messages while loading LMS files
3.4.1	Jun 19, 2014
	-Bug fix (sean) Butterfly effect now creates pixels at {0,1} and {1,0} correctly
	-Enh (sean) added two new butterfly effects
	-Enh (sean) Work in progress. New Papagayo tab. This tab will be used to convert Papagayo files (.pgo) into
		xLight sequences
3.4.0   Jun 5, 2014
	-Bug fix (djulien) tried to fix Piano key wrap (again); added PianoTestNotes.txt file for easier testing (in case more changes are needed)
	-Bug fix (djulien) fixed cross-fade (morph); didn't write to xseq file correctly
	-Bug fix (djulien) add missing tool tip for Text center-and-stop checkbox
	-Enh (djulien) make Preview element size editable; not sure why it wasn't; NOTE: must have a Display Element selected first
3.3.9	May 22, 2014
	-Bug fix (sean). Glediator effects were short one row in the Y direction. This has now been fixed.
3.3.8	May 13, 2014
	-Enhancement (sean) New Effect, Glediator. Save a recorder file from Glediator. Open file and put in your grid.
	-Bug fix (sean). Glediator conversion now works. Select a *.gled file in the CONVERT tab and change into any of the output
		formats.
	-Bug fix, reported (djulien) draw white correctly if it's one of the Piano colors
	-Bug fix (Sean), Fixed Preview for Wave and Glediator. You could see this effects in Nutcracker tab but not in Preview.
		all fixed.
	-Enhancement, requested (djulien) center Piano keys if Clip is unchecked
3.3.7	May 9, 2014
	-Bug fix (dkulp) Added some additional checks to fix the heartbeat.cfg error.
3.3.6	May 08, 2014 
	-Bug fix (djulien) avoid horiz or vert line flashes with plasma balls (Circles effect, Plasma option)
	-Bug fix (djulien) fix piano color caching (reported by multiple people)
	-Enhancement (sean) Added direction drop down for butterfly effect. "Normal" is the effect moving from
		right to left. this is the way it has always been. "Reverse" changes direction
	-Enhancement (sean) New file type in CONVERT tab, *.gled. Create these files using the record button of Glediator.
		Conversion code is still under development.
3.3.5	April 23, 2014
	-Enhancement (Sean). New effect called Wave. This effect will plot a Sine , 
		Triangle and square wave on your display.
	-Enhancement (djulien). ToopTip text added for Morph.
3.3.4	April 17, 2014
	-Enhancement, (sean). Faces effect now creates a scalable face on your matrix or megatree
	-Bug fix, (Sean). Entering a RETURN in the text fields of the the TEXT effect no longer crashes xLights
	-Enhancement, requested (djulien): add center-and-stop option to text movement options (new checkbox); 
		sync text lines; use left/right spaces to delay left/right movement or left/right "\n" to delay up/down movement
	-Bug fix (djulien): allow "," within Text lines (caused "not found" error)
	-Enhancement (djulien): add MORPH checkbox to enable gradual Effect 1 -> Effect 2 transition; 
		can be used for ramp/fade, morph text or image, etc.
	-Enhancement, requested (djulien): add menu item to clear all effects on this row
	-Bug fix (djulien): copy/paste text to Text effect Line 1 - 4, don't interfere with grid
3.3.3	April 3, 2014
	-Enhancement, requested (djulien): add Copy Effect Across Row menu item; copies selected cell (random effect) 
		into other cells on the same row
	-Enhancement, (sean) Added two more lines of text to text effect. Now you can have 4 lines of text.
	-Bug fix (sean). Fixed LSP export. Yeah!
	-Enhancement, requested (djulien): allow user-specified format string with Countdown Text effect; 
		format is <delim>target-date<delim>format-string
		for example: #4/1/2014 12:00:00#only %S seconds until noon on April 1!
		NOTE: don't use "," within Text; there is an XML parsing bug in xLights when the file is reloaded
	-Enhancement, requested (djulien): copy same random effect to all unlocked cells horizontally 
		when Shift key used with Create Random Events button
	-Enhancement (djulien): write playback checkpoints to file for crash monitoring
	-Enhancement (djulien): add flag waving option to Pictures (useful for 4th of July, Memorial Day, etc)
3.3.2	Jan 28, 2014
	-Bug fix (dkulp): When importing LOR *.lms files, certain missing data 
		would cause a crash. 	conversion now fills in any of those missing
		parameters and conversion completes.
3.3.1	Jan 27, 2014
	-Enhancement (djulien): change "Overlay" to "Persistent" for clarity; 
	-Bug fix(djulien): save/restore value within XML
	-Enhancement (sean): add "1 reveals 2", "2 reveals 1" as new layer methods
	-Enhancement (dkulp): Don't assume LOR units are 16 channels to try and convert CCR based sequences.
3.3.0 Jan 7, 2014
	-Enhancement (djulien): vixen 2.x routine as picture
	-Enhancement (djulien): add checkbox to Effects panel to inhibit canvas clear before effect (for layers across models)
	-Enhancement (djulien): allow cancelled schedule to re-run that same day
	-Enhancement (djulien): show serial outbuf overflow amount
	-Enhancement (djulien): add a couple of icicle options to Meteors
	-Enhancement (djulien): add wavey left-to-right up/down option for text
	-Enhancement (djulien): added additional format options for date count-down
	-Enhancement (djulien): pass controller responses to plug-in (in-coming or round-trip data); internal 
		change only - should not affect standard usage, but listed here just in case
	-Enhancement (djulien): add /noauto command line option to prompt for auto-run (allows easier schedule override)
	-Bug fix/enhancement? (djulien): show channel# being tested during Chase test
	-Bug fix/enhancement? (djulien): continue execution after comm output overflow in case controller can catch up
3.2.14	Dec 18, 2013
	-Enhancement (Sean): SingleStrand effects are now moving twice as fast.
	-Enhancement (Frankr) * Added alternating bands options to the bars effect.
	-Enhancement (Frankr) Removed limitation on pasting into the timings and label columns.
3.2.13	Dec 14, 2013
	-Bug fix (djulien): display some details about channel count error
	-Bug fix (djulien): split Effects Direction into separate lists for Text and Picture (some Picture are not applicable to Text)
	-Enhancement (djulien): More movement options in the pictures effect.
	-Enhancement (Sean): Added new song to songs directory.
		(It Must've Been Ol') Santa Claus, Harry Connick Jr.
3.2.12	Dec 11, 2013
	- Enhancement (Sean) Updated all files in songs directory to have readme files showing
		link to where to purchase music. Added some new songs.
3.2.11	Dc 6, 2013
	- Enhancement (Frankr) Nodes per string increased to 65536, You can now create a model that
		will do your entire house.
	-Bug fix (djulien): set Piano shape + map default file path to Piano subfolder within xLights folder, not up one level
3.2.10	Dec 5, 2013
	- Enhancement (Djulien) Piano effect now has been released
	- Enhancement (Frankr) Number of nodes on a single string has been increased from 300 to 1000
    - Enhancement (Matt) Added cut, copy, and paste to Nutcracker tab.
	- Enhancement (Sean) Create a subdirectory in songs for each song rather than all files in one
		place.
3.2.9 Nov 24, 2013
	- Enhancement (Matt) You can now open an LOR sequence from the Nutcracker tab. 
        The first time you open an LOR sequence, the timing from the LOR file will 
        be imported into the grid (only the times where effects start are imported).
    - Enhancement (Sean) Single Strand now has pull down to select chase types.
    - Enhancement (Sean) Single Strand now has 3d Fade checkbox to apply to chases
3.2.8:	- Enhancement (Sean) Re-release of single strand effects. Now they work without need to set
			"fit to time".
3.2.7:	Nov 15, 2013
	- Bug (frankr) Windows model displayed wrong channel counts. 
	- Enhancement (frankr) You can now export just one model instead of every channel that is in your setup
		1st method: "export" This does not look at models and instead exports every channel that has been defined 
			in your setup Tab
		2nd method (new): "Export Model". You now can pick the model that is attached to your sequence and only export
			those channels. For example, I have megatree, arches, eaves. If I select megatree only the channels
			associated to that model will be exported. This will make much, much smaller LOR lms,lcb or vixen vix,vir files.
3.2.6:	Nov 10, 2013
	- Bug (Sean) Removed Linus and Lucy mp3 file from songs directory, we don't distribute mp3's
	- Enhancement (Frankr and djulien). New hot keys created.
		F3 Toggles between PLAY/STOP for far left "Play Effect" button
		F4 Toggles between PLAY/STOP for top "Play" button
		F3 -- will start to play an effect (left play) if none is playing and no sequence is playing
			if an effect is playing it will stop playing. if a sequence is playing it will
			stop playing and the current effect will play.  This allows for easy editing of an effect.
		F4 -- Starts playing a sequence (top play) from the current selected cell
			If a sequence is playing it will be stopped.
		F5 Same as clicking the "Update Grid" button
		Ctl-o Same as clicking the open button
		Ctl-s save as clicking the save file button
3.2.5:	Nov 9, 2013
	- Bug (Frankr) removed 'U' for updating grid. Now only F5 is shortcut key. The 'u' caused a problem 
		when typing in text.
3.2.4:	Nov 7, 2013
	- Enhancement (Djulien) New  filtering on every slider and checkbox for "Create Random Effects"
	- Enhancement (Frankr) F5 or 'U' now performs an update grid
	- Enhancement (Sean) New directory "songs" has audacity label files for many songs. Find the new directory
		at C:\Program Files (x86)\xLights\songs
3.2.3:	Oct 25, 2013
	- Bug: (Dan) Schedule tab missing on Linux
    - Enhancement: (Dan) When outputting e1.31, only output the used channels instead of the full 512 universe. 
		(Lower network traffic, less processing needed)
	- Bug: (Dan) Text Effect not rendering on OSX.  Fixed.
	- Enhancement: (Sean) New Single Strand Effect class. Use this for arches and eaves
3.2.2:	Sep 26, 2013
	- Enhancement: (Dan) When importing hlsIdata, sort by the universes number to make the import predictable and repeatable.
	- Enhancement: (Dan) When importing hlsIdata, change the file extension for the Audio from PCM to mp3.
	- Bug: (Dan) If there is a named test setup, much of the functionality on the Setup tab (up/down arrows, 
		modifying a network, etc..) did not work properly and could result in modifying the wrong network.  Fixed.
	- Bug: (Dan) Preview/Nutcracker tab drawing on the Mac would result in "ghosts" of the flashing lights due to anti-aliasing. Fixed.
	- Bug: (Dan) At certain screen sizes, some pixels in mega-trees and other Nutcracker models may not display.  Fixed.
		
3.2.1: Sep 13, 2013
	- Enhancement: (Frank). In the Model section you now see start and end channels, not just start
	- Bug: (Sean). When exporting HLS sequences if the number of channels was not evely divisible by 3, xLights would crash. Fixed.
	- Enhancement: (Frank). Switched the grids for start channel and custom model to be in scrolling 
		windows so that larger data sets can be handled.
	- Bug: (Matt). Smart RGB arches are only producing a single channel. Fixed
	- Enhancement: (Matt). F1 will bring up help if you are in the Profile tab
3.2.0: Sep 12, 2013
	- Enhancement: (Matt). In the Model creation you can now select type of string that the model will use. 
		There are new string types available. You can pick non-rgb strings. These will use a single channel, For preview purposes 
		you can pick RED, GREEN,BLUE, WHITE as single colour strings.
	- Enhancement: (Matt). You can now select a string type that is strobe lights. This light will flash 3 times per second
		for each bulb.
	- Enhancement: (Matt). You can now pick a multi pointed star as a display element.
	- Enhancement: (Matt). You can select a wreath as a display element.
	- Enhancement: (matt). You can select a string type of 3 channel RGB (In other words dumb rgb strings)
	NOTE! With the change to the latest wxWidgets 2.9.5 , Mac 10.6.8 is no longer supported. We now will only support 10.7 and higher
3.1.4:  Sep 4, 2013
	- Enhancement: (Matt). Custom Model now uses pixel count 1..n. Models can be put on preview. For example, 
		you can define a candy cane out of rgb pixels and then place candy cane in the preview window. Very Nice!
    - Enhancement: (Matt) You can now copy models.
	- Enhancement: (Matt) You can now rotate single strand models in the preview window.
	- Enhancement: (Matt) There is a Sequence open button on the Preview tab. If you use this to open a sequence there is
		an option to set the "Part of my Display" checkboxes to match the models that just got opened.
3.1.3:  Aug 27,2013 
	- Enhancement: (Matt). New Effect Curtain.
	- Enhancement: (Matt)  New tab PREVIEW. There is a new tab that will allow you to see all of your 
		models displaying effects.
	- Bug (Frank) Fixed channel numbering for single string and Window models
3.1.2:  Aug 17,2013
	- Enhancement: (Frankr). Circle effect now has new option called plasma.
	- Enhancement: (Matt Brown).  bars compress & expand so that the two halves mirror each other.
	- Bug: (Frankr).  found and fixed the issues with fire and meteors that were being discussed at DIYC.
	- Enhancement (Dan Kulp). New import from HLS now available on CONVERT tab
	- Enhancement (Frankr). New option in spirals to allow the thickness to grow or shrink
3.1.1:  Aug 12,2013 
	- Bug Fix: (Matt Brown). Matt has fixed the Meteor effect. He has also enhanced it so meteors 
		can fall to the left or right.
		We have	renamed	Starfield to be called "explode" and "implode"
	- Enhancement: (Matt Brown)  I made some improvements to countdown & skip blank cells logic. I also changed 
		a couple of method names to make them more clear. Also, the timer now skips if the last interval takes more than 50ms to process. 
		Before it was blocking until the last interval finished (so if every interval took more than 50ms you had a stackup of waiting timer calls to process).
3.1.0:  Aug 7,2013 	
	- Enhancement: (frankr) new single BACKUP option now.
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
3.0.24: Aug 4,2013
 	- Enhancement: (frankr) New fade in and fade out boxes on every effect. Enter the number of seconds to either fade in/out.
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
3.0.23: Jul 28,2013 
	- Enhancement: (sean) Fire effect now allows hue shift. You can now have green, blue fire. 
    - Enhancement: (sean) Fire effect now has a "Grow Fire?". If you click yes, the height of the fire will grow over time
	- Enhancement: (frankr) Before if you changed or created a sequence and then clicked exit, program would just exit and you would lose your
		change. Now, you will be prompted if you want to save before exit.
	- Enhancement: (frankr) Random effect generation. When creating a Spirograph, the Animate button will always be set and the little radius will be
	  set to its minimum value. By setting r to zero and setting animate to true, random spirographs will now always produce an animation.
	- Enhancement: (frankr) New choice in model generation. You can now select custom. We are not quite done with this but it will support having
	  coro snowflakes, stars, strings wrapped around a car, fire sticks, candles,. Basically any model. Watch this space for a future tutorial.
3.0.22: Jul 26,2013 
	- Enhancement: (frankr) New button to randomly create effects for every row in your sequence
    - Enhancement: (sean) Brightness slider now goes to 400% (Old value was 100%). This will allow improving movies
	- Bug Fix: (sean) If Tree effect was selected for Effect2 and slider was all the way to left, xlights crashed. fixed
	- Bug Fix: (sean) Removed the gauge that was next to Export button. The gauge library does not work in Unix.
3.0.21: Jul 25,2013 
	- Enhancement: (frankr) In the Model setup screen, you can now set the starting channel for each string
    - Enhancement: (frankr) In the Model setup screen, you can now choose how the strings wrap. 3 more choices are now available
		bottom left, upper left, upper right, bottom right
	- Enhancement: (sean) Meteor Effect class has 3 new effects
		  1) Swirl1. As meteors fall they are moved in a sine wave
		  2) Swirl2. Same as Swirl 1 except odd strands move one way, even strands move the other
		  3) Strarfield. Meteor effect starts in center of your display and moves outwards
3.0.20: Jul 19,2013 
	- Enhancement: (frankr) Highlight cell you are editing in so you know where "update" will apply 
    - Bug Fix: (frankr) Opening a music file twice, the second time will not prompt for a new file name. Instead, it overwrites the first file. Fixed
	- Enhancement: (frankr) Ability to read an Audacity or a XML file when creating a blank sequence from a music file
	- Enhancement: (frankr) 1 is Unmask, 2 is Unmask now keeps the brightness value of the mask. This is big! Use a picture as a mask on a colorwash.
		  The new picture will be a mono tone version. Use text as a mask, the edges will feather but in a solid color. 
		  This fixes the antio-aliasing issue of text. To fix use text as effect 1, color wash as effect2. set effect 2 color what you want your text
		  to look like
	- Bug Fix: (frankr) Deleting some models caused crash, fixed.
	- Bug Fix: (frankr) If you had multiple models attached to a sequence, deleting anything other than 1st model caused crash. fixed
3.0.19: Jul 17,2013 
	- Bug Fix: (matt) Crash on exit fixed
    - Bug fix: (matt) Empty export files (LOR, hls,vixen,conductor) fixed
	- Enhancement: (sean) When reading in your sequence, missing attributes will be added if your file is from pre ver 18.
	- Enhancement: (sean) Changed rotation slider in Spirals so it does ten times more rotations.
	- Bug fix: (matt) If you have two lines of text but only one color selected, both lines will be same color
	NOTE!! Please make a backup of all your xml files. This release will be fixing your sequences, protect yourself.
3.0.18: Jul 06,2013 
	- (sean) Added new Brightness and Contrast sliders. Located next to Sparkle slider it allows dimming the overall effect.
        Contrast will make brights brighter and sim colors go towards black.
3.0.17: Jul 01,2013 
	- (sean) Fixed text in Effect2. Added Twinkle, Tree, Spirograph and Fireworks to Effect2
3.0.16: Jun 28,2013 
	- (sean) New option added to the Text effect, Countdown timer.
3.0.15: Jun 27,2013 
	- (sean) Added check to make it so empty picture selection does not cause hundreds of pop up windows
    - (sean) fixed LOR lms and lcb export
	- (sean) added check to make sure model's have at least 1 pixel wide and high
	- (sean) new feature in Meteors effect. Meteors can now fall up
3.0.14: Jun 13,2013 
	- New movie files added to Picture Effect
        Use web page to change your movies into jpg files. http://image.online-convert.com/convert-to-jpg
    - Strobe in Twinkle Effect is now random lights
3.0.13: May 30,2013 
	- New Strobe Effect now available in the Twinkle Effect Class
    - Twinkle Effect Class now allows different steps of twinkle.
3.0.12: May 26,2013 
	- New type of vertical text where letters are not rotated but stacked on
      top of each other. Finally we have Line 1 one color, line 2 second color.
	  Each line of text can have a different font, a different direction.
	  There is a position bar for each line of text.
3.0.11: May 15,2013 
	- Text can now be rotated 0, 90, 180 or 270 degrees
3.0.10: May 12,2013 
	- Changed 32 character limit in the Text effect to be 256 characters
3.0.9:  May 11,2013 
	- Animated Gifs added to the Pictures effect
3.0.8:  Apr 25,2013 
	- Added export for LSP to create UserPatterns.xml
        The actual file created will have a *.user extension. rename it to use it
	- New Effect, Fireworks!. Dave Pitts coded up this new effect. 
3.0.7:  Apr 23,2013 
	- Fixed Pallet 3 button. It was not picking up color when it was clicked.
    - Fixed export of Meteors. This effected all sequencers
	- Fixed export of Life. This effected all sequencers
	- Fixed export of Snowstorm. This effected all sequencers
3.0.6:  Mar 11,2013 
	- Corrected another bug in LOR lcb export.
    - Modifed Nutcracker tab to keep a minimum size screen on left window.
    - Added color selection based on distance from center of object for Spirograph.
    - Added the ability to animate the d parameter in spirograph
    - Moved all effects into their own source file.
    RGBEffects.cpp is now smaller and there are 20 new cpp
        files. RenderBars.cpp, RenderMeteors.cpp, .etc.
3.0.5:  Mar 08,2013 
	- Corrected color swap in lcb files. Corrected bug in HLS export.
3.0.4:  Mar 04,2013 
	- Added new effects, Tree and Spirograph.
3.0.3:  Mar 02,2013 
	- Added new effect, Twinkle.
3.0.2:  Feb 28,2013 
	- Corrected unsigned 24bit value for HLS *.hlsnc export.
3.0.1:  Feb 26,2013 
	- Added wxGauge to show process for exporting sequence files.
3.0.0:  Feb 22,2013 
	- Initial Release. This is Matt Browns original porting of Nutcracker into xLights.









