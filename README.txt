xLights is a program that allows you to play the sequence files from Vixen and
LOR. It has the ability to drive USB DMX controllers, E1.31 Ethernet controllers.
xLights has a extensive scheduler. Nutcracker is a program that generates animated effects
on massive RGB devices such as megatrees, matrices, and arches.

Latest releases are found at http://nutcracker123.com/nutcracker/releases/
Issue Tracker is found here: https://github.com/smeighan/xLights/issues

XLIGHTS/NUTCRACKER RELEASE NOTES:
---------------------------------
   -- enh (keithsw) Allow bars effect centre for compress/expand to be moved
   -- enh (keithsw) Add spinner model
2016.36  Jun 06, 2016
   -- enh (keithsw) Change backup files to *.xbkp extension to minimise risk of users opening them accidentally. Include them in F10 backup.
   -- bug (dkulp) Don't use OpenGL 2.1 if context is 2.0
			Use glMapBufferRange instead of glBufferSubData (latter is SLOOOW on ATI cards)
   -- bug (dkulp) fix the background image button not working
   -- bug (dkulp) Update borders for singlestrand
   -- enh (dkulp) Strip out the defaults for the On/ColorWash and buffer/layer/color panels at load time.   Reduces file size
   -- bug (dkulp) Use an icon bundle with different sized icons to hopefully fix the volume meter issue on windows
   -- bug (keithsw) Fix custom value curves not being editable when you re-open them
   -- enh (keithsw) Value curves added to single strand, garlands, fire, plasma, curtain. color wash
   -- bug (keithsw) Fix tendril effect
   -- bug (keithsw) Fix custom value curves not being editable when you re-open them
   -- bug (keithsw) When changing value curves use a sensible set of defaults.
   -- enh (chrisD)  Add menu items to load/save perspectives directly
   -- enh (gil)     Add new effect Fill.
   -- enh Improve several effect toolbar icons.
2016.35  Jun 02, 2016
   -- bug (dkulp) Fix colorwash stripping out stuff it shouldn't
   -- bug (dkulp) Minor layout things (removing borders on value curve buttons) and hopefully fix subbuffer panel
   -- bug (dkulp)  If using pixel style "Solid Circle" or "Blended Circle", performance with OpenGL 2/3 was really bad.
   -- bug (dkulp) Ability to disable/change the autosave interval.  Move settings query to its own timer (that doesn't need to be precise)
   -- bug (dkulp)  Fix subbufferpanel not clearing the lines
   -- bug (chrisD)  Update makefile for linux
   -- enh (keithsw) Add value curves to butterfly, bars, spirals, circles
   -- enh (keithsw) Add a reset to roto-zoom
   -- bug (keithsw) Fix lights off button. Ensure I send a lights off packet to all lights when it is turned off.
   -- enh (keithsw) Add value curve to brightness
2016.34  May 31, 2016
   -- bug (gil)     Fix crash when delete key was hit in Layout with no model selected.  Fixes #533.
   -- enh (gil)     Restructure how model groups are handled by providing a panel for them and allowing the
                    members of the group to be managed in the property grid area.
   -- enh (gil)     Add tilt parameter to the Arch model to allow better 3D perspective.  Fixes #250.
   -- enh (keithsw) Add starfield option to meteors implode/explode look more like a starfield
   -- enh (keithsw) Add autosave to .working.xml files ... saves every 3 minutes as long as the sequence is not playing
   -- enh (keithsw) Zoom and rotate added to buffer panel including value curves for most parameters
   -- enh (keithsw) Blur Value Curve Added
   -- bug (dkulp)   DOn't corrupt rgb-effects if mouth set to (off).  Fixes #577
   -- bug (dkulp)   Refresh the start/end channels when various properties (string type, strands, etc...) are changed. Fixes #575
2016.33  May 25, 2016
   -- bug (dkulp) Maybe fix #571?   Don't post the event, call directly.
   -- bug (dkulp) don't draw the grid and then immediately refresh anyway.
   -- bug (dkulp) Log a few more opengl things, Force GDI driver down to 1.x
   -- enh (keithsw) General import improvements: filename in title bar, warnings
             if you have not saved mapping and the ability to suppress excessive error messages.
   -- enh (keithsw) Improved xLights sequence import.
   -- enh (gil)   Add right-click delete model option in layout list.
   -- bug (gil)   Fix node layout for custom models. Fixes #574.
2016.32  May 19, 2016
   -- bug (dkulp) Bug where subbuffer might not get written out correctly (and written out when not needed)
   -- enh (dkulp) Optimization to not load the background image for every single mouse click on the Layout panel
   -- bug (dkulp) Use atomics instead of crit section
   -- bug (dkulp) Don't create the debug context by default anymore...
   -- bug (dkulp) Better management of TextureID's - Fixes #576
   -- bug (dkulp) Don't use the pointers for the textures
   -- bug (dkulp) Use the standard timer on OSX, it works fine and no need to have the extra thread
   -- enh (dkulp) Display a warning if GDI driver detected
   -- enh (dkulp) Add the .1 log as well just incase the rolling stuff rolled the important lines
   -- bug (dkulp) Remove some unused vars
   -- enh (dkulp) Use non-shader method for smooth points on Intel
   -- bug (dkulp) Fix setting transparency and black transparency
   -- enh (keithsw) VideoReader ... more protection for possible divide by zero errors.
            Includes logging of any problems found.
   -- bug (keithsw) Make logs 1MB as originally intended. These should zip to well under 50% when sent to us.
   -- bug (chrisD) Force TendrilEffect and TextEffect on foreground thread under Linux
2016.31  May 16, 2016
   -- bug (dkulp)   Fix for Model window not displaying
2016.30  May 16, 2016
   -- enh (keithsw) PGO export for those LOR etc users that would like to use xlights to create their PGO files.
   -- enh (keithsw) Add scrubbing to audio (hold control key and left mouse button on waveform)
   -- bug (gil)     Correction to FSEQ data layer import.  Should only reset sequence data on import not on render.
   -- bug (dkulp)   Fix for OpenGL on Intel graphics card. Intel 2.1 has issues with POINT_SPRITE
2016.29  May 14, 2016
   -- bug (keithsw) Fix prompt for custom model import so it happens after you size the custom model
   -- enh (keithsw) Remove Papagayo tab
   -- bug (gil)     Fix FSEQ filetype data layer import.
   -- enh (gil)     Highlight Model row when effects are selected.  Fixes #145.
   -- enh (dkulp)   Changing timing panel to OpenGL - Mac now can support up to 60fps
   -- bug (dkulp)   Loading XML sequence with no fseq not setting the timer to the appropriate MS time
   -- enh (dkulp)   If changing OpenGL verison, auto-package/submit logs to dkulp for analysis
   -- enh (dkulp)   Click on progress meter brings up dialog showing progress of each model
   -- enh (dkulp)   Progress meter on status bar for rendering
   -- enh (gil)     Fix copy/paste for single effect so it fills cells selected when paste by cell.
   -- enh (gil)     Add automatic channel placement in custom models with mouse clicks.
   -- enh (gil)     Add ability to lighten the custom model background image.
   -- enh (gil)     Add ability to place a background image on a custom model grid.  Fixes #5.
   -- enh (keithsw) Add Manual Custom Model building assisted by image/video.
   -- enh (keithsw) Add LSP timing track import
   -- bug (gil)     Mark the sequence dirty when sequence duration is changed to prompt for save on exit.  Fixes #563.
   -- bug (gil)     Fix jumping back to start briefly after an unpause.  Fixes #565.
2016.28  May 10, 2016
   -- bug (gil) Better fix for effects dragging past zero. This fixes dragging a timing mark to zero seconds.
   -- enh (keithsw) Add papagayo timing track import via Timing Track Import context menu
   -- enh (keithsw) Add progress display during rendering all
2016.27  May 09, 2016
   -- bug (dkulp) Fix problems of the model groups that are set to the old default of grid
                  as per preview using a buffer significantly smaller than they used to
   -- enh (keithsw) Add FPS display when playing the sequence
   -- bug (keithsw) Fix a bug with filename fixups
   -- enh (keithsw) Cleanup scaling options on Music Effect ... now via a choice - None/Individual Notes/All Notes
   -- enh (keithsw) Sparkles now can optionally grow with music intensity
   -- enh (keithsw) Strobe effect now can optionally grow with music intensity
   -- enh (keithsw) Fire effect now can optionally grow with music intensity
   -- enh (keithsw) Add timing track export/import. Includes support for Papagayo and Piano timing tracks.
   -- bug (keithsw) Lots of fixes to auto custom model generation
2016.26  May 06, 2016
   -- enh (gil)   Change Render All toolbar icon and make common on Mac.  Fixes #436.
   -- bug (dkulp) Add a bunch more debugging stuff to the OpenGL things
   -- enh (dkulp) Improve Display Elements toolbar icon.
   -- bug (dkulp) Update to use log properties to create a log file
   -- bug (dkulp) Bunch of optimizations, more gl error logging, some fixes based on the error log
   -- bug (dkulp) Minor work on the subbufferpanel
   -- bug (gil)   Fix selected view not displaying in combo box on sequencer grid.  Fixes #562.
   -- enh (gil)   Jump to sequencer tab after creating new sequence.
   -- bug (gil)   Fix numeric entry size for Text effect position text boxes.  Fixes #557.
   -- enh (gil)   Improve Sequence Elements toolbar icon.
2016.25  May 05, 2016
   -- bug (dkulp) Bunch more fixes to OPENGL calls. Now Radeon cards should work
   -- enh (dkulp) Add a menu item to select the max OpenGL version to use. Higher the version, the higher
           the eprformance of graphics. The lower, the more graphic cards that can be supported.
   -- enh (keithsw) New option "Tools,Log". This will create a zip file of your xlights log file. No longer need to
           search the hidden appdata directory to find it. Keith described it like this: Add "Package Problem Files"
		   to tools menu which lets the user package current config/sequence/logs when reporting a problem.
   -- enh (dkulp) Sub-Buffers. Add ability to specify what section of a buffer you want the effect applied to
   -- enh (keithsw) Block manual switching between wizard tabs in auto generation of custom model.
2016.24  May 04, 2016
   -- bug (dkulp) Fix OPENGL calls so that Sound wavefile shows on Intel Graphics cards
2016.23  May 03, 2016
   -- enh (keithsw) Added new option "Tools, Generate Custom Model"
   -- enh (dkulp) Upgrade to newer OpenGL library.
                  The OpenGL rendering has been updated from the “fixed pipeline” code
				  (which was state of the art in 1998) to using modern OpenGL shaders (>2010).
				  This allows us to better use the capabilities of modern video cards.
				  However, to support older machines, we can drop down to OpenGL 2.1 shaders
				  or even to the old 1.3 fixed pipeline.  Modern machines should be faster,
				  older machines should still work.
   -- enh (dkulp) New option on the Tree model, "Spirals". This is how many times a strand is wrapped
           around the tree. 1.0=one full wrap, 2.5 = 2 1/2 wraps from bottom to top.
   -- bug (gil)   Fix Copy Row/Paste Row when vertically scrolled. Fixes #553.
   -- bug (gil)   Fix application losing focusing when closing a floating panel.
   -- enh (gil)   Re-position view when playback starts or loops so that play marker is visible.  Fixes #550.
   -- enh (gil)   Paste modifications for how single effects paste in various situations.  Fixes #549.
   -- bug (dkulp) Add rename button to groups dialog.  Fixes #548
   -- bug (dkulp) Fix blur setting when clicking on effects. Fixes #547
   -- bug (dkulp) Reset status bar and refresh previews on file close.  Fixes #552
   -- bug (dkulp) Only redirect the menu items that need redirecting
2016.22  Apr 24, 2016
   -- bug (gil)     Fix bug where SingleLine model wouldn't draw when set to
                       3 Channel RGB and controller on right.  Fixes #525.
   -- bug (gil)     Fix so playback works after an XML file import.  Fixes #535.
   -- bug (gil)     Fix SuperStar import to work with horizontal layouts.  Fixes #537.
   -- bug (keithsw) Fix bug in music effect not working unless at the start of the song
   -- enh (keithsw) Add video acceleration and slow down so video fully plays in time periods
                      different to their actual length
   -- bug (keithsw) Fix sensitivity bug in music effect
   -- bug (keithsw) Fix spectrograms wider than the model
   -- bug (keithsw) Similar fix for video
   -- bug (gil)     Fix Pictures effect broken by last release.
2016.21  Apr 22, 2016
   -- enh (keithsw) Add x/y offset and some new movements to tendril
   -- enh (keithsw) Custom model export and import allowing simple sharing
   -- enh (keithsw) Add x offset to piano
   -- enh (keithsw) Add x and y offsets to select meteor effects
   -- enh (keithsw) Add x and y offsets to select vumeter effects
   -- enh (keithsw) Add timing track option for defining piano notes
   -- bug (gil)     Fix first frame and last frame buttons so the grid will scroll.  Fixes #544.
   -- enh (keithsw) Add Music Effect
   -- enh (keithsw) Add Note On sub effect to VU Meter - Like On but you can filter the notes it responds to
   -- enh (keithsw) Add Note Level Pulse sub effect to VU Meter - Like Level Pulse but you can
                      filter the notes it responds to
   -- enh (keithsw) Add Timing Event Colour sub effect to VU Meter - This changes colour on timing events
   -- enh (keithsw) Add option to trigger firework effects based on the audio intensity
   -- enh (keithsw) Add the option to vary meteor effect meteor count based on the audio intensity
   -- enh (keithsw) Add Colour On sub effect to VU Meter - Selects a colour based on the intensity of the audio
   -- enh (keithsw) Add the ability to constrain the frequency range of the spectrogram. Columns in
                     this effect are now aligned with MIDI notes.
   -- fix (keithsw) Ensure pictures, piano and video effect move when moving folders around
   -- enh (keithsw) Move convert tab into a dialog under TOOLS. Now go to convert "TOOLS,CONVERT"
2016.20  Apr 13, 2016
   -- bug (gil)   Fix right-click Paste not active after a Control-C copy.
   -- bug (gil)   Fix single copied effect not pasting to multiple cell selection.
   -- enh (gil)   Enable right-click Copy, Paste, Delete options for timing section of grid.
   -- enh (gil)   Enable Copy Row and Paste Row for Timing tracks.
   -- bug (gil)   Fix bug preventing some timing effects from being pasted.
   -- bug (gil)   Fix copy/paste bug where single effect copy was not pasting with original duration.
   -- bug (chrisD) Install dictionaries/phoneme files on Linux
   -- bug (dkulp) disable the popup as it causes a crash
2016.19  Apr 10, 2016
   -- bug (dkulp) Fix Text not showing
   -- bug (dkulp) Fix icicles buffer width
   -- enh (gil)     Add EditToolBar with Paste By Cell and Paste By Time options.  Fixes #418 and Fixes #421.
   -- enh (gil)     Add ability to select, copy, and paste timing effects. Several restrictions in place.  Fixes #366.
   -- bug (keithsw) Fix implemented ripple missing triangle option #524
   -- bug (keithsw) Cleaned up docking windows toolbar icons #519
2016.18  Apr 7, 2016
   -- bug (gil)   Fix bug when Papagayo timing was collapsed and selected grid cell selection wasn't working.
   -- bug (gil)   Fix Delete key operation so it doesn't do a Cut unless Shift is pressed.
   -- bug (dkulp) Fix shift-del/ins on LayoutPanel as well
   -- bug (dkulp) Fix "individual start channel" calculations
   -- bug (dkulp) Set the buffer size correctly for candycanes.  Fixes #521
   -- bug (dkulp) Find a solution to some crashing in release builds
   -- bug (dkulp) Remove model properties dialog and model list dialog
   -- bug (dkulp) Fix crash in editing groups
   -- enh (keithsw) Add writing of FPP universes file when saving networks
   -- bug (keithsw) Fix logging on windows ... log file is named xlights_l4cpp.log
         found typically in c:\users\<user>\AppData\Roaming
2016.17  Apr 3, 2016
   -- enh (keithsw) Add writing of FPP universes file when saving networks
   -- bug (keithsw) Fix logging on windows ... log file is named xlights_l4cpp.log
          found typically in c:\users\<user>\AppData\Roaming
   -- bug (dkulp) Fix text effect on Windows
   -- bug (dkulp) Fix single lines that are 1 channel/string with multiple lights
   -- bug (dkulp) Fix circle radius
   -- bug (keithsw) Add true cancel when mis-loading a sequence when in wrong folder ...
                 what it actually does is just delete all models that dont exist ...
				 you should then not save the sequence.
   -- enh (sean) Added "bin/xlights.windows.properties" to installation script per Keith's request
2016.16  Apr 2, 2016
   -- bug (dkulp) Fix icicle off by one
   -- bug (dkulp) Fix some scrolling on windows issues
          On Mac, the Test panel checkboxes don't work,
          use double click action to simulate
   -- bug (dkulp) Fix issue with single lines being clicked on too easily
   -- bug (dkulp) Fix crash if a view contains models that no longer exist
   -- bug (dkulp) Fix crash when clicking on checkbox in model list if no model is selected
   -- bug (dkulp) Fix crash on import if source strands aren't in the source sequence
2016.15  Mar 30, 2016
   -- enh(dkulp)  Add arrow key support for small changes to model locations in layout
   -- enh(dkulp)  Add undo support to Layout
   -- enh(dkulp)  Use three point for Arches/CandyCane/Icicles model
   -- enh(dkulp)  Use two point size/location for single strand model
   -- enh(dkulp)  Add "Icicles" model
   -- bug (gil)   Add mutex to prevent crash when deleting presets rapidly.  Fixes #495.
   -- enh (gil)   Shift key will help snap model handles to horizontal/vertical alignments for two handle models.
   -- enh (gil)   Draw red/blue alignment lines for two handle models.
   -- bug (keithsw) Fix test dialog handling of multi-universe E131
   -- bug (keithsw) Fix test dialog handling of different start pixel specifications on models
   -- enh (keithsw) Move windows version to wxWidgets 3.1
2016.14  Mar 23, 2016
   -- enh (keithsw) Piano support for audacity file reading.
   -- bug (dkulp) Fix crash if mapping maps a model that isn't used in the source sequence. This fixes #505
   -- enh (dkulp) Change how the group add/remove/editing is handled in Layout
   -- enh (dkulp) New model type "CandyCanes". Added cane rotation
   -- enh (keithsw) log capture on crash in debug report.
   -- bug (keithsw) Fix bulbs flashing in test mode.
   -- enh (keithsw) log4cpp added. These are library routines that allow enhanced logging of errors.
   -- bug (keithsw) Add back in output control in test mode.
   -- enh (keithsw) Redevelop Piano effect ... this is a ground up rewrite and is not compatible with the old piano effect.
   -- enh (keithsw) Add logging as a runtime capability for better debugability
   -- bug (keithsw) Fix LOR controllers in test mode Fix #502
   -- bug (keithsw) Add back in multiselection in test mode
   -- bug (gil)     Fix timings not draw correctly when window is resized. Fixes #504.
   -- bug (gil)     Fix Pictures scaling issues.  Fixes #501.
2016.13  Mar 15, 2016
   -- enh (gil) Added gradient option to bars effect
   -- enh (keithsw) Add a colour aware test mode ... RGBW.
   -- enh (dkulp)   Optimize the redraws on timer events a bit.
                      Drops CPU usage from 95% to about 60% on my machine.
   -- bug (gil)     Send render event for effects pasted to multiple cells.  Fixes #497.
   -- enh (keithsw) Move test tab to a separate dialog ... add new ways of selecting channels for testing.
                  Access new TEST tab by going "Tools,Test" from main menu. We removed the TEST tab.
                  Side effect of this change is that switching show directories is now 1000% faster
   -- bug (gil)     Restore the audacity style operation of the Play button and fix animation play/pause.  Fixes #493.
   -- enh (gil)     Connect ctrl-Z to Undo function.
   -- bug (gil)     Fix paste/keyboard placement of effects in partial cells.  Was broken by 2/11 checkin.
   -- bug (dkulp)   When using keyboard shortcuts, don't apply defaults for the panels, use any settings they contain
2016.12  Mar 10, 2016
   -- bug (dkulp) Fixed long save times when a model is off the screen.
   -- bug (dkulp) Fix corrupt perspective section. Fix is to Delete attribute before setting it
   -- bug (gil) Restore pause and stop for effect preview.
2016.11  Mar 8, 2016
   -- bug (sean)    Modified Windows install script to it always prompts for install directory
   -- enh (sean)    Updated xlights icon images from Charles Suhr. Fixes ragged edge.
   -- enh (keithsw) Add video looping
   -- bug (keithsw) Fix some video formats that were not playing
   -- bug (keithsw) Fix audio controls on the toolbar
   -- bug (gil)     Fix bug moving effects that aren't fully visible.  Fixes #488.
   -- bug (gil)     Allow min period effects to move with mouse.  Fixes #479.
   -- bug (dkulp)   Imports from other formats were generating invalid key ID's
   -- bug (dkulp)   Pictures imported from SS were not displaying properly
   -- enh (dkulp)   Update On/ColorWash to only output non-defaults in XML
2016.10  Mar 3, 2016
   -- enh (keithsw) Support old-school CTRL-INSERT, SHIFT-INSERT and SHIFT-DELETE (windows only)
   -- bug (dkulp)   Fix problem with first pixel with grouped arches in SingleStrand . Fixes #474
   -- bug (dkulp)   Fix crash on close when preview running
   -- bug (dkulp)   Fix color/blur sliders not updating to defaults.  Fixes #477.  Fixes #476.
   -- enh (dkulp)   Split Layer Timing panel into two panels (Layer Blending and Layer Settings)
   -- bug (dkulp)   Fix duplicate effect data problems if seq data exceeds 4GB (64bit only)
                  This failure was found when testing a sequence of 900K channels on a MAC
   -- enh (dkulp)   Add begin/end effect transitions
   -- enh (gil)     Update Morph to use all 8 colors allowing 6 color tails.
   -- enh (keithsw) Add video effect. Now supports *.mp4, *.mpg, *.avi, *.mov, *.flv, *.mkv, *.asf
   -- bug (gil)     Fix crash when moving effect left or right with arrow keys and not timing track active.  Fixes #473.
   -- enh (keithsw) Replace media handling with ffmpeg and SDL eliminating the VBR problem and adding new media support.
   -- enh (keithsw) Add tooltips to explain controller and channel a xLights channel maps to
   -- enh (keithsw) Handle movement of show directory (as long as it isnt renamed) automatically
2016.9  Feb 24, 2016
   -- enh (gil)   Allow single effect to be moved left and right using arrow keys if timing grid is active.
   -- enh (gil)   Allow single effect to be moved up and down on grid.  No longer restricted to a single model.
   -- enh (keithsw) Add description against rows on the setup tab Issue #422
   -- enh (keithsw) Add View->Windows->Dock All menu item to wrangle all free windows back into their docked position
   -- enh (gil)   Pictures effect change: Separate scale as an independent parameter and allow start and end scaling motions.  Fixes #451.
   -- bug (gil)   Fix bug when pasting single effects and fix copy row bug when multiple effects are selected.
   -- bug (gil)   Clear waveform when switching to an animation.  Fixes #469.
   -- bug (gil)   Fix crash when moving multiple effects introduced by previous change to fix overlap error.  Fixes #472.
   -- enh (dkulp) If "ModelGroup" has node/strand effects in sequence, allow mapping those to new models at load.
   -- enh (dkulp) Add ability to specify a transform and style for the buffer for each effect
   -- enh (dkulp) When displaying/preview group, use the pixel styles from the original models
   -- enh (dkulp) Use dimming curve from individual models when applying effects to Groups
   -- enh (dkulp) Change to "Smooth" as default for pixel appearance when loading old rgbeffects
   -- bug (dkulp) Make sure we don't have Models and ModelGroups of the same name when loading rgbeffects
   -- enh (keithsw) Added blur capability in the layer blending window
   -- bug (gil)  Fix crash if down arrow is hit on a timing track.
   -- bug (keithsw) Fix crash if two 2016 versions are compared
   -- regr(dkulp) If media from XML doesn't exist but is in the Media directory, use it from there.
   -- bug (keithsw) Fix false positive VBR file detection
2016.8  Feb 16, 2016
   -- bug(gil)   Allow media sequence to play to defined duration and not stop at media length. Fixes #138.
   -- enh(gil)   Add Layered description to tooltip.  Fixes #179.
   -- bug(gil)   Remove incorrect hover tips from main menus. Fixes #242.
   -- bug(gil)   Rebuild sequence data length when media file is changed.
   -- enh(gil)   Model groups are now indicated with an icon on sequencer grid.
   -- enh(gil)   Morph corner images now have a minimum size so they are always easily seen.
   -- enh(keithsw) Add music suppport to Tendril effect.
   -- bug(dkulp) Fix faces not blinking properly. Fixes #458
   -- bug(dkulp) DisplayElements panel default display is not sized correctly on Mac.  Fixes #466
   -- bug(dkulp) Check to make sure a model exists before adding it to a sequence
   -- enh(keithsw) Add VUMeter effect (very much beta in this release. Appearance may change in near future releases. Please do not rely on it at this stage)
   -- bug(gil)   Fix selected cells moving into timing area when vertically scrolling.  Fixes #419.
   -- bug(gil)   Fix arrow keys not moving selection properly which could cause hotkey paste in wrong locations. Fixes #432.
   -- bug(gil)   Fix effect duplicating when using arrow keys on grid.  Fixes #424.
   -- enh(gil)   Show mouse position in waveform.
2016.7  Feb 08, 2016
   -- bug(dkulp) Fix crashes on animations
   -- bug(dkulp) Fix crashes caused by old Whole House Models
2016.6  Feb 07, 2016
   -- enh(keithsw) Add warning when using variable bit rate audio files
   -- bug(dkulp) Fix channel number on nodes in CustomModel
   -- bug(gil)   Add missing Circle model to new ModelManager.
2016.5  Feb 05, 2016
   -- bug(gil)   Fix morph effect assist orientation.  Fixes #462.
   -- bug(gil)   Fix creating new image with Pictures Effect Assist.
   -- bug(dkulp) The v3 importer always generates in an "older" v4 format, not the current.   Adjust accordingly.
   -- bug(gil)   Fix effect movement that could allow overlaps.
   -- bug(gil)   Prevent ability to crush effect size to zero when left and right edges of joined effects are moved.
   -- bug(dkulp) Fix Ripple implode/explode.  Fixes #460
   -- bug(dkulp) Restore model group selection for layout/preview
   -- bug(gil)   Correct Save and other buttons not greying out when closing sequence.  Fixes #402.
   -- bug(gil)   Fix not being able to scroll to all models on grid in some instances.  Fixes #405.
   -- bug(gil)   Fix ability to zoom after changing sequence duration.  Fixes #439.
2016.4  Feb 02, 2016
   -- bug(dkulp) Fix crash if model group has no models.
2016.3  Jan 31, 2016
   -- bug(dan)  Slight optization, fix plasma
   -- enh(all)  Upgrade our build environment. Compiler from gcc 4.7.1 to gcc 5.3.0.
                Code::Blocks from 12.02 to 16.1. Recompile every file and every library.
   -- enh (all) Update Butterfly, Fan, Galaxy, Morph, Pinwheel, Plasma, Shockwave,Spiragraph,
                Wave to use table lookup for sine and cosine functions. Performance Improvement.
   -- enh(gil)  Allow easier selection of effect edges.  Now only half the effect is selected
                until you cross the midpoint.
   -- bug(gil)  Fix bug that was allowing effect overlaps.  Occurred when moving effects on
                the grid when they were displayed in multiple rows.
   -- enh(gil)  Rename Copy Model and Paste Model to Copy Row and Paste Row
   -- bug(gil)  Fix the sequence length when duration is changed so user doesn't need
                to delete fseq file. Fixes #196 and #442.
2016.2  Jan 26, 2016
   -- bug(sean) Fixed self install script to overwrite wxwidgets library files. wxmsm*.dll
2016.1  Jan 25, 2016
   -- enh(dkulp) Refactor Effect rendering code to make it easier to create new effects
   -- enh(dkulp) Update Linux Text effect to render on background thread
   -- bug(dkulp) Linux toolbar buttons sizing problem and buttons on effect panel
   -- enh(dkulp) Plasma/Butterfly/Meteors render performance enhancements
   -- enh(keithsw) Add tendrils effect. Also includes introduction of GraphicsContext.
                 Builders please see README.Windows for rebuild instructions.
   -- enh(keithsw) Add natural snowfall without accumulating to the snowflakes effect
4.3.02  Jan 8, 2016
   -- enh(gil)   Add copy/paste tool to the Picture Effect Assist window.
4.3.01  Jan 3, 2016
   -- bug(gil)   Fixed direction of colors for a reversed Marquee.
   -- enh(gil)   Allow Morph endpoint selections using left and right click and drag.
   -- bug(gil)   Improve acceleration for Morph, Fan, Galaxy, and Shockwave. Values 1, 2, -1, and -2 were broken.
   -- bug(gil)   Fix a node rendering bug when clicking on effects next to the end of Marquee effects.
   -- enh(gil)   Improve Fan, Galaxy, and Shockwave render time.
   -- enh(gil)   Add start position slider to Marquee to allow positioning the start color.
4.3.00  Dec 17, 2015
   -- change(dkulp) OSX uses dynamic wxWidgets libraries instead of static
   -- bug(dkulp) Don't call wxYield in Glediator render.  We're on a background thread. Fixes #384
   -- enh(dkulp) Add an "extra small" grid spacing
   -- enh(dkulp) Add a message box if the rgbeffects.xml contains a model with an invalid model type
4.2.25  Dec 9, 2015
   -- enh(gil)   Add Copy Model and Paste Model effects options to row heading right click menu.
   -- bug(dkulp) Fix crashes in lightning effect if not all the effect properties are specified
   -- enh(dkulp) Allow maping the first layer of a model effect onto a strand
   -- enh(gil)   Increase null output maximum from 99999 to 512000 channels.
4.2.24  Nov 23, 2015
   -- bug(dkulp) Fix for crashe occuring during RenderText
   -- enh(dkulp) Increase the range for the text movement to accomodate longer text
   -- enh(dkulp) Add "Update" button to Color panel that will update just the palettes for the selected effects
   -- enh(dkulp) Reduce the number of wxString objects created during rendering.  Speeds up rendering of large LMS imports by 50% or so.
4.2.23  Nov 18, 2015
   -- enh(gil)   Increase max head length and repeat count for Morph.
   -- enh(gil)   Increase max radius from 200 to 250 for Fan, Galaxy, and Shockwave.
   -- enh(gil)   Slight tweak to Fan, Galaxy, and Shockwave so that step angle is 0.1 for > 350 instead of 400.
   -- bug(gil)   Fix Data Layer re-import so it works for all supported file types. Fixes #391.
   -- bug(gil)   Fix pictures effect so wrapping doesn't leave a gap.
   -- bug(dkulp) Fix some problems with the Faces dialog where the first face would get it's data deleted.
4.2.22  Nov 15, 2015
   -- bug(gil)   Correct calculation for acceleration and deceleration of Morph, Fan, Galaxy, and Shockwave.
   -- enh(gil)   Increase spinner limit for model start channel and from output. Fixes #408.
   -- enh(gil)   Add concentric circle capability to new Circle model class.
4.2.21  Nov 13, 2015
   -- enh(gil)  Add Circle model class.
   -- enh(gil)  Smaller step angle for Fan, Galaxy, and Shockwave to support larger models.
   -- enh(sean) Smaller step angle for Pinwheel, this allows the edges to show correctly on larger displays
   -- bug(dan)  Fix chases in single strand.  Calc the correct offset and direction for the starting point
   -- bug (dan) change e1.31 optimization so data gets sent when nothing has changed
4.2.20  Nov 4, 2015
   -- enh(gil)   Add 3 more DMX channels to support larger channel DMX lights.
   -- enh(dkulp) Change the ModelBrightness stuff to a full fledged multi-channel dimming curve supporting
                gamma curves, specific values from a file, etc...
4.2.19  Nov 3, 2015
   -- enh(gil)   Add effect to control DMX outputs
   -- bug(dkulp) Apply offset for SuperStar import to flowy and morphs
   -- bug(dkulp) Change algorithm of single strand chases to work a bit faster when dumped on a model group
      if "group arches" is on.  Still slow, but not "it looks like it's hung" slow.
   -- enh(dkulp) Add an timing offset option for importing of SuperStar and LMS
   -- bug(dklup) Make sure we create an image with a valid size. With invalid models, can cause crash at render.
4.2.18  Oct 28, 2015
   -- bug(dkulp) Update effect toolbar and icon panel to always include all effects
   -- enh(sean) Change output to lights icon to be a light bulb that fills in Yellow when on
   -- enh(sean) changed icon for lights off to have a small red "x"
   -- enh(dkulp) Update Text effect Line 1 to have a "Vector" option (like images) for moving text arbitrarily
   -- enh(dkulp) Update SuperStar import to import the textActions (more or less, fonts are different, but close)
   -- enh(dkulp) SuperStar import allow scaling of ALL images.   Can import 12CCR into a 100x24 model for example.
   -- bug(dkulp) Fix some problems with using the "From Output" stuff with new models.  Crash.
   -- enh(gil)   Allow zero speed for Marquee effect.
4.2.17  Oct 21, 2015
   -- enh(dpitts) Made output/start channel in models have correct end output/end channel.
   -- enh(dkulp) Add some options to SuperStar import for doing some simple image scaling
   -- enh(dkulp) don't scale image in effect assist to allow editing images smaller than the model
   -- enh(dkulp) Start adding ability to resize image from effect assist
   -- enh(dkulp) Able to parse .sup files that have configuration elements (that aren't valid XML) (fixes #394)
   -- bug(dkulp) Prompt for image directory for SUP import up front so Cancel can work. (fixes #393)
   -- enh(dkulp) Make sure user_dictionary can load from show dir and make sure it can override stuff in the standard dictionaries.
   -- enh(sean) Removed built in fade from lightning effect
   -- enh(sean) changed the backup (F10) so that it saves files <20mbytes. Before it was 10mbytes.
   -- bug(gil)   Prevent magnetically connected effects from being crushed to zero duration.
   -- bug(dkulp) Fix problem where numChannels*numFrames > 2G
   -- bug(gil)   Fix Marquee not rendering smooth when reverse is checked.
4.2.16  Oct 13, 2015
   -- bug(keithsw) Fix broken picture wiggle style
   -- enh (sean) Added slider to give horizontal movement to lightning bolts
   -- enh (sean) Added check box to give chose to have forked lightning.
4.2.15  Oct 08, 2015
   -- bug(gil) Fixed the labels showing "off"
4.2.14  Oct 07, 2015
   -- enh(ChrisD) Add application launcher for linux
   -- bug(ChrisD) New timing dialog should be titled as such (partial fix for issue #377)
   -- bug(gil)   Buttons will now re-enable after an LSP export model operation.
   -- enh(Keithsw) New feature in pictures, tiled pictures
   -- bug(gil)   Fix potential crashes for all imports where models may be hidden.
   -- bug(gil)   Fix LSP import effects when a target model is hidden.
   -- enh (sean) New effect class Lightning
   -- bug(gil)   Sequence SaveAs was not saving XML in new directory.
   -- bug(gil)   Remove fixed attribute for timing track converted to papagayo timing.
   -- enh(gil)   Add LOR conversion option to show verbose channel mapping.
   -- enh(Keithsw) Updated tile code to support x/y offsets in Picture effect
   -- bug(dkulp) After deleting an effect, if you hit Up/Down, crash will result
   -- bug(dkulp) Use native ObjectC code for clipboard paste for custom models to support pasting from Numbers
   -- bug(dkulp) Click on a "node" may not hit test properly if an effect exists on the model or strand layers
   -- bug(dkulp) Prevent effect from being resized so start is < 0ms which prevent effect updates/rendering/etc..
   -- enh(dkulp) Add option to twinkle to re-randomize the color after each twinkle
   -- enh(keith) Added tiling options to picture effect
   -- bug(sean) Fixed bottom row of plasma. Bug was the bottom row was copied from top row
   -- bug(ChrisD) Fix issue #368 - missing OK/Cancel on Schedule Playlist
4.2.13  Sep 27, 2015
   -- enh (sean) Added new Plasma effect. Now you dont need to use Butterfly to get Plasma effects
   -- bug(dkulp) Fix crash when rendering models that have channels that extend beyond the configured range
   -- bug(dkulp) The "Enable" flag is only honored when changed while outputting to lights
   -- enh(cjd) Add application launcher for linux systems
4.2.12  Sep 22, 2015
   -- enh (sean) Added a new style to Butterfly, Style=10. This allows setting color of Plasma
         also all plasma effects (Styles 6,7,8,9,10 in Butterfly), will use
         Background chunks to change the plasma effect.
   -- bug (gil) * Fix crash if deleting a timing in new sequence wizards.  Fixes #372.
         this bug was actually fixed in 3.2.11
   -- enh (chrisD) * Implement issue #237 - Enhancement request: Media file missing,
         prompt for user input
   -- enh (chrisD) * Implement issue #367 - Enhancement: Add .out file type
         to Glediator file search
4.2.11  Sep 20, 2015
   -- enh(sean) New effect Plasma. Access this through butterfly. Select Style 6,7,8 or 9
   -- bug(dkulp) On a new sequence, make sure the timer is restarted with the right speed
   -- enh(dkulp) Allow a custom color for each phoneme in faces.
   -- enh(dkulp) Allow multple face defintions for every model type, not just matrix.  Fixes #357.
4.2.10  Sep 11, 2015
   -- bug(gil) Fix effect selection when vertical scrollbar has been moved.
   -- bug(gil) Range check the number of fan and element blades for SuperStar import.
4.2.9  Sep 10, 2015
   -- bug(gil) Fix effect selection especially pointing hands not working.
   -- bug(chrisD) Don't crash when collecting a crash report on linux
   -- bug(chrisD) Fix text effect on linux
4.2.8  Sep 08, 2015
   -- bug(gil) Fix bug where play marker wouldn't work if pressing play was the first thing you do and also
                fixed dropping an effect while model playback is paused such that it executes a stop and
                switches to effect play.  Fixes #322.
   -- enh(gil) Use standard File SaveAs dialog for saving sequences. Fixes #336.
   -- enh(gil) Improved dialog when prompting to save sequence changes.
   -- bug(gil) Fix the single cell selection caused by undocumented logic enhancements last release.  Fixes #359.
4.2.7  Sep 06, 2015
   -- bug (chrisD) Fixed a number of bugs in Linux version of xLights
   -- bug (chrisD) Set colour of multi-selected items to yellow. This fixes the multi model select
      in LAYOUT mode. Ctl-left click to select models. You can then move whole group at once
   -- enh (dkulp A bit more error checking for placement of effects/timing marks
   -- enh (dkulp More work on bg text render stuff, better mouse-over/selection for tiny effects
   -- enh (dkulp) Remove the position things from Effects, base all hit testing on time and handle the specifics in EffectsGrid
   -- enh(gil) Stop effect playback when performing a Save operation.
   -- enh(gil) Add scale and position options to the Marquee effect.
4.2.6  Sep 02, 2015
   -- bug(gil) Fix morph row/column calculation for large model sizes
   -- enh(gil) New model groups default to "minimal grid".
   -- enh(gil) Add scrollbars to lower windows in Model Groups dialog. Fixes #350.
   -- enh(gil) Increase window frame model bottom limit.  Fixes #351.
4.2.5  Aug 29, 2015
   -- enh(gil) Add Marquee effect
   -- bug(gil) Fix crash when renaming model without sequence open.
   -- bug(dkulp) Implement an "Auto" mode for Eyes for Coro/Image faces that will blink roughly every 5 seconds on a rest
   -- bug(dkulp) Fix crash if face effect dropped on a strand or node
4.2.4  Aug 26, 2015
   -- bug(gil)  Fix view combo drop-down not populating when no fseq file existed.  Moved Initialize call
                 to make sure SequenceData object has been initialized first.
   -- bug(gil)  Models that are renamed or deleted will now be updated in all views.
   -- bug (chrisD) Fixed popup error box when selecting models in LAYOUT
   -- enh(chrisD) New checkbox for channel overlap check in LAYOUT.
   -- enh(chrisD) in LAYOUT fixed flashing when selecting models
   -- bug(gil)  Presets will now populate correctly when switching to a new Show directory.
   -- bug(gil)  Presets created at top level will now show up when program is reopened.
   -- bug(chrisP) Fix export of Falcon .eseq Effects files
4.2.3  Aug 22, 2015
   -- bug(gil)   Version older detection code was wrong....this caused morphs to be converted when they shouldn't.
                  Also fixed morph corner 2b point graphic.
4.2.2  Aug 20, 2015
   -- enh(chrisD) in the Layout screen starting and ending channels are now shown. Name,Start and End are clickable columns to sort
   -- bug(dkulp) Fix crashes when renaming models that are part of a group or renaming a group.  Fixes #337.
   -- bug(dkulp) Fix crash on clicking Up/Down if no models selected. Fixes #181
   -- enh(dkulp) Make "Export Model" export the current state of the model, add option to re-render it
   -- bug(dkulp) Enable "Export Model" for groups
   -- enh(dkulp) OSX - Disable AppNap and Timer Coalescing when Output To Lights is on
   -- eng(steve) Add keybindings "O" for Off and "F" for fan effects
   -- enh(steve) Update Readme for Windows builds
4.2.1  Aug 14, 2015
   -- enh(gil)  Timings added to a sequence default to exist in all views.
   -- bug(gil)  Mark rgbeffects file as changed when views are modified.
   -- bug(dkulp) Fix saving the model XML during resize/rotate
   -- bug(dkulp) Fix clearing when an effect is shrunk
   -- bug(dkulp) If a papagayo track is in the sequence, trying to add another timing track will crash
   -- enh(gil)  Models that are copied will now display in the center of the screen.
   -- enh(dkulp) Add ability to enable/disable various outputs (so if controllers aren't hooked up, don't arp flood)
   -- enh(dkulp) Add ability to configure e1.31 ouput to handle consecutive universes in one output
   -- bug(dkulp) Fix problem of rotating elements with unequal scales
4.2.0  Aug 8, 2015
   -- bug(gil)  Fix channel map buttons disappearing when resizing dialog.  Fixes #313.
   -- bug(gil)  Prevent crash when doing Import Effects on xLights xml file.
   -- bug(gil)  Fix sparkles, brightness, and contrast text controls so that sliders update.  Fixes #320.
   -- bug(gil)  Fix right-click crash on grid when no sequence is loaded. Fixes #317.
   -- bug(dkulp) Audio at non-full speed stops too early when only playing a range
   -- enh(dkulp) Add a "Minimal Grid" option for model group to specify just the area of the given models
   -- enh(dkulp) Add ability to have model sizes scaled separately for horizontal and vertical directions
   -- enh(gil) Add ability to import lyrics to a timing track.
4.1.13  Jul 24, 2015
   -- enh(gil) First pass at coding a built in Papagayo into xLights. You will see 4 new dictionary files in xLights/bin
   -- bug(gil)   Column 0 wasn't working properly for pasting to single cell.
   -- enh(gil)   Change the look of the labels
   -- bug(gil)   Prevent text labels from piling up on left side when scrolling.
4.1.12  Jul 20, 2015
   -- enh (dkulp) If VAMP plugins are found, allow using them to generating timing rows
4.1.11  Jul 14, 2015
   -- enh(gil)   Add ability to create/edit/draw images on models using the Pictures effect and the Effect Assist window.
   -- gil (bug)  Prevent crash if image file cannot be found.
   -- enh(dkul
   p) Add a choice to select the view in the top left corner
   -- enh(dkulp) Add ability to "expand" a ModelGroup on row headings to expand out the models.
   -- bug(dkulp) Fix the Align/Distribute menu items on the Layout tab
   -- enh(dkulp) Add options for how the ModelGroup internal buffer is constructed including:
                   a) grid size - the old 400x400 is now configurable
                   b) Horizontal and Vertical "by model" where each model is a row or column and the nodes for that
                      model go the other direction.
   -- (cpinkham) Fix Linux build and update README.linux file. This allows someone to build a Linux xLights
4.1.10  Jul 8, 2015
   -- enh(gil)   Add preview of image to the Effect Assist window for Pictures effect.  More to come for this effect.
   -- bug(gil)   Selecting a single cell with any drag movement of the mouse was causing invalid effect end time when placed.
   -- enh(gil)   Add SaveAs to the File menu.
   -- bug(gil)   Fixed effects dropped on grid were not previewing.
   -- enh(gil)   Update View -> Windows so that Perspectives, and Display Elements windows work and add Effect Assist Window.
   -- enh(gil)   Add settings options for how the effect assist window behaves.
   -- enh(gil)   Added new panel to help in defining Morph positions.  Will be expanded for other uses later.
   -- enh(gil)   Automatically initiate Render All after adding or modifying data layers when dialog is closed to remove confusion.
   -- enh(gil)   Add ability to Delete Views.
   -- enh(gil)   Allow timings to be added/removed from views.  This info remains local to each sequence not stored as part of the view
                  since not all sequences have the same timing names.
   -- bug (sean) Fix README.windows that got overwritten. These are instructions for compiling our code under windows
4.1.9  Jun 30, 2015
   -- enh(gil)   Update wizard timing buttons to be more clear as to what they are defining.
   -- bug(gil)   Bug presets were not getting the version attribute.
   -- enh(gil)   Each sequence will now save the last view that was active so it can be restored.
   -- enh(gil)   Add Views capability.
   -- enh(gil)   Make the display elements window a scrolled window.
   -- enh(steve) Add LOR *.LAS file type to selection for timing imports.
   -- enh(gil)   Clicking effects in grid does not alter play position in waveform.  Allows a selected play range to stay active.
                  If you want to move the play start location use a double-click or click in the waveform.
                  Now you can adjust the waveform play range by putting cursor near the edges of the selected range.
4.1.8  Jun 24, 2015
   -- enh(gil)   Only allow effect endpoints to be dragged to positions that align with sequence timing.
   -- enh(gil)   Add a play scrub line.
   -- enh(gil)   Use fixed point integers for all effect position calculation.
   -- bug(dkulp) OSX crashes if toggling back and forth between Sequencer and other tabs and there are floating windows
   -- enh(dkulp) Allow "-P#", "-P##", " p#", and " p##" for detecting CCR/CCBs in LMS import
   -- bug(dkulp) LMS import fails to convert RGB elements if the effects on the three individual channels don't have the exact start/end times
   -- bug(dkulp) LMS import sets start and end intensity to 0 for ramp downs.
   -- enh(gil)   Changing the active model groups no longer modifies the "Part of my display" attribute.  That can only be modified in model properties.
   -- enh(gil)   Add ability to import timing grids from LOR files.
   -- bug/enh(gil) Modify the presets so that user has full control over group creation and deletion.  Remove Add to Favorites button.
                    Fix dragging effects causing deletions.
4.1.7  Jun 19, 2015
   -- bug(gil)   Fix crash when deleting the last timing mark on a timing row.
   -- bug(gil)   Add exclusive index to layers so that Undo will work on strands and nodes as well as normal layers.
4.1.6  Jun 16, 2015
   -- bug(dkulp) Fix Random effects not being random
   -- bug(dkulp) Fix memory leak on "close sequence"
   -- bug(dkulp) Fix rendering of Color Wash with fades
   -- enh(dkulp) Add a "Promote Node Effects" option to model popup to search for effects on the nodes that can be promoted up.
4.1.5  Jun 14, 2015
   -- enh(gil)   Add Effect Presets dialog back into the program.
   -- enh(dkulp) Add ability to set a brightness that is applied to the preview
   -- bug(dkulp) If the LSP XML file is corrupt, display parse error, but don't crash.
   -- enh(gil)   While playing a show don't halt if media cannot be found...log the error and move on to the next song. Fixes #262.
   -- Add model icon on toolbar. This allows goign directly to models without having to go to the LAYOUT tab
4.1.4  Jun 9, 2015
   -- bug(gil)   Fix v3 import to delete effect2 if Effect1:mix0 or Effect2:mix100 and delete effect1 if Effect1:mix100 or Effect2:mix0. Fixes #144.
   -- enh(gil)   Update Undo to be able to undo modified effects.  Fix a few cases where moved effects weren't tracked.  Right-click menu will now
                  show the type of Undo that is about to be performed.
   -- bug/enh(gil) Updates/fixes to the perspectives.  Windows now hide when switching to other tabs.  Delete now works.  Added a Save button so you
                    don't need to always go to the menus.  All windows are hidden before app closes.  Prevent duplicate names. Fixes #105. Fixes #87.
   -- bug(gil)   Link the slider and text control for the Circles effect
   -- enh(gil)   Select position by time function had int instead of double parameter which was causing small divisions to not be selectable.
   -- enh(gil)   Update Undo to be able to undo moved effects.
   -- enh(gil)   Update Undo to be able to undo added effects.
   -- enh(gil)   Add beginning Undo functionality.  Currently only works for restoring deleted effects and effects moved between layers.
   -- enh(gil)   Select effects based on time to allow off screen block selections to work.
   -- enh(dkulp) * Don't render model groups that don't have any models
   -- bug(dkulp) * More fixes for HiDPI screens.  Fixes #259
   -- enh(dkulp) * Don't render model groups that don't have any models
   -- bug(dkulp) Fix different MAC playback speeds
   -- enh(dkulp) * Updates for the LSP import
4.1.3  Jun 5, 2015
   -- bug(gil)   Fix several selection and copy/paste errors when grid has been vertically scrolled
   -- bug(dkulp) An effect of length 1 frame results in a "position" of NaN which can really mess up a lot of effects.
4.1.2  Jun 4, 2015
   -- enh(gil)   Lots of improvements to cell selection...still need to fix issue with off screen effect selection.
   -- bug(gil)   Add protection around calling cell selection logic with uninitialized values to prevent crashes.
   -- enh(gil)   Grey out the Create Random effects option if there is no selection.
   -- enh(dkulp) Add setting for Render at save.   Turning off will save the fseq in it's current state.
   -- bug/enh (dkulp) Lots of work on SingleStrand Chase effect to get it to work better with small models and single color models and grouped arches.
4.1.1  Jun 1, 2015
   -- enh(gil)   Improve selection logic so it shows a hand when near center of effect to allow drag but also allow selecting the cell underneath
                  if clicking between the center and the edge.  Also fixed the group move so it works with the hand and doesn't incorrectly
                  only move one effect when multiple are selected.
   -- bug(gil)   Fix a couple crashes when clicking in grid without timing track active.
   -- bug(gil)   Fix paste so it offsets within the selected cell if needed.
   -- enh(dkulp) Prompt if changes before close or open of new file.  Fixes #247
   -- enh(dkulp) Start work on Import of channels from LSP msq files
   -- enh(dkulp) Allow Fire to be from top/left/right/bottom   Fixes #245 (kind of)
   -- enh(dkulp) Allow effects to provide a "pre-rendered" background for display on the grid.  Update "SKips" effect to use it.  (provides scroll performance enhancement)
   -- enh(dkulp) Add ability for Garland effect to go Up/Down/Up+Down/Down+Up/Left/Right/Left+Right/Right+Left
4.1.0  Jun 1, 2015
   -- bug(dkulp) Fix several crashes due to rendering just part of an effect
   -- bug(dkulp) Fix several crashes related to multi-threaded rendering
   -- enh(dkulp) Spirograph given a few new options for length, animation speed, etc..
   -- enh(dkulp) Update text to allow a different speed for each line
   -- enh(dkulp) Remove the "Speed" and "Fit To Time" options.  Add speed/cycle settings to effects that would use it.
   -- enh(gil) Add random effects generation to grid right-click menu.
   -- enh(gil) Add ability to move highlighted cell with arrow keys and to expand/contract it with shift modifier.
   -- enh(gil) Add grid right-click copy/paste/delete options.
   -- enh(gil) Add ability to drag effects in a chase fashion using the Alt key.
   -- enh(gil) Add ability to select range of cells and paste effect with key bindings.
   -- enh(gil) Add ability to link show and media directories.
   -- bug(gil) Fix conductor file import to support LSP conversions. Fixes #232.
4.0.35  May 26, 2015
   -- bug (dkulp) fix for crash that occurred in Whole House Models
   -- (dkulp) * Popup location of crash report.   Fixes #239
   -- (dkulp) * Sparkles are only working on the top most layer.
4.0.34  May 26, 2015
   -- enh(dkulp) Add ability to import effects from xLights XML files
   -- enh(dkulp) Gather more information about threads for crash dump including what frame they are on and what effects they are rendering.
   -- bug(dkulp) Fix minor issues in SingleStrandSkips (direction and an extra placement)
   -- bug(dkulp) Fix problem of Brightness and contrast only apply to the first layer
   -- enh(dkulp) Draw a background for the single strand skips
   -- bug(gil)   Load perspectives when show directory is changed.  Fixes #96.
   -- bug(gil)   Fix effect duration for importing SuperStar Spiral into Galaxy effect.  Fix endless hang on Save when Galaxy rev's was set to 0.
   -- bug(gil)   Prevent dropping an effect during model playback from stopping the sequence.
4.0.33  May 23, 2015
      Developers notes:
   -- enh(dkulp) Update the "Strobe" strand type to actually strobe in the preview
   -- bug (gil)  SuperStar import updates. Fix morph default values to match recent changes. Subtract offset before doing mod.
                  Add option to flip Y for visualization imports.
   -- enh(dkulp) Allow start channel to be specified in #:# notation where first part is the output and the second is the offset from the start of that output.  So 3:1 is the first channel of the third output.
   -- bug(dkulp) Copy/Paste models from LibreOffice on Mac not working due to cr/lf issues
   -- bug(dkulp) Fix #205 - Spriograph D animation not working
   -- enh(dkulp) Add Shimmer option to On and ColorWash
   -- enh(dkulp) Add "Single Color Custom" string type that allows custom color selection
   -- enh(dkulp) Change Vixen import to use streaming xml parser to handle bigger files

   The following are the Open issues that have been closed over the last 5 days.
   We now have Steve Giron working triaging the bugs. He has gone through all open bugs (>90) and
   closed bugs where the
      1) problem is now fixed,
      2) closed the bugs if they were trivial (with the original users input), or
      3) we actually know we fixed the bug (you would see dan or gil as commenting on bug) or
      4) We document that it is not a bug but is performing as it was designed.
      5) Bug closed because it is a duplicate of another bug
   If you see an issue closed that you think is still an issue, please re-open the issue.


   Issues closed since release 4.0.32:
   -----------------------------------
   Spirograph - distance animation option does not work #205
   Release 27 Models disappearing while sequencing #221
   ENHANCEMENT: Add additional single line colors to Model Selection #130
   Message when clicking on a effect #227
   Release 4.0.31 crashes #228
   Deleting models causes erroneous messages to appear #85
   in sequencer single strand lights will only show up in model windows if they are red. white,green blue will not show. #191
   File Names not preserving Case when saving sequences. #149
   Data Layer import of a conductor file crashes Nutcracker #226
   Panel Layout/Perspectives - not allowing saving without sequence active. #153
   If you cancel while creating new sequence, xLights hangs (crashed once but can't repeat) #119
   Crash on trying to load Steve Gase's 6,000 effects xml #120
   Papagayo xml not displaying correctly in sequencer grid (xl4.0.23) #214
   Renaming models crashes app #223
   Model Group Crash #217
   Export models to HLS #224
   Bars effect - Speed bug ? #99
   When I move from sequencer to preview popups remain #140
   Feature Request: Drag effects between layers #154
   Crash with no folder set 4.0.20 #207
   xl 4.0.7 crashes #117
   Model Channel Grid odd behaviour with Individual Start Chans check box. #186
   Enhancement: Ability to play/stop sequence with spacebar (or other key in keybindings file) #194
   Playback on sequencer tab will not stop with stop button randomly #192
   Effect Settings Update (F5) #189
   App will crash when importing duplicate timing tracks. #208


4.0.32  May 19, 2015
   -- bug(dkulp) Rename the element in the loaded sequence on model rename.  Fixes #223
   -- bug(dkulp) Fix problem with overlapping effects that both specify fadein or outs causing interference
   -- enh(dkulp) Use On effect for scenes when possible instead of colorwash
   -- bug(dkulp) Don't call SetString on a sorted listbox.  Fixes #217
   -- bug(dkulp) Fix an issue with multi level stars with Single Channel types
   -- enh(dkulp) Add double-click to row headering for expand/collapse strand/nodes
   -- bug(dkulp) Fix RGBW
   -- bug(dkulp) Fix crash on import of Conductor file.  Fixes #226
   -- enh(dkulp) Add Vixen 2.x to import capability
   -- enh(dkulp) Add hlsIdata to import capability
   -- enh(dkulp) Move Import capbability to new Import menu
   -- enh(dkulp) Add abiltiy to play audio sequences at 3/4, 1/2, and 1/4 speed
   -- enh(gil) Morph update.  Improved position calculation. Added support for up to 4 tail colors.
                Added repeat and skip count options. Added choice to quickly create 8 different morph types.
                Removed Use Head Start Color for Tail Start and Use Head End Color for Tail End.
                Added stagger option to allow morph repeats to each have a delay.
   -- enh(gil) Morph: * Finer resolution for stagger option. * Allow negative stagger for morph.
4.0.31  May 14, 2015
   -- bug (sean) The songs directories were not being included in the current releases. 37 song directories are now under
      the songs subdirectory. Remember this directory is located in your "C:\Program Files (x86)\xLights\songs" directory
      if you are  on Windows
   -- bugs (Dan and Gil) A couple of bugs that caused crashes have been fixed
   -- enh (dkulp) Add alpha support to Bars, Meteors, Pinwheel ripple, snowstorm, spirals, strobe, tree, twinkle, snowflakes, shockwave, fireworks
   -- bug (dkulp Fix pinwheel 3d flags.
   -- bug (dkulp) Fix ColorWash fades when not on full matrix

4.0.30  May 13, 2015
  NOTES FROM DEVELOPMENT
   (Dkulp) * Detect ramp up/down when converting to effects
   (Dkulp) * IF the effects panels are disabled (for example, while rendering and saving),
      we cannot update the effect as disabling the controls changes the effect strings.
   (Gil) * Draw Timing lines last. This makes a timing grid look better when selecting 25,50ms.
   (Gil) * Use original effect duration for paste if no timing track is active.
   (dkulp) * Fix problem of "To Center" checkbox on text always being turned off
   (dkulp) * Add the alpha channel handling to text effect
   (dkulp) * Re-render range when effect moves
   (dkulp) * More slider<->text updates. Dan is adding text boxes to all effects that have sliders. This allows entering exact values
  ISSUES CLOSED in 4.0.29 and 4.0.30. See https://github.com/smeighan/xLights/issues
     -- 4.0.30  May 13, 2015
     -- Three channels in Custom Model when selecting 4 Channel RGBW #216
     -- 4.0.2x Tall custom model causes Ok, Cancel Appearance buttons to move out of view, clickable area #213
     -- Unable to change model on lengthy models #202
     -- Nutcracker Timing Mark Import Failure #225
     -- Enhancement - Models - Arches - single color #136
     -- fseq Save Directory doesn't save to location specified. #190
     -- Test tab requires that XL4 be restarted to see the new network configuration in 4.0.18 #211

4.0.29  May 09, 2015
   -- enh (gil) Allow effects to be moved up or down between layers with the arrow keys
   -- Keep track of "dirty" ranges and render the whole range.   Should reduce artifacts and need to re-render as oftern
   -- For Effects that display a background and Node layers where seeing the background colors is important, use smaller icons
   -- enh (gil) Ability to insert new layers above and below selected layer
   -- enh (dkulp) Add ability to turn off the node value display in the grid to improve performance (particularly
                   if all that is important is the xLights effects dropped on the nodes)
   -- Improved SuperStar import to handle all effects, images and scenes (text still not supported)
   -- Added more SuperStar import options to support visualization sequences
   -- Ability to import LMS files/channels into xLights effects
   -- Added Normal blending mode which supports alpha channels if used within effects
   -- Optimize strand/node rendering performance
   -- Improved waveform rendering
   -- Fix crashes if no color is selected for certain effects
   -- Added vector mode for Pictures
   -- Removed fseq directory
   -- Improved handling of text controls and associated sliders
   -- High DPI display improvements
   -- New Tree Ribbon type and Tree Flat type
   -- Improved display of 360/270/180 Trees
   -- Fixed bug with timing imports #225.
   -- Morph, Fan, and Fire effects updated to support alpha blending
   -- Color Wash effect updated to provide capability to limit it to a rectangle
   -- Fixed crash with strand/nodes on custom models
   -- Smaller icons displayed on effects with backgrounds and node lines so background colors are not obscured
   -- Fix crashes with Fan/Galaxy/Shockwave when used on WholeHouseModels
4.0.28  Apr 29, 2015
   -- enh (dkulp) Allow specifying number of segments and lights per segment for arches
   -- bug (dkulp) "3 Channel RGB" string types not working correctly
   -- bug (dkulp) #213/202 Issues with the Model Dialog losing the buttons if too big of a model is loaded
   -- bug (dkulp) #216 Start channels are wrong if using RGBW with custom models
4.0.27  Apr 28, 2015
   -- bug (dkulp) Fix for custom models when expanding a model into strands.
4.0.26  Apr 28, 2015
   -- enh (dkulp) New ability to add effects to individual stands and nodes
   -- enh (dkulp) Ability to see current colors/status of individual nodes
   -- bug (dkulp) Disable "Quit" during rendering as that causes a crash
   -- enh (dkulp) Widen the top of the tree a little bit
   -- enh (dkulp) Add ability to specify the display names for strands and nodes
   --
4.0.25  Apr 28, 2015
   -- enh (gil) New Efffect: Shockwave
   -- enh (gil) Allow a single effect to be selected and moved with both endpoints.
   -- enh (gil) Switch spacebar back to work like Audacity which is Stop instead of pausing.  Only deviation is I do unpause
                 instead of stop if spacebar is hit while model play is paused.  Also fixed timeline click not receiving
                 focus to allow spacebar to work.
   -- enh (gil) Allow selection in empty area when no timing is active.  Provides a paste target without timing marks.
   -- bug (gil) Store media directory and fseq directory in global config instead of networks file. Fixes #173.
4.0.24  Apr 23, 2015
   -- enh (gil) Clicking on effect while model play is paused will restart effect play.  I believe this will remove some confusion
                 for one case where you click on an effect and it doesn't appear to do anything.
   -- enh (gil) Spacebar always turns on/off model play.  Use pause button to pause effect play.
   -- enh (gil) Always trigger selected effect even if already selected since it may have been paused.
   -- bug (gil) Unpausing from spacebar should restart from pause point.
   -- bug (gil) Fan effect not working correctly when radius2 > radius1
4.0.23  Apr 20, 2015
   -- enh (gil) Add text fields to the Morph effect.
   -- enh (gil) New Effect: Fan
   -- enh (gil) Created macros for handling slider and text control events
   -- enh (gil) Add linear fade option to Circles effect
   -- enh (gil) Add text field to Pictures effect and inverse Pictures Y position slider.
   -- enh (gil) Galaxy effect update.  New options for edge blending and spiral inwards.
4.0.22  Apr 15, 2015
   -- bug  (gil) Marking timing marks with "t" does not create start time and end time #212
   -- bug (dkulp) crashes when add timing mark to sequencer, then click delete #210
   -- enh (sean) Include two icon files. xlights_nutcracker.ico and xlights_orig.ico. By default the xlights_nutcracker.ico is used.
      This icon (Nutcracker) matches the icon used on the wiki page. http://www.nutcracker123.com/wk/index.php?title=Main_Page
      to change icon to the original xLights icon. Copy xlights_orig.icon on top of xlights.ico. You will find these icon files
      in the installation directory. On my Windows 7 machine that would be C:\Program Files (x86)\xLights
4.0.21  Apr 14, 2015
   -- bug (dkulp) App will crash when importing duplicate timing tracks. #208
   -- bug (dkulp) Crash after export #201
4.0.20  Apr 14, 2015
   -- enh (gil) Remove multipliers from Galaxy effect and add text fields.
   -- bug (dkulp) 4.0.19 ignores xlights_keybindings.xml file #203
   -- enh (dkulp) Model Groups can be added to the sequence grid and have effects
         and layer associated with them directly. No need to create Whole House Models from the groups.
4.0.19  Apr 12, 2015
   -- enh (gil) Copy/Paste for multiple effect selection.
   -- enh (gil) Added settings option to toggle between showing effect icons and backgrounds.
   -- enh (gil) New effect: Galaxy (similar to SuperStar Spiral effect)
   -- bug (gil) Prevent timings from being inserted over other timings due to rounding of position to time conversion.
   -- bug (gil) Fix Model dialogues issues caused by sorting.
   -- bug (gil) Allow 25ms and 100ms sequences and limit which timings are available to be added.  Fixes #198.
4.0.18  Apr 10, 2015
   -- enh (gil) Provide 2 render modes. Erase mode clears canvas then renders. Canvas mode renders over old data. #141
   -- bug (gil) Display sequence duration cutoff on Timeline and honor it during playback. Fixes #193.
   -- bug (gil) Fix adding timing sections to open sequence. Fixes #195.
   -- bug (dkulp) WholeHouseModel now records the node type and uses that info at rendering.  #177
   -- bug/enh (dkulp) Bunches of fixes in dialogues and rendering for Retina and HiDPI displays
   -- change (dkulp) Background image is proportionally scaled to best fill the area
   -- bug (dkulp/gil) fix problems on the Preview related to mouse positioning #124
   -- bug (dkulp) Use floats for the model sizing to provide better control as well as problems converting to/from ints causing truncation and such.
   -- enh (dkulp) add updates to Model dialog to allow controlling pixel rendering.
                  Includes transparency of pixels, size of pixels, and "type" of rendering (point, smooth point, circle, blended circle)
   -- bug () Incoming file conversion not correct #80
4.0.17  Apr 8, 2015
   -- enh (dkulp) Add changes to support older graphics cards. This code fixes white boxes seen in the sequencer grid.
   -- bug (Gil) Delete model and effects from Sequencer when model is deleted from Preview page. #178
   -- bug (Gil) Fix Vixen import issue.
   -- bug (dkulp) You can't use a comma in text effect. :-) #188
   -- bug () Drag and drop crash #161
   -- bug () 4.0.14 Hot keys for O, D, U inconsistent #171
4.0.16  Apr 7, 2015
   -- enh (sean) Added new icons (16,24,32,48 bit sizes) for pictures, text, spirograph, twinkle,wave,tree,snowflakes,snowstorm
   -- enh (Gil) Add SuperStar import function (Morphs only).
   -- enh (dkulp) ctl-1 thru ctl-4 and alt-1 thru alt-4 now select the icon sizes for sequencer area and tools area respectively
   -- enh (dkulp) Bunches of size/position adjustments to try and get things to display better on HiDPI screens
   -- bug (dkulp) Fix potential crash when change to Preview tab.
   -- bug (gil) Models window Alphabetizing? #167
   -- bug (dkulp) 4.0.14 Fade Up, Fade down have improper timing #172
   -- enh (gil) Application allows multiple simultaneous instances to be launched #74. We will continue to allow multiple instances to be launched.
   -- enh (dkulp) Enhancement request: Effect icon sizes #162
4.0.15  Apr 6, 2015
   -- enh (dkulp) Add more key bindings.  Save/Load them to xlights_keybindings.xml.
   -- enh (dkulp) Add support for different icon sizes
   -- enh (sean) create dozens of new icons in 16x16, 24x24, 32x32 and 48x48 to support dan's new icon size  chooser
   -- enh (dkulp) Add ability to reset the toolbar locations back to default
4.0.14  Apr 4, 2015
   -- enh (dkulp) New keys bound as short-cuts for dropping effects. Do this instead of dragging effects.
   -- enh (gil) Allow timings to be added while model playback is paused. This is like xltap. Press play and use the "t" key to create timing marks
   -- bug (gil) Disallow effect play while paused in model playback.
   -- enh (gil) Add an empty timing track to a new sequence by default.
   -- enh (gil) Ability to add timing marks while sequence is playing
   -- bug (gil) Dropping or Control-V pasting into an empty area with a partial effect consumes the effect #160
   -- enh (gil)  When dragging effects, do not require timing track to be active #143
4.0.13  Apr 3, 2015
   -- bug (gil) When dropping in effect on top of another, original timing marks should be used #159
   -- enh (dkulp) Implement a simple copy/paste (single "cell" only, only into empty cells). Gil made Windows version work.
   -- bug (dkulp) Send the correct event so the effect panel updates
   -- bug (dkulp) Fix crash switching back and forth between Papagayo tab
   -- enh (gil) Smoother horizontal scrolling on effects grid.
   -- bug (gil) Fix it taking multiple clicks to get focus on an effect for delete/copy/etc.
   -- bug (gil) Don't allow dropping effect past last timing cell on right
   -- enh (gil) Improve efficiency of effect background render on grid.  Let OpenGL do the clipping.
   -- enh (gil) Improve morph acceleration option and add deceleration option
4.0.12  Apr 2, 2015
   -- enh (gil) Render Morph effect into background of effect layer. This follows on ON and COLORWASH
   -- bug () Add show directory to SETUP tab #156
   -- bug () Undocking panels for first time - not showing complete panel #147
   -- bug () Edit Display Elements window not full view #129
   -- bug () Text element with Horizontal matrix and output to lights enabled - Crash #127
   -- bug () Resizing effects then deleting - Effect still shows in preview #125
   Gil had this comment on his changes for Morph "Since Dan already laid the ground work for effect backgrounds
      I was able to code up the Morph in about 10 minutes.  I'm currently not handling when they clip on the
      window edges so I'm try to fix that and then check it in.  Most of these I used white for the head except
      for 2 in the middle you can see the head color transition."
4.0.11  Apr 1, 2015
   -- bug (gil) Fix convert error where E1 or E2 color values were changed. Example #E1987C was being changed into #E987C
   -- enh (dkulp) ON effect now paints the sequence grid with the actual color of the On
   -- enh (dkulp) COLORWASH now draws on sequencer area actual colors of effect
   -- enh (dkulp) STOP lights now, STOP gracefully, output to lights are now icons
   -- bug (dkulp) Edit Display Elements window not full view #129
   -- bug (gil) Not able to delete models in sequencer #122
   -- bug (sean) House preview screen in the sequencer tab version 4.0.10 #121
   -- bug (dkulp) Model assigning incorrect number of channels #126

4.0.10  Mar 31, 2015
   Gill added ability to position a picture in the X & Y direction
   -- bug (gil) Deleting a timing mark crashes X/Lights #118
   -- bug (gil) Enhancement: Picture Position #14
   -- bug (gil) Fade across timing marks not working correctly #56
   -- bug (sean) Fixed the Life, Pictures and Shimmer icons for effects.
4.0.9  Mar 31, 2015
   -- bug (gil) Move multiple elements at once using Render Position (move up/move down) in Sequence Elements Frame #114
   -- bug (dpitts) modified move model up and down in the Sequence Elements screen to keep the moving model in the screen
   -- bug ()   Fade across timing marks not working correctly #56
   -- bug ()   Tabs can be moved (like moving frames from the Sequence tab around) #57
   -- bug ()   Changing default perspective causes uncaptured crash on exit #64
   -- bug ()   dragging the main window larger (repeat of closed #69) #70
   -- bug ()   4.0.6 Hard Crash on opening subsequent sequence #110 (Just a retest to make sure still good)
4.0.8  Mar 28, 2015
      Gil Jones has been busy on this Saturday, Mar 28th
   -- bug (gil) Error message when audio file missing #116
   -- bug (gil) Drawing and selection issues in EffectsGrid #115
   -- bug (gil) Move multiple elements at once using Render Position (move up/move down) in Sequence Elements Frame #114
   -- bug (gil) Purple effect highlight in grid is not moving with the mouse click to next effect. #113
   -- bug (sean) Title bar says 4.0.67 versus 4.0.7 #112
   -- bug (gil) 4.0.6 File, New sequence does not scrub screen from previous sequence #111
   -- bug (gil) 4.0.6 Hard Crash on opening subsequent sequence #110
   -- bug (gil) Can't cancel New Sequence #76
   -- bug (gil) Ability to close the current open sequence #37
   -- bug (gil) Dragging selection of effects similar to #59 #79
   -- enh (sean) Created "House Preview" icon to be used on tool bar. This will cycle display on/off

4.0.7  Mar 27, 2015
   -- enh (dkulp) Whole House Preview is now part of SEQUENCER tab. This was a major code rewrite!
   -- bug (gil) Unable to delete models from the Sequence Element Screen #101
   -- bug (gil) Timing insert when near an endpoint causes overlapping timings. #100
   -- bug (gil) Adding and deleting models #54
   -- bug (gil) Waveform showing as black window #58
4.0.6  Mar 26, 2015
   A number of fixes from both Dan and Gil
   -- bug () Load Background image in 4.0.5 just gives a gray screen #107
   -- bug () 4.0.4 3-24 Crash every time I save this sequence #106
   -- bug () Win 7/8.1 issue #102
   -- bug () Model Preview does not display under Sequencer tab #95
   -- bug () Setup TAB - Directories my observed issue #93
   -- bug () No Icon displayed #109
4.0.5  Mar 25, 2015
   -- bug (dkulp) Move more items back into opengl. The preview window in the Sequencer tab is now updated
   -- enh (sean) Created windows icons for xlights to be higher resolution
4.0.4  Mar 24, 2015
   -- bug (dkulp) Dan has fixed it so aero and non-aero now work on both windows 7 and 8!
4.0.3   Mar 24,2015
   -- bug (gil) Fix animation playback in effect grid. Also noticed this fixed something I hadn't noticed where the pause/play wasn't restarting the render in the proper spot. Fixes #78.
   -- bug (dkulp) Fill out the pinwheel to the edges of a model. Use the max of the buffer width or height as basis for pinwheel arm length
   -- enh (dkulp) Experiment with different OpenGL setup for Windows. This seems to improve Windows 8 display
4.0.2   Mar 23,2015
   -- bug (dkulp) Only RED shows when outputting from the SEQUENCER tab #92
   -- bug (gil) Waveform does not respond to mouse wheel zooming unless effect grid has focus. #90
4.0.1   Mar 21,2015
      Many bugs fixed. Go here to see closed bugs. https://github.com/smeighan/xLights/issues?q=is%3Aissue+is%3Aclosed
   -- bug (Gil) Must click in EffectGrid or timing marks can't be added/modified #89
   -- enh (Gil) Add ability to split timing marks #88
   -- bug (Gil) Single strand and garland use the same icon #86
   -- bug (Gil) Crash when creating new sequence with no timing track selected #84
   -- bug (Gil) Request: Zoom level minimum for effect edge dragging #83
   -- bug (Gil) Right scrolling arrow #67
   -- bug (Gil) Zoom doesn't focus on selection target #77

         Some new features: Click in the waveform and the type "s", the current timing mark will be split in half. This complements the "t" command that creates timing marks.
         Double clicking in the waveform will zoom in. Ctl-double click in the waveform will zoom out
4.0.0   Mar 20,2015
    First Beta release of xLights 4. Dan Kulp, Dave Pitts, Gil Jones, Sean
   -- There are still bugs in this release. Go to https://github.com/smeighan/xLights/issues to see current bugs and enhancements
       -- You need to run Windows 7 and 8 in non-aero mode.
       -- Make a backup of your current sequecnes. Copy over the sound files and the xml files (xseq and fseq do not need to be copied)
   -- If you want exact placement of the cursor inside the waveform, resample sound files to 192k constant bitrate using audacity
   -- In 4.0 we do not have the "Create Random Effects" button, that will come out in XL 4.1


============================================== xLights 3.6.x =========================================================================

3.6.15 Dec 25, 2014
   -enh (dkulp) Modify F3 so that effects like morph can try to work.
    -enh (djulien) Allow option to continue Preview playback if channel data is missing from .xseq file (helps avoid the need to re-render everything if you've added props at the end)
    -enh (djulien) Default Brightness to 100% if setting not present in XML file (only affects externally generated XML files)
    -enh (djulien) show elapsed time on file loads
    -bug (djulien) Fixed Wave effect when rendered in a .xseq (FillColor was not selected, which resulted in no output)
3.6.14 Dec 17, 2014
    -enh (djulien) Add collapse/expand button (located next to "Create Random Effects")
      to allow more grid space on laptops
    -enh (djulien) Add "My Display" and "Brightness" columns to Preview model CSV - makes it easier to see why a prop is not getting any data. 8/
    -bug (djulien) Avoid "error 0" when exporting CSV of Preview models.  Show correct value for "start channel#".
    -enh (gjones) Update On effect to have 3 color sliders, associated text fields, and bitmap showing selected color.
                  The bitmap button can be used to select the color which will update the sliders and text fields.
    -bug (dkulp) Should not be able to select fseq for model export, only eseq
    -bug (dkulp) On OSX only, could not open notes and map files that used txt extensions.
    -enh (gjones) Add On effect.  This effect simply allows you to turn on all pixels in a model to the selected color.
    -bug (gjones) Remove extraneous whitespace from the LMS filetype in the drop-down on the file select dialog when
                  selecting a file to convert.
    -bug (dkulp) Use cos/sin/radians/etc... to render circles effect.  Removes some extra noise.
    -enh (dkulp) When loading a sequence on Nutcracker tab, reset flags so you don't also need to re-load it on Preview pane
3.6.13 Dec 9, 2014
   -enh (dkulp) Allow a skip size of zero. This allow the skips effect to stand still
    -enh (djulien) Allow continue if port open fails (useful for diagnostics/debug)
    -enh (djulien) Add up-once and down-once options to Picture.
    -enh (djulien) Add Bubble option to Circles effect.
    -enh (djulien) Add Decaying Sine and Ivy/Fractal Wave options.
    -bug (djulien) Remove extraneous leading "-" in FV attr in xlights_papagayo.xml
    -bug (djulien) Sort list of channel#s in drop-down list on Papagayo tab
3.6.12 Dec 5, 2014
    -bug (dkulp) If Column 1 renders really fast, it could hang on save due to Thread 2 waiting for
      information from thread 1 which is already gone. This could cause hangs on save.
    -bug (dkulp) Fix some thread safety issues in RenderCircle, RenderText, RenderPicture. This would have muged the effects.
    -test (dkulp) Try flipping to a busy wait for TextEffect rendering to see if that prevents some crashes/hangs
    -enh (dkulp) RenderPictures now has a 20FPS check box so rendering can just advance per frame if the movie is already in 20fps.
      If unchecked, the "Speed" slider adjusts the speed (with 10 being "normal", <10 slower, >10 faster). A speed of 10 = 20fps.
3.6.11 Dec 4, 2014
    -enh (dkulp) Add a "Settings" menu.  First setting: ability to disable the multi-threaded saves.
         When disable, all is rendered on the main thread.  Slower, but maybe safer.
         With this change, there are now four ways to save
         1) Normal - threaded
         2) Normal + Fast Save - threaded but only rendering the stuff that has changed
         3) Threading disabled (settings menu) - all rendering is on the main thread, one column at a time.
         4) Threading disabled + fast save - all rendering is on the main thread,
            one column at a time, but starting with the top left most changed cell.
    -bug (dkulp) When "Play" hits the bottom of the grid, the "Stop" button wasn't changed back to "Play"
   -bug (sean) Ripple effect was not calculating corners correctly
   -enh (sean) Added thickness slider and 3D button to Ripple.
3.6.10   Dec 3, 2014
   -bug (sean) Added missing xLights.ico file. You should see icon in tray now
   -bug (sean) "Create Random Effects"Would create None,None about 20% of time. This is now fixed.
   -bug (sean) Ripple was not rendering to PREVIEW screen or to actual lights. fixed
   -bug (dkulp) If "Save" is clicked multiple times real quick, it could hang/crash as it tries to save/render things at the same time.
         Fixed by disabling the Save buttons while saving.
   -bug (dkulp) Insert row inserting two rows, not one
   -bug (dkulp) On the MAC, the clipboard sometimes used \r instead of \n.
      (From Numbers for example) Update the paste to the grid to accommodate that.
   -enh (dkulp) Change the "Play (F4)" button in the NUTCRACKER tab to be a Play/Stop toggle.
3.6.9   Nov 29, 2014
    -bug (dkulp) Move Text effect rendering back to main thread due GDI calls it makes (to get Font info and to
      render the text) which is required to be done on the main thread.
    -bug (dkulp) Custom models that don't have an element in the grid for all channels in the range would cause
      buffer overruns on ModelExport
    -bug (dkulp) ModelExport was not using the SettingsMap to determine the state of the "Persistent" checkbox
      and instead looked at the actual checkbox on the page.
    -enh (dkulp) MAC - turn off anti-aliased text rendering to make it match Windows (and look better)
    -bug (dkulp) Model Export was crashing, now fixed.
3.6.8   Nov 28, 2014
   -enh (sean) Modified install program so that it has an auto launch button for Windows installs
    -enh (mbrown) Removed check that number of Renard channels is a multiple of 8.
    -enh (dkulp) Add ability to ignore LOR channels that do not have a deviceType and network defined. (common
                  for beat tracks and unused channels.
   -bug (sean) Fixed label creation of SETUP and convert so export of lms file now creates the RGB data
   -enh (dpitts) Added Preview Groups into PREVIEW tab, Create a group, when you select that group the
         "Part of My Display" will be set automatically for you. This allows you to set groups of models and be
         able to turn on the "Part of My Display"
    -bug (dkulp) Fix a bug where the LastChannelCount always assumed RGB nodes
    -bug (dkulp) On Mac, could not paste stuff into custom model due to using \r instead of \n for line delimiter
    -enh (dkulp) SPEED: Multi-thread the Nutcracker tab saving to speed up the rendering/saving
    -enh (dkulp) SPEED: Change the internal representation of a color to avoid some unnecessary OS calls.  Results in
                  significantly faster rendering of effects.
    -enh (dkulp) SPEED: Add "Fast Save" option on Nutcracker tab to allow the save command to only render the cells below
                  and to the right of the cells that have changed.  Placing the models you are working on to the
                  right on the grid can significantly speed up turn around time if you flip between Preview and
                  Nutcracker often.
    -bug (dkulp) Fix bug in SingelStrand chase effect that could cause a crash/memory corruption if used on
                  models with more than 1000 nodes.  (like whole house)
    -bug (dkulp) Fix a crash that would occur if the image effect was used, but no image was available.
    -bug (dkulp) Column  Shift left/right now copies cell protection.
3.6.7   Nov 23, 2014
   -bug (djulien) scrunch UI so it will fit on a laptop (needs to be < 760 px high)
    -bug (dkulp) Fix Rendering of Ripple effect during save, now shows up in PREVIEW and lights
    -bug (dkulp) Fix Rendering of Strobe effect during save, now shows up in PREVIEW and lights
3.6.6   Nov 22, 2014
   -bug (dkulp) Size of picture and setting size of display window in PREVIEW could cause a condition where you
         could not select items. fixed.
   -enh (cpinkham) Add 'Pixelnet-Open' Network Type for Pixelnet output using Generic FTDI chipset USB to RS485 dongles.
   -enh (dkulp) During conversions, buffer non-critical updates to the messages list and append
      to the list box in blocks of about 10K.   Significantly speeds up conversions for HLS and LOR
      with very large files that display a lot of messages.
3.6.5   Nov 20, 2014
   -enh (sean) New effect class , Strobe.
   -enh (sean) Icons on top of each effect improved. This icons will be what you drag and drop effects in xLights 4.0. This early introduction
      is to help familiarize users with the icons to an effect.
   -bug (frankr) Non music sequences would freeze preview play. fixed
   -enh (djulien) Add Papagayo option for scaled vs. fixed picture mode
   -enh (djulien) Allow Papagayo grid row labels to be aliased (click on row label to change it); cosmetic only - does not change xml file
   -bug (djulien) Avoid "error 0" after writing new Papagayo file
    -bug (dkulp) SingleLine models with multiple string bug one node per string were displayed as a single light
    -bug (dkulp) Rotating single lines would cause the number of lights to display to drop by 2. (lose the ends)
    -enh (dkulp) Added pixel level "Skips" effect to SingleStrand effects.
    -enh (dkulp) Add ability to shift columns in the grid left or right to reorder them.
    -enh (dkulp) If the effect is NONE and the buffer is already clear, skip clearing it.  For large Whole House models, clearing the
                buffer can take time so skip it if we can.
    -bug (dkulp)  If the preview is too large for the viewport such that part of the preview was cut off
                at the top, clicking wasn't adjusting the Y coordinate to compensate when determining what
                was being clicked on.
3.6.4   Nov 12, 2014
   -bug (frankr) Fixed playback issue. If you play a sequence in the PREVIEW tab, stop, go and edit some effects in the NUYTCRACKER tab,
         press save and then go back to the PREVIEW tab, xlights would freeze up. This should be fixed now
   -enh (frankr) improvements to audio scrubbing and better handling of stop now button on
   -bug (frankr) When first entering teh PREVIEW tab , if you right click and select model xlights crashed. Fixed.
   -enh (sean) New effect "Off". This effect is the equivalent of "Colorwash, 1color=BLACK"
   -enh (sean) When saving a sequence the elapsed time for the save is now shown in the bottom status line.
   -enh (sean) New favicons have been added to the left most corner of Effects. The purpose of these is to have a graphical
         representation of effects. This is pre work for the new xlights 4.0 coming early next year. In xlights 4, effects will
         be dropped onto horizontal timelines, for for each model.
    -bug (dkulp) OSX - selecting cells on the Nutcracker grid was not updating the colours on the palette buttons
    -bug (dkulp) On OSX Yosemite, the Font dialogue for the text effect would cause a crash.
   -bug (dkulp) sometimes xlights would crash when you selected multiple objects and then right clicked to the alignment menu. Fixed

3.6.3   Nov 11, 2014
    -bug (dkulp) Fix problem of stars using a single RGB node per concentric layer not displaying correctly.
   -enh (sean) Moved "Create Whole House Model" button to be higher on the screen. This helps people with lower
      resolution computers.
   -enh (sean) reformatted Pinwheel screen so that it does not take as much screen to draw form.
3.6.2   Nov 7, 2014
    -enh (dkulp) Tree360 and Tree270 can now specify which strand to use for the first column when exporting and individual model.
                    This allows the exported models to work around issues with the 12:00 first strand for Nutcracker but the first
                    strand is expected to be the 9:00 position in HLS.
   -enh (frankr) New play,pause,stop buttons in PREVIEW screen. Moving the slider plays sequence data and plays sound,
      Move it to place where you want to start. Press PLAY, sequence starts from there. STOP button resets sequence to 0:0.0
   -enh (dpitts) New feature to select multiple objects in the PREVIEW window. 1st) Click and select first object that will be used
      as baseline. 2nd) Hold the shift key down and draw a rectangle. Any objects that fall within the rectangle will be selected.
      3rd) Right click and select how you want the selected objects to be aligned (Top, Bottom, Left, Right, Horizontal Center,
         Vertical Center). If items are stretched out horizontally, use Top/Bottom. If item are stretched out vertically, use
         Left/Right.
         A group that is highlighted in yellow can be moved anywhere on the screen, hold the ctrl key down before moving.
         Note: A highlighted group cannot be sized or rotated. That will be added in a future release.
      NOTE: If you screw something up, exit without saving and come back in. Later we will add an undo key.
      NOTE2: If you have been editing your preview and it looks good, do a Save on the preview screen. Also do a "File,Backup".
      This will save your current preview work AND make a copy of it in the Backup subdirectory.
3.6.1   Nov 6,2014
   -bug (dkulp) Pinwheel effect was not producing data in Preview screen or on lights, it is fixed now
   -enh (sean) Move Effect 1 and Effect 2 into tabs  . This gives more space for the preview screen
   -enh (dpitts) Selected objects in the preview are yellow , not magenta
3.6.0   Nov 6,2014
   -enh (dpitts) NUTCRACKER and PREVIEW windows now use OpenGL to draw objects. The result of this is xLights can now draw very
      large objects with no delay.
   -enh (dpitts) New ability to load an image into the background of the PREVIEW tab. There is a brightness slider to dim the image you
      just loaded to give it a dusk time feel.
   -enh (dpitts) New ability to set the resolution of the PREVIEW screen. Set the size and it will be remembered. This is needed to
            help when making Whole House Models. This way you can always come back to the layout you had been working on.
   -enh (dpitts) You can now click and drag objects in the PREVIEW tab. If there are multiple objects under your mouse click multiple
      times and it will cycle through all of them
   -enh (dpitts) Now objects are sized and rotated about their graphical center, This means objects do not move around
      the screen if you resize or rotate.
   -enh (dpitts) New rotate handles. Grab the handle and move mouse to rotate. Highlighted model is now magenta insteads of yellow
   -enh (sean) Added Pinwheel effect
   -bug (dpitts) Model Brightness Now works
   -enh (dkulp) Allow star layers to be ordered low to high (to inside layer first)
   -bug (captmurdoch,materdaddy,dkulp) Modify make files and libraries so 3.6.0 can be compiled on MAC and on Linux
3.5.2   Oct 30,2014
   -bug (frankr) Fix for preview moving some elements into the wrong place. Before the ability to rotate any element
      rotations were in 3 degree increments, now they are 1 degree. This change caused existing previews to draw elements in
      a new position. New code detects old rotation or new rotation values and draws both correctly.
   -bug (frankr) Fixed issue related to  views when switching to new show folder. Under some circumstances, xlights would crash.
3.5.1   Oct 30, 2014
   -bug (dpitts) Found bug that causes crash. Single color models that have multiple lights (think 1 string of single color
      with 50 lights uses 1 channel, but draws 50 places) was not allocating memory correctly for each pseudo light.
   -enh (frankr) Ability added to preview screen, you now can rotate any object not just single string.
3.5.0   Oct 29, 2014
   -enh (dpitts) New button added to the preview screen. This will create a Whole House Model for you automatically.
      There is now a new type of model "Whole House". You no longer need to use spreadsheets and the custom model.
      Procedure: Layout your preview. When everything is correct, press "Create Whole House Model". Give it a name.
      This new whole house model should NOT have "Part of my Display" checked.
      Add the new whole house model as a column in your Nutcracker grid.
      enjoy the new feature, many thanks to Dave Pitts!!
   -bug (dkulp) fixed issue with Star model with users sequences that were older than Oct 20th. This was crashing
      xlights at launch time.
3.4.24   Oct 28, 2014
   -enh (dpitts) New feature to allow creating views. A view is a collection of models. When you select a view it acts
      like a filter and only shows you the models contained in your view. T
    -enh (dkulp) Star model can now have multiple star size “layers” to model exploding star type things
                     Example: a 100 node star may have 50 for the outer layer, 30 for the middle, 20 for the inner.
   -bug (djulien) Fixed multi-cell morph (caused Papagayo flicker), avoid error if try to save file with no pgo data
   -enh (sean) Increased custom model from 300x300 to 1000x1000
3.4.23   Oct 25, 2014
   -enh (sean) Added Ripple Effect Class
   -bug (sean) Recompiled entire rpoject , we believe a few modeuls were compiled with debug and that was
            causing the error message in the model screen. Since that recompile, we have not see the error
   -bug (djulien) Fixed up auto-fade arithmetic (morph flicker still occurs); fix pgo grid column label with presets > 4 voices
   -enh (djulien) Papgayo: Rework auto-rest options, insert rests between phonemes
3.4.22   Oct 23, 2014
   -enh (sean) Added new effect class "Shimmer". This effect is basically a square wave, it turns all lights on and then all
      lights off.
   -enh (sean) Removed FACES, COROFACES and PINWHEEL from the "Create Random Effects" button
   -enh (djulien) Change Pgo "auto fade element" to "auto rest" (not functional yet), leave initial Pgo Preset cleared for stitch
3.4.21   Oct 20, 2014
   -enh (djulien) Added model CSV export to the Models screen. This csv file can be opened by Excel or
      Open Office spreadsheets
3.4.20   Oct 19, 2014
   -enh (djulien) Added Papagayo Preset stitch (allows Presets to be combined dynamically at run time)
   NOTES:
    - Papagayo Presets match by model or voice name now rather than column#.  This means they can move around between columns and still work.
    - The format of the xlights_papagayo.xml file (where pgo Presets are stored) changed but should be backward-compatible.
    - Custom model info is cached with the Presets; if you change your Custom Models then re-Save the associated Presets
    - workflow for horizontal (side-by-side) stitch of multiple pgo files + presets is as follows:
      1. clear grid
      2. load first pgo file
      3. select corresponding Preset; this will "bind" the Preset info to the voice(s) in the pgo file
      4. load next pgo file using "+" button, specify "0" for #frames to shift
      5. select corresponding Preset
      6. select output file if not already set
      7. click button at the bottom to generate the sequence
      8. load additional pgo files as desired; for vertical stitch (concatenate) enter a non-0 value for #frames to shift
3.4.19   Oct 18, 2014
   -bug (djulien) Fixed Coro Faces render problem on models with > 26 columns, adjust text size on model zoom in/out, Papagayo Preset delete button
   -enh (djulien) Added Papagayo horizontal stitch (supports any number of voices within reason), accept 39 extended phonemes (auto-map to basic 10), removed duplicate phoneme in frame message (CAUTION: phonemes will be dropped with no warnings), added Preset consistency check, start making Model dialog resizable, show reason for "reseting flags" message
3.4.18   Oct 13,2014
   -enh (djulien) Merged Papagayo sub-tabs; implemented Image option (Movie option TBD); enabled fade on Auto faces; added Copy column button; flattened xlights_papagayo.xml file (old Presets still used for backwards compatibility, but new Presets are written a level higher and merged)
   -bug (djulien) Fixed Papagayo auto-fade; mostly works now, but there seems to still be some flicker (caused by something else within xLights)
   -enh (sean)    New effect class PinWheel. This is NOT working yet, it is a place holder.
3.4.17   Oct 8, 2014
   PRE-RELEASE. There are still bugs we are working in the corofaces code. One we know of , outline does not show in PREVIEW
   -enh (djulien) This release sees the Papagayo Tab for Corofaces working.
               To use this, choose a pgo file as input, choose a file for output.
               Choose your custom models from a pull down list. Be sure they have "Part of my Display" checked.
               Assign channels to Phonemes. You can assign multiple channels by doing ctl-click.
               Create an xml file of the pgo.
3.4.16   Oct 1, 2014
   -enh (djulien) Add zoom in/out buttons on Custom Model grid display (makes it easier to see larger grids)
   -enh (djulien) On Papagayo Coro faces tab, moved model name into grid, show drop-down list of choices in grid cells, allow Pagagayo file stitch ("+" button), add UI controls for face element auto-fade, save/restore last tab, show list of checkboxes in Coro Faces effect panel
3.4.15  Sep 28, 2014
   - ENH (Sean) merge_xml.awk will combine multiple xlights xml sequences into one.
   - ENH (Sean) corofaces.awk awk now will stitch together multiple files for the same voice.
3.4.14  Sep 26, 2014
   - ENH (Sean) scripts added that can take Papagayo voices and make xlights sequences from them
   - ENH (Sean) Scripts are found in your installation directory. corofaces.awk, coroface.bat
   - ENH (Sean) Installation of xlights now installs gawk.
3.4.13   Sep 19, 2014
   -ENH (sean) The scale slider in the PREVIEW tab now goes to 300. before range was 1-100, now 1-300
   -bug (djulien) Avoid wx assert error during save/load pgo settings (bad grid row#), default to first group after loading settings (however, this can lead to mismatches), don't show "suspicious" message for missing voice data, list all models (even if not in Preview)
   -bug (djulien) Allow pgo parser to tolerate blank voices
3.4.12   Aug 19, 2014
     --  enh (dkulp) HLS import - now uses wxYield to display progress during load
     --  enh (dkulp) HLS import - if an HLS universe has less channels than configured in xLights, use the xLights value.
                     If the HLS universe has more, display a warning.
   -ENH (sean) 4th Open option of Nuctacrker, "New Animation Sequence", now allows fractional times. Before
      only values like "10,11,15" were allowed. Now "10.123,11.32123,15.2"
3.4.11   Aug 15, 2014
   -ENH (sean) Single Strand Effect now can use "Fit to Time".
      If this is checked, one cycle of an arch will be done.
      It will exactly fit the time on the current row of the grid.
   -Bug (sean) Two places in the code would cause a crash. Both have been fixed.
3.4.10   Aug 14, 2014
   -ENH (sean) complete rewrite of Single Strand effect. New sliders: Number of Chases,
      color (Rainbow,palette)
3.4.9   Aug 7, 2014
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
     --  enh (dkulp) If an empty LMS channel is part of an RGB group, don’t consider it empty.
     --  enh (dkulp) Add a checkbox to TabConvert to go ahead and map the empty channels
     -- bug fix (mbrown) Fix Custom RGB Models that have multiple nodes with the same number
     -- bug fix (djulien) fix segv due to incorrect compiler code in Piano effect
     -- bug fix (mbrown) Custom models now work with traditional strings
3.4.4   Jul 2, 2014
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
3.4.3   Jun 30, 2014
   -Enh (dkulp) Reduce memory usage of LMS import
     --  enh (dkulp) Change XML parser to spxml to avoid loading entire file into memory
3.4.2   Jun 26, 2014
     -- bug fix (dkulp) Problems loading large LMS files causing crashes/hangs
     --  enh (dkulp) Add additional status messages while loading LMS files
3.4.1   Jun 19, 2014
   -Bug fix (sean) Butterfly effect now creates pixels at {0,1} and {1,0} correctly
   -Enh (sean) added two new butterfly effects
   -Enh (sean) Work in progress. New Papagayo tab. This tab will be used to convert Papagayo files (.pgo) into
      xLight sequences
3.4.0   Jun 5, 2014
   -Bug fix (djulien) tried to fix Piano key wrap (again); added PianoTestNotes.txt file for easier testing (in case more changes are needed)
   -Bug fix (djulien) fixed cross-fade (morph); didn't write to xseq file correctly
   -Bug fix (djulien) add missing tool tip for Text center-and-stop checkbox
   -Enh (djulien) make Preview element size editable; not sure why it wasn't; NOTE: must have a Display Element selected first
3.3.9   May 22, 2014
   -Bug fix (sean). Glediator effects were short one row in the Y direction. This has now been fixed.
3.3.8   May 13, 2014
   -Enhancement (sean) New Effect, Glediator. Save a recorder file from Glediator. Open file and put in your grid.
   -Bug fix (sean). Glediator conversion now works. Select a *.gled file in the CONVERT tab and change into any of the output
      formats.
   -Bug fix, reported (djulien) draw white correctly if it's one of the Piano colors
   -Bug fix (Sean), Fixed Preview for Wave and Glediator. You could see this effects in Nutcracker tab but not in Preview.
      all fixed.
   -Enhancement, requested (djulien) center Piano keys if Clip is unchecked
3.3.7   May 9, 2014
   -Bug fix (dkulp) Added some additional checks to fix the heartbeat.cfg error.
3.3.6   May 08, 2014
   -Bug fix (djulien) avoid horiz or vert line flashes with plasma balls (Circles effect, Plasma option)
   -Bug fix (djulien) fix piano color caching (reported by multiple people)
   -Enhancement (sean) Added direction drop down for butterfly effect. "Normal" is the effect moving from
      right to left. this is the way it has always been. "Reverse" changes direction
   -Enhancement (sean) New file type in CONVERT tab, *.gled. Create these files using the record button of Glediator.
      Conversion code is still under development.
3.3.5   April 23, 2014
   -Enhancement (Sean). New effect called Wave. This effect will plot a Sine ,
      Triangle and square wave on your display.
   -Enhancement (djulien). ToopTip text added for Morph.
3.3.4   April 17, 2014
   -Enhancement, (sean). Faces effect now creates a scalable face on your matrix or megatree
   -Bug fix, (Sean). Entering a RETURN in the text fields of the the TEXT effect no longer crashes xLights
   -Enhancement, requested (djulien): add center-and-stop option to text movement options (new checkbox);
      sync text lines; use left/right spaces to delay left/right movement or left/right "\n" to delay up/down movement
   -Bug fix (djulien): allow "," within Text lines (caused "not found" error)
   -Enhancement (djulien): add MORPH checkbox to enable gradual Effect 1 -> Effect 2 transition;
      can be used for ramp/fade, morph text or image, etc.
   -Enhancement, requested (djulien): add menu item to clear all effects on this row
   -Bug fix (djulien): copy/paste text to Text effect Line 1 - 4, don't interfere with grid
3.3.3   April 3, 2014
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
3.3.2   Jan 28, 2014
   -Bug fix (dkulp): When importing LOR *.lms files, certain missing data
      would cause a crash.    conversion now fills in any of those missing
      parameters and conversion completes.
3.3.1   Jan 27, 2014
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
3.2.14   Dec 18, 2013
   -Enhancement (Sean): SingleStrand effects are now moving twice as fast.
   -Enhancement (Frankr) * Added alternating bands options to the bars effect.
   -Enhancement (Frankr) Removed limitation on pasting into the timings and label columns.
3.2.13   Dec 14, 2013
   -Bug fix (djulien): display some details about channel count error
   -Bug fix (djulien): split Effects Direction into separate lists for Text and Picture (some Picture are not applicable to Text)
   -Enhancement (djulien): More movement options in the pictures effect.
   -Enhancement (Sean): Added new song to songs directory.
      (It Must've Been Ol') Santa Claus, Harry Connick Jr.
3.2.12   Dec 11, 2013
   - Enhancement (Sean) Updated all files in songs directory to have readme files showing
      link to where to purchase music. Added some new songs.
3.2.11   Dec 6, 2013
   - Enhancement (Frankr) Nodes per string increased to 65536, You can now create a model that
      will do your entire house.
   -Bug fix (djulien): set Piano shape + map default file path to Piano subfolder within xLights folder, not up one level
3.2.10   Dec 5, 2013
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
3.2.8   Nov 21, 2013
   - Enhancement (Sean) Re-release of single strand effects. Now they work without need to set
         "fit to time".
3.2.7   Nov 15, 2013
   - Bug (frankr) Windows model displayed wrong channel counts.
   - Enhancement (frankr) You can now export just one model instead of every channel that is in your setup
      1st method: "export" This does not look at models and instead exports every channel that has been defined
         in your setup Tab
      2nd method (new): "Export Model". You now can pick the model that is attached to your sequence and only export
         those channels. For example, I have megatree, arches, eaves. If I select megatree only the channels
         associated to that model will be exported. This will make much, much smaller LOR lms,lcb or vixen vix,vir files.
3.2.6   Nov 10, 2013
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
3.2.5   Nov 9, 2013
   - Bug (Frankr) removed 'U' for updating grid. Now only F5 is shortcut key. The 'u' caused a problem
      when typing in text.
3.2.4   Nov 7, 2013
   - Enhancement (Djulien) New  filtering on every slider and checkbox for "Create Random Effects"
   - Enhancement (Frankr) F5 or 'U' now performs an update grid
   - Enhancement (Sean) New directory "songs" has audacity label files for many songs. Find the new directory
      at C:\Program Files (x86)\xLights\songs
3.2.3   Oct 25, 2013
   - Bug: (Dan) Schedule tab missing on Linux
    - Enhancement: (Dan) When outputting e1.31, only output the used channels instead of the full 512 universe.
      (Lower network traffic, less processing needed)
   - Bug: (Dan) Text Effect not rendering on OSX.  Fixed.
   - Enhancement: (Sean) New Single Strand Effect class. Use this for arches and eaves
3.2.2   Sep 26, 2013
   - Enhancement: (Dan) When importing hlsIdata, sort by the universes number to make the import predictable and repeatable.
   - Enhancement: (Dan) When importing hlsIdata, change the file extension for the Audio from PCM to mp3.
   - Bug: (Dan) If there is a named test setup, much of the functionality on the Setup tab (up/down arrows,
      modifying a network, etc..) did not work properly and could result in modifying the wrong network.  Fixed.
   - Bug: (Dan) Preview/Nutcracker tab drawing on the Mac would result in "ghosts" of the flashing lights due to anti-aliasing. Fixed.
   - Bug: (Dan) At certain screen sizes, some pixels in mega-trees and other Nutcracker models may not display.  Fixed.

3.2.1 Sep 13, 2013
   - Enhancement: (Frank). In the Model section you now see start and end channels, not just start
   - Bug: (Sean). When exporting HLS sequences if the number of channels was not evely divisible by 3, xLights would crash. Fixed.
   - Enhancement: (Frank). Switched the grids for start channel and custom model to be in scrolling
      windows so that larger data sets can be handled.
   - Bug: (Matt). Smart RGB arches are only producing a single channel. Fixed
   - Enhancement: (Matt). F1 will bring up help if you are in the Profile tab
3.2.0 Sep 12, 2013
   - Enhancement: (Matt). In the Model creation you can now select type of string that the model will use.
      There are new string types available. You can pick non-rgb strings. These will use a single channel, For preview purposes
      you can pick RED, GREEN,BLUE, WHITE as single colour strings.
   - Enhancement: (Matt). You can now select a string type that is strobe lights. This light will flash 3 times per second
      for each bulb.
   - Enhancement: (Matt). You can now pick a multi pointed star as a display element.
   - Enhancement: (Matt). You can select a wreath as a display element.
   - Enhancement: (matt). You can select a string type of 3 channel RGB (In other words dumb rgb strings)
   NOTE! With the change to the latest wxWidgets 2.9.5 , Mac 10.6.8 is no longer supported. We now will only support 10.7 and higher
3.1.4  Sep 4, 2013
   - Enhancement: (Matt). Custom Model now uses pixel count 1..n. Models can be put on preview. For example,
      you can define a candy cane out of rgb pixels and then place candy cane in the preview window. Very Nice!
    - Enhancement: (Matt) You can now copy models.
   - Enhancement: (Matt) You can now rotate single strand models in the preview window.
   - Enhancement: (Matt) There is a Sequence open button on the Preview tab. If you use this to open a sequence there is
      an option to set the "Part of my Display" checkboxes to match the models that just got opened.
3.1.3  Aug 27,2013
   - Enhancement: (Matt). New Effect Curtain.
   - Enhancement: (Matt)  New tab PREVIEW. There is a new tab that will allow you to see all of your
      models displaying effects.
   - Bug (Frank) Fixed channel numbering for single string and Window models
3.1.2  Aug 17,2013
   - Enhancement: (Frankr). Circle effect now has new option called plasma.
   - Enhancement: (Matt Brown).  bars compress & expand so that the two halves mirror each other.
   - Bug: (Frankr).  found and fixed the issues with fire and meteors that were being discussed at DIYC.
   - Enhancement (Dan Kulp). New import from HLS now available on CONVERT tab
   - Enhancement (Frankr). New option in spirals to allow the thickness to grow or shrink
3.1.1  Aug 12,2013
   - Bug Fix: (Matt Brown). Matt has fixed the Meteor effect. He has also enhanced it so meteors
      can fall to the left or right.
      We have   renamed   Starfield to be called "explode" and "implode"
   - Enhancement: (Matt Brown)  I made some improvements to countdown & skip blank cells logic. I also changed
      a couple of method names to make them more clear. Also, the timer now skips if the last interval takes more than 50ms to process.
      Before it was blocking until the last interval finished (so if every interval took more than 50ms you had a stackup of waiting timer calls to process).
3.1.0  Aug 7,2013
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
3.0.24 Aug 4,2013
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
3.0.23 Jul 28,2013
   - Enhancement: (sean) Fire effect now allows hue shift. You can now have green, blue fire.
    - Enhancement: (sean) Fire effect now has a "Grow Fire?". If you click yes, the height of the fire will grow over time
   - Enhancement: (frankr) Before if you changed or created a sequence and then clicked exit, program would just exit and you would lose your
      change. Now, you will be prompted if you want to save before exit.
   - Enhancement: (frankr) Random effect generation. When creating a Spirograph, the Animate button will always be set and the little radius will be
     set to its minimum value. By setting r to zero and setting animate to true, random spirographs will now always produce an animation.
   - Enhancement: (frankr) New choice in model generation. You can now select custom. We are not quite done with this but it will support having
     coro snowflakes, stars, strings wrapped around a car, fire sticks, candles,. Basically any model. Watch this space for a future tutorial.
3.0.22 Jul 26,2013
   - Enhancement: (frankr) New button to randomly create effects for every row in your sequence
    - Enhancement: (sean) Brightness slider now goes to 400% (Old value was 100%). This will allow improving movies
   - Bug Fix: (sean) If Tree effect was selected for Effect2 and slider was all the way to left, xlights crashed. fixed
   - Bug Fix: (sean) Removed the gauge that was next to Export button. The gauge library does not work in Unix.
3.0.21 Jul 25,2013
   - Enhancement: (frankr) In the Model setup screen, you can now set the starting channel for each string
    - Enhancement: (frankr) In the Model setup screen, you can now choose how the strings wrap. 3 more choices are now available
      bottom left, upper left, upper right, bottom right
   - Enhancement: (sean) Meteor Effect class has 3 new effects
        1) Swirl1. As meteors fall they are moved in a sine wave
        2) Swirl2. Same as Swirl 1 except odd strands move one way, even strands move the other
        3) Strarfield. Meteor effect starts in center of your display and moves outwards
3.0.20 Jul 19,2013
   - Enhancement: (frankr) Highlight cell you are editing in so you know where "update" will apply
    - Bug Fix: (frankr) Opening a music file twice, the second time will not prompt for a new file name. Instead, it overwrites the first file. Fixed
   - Enhancement: (frankr) Ability to read an Audacity or a XML file when creating a blank sequence from a music file
   - Enhancement: (frankr) 1 is Unmask, 2 is Unmask now keeps the brightness value of the mask. This is big! Use a picture as a mask on a colorwash.
        The new picture will be a mono tone version. Use text as a mask, the edges will feather but in a solid color.
        This fixes the antio-aliasing issue of text. To fix use text as effect 1, color wash as effect2. set effect 2 color what you want your text
        to look like
   - Bug Fix: (frankr) Deleting some models caused crash, fixed.
   - Bug Fix: (frankr) If you had multiple models attached to a sequence, deleting anything other than 1st model caused crash. fixed
3.0.19 Jul 17,2013
   - Bug Fix: (matt) Crash on exit fixed
    - Bug fix: (matt) Empty export files (LOR, hls,vixen,conductor) fixed
   - Enhancement: (sean) When reading in your sequence, missing attributes will be added if your file is from pre ver 18.
   - Enhancement: (sean) Changed rotation slider in Spirals so it does ten times more rotations.
   - Bug fix: (matt) If you have two lines of text but only one color selected, both lines will be same color
   NOTE!! Please make a backup of all your xml files. This release will be fixing your sequences, protect yourself.
3.0.18 Jul 06,2013
   - (sean) Added new Brightness and Contrast sliders. Located next to Sparkle slider it allows dimming the overall effect.
        Contrast will make brights brighter and sim colors go towards black.
3.0.17 Jul 01,2013
   - (sean) Fixed text in Effect2. Added Twinkle, Tree, Spirograph and Fireworks to Effect2
3.0.16 Jun 28,2013
   - (sean) New option added to the Text effect, Countdown timer.
3.0.15 Jun 27,2013
   - (sean) Added check to make it so empty picture selection does not cause hundreds of pop up windows
    - (sean) fixed LOR lms and lcb export
   - (sean) added check to make sure model's have at least 1 pixel wide and high
   - (sean) new feature in Meteors effect. Meteors can now fall up
3.0.14 Jun 13,2013
   - New movie files added to Picture Effect
        Use web page to change your movies into jpg files. http://image.online-convert.com/convert-to-jpg
    - Strobe in Twinkle Effect is now random lights
3.0.13 May 30,2013
   - New Strobe Effect now available in the Twinkle Effect Class
    - Twinkle Effect Class now allows different steps of twinkle.
3.0.12 May 26,2013
   - New type of vertical text where letters are not rotated but stacked on
      top of each other. Finally we have Line 1 one color, line 2 second color.
     Each line of text can have a different font, a different direction.
     There is a position bar for each line of text.
3.0.11 May 15,2013
   - Text can now be rotated 0, 90, 180 or 270 degrees
3.0.10 May 12,2013
   - Changed 32 character limit in the Text effect to be 256 characters
3.0.9  May 11,2013
   - Animated Gifs added to the Pictures effect
3.0.8  Apr 25,2013
   - Added export for LSP to create UserPatterns.xml
        The actual file created will have a *.user extension. rename it to use it
   - New Effect, Fireworks!. Dave Pitts coded up this new effect.
3.0.7  Apr 23,2013
   - Fixed Pallet 3 button. It was not picking up color when it was clicked.
    - Fixed export of Meteors. This effected all sequencers
   - Fixed export of Life. This effected all sequencers
   - Fixed export of Snowstorm. This effected all sequencers
3.0.6  Mar 11,2013
   - Corrected another bug in LOR lcb export.
    - Modifed Nutcracker tab to keep a minimum size screen on left window.
    - Added color selection based on distance from center of object for Spirograph.
    - Added the ability to animate the d parameter in spirograph
    - Moved all effects into their own source file.
    RGBEffects.cpp is now smaller and there are 20 new cpp
        files. RenderBars.cpp, RenderMeteors.cpp, .etc.
3.0.5  Mar 08,2013
   - Corrected color swap in lcb files. Corrected bug in HLS export.
3.0.4  Mar 04,2013
   - Added new effects, Tree and Spirograph.
3.0.3  Mar 02,2013
   - Added new effect, Twinkle.
3.0.2  Feb 28,2013
   - Corrected unsigned 24bit value for HLS *.hlsnc export.
3.0.1  Feb 26,2013
   - Added wxGauge to show process for exporting sequence files.
3.0.0  Feb 22,2013
   - Initial Release. This is Matt Browns original porting of Nutcracker into xLights.









