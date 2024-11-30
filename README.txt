xLights is a show sequencer and player/scheduler designed to control
USB/DMX/sACN(e1.31)/ArtNET(e.1.17)/DDP controllers.
xLights also integrates with the Falcon Player.
xLights imports and exports sequence data from sequencers such as LOR (SE, PE, SS and S5),
Vixen2, Vixen 3, LSP, and HLS.

Latest releases are found at www.xlights.org
Videos tutorials are found at videos.xlights.org
Forums are located at nutcracker123.com/forum
Issue Tracker is found here: www.github.com/xLightsSequencer/xLights/issues

XLIGHTS/NUTCRACKER RELEASE NOTES
---------------------------------
2024.19 November 24, 2024
    - enh (cybercop23)      Re-attempt to resolve host names on CTRL-click or Test if not resolveable when xLights started
    - enh (dkulp)           If a sequence cannot be saved, report the error to the user
    - enh (dkulp)           Use better coloring in Dark mode for Import dialog and TwoPoint/PolyPoint model X/Y/Z coords
    - enh (cybercop23)      Add Skip option to Discovery for controllers that match existing
    - enh (cybercop23)      Add ability to highlight to/from models in Import dialog to only map that subset
    - enh (degracode)       Sort controllers in Import from controller (Twinkly) dialog
    - bug (degracode)       Fix how Custom Model grid is created to create all rows/cols in one call.  Much faster.
    - bug (cybercop23)      Fix performance of AutoMap on Import Dialog
    - bug (dkulp)           Fix some crashes if ip resolves are still outstanding when switching show directories or similar
    - bug (dkulp)           Fix some issues trying to determine if a directory is writable
    - bug (dkulp)           Make sure we can access/write to show directories when selected from Recents menu
    - bug (dkulp)           Fix crash with Ethernet based controllers that don't have outputs (FPP Player Only for example)
    - bug (dkulp)           Fix hang on MacOS when using a lot of Image models
    - bug (dkulp)           Fix crash on MacOS with "Blur" when using large models in "Single Line" render styles
2024.18 November 14, 2024
    - enh (keith)           Add ability to "Level" the brightness of color components
    - enh (scott)           Windows installer works on Arm64
    - enh (dkulp)           Remove some optimizations that were helpful in the old 32bit builds, not for SSE/Neon (should help Galaxy, plasma, butterfly, etc...
    - enh (scott)           Add align to closest timing mark for effects
    - enh (scott)           Add group count support for Genius controllers
    - enh (scott)           Ability to "invert" channels in DMX mapping
    - enh (derwin12)        Add last render date to Batch Render dialog
    - enh (keith)           Optionally disable key acceleration when nudging models on layout panel
    - enh (scott)           Import layer names from original sequences
    - enh (cybercop23)      Add sequence number progress indicator to FPP Connect
    - enh (cybercop23)      Allow models to be "visualized" in either base or show folder
    - enh (KyleGrymonprez)  Add rondomization functionality to metronome timing track generation
    - enh (derwin12)        Add submodel support to auto mapping
    - bug (dkulp)           Fix a potential crash when using host names instead of ip addresses from a base folder
    - bug (scott)           Windows Upgrade Available notification not working
    - bug (derwin12)        Don't trigger a save when clicking on a locked model
    - bug (scott)           Add some internal checks to prevent overlapping and zero length effects
    - bug (dkulp)           FPP Connect would not delete some tmp files
    - bug (dkulp)           MacOS - also delete the bookmarks file when -w passed to reset permissions
    - bug (dkulp)           MacOS - make sure we have access to the base directory, ask for permission if required
    - bug (scott)           Fix crash with negative values in audio editor
    - bug (MrPierreB)       Fix Single Strand Fade type "Head and Tail" for all blending types
    - bug (PeteMatthews)    Fix color order inheritance issue on ILightThat controllers
    - bug (degracode)       Fix problems with property grid now being updated when protocol switches to Twinkly
2024.17 October 14, 2024
    - change(dkulp)         MacOS builds now require MacOS 11 or newer
    - enh (dkulp)           Resolve hostnames in parallel at startup, much faster startup and FPP Connect
    - enh (derwin12)        Add key bindings for Jukebox
    - enh (derwin12)        Add key bindings for increase/descrease speed, prev/next tags
    - enh (derwin12)        Optionally reverse submodels when reversing models
    - enh (thekev)          Add support for default gamma with Experience controllers
    - enh (dkulp)           FSEQ save - allow using more compression blocks for larger channel count sequences
    - enh (derwin12)        Add validation of in/out fade times
    - enh (dkulp)           More updates for small waveform time/selection display
    - enh (scott)           Add support for Wasatch controllers
    - bug (derwin12)        Fix display of timeline tags, fixes flood of error messages in logs
    - bug (derwin12)        Fix crash when updating aliases on Strand mappings
    - bug (scott)           Fix import of "old" moving head models
    - bug (scott)           Fix HinksPix audio exporting
    - bug (scott)           Fix DMX moving head advanced export missing parameters
    - bug (derwin12)        Fix bug flagging versions when importing sequences from the same version
    - bug (dkulp)           Fix crash on MacOS if monitor with a running preview is disconnected
    - bug (derwin12)        Fix looping replay of selection not working with audio
    - bug (dkulp)           Fix crashes on layout panel if protocol not set
    - bug (Kyle Grymonprez) Fix "Open Base Show Directory" not working
2024.16 September 18, 2024
    - enh (derwin12)        Add copy model including submodels right click option
    - enh (Kyle Grymonprez) Disable prompting for alias saving of newly generated models by default
    - enh (Kyle Grymonprez) Node Layout: changing the background colors for dark mode
    - enh (dkulp)           Make sure we have access to media directories, re-prompt if required
    - enh (dkulp)           Optimize the init/cleanup of CandleEffect, particularly when applied to large groups
    - enh (dkulp)           If submodel buffer is vertial, use left/col/right labels
    - enh (derwin12)        Add RGB support to Image Model
    - enh (derwin12/dkulp)  Various updates to Time Display if Small Waveform is used
    - enh (derwin12)        Save Small Waveform preference
    - enh (derwin12)        Make various abandon messages consistent
    - enh (keith)           Add F-Prop as a dummy controller
    - enh (scott)           Visualizer - change export from csv to xlsx and add coloring/formatting
    - enh (scott)           Visualizer - add gamma
    - enh (derwin12)        Import Dialog - verify imported sequence framerate and xLights version
    - enh (scott)           SubModels Dialog: add import of submodels from another layout
    - enh (scott)           Faces Dialog: add clear selected rows
    - enh (scott)           States Dialog: add clear, fix validation on drag
    - enh (scott)           States Dialog: add import from Vendor Dialog downloads
    - enh (kylegrymonprez)  DDP: Controller config can mandate to not support "Keep Channel Numbers"
    - enh (scott)           State Effect: Add fading
    - enh (keith)           Allow setting serial port speed for Renard (Falcon controllers)
    - enh (scott)           Remember position of State Dialog
    - bug (MrPierreB)       Fix moving head effect to not reset effects when "None" selected
    - bug (scott)           Fix Fan Effect: Fan Scale to Buffer giving unexpected results in some cases
    - bug (dkulp)           Fix: blure/zoom on node level effects
    - bug (dkulp)           FPP Connect: fix the HDMI connector index for FPP8
    - bug (scott)           Experience Controllers: fix gamma rounding
    - bug (keith)           Fix single channel matrix displaying in wrong location
    - bug (scott)           WLED: Fix WRGB upload
    - bug (derwin12)        Retain Warp settings when copying
    - bug (derwin12)        Disable Update button for Base Showfolder
    - bug (scott)           Hide Individual Start Channels for Cube Model
    - bug (scott)           Experience Controllers: fix single channel models
    - bug (Kyle Grymonprez) Fix crash on old DMX model
2024.15 August 22, 2024
    - enh (kylegrymonprez)  Add ability to set the alias behavior for renaming models
    - enh (kylegrymonprez)  Add FPP Connect button to setup tab
    - enh (scott)           Allow overriding named color pallets
    - enh (scott)           Add face definition name to check sequence for missing pictures
    - enh (scott)           Add Set Palette Name
    - enh (scott)           Galaxy Effect - add scale to buffer
    - enh (derwin12)        Warp Effect - Add Flip option
    - enh (sidelights)      Single Strand Effect - add additional fade types
    - enh (cybercop23)      Allow animations for the Model Viewer
    - enh (PeteMathews)     Add support for BaldrickDMX board
    - enh (jasonsomers)     Add support for new variant of RGB2Go-Duo2Go
    - enh (dkulp)           Skull Model - generate node names for color channels and eye servos
    - enh (dkulp)           Detect when Show Directory, FSEQ Directory, or Backup directory are unwritable and re-prompt
    - enh (cybercop23)      Add pre/post silence to media
    - enh (derwin12)        Add summary to Check Sequence
    - bug (scott)           Fix ESPixelStickv4 DMX upload
    - bug (derwin12)        Fix inconsitent paste behavior with timing tracks selected
    - bug (scott)           Fix State Dialog buttons being too squished
    - bug (keith)           Don't allow saving invalid states in State Dialog
    - bug (derwin12)        Allow some larger buffers that were restricting some rendering
    - bug (derwin12/dkulp)  Fix status bar height on HiDPI windows displays
    - bug (dkulp)           Make sure temp directory is properly cleaned after loading xsqz files
    - bug (dkulp)           More fixes for Mac xsqz icons
    - bug (derwin12)        Copy/Paste for AC effects was clearing wrong row
    - bug (derwin12)        Avoid single dot Ripple outlines
    - bug (scott)           Fix problems with uploading proxies to FPP
    - bug (keith)           Fix some issues with "rubber band" selection of effects and timings
    - bug (scott)           Fix loading some new xmodel formats
    - bug (dkulp)           Fix upload of virtualdisplaymap if 2d preview uses 0 for center
    - bug (cybercop23)      Fix some crashes when  creating aliases from mappings
    - bug (keith)           Fix labels in Matrix Dialog
    - bug (cybercop23)      Fix Moving Head button hightlight color for light mode
    - bug (scott)           Fix FPP upload for single channel model types
    - bug (scott)           Fix DDP channel number for Genius Pro DMX output
2024.14 August 5, 2024
    - enh (scott)       Added Scale to Buffer to Fan Effect, Radius 1 and 2 become Percentage values(100=100% of buffer) when enabled
    - enh (keith)       Add key bindings to switch between timing tracks, unselect all timing tracks
    - enh (dkulp)       Update VirtualMatrix upload to FPP8 to use HDMI-A-# and not fb#
    - enh (dkulp)       Add support for PWM ports to visualizer
    - enh (dkulp)       Add upload for PWM ports to FPP8
    - enh (cybercop23)  Add option to bulk edit groups Preview
    - enh (derwin12)    Text Effect - Add Center function for XLFonts and No Repeat
    - enh (keith)       Add LB1908 to the pixel type list
    - enh (keith)       Fix "Prepare Audio" to create proper MP3 files (Windows/Linux)
    - enh (dan)         Update "Prepare Audio" to create m4a/AAC fles. Use AudioToolbox AAC encoder on MacOS.
    - enh (dan)         Update "Prepare Audio" to copy metadata/tags from source audio to target to no lose that information
    - enh (keith)       Add filter text to value curve timing track events
    - enh (derwin12)    Show colors from Color Wheel in MH Preset
    - enh (scott)       DMX Model Presets to DMX Fixed Channels and added support Moving Head Effect
    - enh (dkulp)       Servo models can now have "controller properties" (min/max/reverse/etc...) set on them to upload to the controller
    - enh (derwin12)    Add validation to the preset channel values
    - enh (keith)       Add ability to have associated models within an xmodel file
    - enh (derwin12)    Add the generate node names ability to the New Moving Head Model
    - enh (dkulp)       Add the generate node names ability to the Skull Model
    - enh (keith)       New xsqz icon
    - enh (derwin12)    Ruse existing shader files when doing a File Cleanup
    - enh (keith)       Add falcon PiCap v2
	- enh (keith)		Add some xlights model support to matrices and virtual matrices in xSchedule
	- enh (keith)		Add the ability to hard code a timecode offset for each step in a playlist for ultimate control over timecode
    - bug (derwin12)    Add all layers for the custom model dialog
    - bug (keith)       Fix sequences packaged in subfolders not loading in xsqz
    - bug (dkulp)       Fix issues with canvas mode effects and DMX buffers
    - bug (derwin12)    Catch overly large buffer and limit them
    - bug (keith)       Loop effects outputs a blank frame on loop
    - bug (keith)       Fix export of model can crash if file create fails.
    - bug (keith)       If model import fails then delete the model rather than leaving a half constructed model behind
    - bug (keith)       Fix missing hinkspix data in tooltip in visualiser
    - bug (dkulp)       Don't register to handle .zip files
    - bug (scott)       Fix duplicate Smart Receiver pixel count on Genius Controllers
2024.13 July 14, 2024
    - enh (scott)       Added "auto shutter" to Moving Head Effect for Color Wheels
    - enh (PeteMatthews) ILightThat controllers: Set universe size in config, add Switchy
    - enh (scott)       Added Undo/Redo to Custom Model Auto Increment
    - enh (scott)       Added ability to rename controller model name
    - enh (keith)       Change the packeged sequence default extension to xsqz
    - enh (keith)       Add ability to directly open packaged sequences from zip/xsqz file in read only mode in xlights by double clicking on zip or dragging it onto the icon
    - enh (dkulp)       Support for linking Servo effects so that changes to end will effect the start of the next servo and vice/versa
    - enh (dkulp)       When splitting an effect, use the location of the mouse as the split point (instead of the center)
    - enh (keith)       For sparse custom models add a save format which is sparse
    - bug (cybercop23)  Reset all MH Panel options
    - bug (scott)       Fix some of the Experience SR Upload issues
    - bug (scott)       Bulk Edit missing in Layout Tab
    - bug (scott)       If windows hardware video render fails, fallback to software
2024.12 Jun 23, 2024
    - enh (keith)       Add F48v5 support
    - enh (dkulp)       Add K2-Pi0 and K2-Pi-Servo (pixel ports only) support
    - enh (dkulp)       Add "Sync" checkbox to Servo panel to keep start/end in sync while adjusting
    - enh (scott)       FPP8 - Add descriptions to uploaded proxies
    - enh (scott)       FPPConnect - Tag/Remove xLights generated models
    - enh (scott)       Import/Export custom strings on Custom Models
    - bug (dkulp)       Visualizer should not show four pixel ports for controllers with only two ports
    - bug (dkulp)       Fix crash with layer names on SubModels with only one layer
    - bug (dkulp)       Fix value curve on servo effect
    - bug (scott)       Fix some issues with Hardware video decoding with Cuda as well as AMD
2024.11 Jun 15, 2024
   - enh (dkulp)        Add ability to set display names for Model/Submodel layers
   - enh (dkulp)        For VSA import, set the layer name to mach the servo
   - enh (dkulp)        Add effect background for Servo Effect
   - enh (dkulp)        Make Servo effect easier to use for common "Ramp" use cases by not requiring the value curves for those
   - enh (dkulp)        Make sure preview honors the min/max configured for the servo
   - enh (dkulp)        Change MovingHeadPanel to use dialog units and/or default sizes for buttons
   - enh (gil)          Show Active Timing Marks in Moving Head Effect Dimmer Panel; Only set MH channels that have actually been changed
   - enh (dkulp)        Allow Canvas mode effects to set pixels transparent (ex: shaders only applying to part of buffer)
   - enh (cybercop23)   Sort Model Icons + swapped Download & Import
   - enh (cybercop23)   Removed Wreath model from being created
   - enh (scott)        Added Timing Event Bar Bounce to VU Meter
   - enh (derwin12)     Ability to order the views in the Display Editor window
   - enh (cybercop23)   Add Aliases for Models and Groups to the Export Model
   - enh (cybercop23)   Layout Panel - Remove start/end channels for shadow items, MH type
   - enh (cybercop23)   Add quick open base button
   - enh (cybercop23)   Sort dropdowns for In/Out Transitions
   - enh (PeteMatthews) Add sACN/ArtNet support for ILightThat controller
   - enh (cybercop23)   Stack button on ctrl page and remove the slider to prevent full close of right panel
   - enh (derwin12)     Allow for a model vid export to be done at high or lowdef render via scripting
   - enh (derwin12)     Add middle mouse to pan sequencer
   - enh (derwin12)     Check that timing track name doesnt conflict with models
   - enh (derwin12)     Show Set Center Point dialog for all groups
   - enh (cybercop23)   Move Delete to lower in thej list to reduce accidental click
   - enh (derwin12)     Optionally retain the effect duration when duplicating an effect
   - bug (derwin12)     Keep buttons visible in submodel dialog
   - bug (gil)          Fix Pan value curve
   - bug (derwin12)     Model groups with one model causing stalling/crashing
   - bug (MrPierreB)    Bars effect - Fix bug where the highlight color is not used when other layer blending options are selected
   - bug (gil)          Fix model paste when no model is selected
   - bug (dkulp)        Fix ValueCurve bottom not being visible or selectable on Mac.
   - bug (dkulp)        Some fixes for colored buttons on sonoma having their sizes changing
   - bug (dkulp)        Make sure sorting is turned off during the model list "Freeze" and re-enabled on "Thaw"
   - bug (dkulp)        FireEffect: only call GetMaxBuffer during init, use that size throughout the effect rendering for consistency
   - bug (scott)        DMXEffect - some more fixes after update to 48dmx channels
   - bug (gil)          Fix major bug with MovingHeadEffect on groups...it was triggering all heads whether they were selected or not
   - bug (gil)          Fix Sketch panel resize
   - bug (keith)        Fix some E4-S Mk3 controller issues with v1.5 firmware, zig-zag, content-type
2024.10 Apr 30, 2024
   - enh (MrPierreB)    Add ability to control the highlight color for the Bars effect
   - enh (gil)          Add Preset support for Dimmers
   - enh (gil)          Adjust Moving Head mesh materials and allow user to control brightness
   - enh (PeteMathews)  Support controller from ILightThat
   - enh (gil)          Preview Moving Head beam brightness uses dimmer
   - enh (gil)          Add "Dimmer" tab to new moving head effect
   - enh (cybercop23)   Allow export/import of aliases for all model types
   - enh (derwin12)     Add option to Display Elements to sort bases on node counts
   - bug (derwin12)     Fix submodel node updates when changing to new submodel choice
   - bug (cybercop23)   Suppress "Add To groups" if source model doesn't belong to any groups
   - bug (gil)          Fix crash in MovingHeadPanel with non-moving head models
   - bug (gil)          Fix migration of moving heads with negative range or motion
   - bug (gil)          Fix DmxMovingHead orientations ot match new model
   - bug (gil)          Fix Moving Head default node names
   - bug (gil)          Fix Moving Head Copy/Paste and presets
   - bug (derwin12/scott) Fix some issues with UTF-8 characters in media filenames
   - bug (scott)        Fix State effect panel using Faces intead of states
   - bug (dkulp)        Fix various layer blending issues with Moving Heads/DMX models
   - bug (dkulp)        Fix some crashes with effects on empty groups
   - bug (dkulp)        Fix Model Preview of Tree/Cube/Sphere in some unusual circumstances
   - bug (derwin12)
2024.09 Apr 16, 2024
   - enh (dkulp)        MacOS - Use GPU for  layer blending
   - enh (derwin12)     Add preference to override the highlight color for lists
   - enh (cybercop23)   Remove start/end channels from groups and sub-models in Model list
   - enh (cybercop23)   Add LayoutGroup to Import/Export for CustomModel
   - enh (gil)          New Moving Head effect - see https://github.com/xLightsSequencer/xLights/discussions/4260
   - enh (scott)        Windows - Update to FFMpeg 6.1.1, fixes Vulkan, add AMD AMF decoders
   - enh (dkulp)        Check Sequence - check effects against proper model/submodel and log SubModel the effect is on
   - enh (cybercop23)   Add SubModel alias to info panel
   - enh (derwin12)     Add SubModel alias editing to Layout Panel
   - enh (cybercop23)   Add option to add new models to the same groups during copy/paste
   - enh (dkulp)        Add the "Model Blending" layer to the list of layers selectable for Canvas mode effects
   - enh (dkulp)        Faster audio processing/loading using far less memory
   - enh (dkulp)        Optimize internal "Node" creation/memory for faster loading and reduced overhead
   - bug (dkulp)        Disabling model from rendering should also disable strands/nodes/submodels
   - bug (dkulp)        Disabled Canvas effects shouldn't render or blend
   - bug (gil)          Fix - 3D export of tree model is Z-Order backwords
   - bug (gil)          Fix - "FromBase" XML attribue being set multiple times can corrupt the xlights_rgbeffects.xml file
   - bug (cybercop23)   Remove duplicate Models/SubModels from groups when replacing a model
   - bug (derwin12)     Shape Effect Emoji cannot be Y centered
   - bug (gil)          Fix DMX Model color issues
   - bug (derwin12)     Window - Enable/Disable HW Video Decoding dropdown without having to close/re-enter preferences
   - bug (dkulp)        Fix sparkles with GPU rendering
   - bug (dkulp)        Fix some renderings of SubModels in groups
   - bug (derwin12)     Fix print dialog in Layout and Wiring views
   - bug (dkulp)        Fix potential memory corruption with states/faces effects in groups
   - bug (dkulp)        ShaderEffect - fix shaders that ask for AudioFFT on sequences with no audio
   - bug (derwin12)     Fix Total Nodes tooltip on groups containing submodels
   - bug (scott)        Increase ValueCurve text field sizes to account for negative sign on values
2024.08 Apr 6, 2024
   - enh (scott)        Ability to set type of Video Hardware Decoder on Windows
   - enh (dkulp)        Optimize faces effect initialization on large models
   - enh (scott)        Support two mouth colors if using Force Custom Colors
   - bug (dkulp)        Fix crash with sparkles
   - bug (dkulp)        Fix some crashes when deleting layers while rendering
   - bug (dkulp)        Fix issues with model blending not working
   - bug (derwin12)     Fix handling of custom models from base folders
   - bug (keith)        Fix crash when adding aliases to newly created submodels
   - bug (dkulp)        Fix saving zstd fseq files with massive channel counts
2024.07 Apr 2, 2024
   - enh (joero4)       Update ColorCurve dialog to make it easier to adjust near edges
   - enh (dkulp)        MacOS - Add Shockwave to GPU rendered effects
   - enh (dkulp)        MacOS - transitions handled on GPU
   - enh (derwin12)     Lua script updates to allow updating the master view
   - enh (scott)        Added RGB2Go controllers
   - enh (derwin12)     Add ability toadd and edit aliases on SubModels
   - enh (derwin12)     Save and use 3D mode selection in xlight_rgbeffects.xml file
   - enh (derwin12)     Show any model warnings (missing nodes, etc..) before allowing users to create faces
   - bug (dkulp)        Fix some issues where Buffer Styles list would not update to syles valid for submodels
   - bug (dkulp)        Fix disabled effects still rendering
   - bug (scott)        Fix SVG shape effect (nanosvg update)
   - bug (scott)        Fix Scripting dialog not working
   - bug (dkulp)        MacOS - limit number of OpenGL contexts created for Shader Effect, reduce memory usage
   - bug (dkulp)        MacOS - Fix non-mask transitions not rendering properly with GPU rendered effects
   - bug (dkulp)        Fix timing of "From Middle" transition when buffer ht is not 1
   - bug (dkulp)        Switch to sequence tab before loading sequence to avoid a very hard to reproduce/debug crash
   - bug (derwin12)     Fix crash in DMX Remap Channel Dialog
2024.06 Mar 25, 2024
   - enh (dkulp)        Update wxWidgets to latest patches to fix various Dark Mode issues and Mac docking issues
   - enh (dkulp)        Add Horizontal/Vertical By Strand buffer styles to SubModels
   - enh (dkulp)        Rename "Group all arches" to "Group all strands"
   - enh (dkulp)        Map old SingleStrand/SingleLine effects on SubModels to proper setup
   - enh (dkulp)        Update to c++20, requires update to Ubuntu Jammy
   - enh (derwin12)     Add Select/Deselect Highlighted to Tools Test dialog
   - enh (derwin12)     Add delete submodels to sequencer model right click menu
   - enh (derwin12)     Add color palette to Picture Assistant
   - enh (scott)        Test dialog - add some controller based tests
   - bug (cybercop23)   Allow state effect on SubModels
   - bug (dkulp)        Fix PolyLine single node style rendering
   - bug (dkulp)        BufferStyle panel not showing buffer styles for SubModels
   - bug (cybercop23)   Fix auto-map alias at SubModel level with spaces in names
   - bug (derwin12)     Clear the red status of the shader after downloading shader
   - bug (derwin12)     Fixes/Improvement to the sort model by location on Group within a Group   
2024.05 Mar 14, 2024
   - enh (scott)        Add some additional columns to controller list
   - enh (cybercop23)   Increase default size of Display Elements window,
   - enh (cybercop23)   Remove duplicate prompts for Save Alias
   - enh (derwin12)     Add "Color by word" option for Text effect
   - enh (derwin12)     Add sort by location to model groups
   - enh (cybercop23)   Allow inactive models to be deleted
   - enh (derwin12)     Add X/Y positioning to Circle effect when using radial options
   - enh (derwin12)     Face Dialog - sort and compress entered nodes
   - enh (derwin12)     Lua Scripting - expose the "Force High Definition" option
   - bug (derwin12)     Fix chapter bookmark display issues on Linux
   - bug (scott)        HinksPix - Fix overlap check and empy playlist name issues
   - bug (scott)        Fix crash in import dialog
   - bug (dkulp)        Fix several crashes due to effects with rendering disabled   
2024.04 Feb 29, 2024
   -- enh (gjones)      Allow model group centers (red cross) to be defined with right-click (Set Center Offset Here)
   -- enh (gjones)      Prevent model group centers (red cross) from moving when models are added/removed/moved
   -- enh (gjones)      Modified center offset (red cross) spin controls so enter key will latch values
   -- enh (gjones)      Expand range from +/- 1000 to +/- 5000 for red cross offsets
   -- enh (derwin12)    Add Select Effects dialog to UI toolbar
   -- enh (derwin12)    Add expand and collapse to import mapping dialog
   -- enh (derwin12)    Allow editin dimensions for Image type props
   -- enh (derwin12)    Include creator in the search for Sequence Downloads
   -- enh (derwin12)    Add preference to hide color update warning
   -- enh (derwin12)    Scale the sphere ot get a better 3d export
   -- enh (derwin12)    Resize/relayout several dialogs
   -- enh (derwin12)    DMX effect put 16 channels on one page
   -- bug (derwin12)    Remapping of missing submodels skips over some
   -- bug (dkulp)       Fix crash when sorting by effect count in Import dialog
   -- bug (dkulp)       Fix crash in Single Strand Chase effect if value curves are used for chase size
   -- bug (scott)       Experience controllers support ArtNet
   -- bug (scott)       Fix Minleon upload
2024.03 Feb 14, 2024
   -- UPDATE (dkulp)    New PayPal donation URL
   -- end (gjones)      Add preference whether layout view zooms to center or mouse cursor
   -- enh (gjones)      Expand size of Default View for New Sequences preference box. Fixes #4255
   -- enh (MartinMueller) Several additional ESP variants
   -- enh (keith)       Autosize the face dialog columns
   -- enh (merryoncherry) Options for arches to not zig/zag
   -- enh (dkulp)       Allow SubModels to have duplicate nodes for "Ranges" default and stacked styles
   -- enh (derwin12)    Add comments to failed backup notice
   -- enh (derwin12)    Added right click sort along with buttons to move presets up/down/top and botton within the list
   -- enh (derwin12)    Dont initially expand the vendors on the load of the download models
   -- enh (derwin12)    Faster text movement and longer twinkly settings
   -- enh (derwin12)    Add background effect image for the Single strand chase
   -- enh (derwin12)    Add select all and deselect all on FPP Connect controller dialog
   -- enh (derwin12)    Add bulk edit tag color for groups
   -- enh (derwin12)    Render and Export Model now renders all
   -- enh (derwin12)    Remove the default sorting of Import View
   -- enh (derwin12)    Add select highlighted to import previews right click menu
   -- bug (derwin12)    Shape Effect - Flip the Y value for Emoji
   -- bug (scott)       Exported timing tracks to include subType
   -- bug (gjones)      Fix dragging Preset group so it stays a group.  Fixes #4304
   -- bug (scott)       Fix crash in ESPixelStick upload to non-PixelStick
   -- bug (derwin12)    Remap Custom Model ignores placeholders
   -- bug (tygunn)      Fix some bugs/issues with Polyline import/export
   -- bug (derwin12)    Honor the dont send data to unused ports when opening the tools test dialog
   -- bug (derwin12)    Render after performing an effect convert to 'per model'
2024.02 Jan 26, 2024
   -- enh (derwin12)    Allow bulk edit of sketch effect bg filename
   -- enh (derwin12)    Add preference to skip loading of preset gif images
   -- enh (derwin12)    Handling pasting of network drive files
   -- enh (scott)       Force "Don't keep start channels" for ESPixelStick discovery
   -- enh (derwin12)    Store imported shaders the same was as imported images/videos
   -- enh (scott)       Try to detect FPP controller types during discovery
   -- bug (dkulp)       Some fixes for locks/hangs when moving/editing "main thread effects" (Shader on Windows)
   -- bug (derwin12)    Dont use render cache if using words from the lyric track
   -- bug (derwin12)    VU Meter missing the last bar
   -- bug (derwin12)    Fix faces download with faces in subdirectories
   -- bug (scott)       Fix Falcon v4 discovery through FPP
   -- bug (merryoncherry) Fix video stuttering with some video framerates
   -- bug (derwin12)    Fix some data entry issues with Waveform Filter
   -- bug (derwin12)    Fix "number of effects" displayed for models on import dialog (it included submodels which display separately)
2024.01 Jan 17, 2024
   -- change(dkulp) Move repo to xLightsSequencer org, update headers and url locations
   -- enh (dkulp) Auto-Build in xCode cloud, initial setup for TestFlights builds
   -- enh (keith) Limited support for RGBWW pixels
   -- enh (derwin12) Use rest for missing face phoneme
   -- enh (tygunn) When a ruler is defined, show the X/Y/Z coordinates of models in the models export
   -- bug (Scott) Clear test presets when switching show folders
   -- bug (dkulp) Hopefully fix crash on macOS when changing audio devices
   -- bug (dkulp) Use BitmapBundles for icons in Import Dialog (HiDPI updates)
   -- bug (scott) Fix upload to WLED if different pixel types are used
   -- bug (cpinkham) Fix compile with gcc 13
2023.23 Dec 28, 2023
   -- enh (dkulp)     Convert "New Style" Pinwheel effect to use Meta GPU rendering on MacOS
   -- enh (scott)     Added Scheduling to HinksPix Export Dialog.
   -- enh (scott)     Persist Custom Colors across Color Dialogs.
   -- enh (dkulp)     Update to newer version of wxWidgets that fixes appearance of checkboxes on properties panels
   -- enh (keith)     Bell at end of Batch Render, extra progress displayed during batch render
   -- enh (keith)     Allow user control of maximum Render Cache size
   -- bug (scott)     Fix Import Dialog AutoMapping Aliases to Submodel/Stands.
   -- bug (scott)     Fix Import Dialog Groups Blue turning white.
   -- bug (scott)     Fix 3D Model Export starting at zero.
   -- bug (scott)     Fix DMX Shutter value with On effect and Flood lights.
   -- bug (keith)     Fix crash with parsing JSON with certain UTF-8 characters
   -- bug (tulzscha)  Fix fire effect buffer sizes
   -- bug (dkulp)     Fix some random crashes that occur with various actions on the View Models panel
   -- bug (dkulp)     Fix discovery of Falcon controllers as remotes if needing to use http discovery
   -- bug (dkulp)     Fix potential crash/memory corruption at startup when resolving IP's on background threads
   -- bug (dkulp)     Fix leftover files remaining in tmp directory
   -- bug (dkulp)     Fix some problems writing ZSTD fseq files for layouts with well over 2M channels and complex effects
2023.22 Dec 5, 2023
   -- enh (claudio)   Add virtual matrix to xSchedule using RGBW data
   -- enh (lutz)      Add palette shift buttons
   -- enh (scott)     Add cycles and scale to ripple effect
   -- enh (scott)     Add set smart remote and increment to visualiser
   -- enh (scott)     Add importing submodels as states
   -- enh (scott)     Add more controller connection property setting to the visualiser
   -- enh (keith)     Show progress in batch render including number left to render
   -- enh (keith)     Add model aliases used to improve automap and handling opening sequences with model names that have changed
   -- enh (keith)     Add an adjust effect - primarily for use on DMX effects to modify results without changing the underlying effects
   -- enh (keith)     Add support for non-zig zagging trees with multiple strands but one string
   -- enh (dartunder) Add bulk edit to clear shadow models
   -- bug (dartunder) Fix 3 point model dimensions incorrect
   -- bug (keith)     Fix Falcon V4 upload without full xlights control should not override port brightness
   -- bug (keith)     Fix layout model locking menu items inconsistently available
   -- bug (keith)     Fix Falcon V4/V5 reversed not uploading correctly
   -- bug (keith)     Fix click on import effect count column does not sort by effect count
   -- bug (keith)     Fix servo effect incompletely resets parameters
   -- bug (keith)     Fix Eseq playback on xSchedule
   -- bug (keith)     Fix default superstring to RGB handling
   -- bug (scott)     Fix FPP compatible audio file types incomplete
   -- bug (scott)     Fix missing bulk edit from some face effect properties
   -- bug (dkulp)     Fix 4GB+ uncompressed FSEQ files
   -- bug (dkulp)     Fix crash on player only controllers
   -- bug (dkulp)     Fix OSX font crash
2023.21 Nov 25, 2023
   -- enh (dkulp) Allow upload of config to inactive controllers
   -- enh (scott) Add More Right Click Options to Multi-Controller Upload, Save/Load Selected
   -- enh (scott) FPP Connect - Add Falcon V5 Support
   -- enh (keith) Downgrade sketch effect image missing to warning and allow suppression of the warning in preferences.
   -- enh (scott) FPP Connect - add Genius FSEQ upload
   -- bug (dkulp) FPP Connect - fix crash with discovery/fpp connect if "Player Only" controller configured
   -- bug (billnapier) Remove invisible unicode character in AlphxPix 4 settings
   -- bug (dkulp) Fix Faces on models from Base show directory will always trigger the "dirty" flag
   -- bug (keith) Fix copy on empty cell overwrites clipboard
   -- bug (dkulp) Retry FSEQ upload on errors (up to three times)
   -- bug (scott) HinkxPix won't allow assignment to smart receiver 3-9
   -- bug (dartunder) Fix face blink effect
   -- bug (keith) Fix eyes closed not mapping correctly
   -- bug (keith) Fix PixLite config retrieval
2023.20 Nov 15, 2023
   -- enh (scott) FPP Connect - Remove ancient FPP 1.x upload via mounted USB
   -- enh (scott) FPP Connect - Add ability to upload Proxy Controller IPs
   -- enh (MrPierreB) Selectively Offset parameters on remap of dmx channels
   -- enh (keith) Add a quick way to make a block of ports a dumb receiver
   -- enh (kylegrymonprez)  Adding ability to adjust the frequency of the "Auto" eye blink
   -- enh (degracode ) Allow the Setup tab's Activate, Activate in xLights, Deactivate, and Unlink From Base menu items to be used when multiple controllers are selected
   -- enh (keith) Add option to remove placeholders from submodels
   -- enh (scott) Make ModelChainDialog growable
   -- enh (dartunder) Show fpp connect results in status bar
   -- bug (djulien) Allow Preview to be selected when showing 3D models
   -- bug (dartunder) Fix delete warning for locked models, delete all models including locked after confirmation dialog.
   -- bug (dkulp) FPP String upload: don't upload "DUMMY" entries
   -- bug (keith) Show smart remotes as 0-15 on HinksPix
   -- bug (dartunder) Fix undo for replace model command
   -- bug (scott) Fix alphapix upload
   -- bug (keith) Fix crash in Check Sequence in liquid effect
2023.19 Nov 4, 2023
   -- enh (keith) Add webp image support
   -- enh (dartunder) Add warning and make it easier to see that a model had a shadow model in visualizer
   -- bug (Daniel Hacquebord) Fix dark mode for shadow models in visualizer
   -- bug (scott) Fix AlphaPix upload
   -- bug (keith) Fix crash with liquid effect on large models
   -- bug (scott) LUA Script: Select Sequences and deselect highlighted does nothing
   -- bug (dkulp) FPP Connect: not uploading serial outputs from FPP Connect (worked fine from controller page)
   -- bug (dkulp) FPP Connect: not able to upload to FPP instances with UI password set
   -- bug (dkulp) Fix crash on MacOS with RotoZoom if GPU is busy
   -- bug (dkulp) Workaround some other common crashes, but underlying cause of crash still under investigation
2023.18 Oct 21, 2023
   -- enh (dkulp) FPP Connect - query and media in parallel
   -- bug (scott) Custom Model Export - Export Default Render Buffer as Submodel
   -- bug (scott) Fix Submodel Dialog Sizing Issues
   -- bug (dartunder) FPP upload - fix e1.31 priority not being set if universes are all the same size
   -- bug (scott) HinksPix - fix folder and filter selector
   -- bug (dkulp) FPP Connect - attempt to fix some FPP Connect issues by remove "Expect: Continue" headers
   -- bug (robfallone) Fix OpenGL crash on startup with some mesh objects
2023.17 Oct 16, 2023
   -- enh (dkulp) Add monitor attribute for controllers, FPP upload of co-universes will set priority/monitoring/deDup settings
   -- enh (Kyle Grymonprez) Add Node Count to model tooltips
   -- enh (keith) FPPConnect also show eseq files in subdirectories
   -- enh (dartunder) Change the default color for locked selected effects to increase contrast
   -- enh (keith) Add optional arrow to gridlines that points to the front
   -- enh (Edward Catchpole) Show friendly names for known devices in the Import From Controller dialog
   -- bug (dkulp) Add a bunch of logging to FPPConnect HTTP messages
   -- bug (dkulp) Importing/loading some old sequences may cause crashes as FadeIn/Out used to allow spaces
   -- bug (dartunder) Set the rotation to match the original when replacing a model
   -- bug (keith) Fix total channels returns zero if FPP or other non-channel controller is last in the list
   -- bug (dkulp) FPPConnect - tried to examine empty IP address/host names
   -- bug (dkulp) If model from base directory is updated, it is removed from all groups
   -- bug (dkulp) If model from base directory's start channel doesn't match, don't mark "changed" if it will be auto-calculated
   -- bug (keith) Add back in divide timings for layers 1+ on timing tracks
   -- bug (scott) HinksPix - ac receivers have inconsistent channel counts, just use 3 channels per pixel
   -- bug (Edward Catchpole) Fix inactive twinkly controllers blocking UI
   -- bug (dartunder) FPPConnect - Don't resolve bad.address to 255.255.255.255
   -- bug (dartunder) FPPConnect - eliminate duplicate FPP entries for FPP connect
   -- bug (dkulp) FPPConnect - uploads eseq files to sequences directory
2023.16 Oct 9, 2023
   -- enh (keith) Pre-populate effect preset rename field with current name
   -- enh (dartunder) Save/restore test mode tab selection
   -- enh (keith) Add Find for effect import dialog
   -- enh (keith) Add a Halloween palette with orange and a purple that look good on pixels without playing with gamma
   -- enh (dkulp) Import effects - save/restore the "Lock" checkbox value
   -- bug (dkulp) Fix crash with popup menus on layout panel
   -- bug (dkulp) FPP Connect - fix hang when uploading to older controllers that use jqupload method
   -- bug (dartunder) Fix test mode for controller sync enabled with FPP "Player Only"
   -- bug (dkulp) Possibly fix some audio device crashes when Mac goes into/out of sleep mode
   -- bug (dkulp) Fix a spurious log warning on file load that was incorrect
   -- bug (dkulp) Fix some issues of loading sequences always requiring a re-render
   -- bug (dkulp) FPP Connect - fix detection of some capes that use "v2" pinouts
   -- bug (keith) Fix "Base layout" appearance in dark mode
   -- bug (keith) Fix Color Wheel does not always return a feedback color on the UI
2023.15 Oct 2, 2023
   -- enh (keith)     Add Twinkly 3D Model import
   -- enh (keith)     Add auto label of timing tracks
   -- enh (keith)     Add option to lock effects on import
   -- enh (keith)     Allow navigation of sequence settings metadata
   -- enh (keith)     Add support for base show folders with inherited controllers and models
   -- enh (keith)     Add filter labels to shape effect
   -- enh (keith)     In colour tooltips show closest colour name
   -- enh (keith)     Add duplicate effect
   -- enh (keith)     Add visual indicator if model/group contains effects on visible or invisible layers/submodels/strings/nodes
   -- enh (keith)     Add visual hint in visualiser if pixels per port makes 40 FPS impossible
   -- enh (keith)     Add option to fade piano keys on press
   -- enh (matte)     Add optional timeline zooming to mouse
   -- enh (matte)     Add keybindings for various toggling of render
   -- enh (djulien)   Allow blank effect type to match all effects in select effects
   -- enh (djulien)   Show models with effects when trying to make it master or when opening a sequence containing missing models
   -- enh (dkulp)     Increase BBB max pixels to 1600
   -- enh (scott)     Add genius pro controller
   -- enh (dartunder) Add option to keep size and location of models being replaced
   -- bug (keith)     Fix buffer stagger disabled if effect dropped using key bindings
   -- bug (keith)     Fix sketch effect check sequence not reporting missing images
   -- bug (dklup)     Fix marque effect in release builds calculating incorrect buffer size
   -- bug (scott)     Fix falcon v4 upload of sequences
   -- bug (dartunder) Fix current calcs in visualiser for multi-light nodes
   -- bug (scott)     Fix ES Pixel Stick 3.2 upload
2023.14 Sep 16, 2023
   -- enh (keith)   Twinkly reconnect after power cycle
   -- enh (scott)   Add more WLED pixel types
   -- enh (scott)   Update SanDevices upload to work with 5.5/5.1 firmware
   -- enh (keith)   Preliminary Falcon V5 support
   -- enh (scott)   FPP controllers default to DMX-Open instead of DMX-Pro
   -- enh (scott)   Add Full xLights Control for HinksPix
   -- enh (dkulp)   FPP Connect - show discovered but unsupported FPP instances
   -- enh (lasershane) Implement model layout options for Distribute Depth and Alight Depth Center
   -- enh (dartunder) Update group center mark to use handle size selection
   -- enh (MrPierreB) Two new layer blending styles
   -- bug (dkulp)   Fix parsing of JSON with unicode characters
   -- bug (scott)   ESPixelStick should not allow "KeepChannelNumbers" setting
   -- bug (scott)   Allow more than 12 universes for ESPixelStick on controllers that support it
   -- bug (scott)   AlphaPix/ESPixelStick/SanDevices/WLED - make sure valid controller settings are used for various things
   -- bug (keith)   Disable Twinkly outputs still try to connect
   -- bug (dartunder) Fix hyperlink for FPP instances behind proxy in FPP Connect
   -- bug (gjones)  Fix bug where inactive models could be selected in 2D
   -- bug (gjones)  Fix import bug for models with Per Model render buffers when a single model of same name exists.
2023.13 Aug 22, 2023
   -- enh (ClutchplateDude) Expose color count to shaders
   -- enh (scott)   FPP 6+ no longer needs to switch to bridge mode
   -- enh (dartunder) FPPConnect - enhance "Cancel" button handling
   -- enh (scott)   Change downloading of Lua Scripts to github
   -- enh (scott)   Add RGB hex values to color tooltips
   -- enh (scott/dkulp) FPP Connect - bunch more logging to help diagnose FPPConnect/discovery issues
   -- enh (MartinMueller) Add support for HTTP based ESP configuration
   -- enh (dartunder) Increase size of group center mark, implement dynamic scale
   -- enh (dkulp)   Allow up to 100 layers for Circle and Star models
   -- enh (scott)   Add Experience Pro controllers
   -- bug (dkulp)   Bunch of FPP Connect fixes/updates to perform better, handle errors better, etc...
   -- bug (scott)   Fix label alignment in DMXPanel
   -- bug (scott)   Controller model property disappears on controllers in e1.31 mode
   -- bug (mjunek)  Fix issues with DPIPixels uploads for Hanson rPi-28D
   -- bug (scott)   Fix Force Custom Color in State Dialog
   -- bug (merryoncherry) Fix some potential thread safety issues in WLED effects
   -- bug (dartunder) Fix FPPConnect progress dialog always on top
   -- bug (scott)   Metronome effect with tags doesn't save tags
   -- bug (scott)   Added hinkspix checks to verify correct output boards are connected
2023.12 Aug 9, 2023
   -- enh (gil)     Improve model creation in 3D mode.  Models used to not place well depending on how you had the view rotated.
                    Now xLights will detect the view angle and make a decision on the best place to start the model so you shouldn't
                    run into models being created way in front or behind the view camera.
                    You can now rotate to a side view and draw the models rotated 90 degrees.
                    If you run into model placement issues let me know.
   -- enh (dkulp)   FPPConnect - use parallel uploads of FSEQ files to multiple controllers
   -- enh (scott)   Allow picture type singing faces on groups
   -- enh (scott)   Add effect count to SearchPanel
   -- enh (dkulp)   Use FPP7 Cape API's to verify cape pinout is usable prior to upload
   -- enh (dkulp)   Minor render speedup by allowing models to render non-dependent layers while waiting for dependent models/groups to finish the frame
   -- enh (merryoncherry)  Adjust group default camera mapping on import
   -- enh (scott)   Purge Backup needs to leave one folder so there is at least one backup
   -- bug (dkulp)   Fix hang in SingleStrand effect if the effect is a single period long
   -- bug (scott)   Fix HinksPix 80 only updating 48 ports, faster upload to HinksPix
   -- bug (scott)   Fix RGB2Go and E1-Wifi auto layout
   -- bug (gil)     Fix model selection in 2D for models based on BoxedScreenLocation (Matrix, Trees, etc.). Fixes #3887
   -- bug (gil)     Fix arches not rendering correctly when rotated
   -- bug (gil)     Fix some bugs with SketchEffect with closed paths
2023.11 July 23, 2023
   -- enh (keith)   Make Smart Receivers a more visual component of the visualizer
   -- enh (keith)   Add smart remote tooltip to visualiser
   -- enh (keith)   Fast change smart remote number (and all models on it) by right-clicking on the remote in visualiser
   -- enh (keith)   Add smart remote drag/drop in visualiser
   -- enh (scott)   Add Controller Cap and Serial protocol information to automation API JSON
   -- enh (scott)   Add ability to set Model Properties through automation API
   -- enh (scott)   Added Metronome w/ Tags for use with state effect
   -- enh (scott)   Added name support to getControllerPortMap command, added group and models filters to getModels
   -- bug (scott)   Added FPP Proxy IP checks
   -- bug (dkulp)   Attempt to workaround issues on Windows when filenames and FPP Connecte are not in UTF-8 encoding
   -- bug (keith)   Fix incorrect month in log files
   -- bug (keith)   Workaround for Minleon Smart T's with RGB+ pixels
   -- bug (dkulp)   Fix crash on save if sequence references models that don't exist
   -- bug (dkulp)   Another attempt to get the controller properties to not rebuild the entire list all the time, prevents crashes
   -- bug (Pierre Beukes) Fix Moving Head Export: Include slew limits
   -- bug (dkulp)   Fix problems with displaying models with circle/blended circle after changing it on layout page
   -- bug (dkulp)   If resizing/moving/rotating multiple models, make sure the Undo records everything instead of the single selected model
2023.10 June 30, 2023
   -- enh (dkulp)   Add node number under mouse cursor to lower right corner of SubModelDialog
   -- enh (scott)   Add zero layer size for circle model (for center pixel)
   -- enh (keith)   Enable ESC to close visualizer
   -- enh (merryoncherry) "Batch Render" enhancements to disable various warnings, modal dialogs, etc...
   -- enh (keith)   Allow override of Twinkly http port
   -- enh (MartinMueller) Add full set of ESPv4 variants
   -- enh (scott)   Add HinksPix Pro 80 support
   -- enh (dkulp)   Enable HTTP/2 for FPP Connect and Discovery
   -- bug (merryoncherry) Fix issues with bulk edit of effects
   -- bug (dkulp)   Fix crashes when selecting Vendor/Model/Variant using keyboard
   -- bug (dkulp)   Fix FPP Connect if there is a powered on Falcon v4 controller on the network
   -- bug (dkulp)   Fix square pixel type on Windows
   -- bug (scott)   Serial port configuration fixes
   -- bug (dkulp)   Fix animated GIFs not working properly
2023.09 June 3, 2023
   -- enh (scott)   Add CSV Submodel import
   -- enh (merryoncherry) Single strand chase offset and static modes
   -- enh (scott)   Add Effect Count to xLights and Vixen3 imports
   -- enh (dkulp)   Cache animated GIF frames during render to speed up looping renders. (uses more memory)
   -- bug (dkulp)   Sort by Z for house preview on Sequencer tab to match layout tab
   -- bug (dkulp)   Fix loading of older xlights_network files causing crashes
   -- bug (dkulp)   Fix pictures effect value curve min/max
   -- bug (keith)   Don't use NOWAIT on datagram sockets to avoid dropping packets
   -- bug (keith)   Fix Falcon F48v4 detection
   -- bug (scott)   Fix Linux app name parsing for launcher
   -- bug (dkulp)   Fix display issues on OSX with "Blended Circle" pixels in a group
2023.08 May 13, 2023
   -- enh (dkulp)   Windows/Linux - redo OpenGL to be based on Metal architecture
                     - removes OpenGL 1.x support (retains OpenGL 2.x)
                     - Provides shading of 3D models similar to Metal
   -- enh (keith)   Violin support for guitar effect
   -- enh (keith)   Greater control of the wave shape on guitar strings
   -- enh (kevin)   Allow customization of lightning width
   -- enh (dkulp)   Don't mark GPU rendering as experimental, it works
   -- enh (kevin)   Add simple path re-ordering to SketchAssistPanel
   -- bug (keith)   Dont show falcon controllers in FPP connect if in bridge mode
   -- bug (keith)   Fix Falcon V4 DDP one based does not upload correctly
   -- bug (dkulp)   Fix some DPI scaling issues on Windows
   -- bug (dkulp)   Fix a BUNCH of issues with DPI scaling on Linux
   -- bug (keith)   Fix duplication of face files in zip packages
   -- bug (keith)   Fix multiple panels attached to on port in visualizer
   -- bug (scott)   Fix import of some AC model effects
   -- bug (keith)   Timing marks cannot be dropped by clicking waveform and pressing t
   -- bug (keith)   Suppress controller name IP address resolution if a controller is marked as inactive
   -- bug (keith)   Color multistring custom model nodes differently in node layout
2023.07 April 25, 2023
   -- enh (keith)   Add -x option to xSchedule to silently exit if xSchedule is already running
   -- enh (keith)   Add some Hanson Electronics boards
   -- enh (keith)   Add ability to invert channels on DMX effect
   -- enh (keith)   Add value curve to picture effect centre
   -- enh (keith)   When user presses update on effect/colours panel and has > 1 effect selected warn the user about what will happen
   -- enh (keith)   Add cleanupFileLocations and saveLayout APIs
   -- enh (keith)   Add stagger to stacked model group buffers
   -- enh (keith)   Add SVG support to VU Meter level shapes
   -- enh (keith)   Add warmup frames to snowflakes and meteors effect
   -- enh (keith)   Allow suppression of dark mode on windows
   -- enh (keith)   Add video sampling instead of image resizing to video effect
   -- enh (keith)   Add ability to reset an effects settings explicitly
   -- enh (keith)   Add chord support to guitar effect ... prefix C before note so CA is the A chord
   -- enh (keith)   For superstrings > 3 channels add support for RGBW colour handling to control behaviour
   -- enh (dklup)   FPP Connect upload content to the appropriate directory
   -- enh (scott)   Increase ESPixelStick outputs to 8
   -- bug (moc)     Fix ripple effect SVG settings
   -- bug (keith)   Fix text effect not respecting \n as new line
   -- bug (keith)   Fix QM crash if using them after doing a notes import
   -- bug (keith)   Fix QM import creates illegal timing tracks with zero length timing marks and overlapping timing marks
   -- bug (keith)   Fix show/hide effect assist overwrites effect assist preference
   -- bug (keith)   Only show line segments buffer on a polyline with indiv segments
   -- bug (keith)   Fix Advatek upload broken
   -- bug (keith)   Update time display when waveform is clicked
2023.06 April 10, 2023
   -- enh (keith)   Add Guitar effect
   -- enh (scott)   Add JSON endpoints getModel, getControllerPortMap, getShowFolder
   -- enh (moc)     Add a bunch of improvements to Ripple
   -- enh (moc)     Add a bunch of utilities to the submodels dialog
   -- enh (kevin)   Add transformations - rotate & flip
   -- bug (dkulp)   Upgrade wxWidgets on Linux to address GTK issues
   -- bug (dkulp)   Update CURL calls to set Accept-Encoding to allow gzip/deflate to work for all
   -- bug (dkulp)   Fix some discovery issues
   -- bug (dkulp)   Fix FPP file manipulation on 6.3.3+
   -- bug (dkulp)   Fix models not drawing in layout
   -- bug (keith)   Fix compare between version numbers to work with subversions
   -- bug (scratch) Fix some effects panel scroll bar
   -- bug (moc)     Fix some value curves not resetting
2023.05 March 27, 2023
   -- enh (dkulp)  MacOS - Use Bonjour to help discover FPP instances
   -- enh (dkulp)  FPPConnect - better support for FPP7 instances
   -- enh (merryoncherry) Ability to press ENTER to save a submodel name
   -- enh (scott)  Add Copy States to State Dialog
   -- enh (Pierre.Beukes) Confirmation prompt when cancelling the Submodels creation dialog
   -- enh (keith)  Add a text filter to the VUMeter timing track triggers to selectively react to timing track effects
   -- bug (dkulp)  Z-Sort models before display to better handle transparency
   -- bug (keith)  MP4 export requires at least 16x16
2023.04 March 11, 2023
   -- enh (keith)  Add "Build a Light Show" vendor to prop downloads
   -- enh (dkulp)  Add new K64D-B and K8-Pi controllers
   -- enh (keith)  Add CMY support to DMX moving heads
   -- enh (keith)  Add ability to zoom to the selection in the sequencer tab
   -- change (keith) Default ArtNet universes to 510 channels when discovered
   -- bug (keith)  Audio does not stop when jukebox stops
   -- bug (merryoncherry) Fix some threading issues with Text/Shape effects on Windows
   -- bug (dkulp)  In some cases, the logs on MacOS could not be created.
   -- bug (merryoncherry/dkulp) Fix some bad rendering with Warp effects
   -- bug (merryoncherry) Fix bugs with compress/expand of node ranges in SubModel node selection
   -- bug (merryoncherry) ValueCurves for Ripple Rotation were not working
   -- bug (MrPierreB) Metronome timing tracks were not available for import
   -- bug (scott)  Fix background image crash on Windows
2023.03 February 22, 2023
   -- enh (dkulp)  Add ability to create "FPP Command Preset" and "FPP Effects" timing tracks and save that in fseq files (FPP 7)
   -- enh (dkulp)  Add FPPConnect upload of fseq/media to FPP7
   -- enh (keith)  Change jukebox to play the sequence looped for the effect duration rather than just selecting the effect
   -- enh (merryoncherry) New 5x5 Font for text effect
   -- bug (merryoncherry) Fix icons for Import Effects panel not matching model
   -- bug (merryoncherry) Fix save icon remaining pink if saving linked network/models
   -- bug (keith)  Overlap checks does not always identify model
   -- bug (scott)  Fix crash when switching from player
   -- bug (scott)  Allow Serial Outputs On Virtual Matrix
   -- bug (keith)  Fix crash when setting smart remote type on a port with no models.
   -- bug (scott)  Fix F##v4 upload
2023.02 February 01, 2023
   -- enh (scott)  Add "dark mode" for Windows
   -- enh (lutz)   Add Effect Duration to the variables available to shaders
   -- enh (dkulp)  Allow xcontroller variants to specify which "ConfigDriver" they need.  Not based soley on vendor.
   -- enh (keith)  Fix #3687 add optional frameMS and view parameters to xlDo newsequence
   -- enh (scott)  Add Wally's WB48
   -- enh (dkulp)  Add Warning about potential removal of OpenGL 1.x/2.x
   -- enh (dkulp)  Add Warning about potential removal of Windows 7 support
   -- enh (dkulp)  Add Warning about potential removal of FPP <6 support
   -- bug (dkulp)  Fix Twinkly output on M1 Macs
   -- bug (keith)  Fix crash when dropping sketch effect
   -- bug (scott)  Hopefully fix IP/Host Validation
   -- bug (dkulp)  Fix duplication of media dirs if flip back/forth of tabs in preferences panel
   -- bug (dkulp)  Fix DDP packet data type to match spec
   -- bug (dkulp)  Prevent visualizer from opening up if Output to lights is on (prevents some crashes)
2023.01 January 05, 2023
   -- enh (dkulp)  Speed up aborting of rendering so UI tasks can complete quicker
   -- bug (dkulp)  Fix Aborted rendering not marking ranges as "Dirty" so they render properly later
   -- bug (dkulp)  Fix some crashes with resizing/moving effects and timing marks
   -- enh (dkulp)  Update to latest wxWidgets+patches so Scott can play with Dark Mode on Windows
2022.26 December 29, 2022
   -- enh (scott)  Added ability to duplicate or "tile" an effect
   -- bug (dkulp)  macOS - Fix some issues in SubModel dialog where everything will grey out after deleting a submodel
   -- bug (dkulp)  Fix some crashes if supress frames is used at the beginning of an effect
   -- bug (dkulp)  Fix memory leak with GIF images
   -- bug (gil)    Fix mutex deadlock and fix hitting wxassert in the debugger when importing sequences with timing tracks
   -- bug (dkulp)  Fix memory leak in faces effect with images on Matrices
   -- bug (dkulp)  Fix memory leak in video reader
   -- bug (dkulp)  Fix memory leak in MusicEffect
2022.25 December 16, 2022
   -- enh (Shelby) Bunch of Vixen 3 import enhancements to aid migration from Vixen 3
   -- enh (scott)  Add date modified to Batch Render dialog
   -- enh (scott)  Save size/position of Batch Render dialog
   -- enh (scott)  Add ESP32-AE+ variants
   -- enh (keith)  Increase maximum size of background images to 4K
   -- bug (dkulp)  Fix crash in GPU renderer if too many commands are submitted at once
   -- bug (keith)  Fix second and subsequent strings on multi string models which are shadowed not showing the visual cue that they are shadowed
   -- bug (gil)    Fix model handle size with low zoom settings
   -- bug (scott)  Use Node Name and Strand names on effect export
   -- bug (keith)  Fix Visualizer printing dark when OS is in Dark mode
   -- bug (dkulp)  Fix crash with twinkly export on macOS
2022.24 December 1, 2022
   -- enh (keith)  SubModel creation - allow user to press space when a cell is the active cell to "double click" on the cell
   -- enh (keith)  Improve some face/state effect render times
   -- enh (dkulp)  macOS - Add "Open New Instance" to Doc icon
   -- enh (keith)  Allow removal of Sketch effect background image
   -- enh (keith)  Allow keyboard delete of a preset
   -- enh (scott)  CustomModel dialog - allow h/v flip of selected area
   -- enh (keith)  Expand fan effect radius
   -- enh (scott)  Added search to Face Download Dialog
   -- enh (scott)  Added support for QuinLED-Dig-Octa
   -- bug (gil)    Fix bug when drop sizes set negative. Introduced in 2022.22 when fixing drop size set to 0 bug
   -- bug (dkulp)  Fix crash in render on Linux
   -- bug (scott)  Fix upload to Dragon controllers
   -- bug (keith)  Bulk Edit Combo box does not work as a combo box
   -- bug (keith)  FPP Connect uses different method to determine FSEQ location than save sequence
   -- bug (keith)  Put warnings in log when zip extract filenames exceed MAX_PATH
   -- bug (keith)  Add more logging to Twinkly enable
   -- bug (keith)  Fix crash on render model to LCB
2022.23 November 17, 2022
   -- enh (keith)  Upgraded to newer Visual Studio, hopefully fixes some image issues
   -- enh (dkulp)  Add Shader filename in log if shader cannot be compiled to make debugging shader issues easier
   -- enh (dkulp)  Finer grained FixFiles locking results in faster startup when bouncing back/forth between Windows/macOS
   -- enh (keith)  Add a default camera to groups and handle using it when selecting per preview render buffers
   -- enh (MrPaulAR) Add serial outputs to visualizer to some Pi based controllers
   -- enh (keith)  Add Mattos Designs controller
   -- enh (KMojek) Add "Reverse Fades" checkbox to ColorWashPanel
   -- enh (merryoncherry) Import custom model - add submodels to groups
   -- bug (keith)  Export model does not allow selection of groups containing submodels of the model being exported
   -- bug (keith)  Sketch effect background image not included in packaged sequences
   -- bug (scott)  Fix hinkspix upload if only using SR ID of B,C,D on 16SR.
   -- bug (scott)  Fix model sorting in import dialog
   -- bug (scott)  Fix SVG import from packaged sequences
   -- bug (scott)  Fix ExperienceLights upload color order
   -- bug (dkulp)  Fix crash in Shader effect if shader is not a proper shader or is using invalid unicode characters
   -- bug (dkulp)  Fix hang on sequence close with RenderCache enabled
   -- bug (gil)    Fix start channel bug when PolyLine Lights/Node is greater than 1
2022.22 November 7, 2022
   -- enh (gil)    Refactored several huge sections of code in PolyLine reducing the complexity of the code
   -- enh (keith)  Add a way to add show players and pure media players to the controller tab
   -- enh (keith)  Prompt user if sequence unsaved and they try to access FPP connect
   -- enh (keith)  Add option to output to lights when defining faces, states, submodels and using the node select dialog
   -- enh (keith)  Add support for the original Minleon NDB controller
   -- enh (keith)  Add Custom Christmas Creations as a prop vendor in the model download
   -- enh (keith)  Add visual model node selection to the test function
   -- enh (scott)  Add experience lights discovery
   -- enh (dkulp)  Increase maximum matrix size
   -- bug (gil)    PolyLine: Now all variations of using lights/node > 1 should work
   -- bug (gil)    Fix bug where program appeared to hang if dropsize is set to 0 for PolyLine
   -- bug (keith)  Address a crash in render cache. This is not a complete fix as we are seeing some hangs in batch render related to this.
   -- bug (keith)  Fix Twinkly start channel bug
   -- bug (keith)  Fix node select does not work on windows if dark mode selected
   -- bug (keith)  Fix some Pixlite controller uploads failing
   -- bug (scott)  Fix experience lights DDP start channel and max pixels per port
   -- bug (dkulp)  Fix falcon FPP discovery
   -- bug (dkulp)  Fix blended/solid circle node display not dependent on camera view point
2022.21 October 28, 2022
   -- enh (dkulp)  Much faster algorithm for finding frames in the RenderCache, should speed up initial rendering after load
   -- enh (dkulp)  macOS: RenderCache uses mmap to reduce actual memory usage and allow the OS cache to handle loading/discarding
   -- enh (dkulp)  macOS: Add RotoZoom to Metal based GPU rendering engine
   -- enh (dkulp)  macOS: Add Plasma to Metal based GPU rendering engine
   -- enh (dkulp)  Several performance improvements for the Parallel job pool
   -- enh (dkulp)  Display submodel effect rendering status as tool tips in Rendering progress dialog
   -- enh (scott)  Several updates for Experience Lights controllers
   -- enh (gil)    PolyLine enhancement. Add ability to define which segment owns the pixel at each endpoint.
   -- bug (dkulp)  Fix Shift/Two Finger panning on TouchPad when not viewing from front
   -- bug (dkulp)  Fix problems with GPU rendered effects saving as "black" into RenderCache
   -- bug (dkulp)  Fix crash when creating new sequence if "More Options" button is used aand certain other values are adjusted
   -- bug (dkulp)  Fix crash in "Log Rendering Status" if models have finished
   -- bug (dkulp)  Fix display of model groups when some models are added/removed
   -- bug (dkulp)  Fix crash with certain effects if they occur after a GPU rendered effect
   -- bug (dkulp)  Fix fire effect on "Per Model Per Preview" render buffers
   -- bug (dkulp)  Fix fire effect on submodels with non-default render style
   -- bug (dkulp)  Honor the "max group size" in more cases to avoid massive render buffers
   -- bug (gil)    PolyLine: Fix #Lights/Node not working when > 1.  Still fails when Icicle drops are used.  Working on it.
   -- bug (keith)  Fix crash after removing a submodel from a group
   -- bug (keith)  Context menu of SubBuffer has options that don't make sense in that context and can cause a crash
   -- bug (keith)  RGBW doesn't always zero out the unset channels
2022.20 October 17, 2022
   -- enh (scott)  Add Experience Lights upload support
   -- enh (scott)  Add VU Meter Timing Event chase to/from middle
   -- enh (keith)  Add menu option to re-enable rendering on all models
   -- enh (keith)  Add the ability to light up the port number of pixels on each port on a controller in controller test
   -- bug (dkulp)  Fix auto-upload of FPP config causing reset to player mode with FPP6
   -- bug (dkulp)  Attempt to fix crash on macOS when double clicking on xseq files
   -- bug (keith)  Fix several Shader parameters not resetting and Offset X not loading
   -- bug (scott)  Fix FPP upload not remembering models from Controller
   -- bug (scott)  Fix/workaround a VS2022 optimizer bug causing a crash
   -- bug (scott)  Fix Bulk Set Port and increment with multistring models
   -- bug (scott)  Fix SerialiseModelGroups with groups with other models
   -- bug (scott)  Max Cascade should not be dependent on cascade on ports
   -- bug (scott)  Fix Export 3D custom model trees
   -- bug (scott)  Fix serial on espixelstick v4
   -- bug (scott)  Mapping dialog: Remove *.* from save filter, switch format if JSON formatting is found
   -- bug (dkulp)  Fix crash on Effect Import dialog on Linux
2022.19 October 4, 2022
   -- enh (keith)  Add DMX512P pixel support to Advatek controllers
   -- enh (keith)  Add Advatek v8 protocol support
   -- enh (keith)  Separate Advatek expanded mode support as a variant. 
                   Users who were using expanded mode may need to change variant to upload.
   -- enh (keith)  Add twinkly products as recognized controllers
   -- enh (keith)  Show as an error non selection of controller model
   -- enh (keith)  Give remote falcon users in xSchedule more control over logging volume
   -- bug (dkulp)  Fix issue where model groups don't update until xLights restart
   -- bug (dkulp)  Fix crashes where Fire/Candle effect is used on a submodel
   -- bug (dkulp)  Fix crashes where inactive models are added to groups
   -- bug (dkulp)  Ensure BBB serial output uses the same pinout version as strings
   -- bug (keith)  Possibly address some twinkly start output crashes and if not capture more information
   -- bug (keith)  Fix expanded mode not being set on Advatek boards when it should be
   -- bug (keith)  Fix Twinkly protocol not proxyable via FPP
   -- bug (keith)  Fix not all minleon controllers show the minleon protocols
   -- bug (keith)  Slightly improve visual artifacts on pixture effect on windows due to wxWidgets bug on visual studio
   -- bug (keith)  Fix unnecessary reboot prompt when uploading DDP to F48 V3 controllers
   -- bug (keith)  Fix face shimmer is not frame rate independent
   -- bug (gil)    Fix moving heads don't move smoothly on windows
2022.18 September 28, 2022
   -- enh (dkulp)  Add discovery for new BBShiftString FPP capes
   -- enh (dkulp)  Add upload config support for K128D-B cape
   -- enh (scott)  Support for new HinksPix firmware to support 402 universes
   -- enh (gil)    Improve drawing of 3 point models (Arch, Candy Cane, Icicles) when green square is right of blue square.
                   Note: If model changes look because it was on a tilt an X rotation with the red rotate handle should fix it.
   -- enh (scott)  Added the ability to scale/offset DMX Values in the DMX Remap Dialog
   -- enh (scott)  Added per model color to DXF file export using the Tag Color
   -- enh (scott)  Added Search Box to SubModel Dialog
   -- enh (scott)  Add Ability to Merge/Copy The Submodels on Replace Model with a Model
   -- enh (scott)  add "${UPPER}" and "${LOWER}" to Text Effect
   -- enh (scott)  Added JSON format import mapping file.
   -- enh (scott)  Added Align Front, Back, & To Ground KeyBindings.
   -- enh (keith)  Add German dictionary to install, user will need to manually add
   -- enh (keith)  Allow shimmer to be suppressed on a face effect
   -- enh (keith)  Allow double click to manually set custom value curve Y values
   -- enh (keith)  Add ability to match video effect duration to the selected video
   -- enh (keith)  Add DMX512P and SJ1221 pixels to Falcon
   -- enh (keith)  Allow renaming of previews
   -- enh (keith)  Aldd option to delete model strand and node effects
   -- bug (keith)  Suppress Tip of the Day if not show folder is set
   -- bug (keith)  More workarounds for image scaling on Windows release builds with MSVC22
   -- bug (keith)  Don't allow opening of _.xbkp files without renaming
   -- bug (keith)  Fix controller name, advanced config, zigzag for pixlite controllers
   -- bug (scott)  Fix DMX Beam Always On when using Color Wheel
   -- bug (scott)  Fix Package Sequence from automation API
   -- bug (scott)  Fix SubModel Dialog not persisting screen position and size.
   -- bug (dkulp)  Older BBBSerial outputs may have device improperly configured during FPP upload
2022.17 September 8, 2022
   -- enh (keith)  Add xSchedule Remote Falcon option to send the first playing effect as the currently playing sequence
   -- enh (keith)  Add SVG support to the shapes effect (some SVGs with gradients may have an issue)
   -- enh (keith)  Allow optional disabling of more pedantic Check Sequence checks
   -- enh (keith)  Disable autosave during render all
   -- enh (keith)  Add shimmer support to matrix faces
   -- enh (dkulp)  Update wxWidgets to 3.2/3.3
   -- enh (scott)  Add M16-PI controller
   -- enh (scott)  Added duplicate node custom model checkbox
   -- enh (scott)  Add smart remote and model chaining details to DXF export
   -- enh (scott)  Added Bulk Edit to Text Effect File Picker
   -- bug (keith)  Fix picture files showing red as if there is an error when there isn't an error
   -- bug (keith)  Dont use render cache for text effect if text is coming from a file
   -- bug (keith)  Fix a bug with importing models and adding them correctly to existing groups
   -- bug (keith)  Fix speed value curve reset on the video effect
   -- bug (keith)  Fix brightness blend does not respect alpha channel
   -- bug (keith)  Fix xSchedule adds multiple sequences in reverse order than expected
   -- bug (keith)  Fix reverse video does not work for start positions other than 0ms
   -- bug (keith)  Fix servo effect crash due to missing Nod channel
   -- bug (keith)  Fix Falcon v3 upload leaves unused ports with invalid settings
   -- bug (keith)  Fix hang if indiv start channels refers to a model on no-controller
   -- bug (keith)  Fix crash when xlDo killed while executing a render
   -- bug (keith)  Fix shadow model for errors which cannot be corrected through UI (on load)
   -- bug (scott)  Fix WLED pixel limis and universe limits to match current capabilities
   -- bug (scott)  Escape model names for xmaphints export
   -- bug (scott)  Disable Universe Per String, when Auto Size is disabled as it does nothing
   -- bug (dkulp)  Fix crash for some per model renders
   -- bug (dkulp)  Fix DMX models returning zero size resulting in problems with centre calculation
   -- bug (cyber)  Fix hostname validation for controller reports some valid names as errors
2022.16 August 22, 2022
   -- enh (keith)  Allow optional turn off of house and model window docking
   -- enh (keith)  On Falcon and FPP allow definition of zigzag in xlights for upload
   -- enh (keith)  Include controller connection zigzag in xmodel export and import
   -- enh (ruskin) added an xSchedule command to clear all pixel overlays
   -- enh (scott)  WLED 0.13.2 updates
   -- enh (scott)  Add xmaphint mapping for strands and submodels
   -- bug (keith)  Fix hang if xSchedule is both FPP master and remote
   -- bug (keith)  Fix flash when xSchedule starts video
   -- bug (keith)  Address xLights lagging due to timer issues
   -- bug (keith)  Resize of effect row does not immediately adjust number of rows drawn
   -- bug (keith)  Fix multiselect of state effect corrupts state radio button
   -- bug (keith)  Fix xlDo closexLights does not work
   -- bug (keith)  Fix ugly outline on effect buttons
   -- bug (keith)  Fix xlDo openSequence does not accept sequence name in json
   -- bug (keith)  Fix picture effect warns multiple times that file not valid
   -- bug (keith)  Fix replace model loses group membership
   -- bug (keith)  Fix Falcon FPP upload over wifi not working
   -- bug (ruskin) Fix  "Set pixel range" doesn't clear pixels when channel count of zero is used
   -- bug (scott)  Adjust x/y locations of FPP virtual display map to the whole preview, not the arbitrary bounding box
   -- bug (scott)  Fix stand rendering with layered arches and stars
   -- bug (scott)  On Hinkpix Pro move DMX after pixels
2022.15 August 4, 2022
   -- enh (keith)  Add a Liquid effect gravity direction.(fixes #3343)
   -- enh (keith)  Add ZCPP support for default port brightness.
   -- enh (scott)  Added Model Icons to Import Dialog if a rgbeffects file is found.
   -- enh (scott)  Added Wally Light's WB1616 Controller.
   -- bug (keith)  Fix possible xScanner crash.
   -- bug (scott)  Fix virtual display map upload on FPP 6.
   -- bug (scott)  Fix strand order for rendering on stars.
   -- bug (scott)  Fix loading matrix face images from Network Shared Folder.(fixes #3342)
   -- bug (scott)  Fix Text Effect Replace with encoded characters.(fixes #3359)
   -- bug (scott)  Fix Import Effects Automap not matching strand/node name.(fixes #3328)
   -- bug (pierre) Prevent duplicate values when bulk editing comboboxes.
   -- bug (keith)  xSchedule: Fix Non-Musical Sequences Playback Slower Than Expected.(fixes #3322)
2022.14 July 10, 2022
   -- enh (keith) Add a tool for remapping custom models to alternate pixel layouts
   -- enh (keith) Make random effect generation cancellable
   -- enh (keith) Add option to RGB Cycle testing to tag every 50th pixel
   -- enh (keith) Allow user dictionary items to override the standard dictionaries
   -- enh (keith) Allow visualiser and upload support for APA109 compatible pixels on a WS2811 port
   -- enh (dkulp) Add host specific media support to FPP connect
   -- enh (scott) Add backup restore process
   -- bug (dkulp) Fix some issues where multiple audio devices exist
   -- bug (dkulp) Fix some crashes where render buffer needs to be larger than model
   -- bug (dkulp) Fix fractal/ivy wave effect
   -- bug (dkulp) Fix playlist item duplication in FPP upload
   -- bug (keith) Fix some ripple parameter scales on S5 import
   -- bug (keith) Fix rotozoom order not saved
   -- bug (keith) Fix issue with audio file upload to FalconV4
   -- bug (keith) Fix export as custom model does not export depth
   -- bug (keith) Fix some duplication in transition time combobox behavior
   -- bug (keith) Fix video reported as missing if using sequence media
   -- bug (scott) Fix color error in sketch effect
   -- bug (scott) Fix issue with PiHat upload
   -- bug (scott) Fix fire effect crash
   -- bug (moc)   Fix some issues with model state dialog
2022.13 June 26, 2022
   -- enh (scott) Added Controller Hyperlinks to FPP Connect
   -- enh (scott) Add FPP 6 DPIPixels support
   -- enh (scott) Added setting to Select Video Export Video Codec
   -- enh (scott) Added right click option to change tree/matrix/custom model string count from the visualizer
   -- enh (scott) Add Bulb Type to LOR S5 Preview Import
   -- enh (scott) Remember Controllers Splitter Location
   -- enh (keith) Add infrastructure for tip of the day functionality. Currently not enabled.
   -- enh (keith) Allow xSchedule to direct and source audio from specific sound cards
   -- enh (keith) Add support for the Advatek MK3 controllers
   -- enh (moc)   Add horizontal tree option to the tree model
   -- enh (dkulp) Move to latest wxWidgets on OSX and Windows
   -- bug (scott) Fix Empty Brightness Field in Dimming Curve Dialog
   -- bug (scott) Fix Loading Timing Labels flagging the Sequence as changed
   -- bug (scott) Fix DMX Validate Settings off by 1 error
   -- bug (keith) Increase colour dropper limit from 200 to 1024
   -- bug (keith) Add a way to optionally force strict source port compliance for artnet
   -- bug (keith) Add more import of Ripple properties from S5
   -- bug (keith) Ensure ZCPP respects default controller brightness
   -- bug (keith) Show search and select effect times formatted.
   -- bug (keith) Fix link colour on windows when in dark mode
   -- bug (keith) Add back manual custom model generation for tricky models to video
   -- bug (nash)  Fix numerous issues with prepare audio feature
   -- bug (dkulp) Fix some corruption of canvas effect buffers when layers below use subbuffers
   -- bug (dkulp) Fix canvas mode effects that use Per Model render styles.
   -- bug (dkulp) Fix issue when loading a xsq by double clicking on it
   -- bug (dkulp) DMX moving heads use sequence frame timestamp to calculate pan/tilt rates instead of real time
   -- bug (moc)   Fix drag select on submodels dialog loses mouse if you leave the window area.
2022.12 June 7, 2022
   -- enh (scott) Move ScottNation controllers under their own vendor
   -- enh (keith) Rewrite custom model generation to use a faster algorithm
   -- enh (scott) Add DMX color wheel support
   -- enh (keith) Lift Falcon V4 max group size to match latest firmware
   -- enh (dkulp) Add twinkly output upload to FPP
   -- enh (dkulp) Merge artnet multi-lines to one when uploading to FPP
   -- enh (scott) Added the Ability to define "Preset" channel values when render non DMX Effects on a DMX model
   -- enh (keith) When a ruler is defined on import or export of some model types save the real world dimensions 
                  in the xmodel file and apply them when loading it
   -- enh (keith) Add an option to silence the audio in xLights
   -- bug (scott) Fix faces effect required a minimum of 10 states
   -- bug (scott) Fix default gamma not saving
   -- bug (keith) Fix twinkly model data import imports upside down
   -- bug (dkulp) Abort render whenever a model group is changed
   -- bug (keith) Abort render whenever a timing track is deleted or renamed
   -- bug (keith) If there is no LUA mime handler use the txt handler instead
2022.11 May 21, 2022
   -- enh (scott) Added Load from State to DMX Effect
   -- enh (scott) Add import from SVG to Sketch Assistant Panel
   -- bug (dkulp) Fix toolbars/images on Windows with non-100% scaling (WIP)
   -- bug (dkulp) Fix drag/drop in Import Dialog on MacOS
   -- bug (dkulp) Fix video stream of exported House Videos if codec has to change from defaults
   -- bug (dkulp) Maybe fix green video export on MacOS
   -- bug (dkulp) Change how buttons with background colors set are drawn on MacOS
   -- bug (dkulp) Fix hang with drag/drop of Faces Effect
   -- bug (dkulp) Fix a crash situation if a buffer is reported as being larger than it actually is
   -- bug (keith) Fix a possible OpenGL crash  if context cannot be created
2022.10 May 15, 2022
   -- enh (aarcangeli/keith) Add support for Twinkly controllers including discovery
   -- enh (keith) Add ability to import Twinkly pixel layout into a custom model
   -- enh (keith) Add support for the Minleon NDBPro
   -- enh (scott) Update FPP Connect model generation to support all models, models only on controller and get models from FPP
   -- bug (keith) Fix crash in effects rendering with model as a pixel on inactive models in a group
   -- bug (keith) Fix when editing faces/states/submodels using key binding rgb effects file not marked as dirty
   -- bug (keith) Fix choice controls try to set before list is populated resulting in loss of value
   -- bug (keith) Fix DMX model export creates invalid xmodel file
   -- bug (scott) Fix a crash when working with Hinkspix 16 port remotes
   -- bug (dklup) Fix issues with shear on 3 point models
2022.09 April 29, 2022
   -- enh (kevin) Allow for reverse of sketch-effect paths
   -- enh (kevin) Sketch effect remembers background image and opacity
   -- enh (kevin) Add value curve for Sketch effect motion-percentage
   -- enh (kevin) Add closed paths with motion to Sketch effect
   -- enh (dkulp) Add value curve for Sketch effect draw percentage
   -- enh (scott) Add BBB16 Expansion
   -- enh (scott) Preliminary support for Picture XY and Marquee LOR S5 Import
   -- enh (dkulp) Add "Remove models from group" to Popup for SubModels in groups
   -- bug (dkulp) Fix finding of images for Sketch effect on MacOS
   -- bug (scott) Fix picture XY movement settings
   -- bug (gil)   Fix DmxServo3D Import problems
   -- bug (scott) Fix Falcon v3 pixel type decoding and prevent lockup of controllers
   -- bug (scott) Fix Falcon v4 missing Default Brightness
   -- bug (dkulp) Fix problems with exported House video if Hardware encoders are unavailable/busy
   -- bug (keith) Fix test mode outputting double frames
   -- bug (keith) Fix xCapture not ignoring non lighting data E1.31 packets.
   -- bug (dkulp) Fix content scale in export House Videos on mixed Retina/non-Retina multi monitor setups
   -- bug (robfallone) Fix stack traces in crash logs on Windows
2022.08 April 12, 2022
   -- enh (keith)    Better music/piano effect horizontal scaling
   -- enh (scott)    Updates to effect search to use regex, other options
   -- enh (kevin)    New "Sketch Effect"
   -- enh (dkulp)    Allow ENTER/RETURN to end Polyline (request from FMM)
   -- enh (dkulp)    Add "Remove Models from Group" to popup menu (request from FMM)
   -- enh (keith)    New Controller Oriented test channel selection
   -- enh (scott)    Attempt to auto-find singing faces files
   -- enh (scott)    Several updates to Faces dialog
   -- enh (keith)    Add tag color to model groups
   -- enh (keith)    Check Sequence - flag invalid/missing background images
   -- enh (keith)    Move some toolbar buttons around, reduce screen usage of toolbars
   -- enh (keith)    State Effect  - Significantly increase maximum states supported
   -- enh (keith)    Select all timing events using right click on a timing track
   -- bug (keith)    Only show default brightness and gamma on controllers that support it
   -- bug (scott)    Fix Hinkspix Serial issues, move DMX to end
   -- bug (keith)    Fix crash in check sequence with mesh's with spaces in MTL files
   -- bug (keith)    Some videos may not fully play due to strange way they calculate length
   -- bug (keith)    Make backup of backup folder exclusion case insensitive
   -- bug (keith)    State  Effect - fix not rendering on first dropped
   -- bug (keith)    Node layout for custom model with multiple strings but no individual start nodes incorrectly shows the string number
   -- bug (dkulp)    Fix Image Model transparency issues, make sure not "dark" on layout panel
   -- bug (keith)    Reset to default waveform on load of new audio after custom filter applied
2022.07 March 28, 2022
   -- enh (gil)      Add Jaw Orientation property for the DMXSkull model
   -- enh (gil)      DMXSkull model changed so color error checking is only done when color channels are active
   -- enh (scott)    Added Effect Settings Search Panel.
   -- enh (scott)    Added Text Replace to Text Effect ${TITLE},${ARTIST},${ALBUM},${FILENAME}.
   -- enh (scott)    Added Support to Import S5 .lorprev Preview Files.
   -- enh (scott)    Added WLED Dig-Uno ESP32 Variant with correct pins.
   -- enh (scott)    Added WLED Warning For Firmware v0.13 & v0.13.1 not working with Controller Upload, use v0.13.2 or v0.13b5.
   -- enh (scott)    Added xLightsAutomations APIs to get and set Effect Settings.
   -- enh (scott)    Added Lua commands for TableToJSON and Table Logging.
   -- enh (dkulp)    Add a timer for video export
   -- enh (dkulp)    Move 3D checkbox so it does not get lost when screen sizes are changed
   -- enh (keith)    Add the minleon 4 port NDB controller
   -- enh (keith)    Use number aware sorting in test dialog for models and submodels
   -- enh (keith)    Add the ability to split the current effect
   -- bug (scott)    Fixed wrong HSV value with On Effect added in 2022.6
   -- bug (keith)    Fix state dialog shows colours even when force colours is not set
   -- bug (keith)    Fix wave effect frozen at frame rates over 20ms
   -- bug (keith)    Fix visualiser does not show port number for models on other controllers
   -- bug (keith)    Fix jerky model video playback due to frame count interpretation error
   -- bug (keith)    Restore AVI export on windows for model video so lossless compression works
   -- bug (keith)    Fix issues with make all model start channels valid not working
   -- bug (keith)    Fix xSchedule POST requests issued as GET requests ... this will need to be re-setup.
   -- bug (keith)    Fix uploading to minleon controllers
   -- bug (keith)    Fix sequence number bug in KiNet protocol
   -- bug (keith)    Fix crash in text effect in animation sequences
   -- bug (keith)    Fix save sequence does not update MRU
2022.06 March 6, 2022
   -- enh (gil)      Add auto repeat option to Morph.  Automatically scales the repeat to model size.
   -- enh (keith)    Add a low definition rendering mode to matrices
   -- enh (keith)    Enhance we way transition times are set
   -- enh (keith)    Add exportModelWithRender to API
   -- enh (keith)    Add radial colour curves to the on effect
   -- enh (keith)    Add greater precision to wave effect speed
   -- enh (keith)    Allow disabling of render at the element level
   -- enh (keith)    Add word flip movement to the text effect
   -- enh (keith)    Add a reverse lookup from rendered data to source effects
   -- enh (cpinkham) Add z coordinate to the FPP virtual display map
   -- enh (dkulp)    Adjust xLights calls to FPP to use tweaked API
   -- bug (keith)    Fix presets using per model buffers don't preview correctly
   -- bug (keith)    Fix model default type causes face definition to not save correctly
   -- bug (keith)    Fix polyline reports incorrect real world dimensions
   -- bug (keith)    Fix missing gridlines in morph effect assist
   -- bug (keith)    Fix blended circles have an incorrect coloured halo
   -- bug (keith)    Fix when effect dragged from palette drop location not shown in the waveform
   -- bug (keith)    Dont add inactive models to a model group ... this will stop them displaying but may create other issues
   -- bug (dkulp)    Provide a workaround to spaces in material file names
   -- bug (scott)    Fix issues with ordered node selection for submodels
2022.05 February 26, 2022
   -- enh (keith) Upgrade windows to use FFMPEG v5 
   -- enh (keith) Rewrite windows hardware video processing
   -- enh (keith) Reduce minimum frame time to 10ms
   -- enh (keith) Allow xmodels to include shadow models for simpler import
   -- enh (keith) Improve the face defaults based on the type of model
   -- enh (keith) Add to the model export dialog the type of export
   -- enh (keith) Add a colour search and replace function to edit menu
   -- enh (keith) Where DMX models cant render draw an error box and also explain why in check sequence
   -- enh (scott) Add 3D mouse support to windows 
   -- enh (scott) Allow selection of matrix faces by double clicking the row title
   -- enh (dkulp) Add bulk edit of pixel appearance settings 
   -- enh (dkulp) When control held down and object selected rotate/move the view rather than the object 
   -- bug (scott) Fix batch render
   -- bug (scott) Fix Q1 pin on QuinLED-Dig-Quad
   -- bug (dklup) Fix FX not rendering 
   -- bug (dkulp) OSX Fix node select grid colours in dark mode 
   -- bug (dkulp) OSC Fix fallback for 3D mouse where drivers are missing
   -- bug (slupsky) Fix default view for new sequences 
   -- bug (keith) Fix ruler does not initialise if you are not in 3D mode 
   -- bug (keith) Fix heart render has inconsistencies 
   -- bug (keith) Universe per string allocates too many universes for serial port models 
   -- bug (keith) Sequence save as does not update recent sequences menu 
   -- bug (keith) Fix DMX general model does not display 
2022.04 February 5, 2022
   -- enh (dkulp) MacOS: Move to FFMPEG 5 for accelerated ProRes video decoding on M1 Pro/Max
   -- enh (dkulp) MacOS: Update 3D Mouse support to manipulate individual control points
   -- enh (dkulp) MacOS: Enhanced iCloud support to make sure files are fully downloaded when needed
   -- enh (dkulp) Change export model videos to mp4 (from avi) to fix issues trying to import into various apps (like FinalCut)
   -- enh (keith) Add FX to single strand effect. This is a port of WS2812FX/FastLED as modified by WLED into xLights.
   -- enh (keith) Add keybindings for model data, faces, states and submodels
   -- enh (keith) VUMeter: Use sensitivity setting in VU Meter Timing Event Colour to control the transparency of the colour when no timing mark is actually present
   -- enh (scott) Change xlDo controller and model commands to use JSON list type. Added more lua type wrappers to methods
   -- enh (keith) Add ability to change temporarily multiple times before restoring to permanent
   -- enh (keith) Warn user if they change show folder to a non-empty folder that does not contain networks or rgbeffects
   -- bug (MikeSoukup) Include middle pixel for Single Strand / From Middle effect with odd number of nodes
   -- bug (keith) Fix falcon WAV file header validation
   -- bug (keith) Prevent negative drops on icicles
   -- bug (keith) F1 on layout tab can cause a never ending loop
   -- bug (dkulp) Fix undo of view object movements
   -- bug (dkulp) Fix video preview on Windows
   -- bug (keith) VUMeter:  Level colour shows white before it is first triggered
   -- bug (keith) Crash loading submodel which references nodes that are out of range in the model
   -- bug (dkulp) Fix arch/single line/candycane rotation around center
   -- bug (gil)   Fix issue with Per Model Default Deep not creating large enough buffer
2022.03 January 22, 2022
   -- enh (dkulp) MacOS: Initial support for 3DConnexion 3D mice in Preview windows
   -- enh (dkulp) MacOS: Support for using normals for shading 3D meshes
   -- enh (sslupsky) Add support for differential ports for BBB16v2
   -- enh (gil)   Add new render buffer "Per Model Default Deep" that will recurse nested model groups
   -- bug (dkulp) MacOS: Fix scrolling/displays on OSX after play finishes
   -- bug (dkulp) Fix group centering + if  submodels are in the group
   -- bug (dkulp) Fix several issues with mesh/obj files not rendering properly
   -- bug (dkulp) Fix crashes if material files cannot be loaded
   -- bug (dkulp) Fix problems on OpenGL with single color strings
   -- bug (scott) Fix crashes in custom model wiring view
   -- bug (gil)   Fix issues with PolyLine caused by multiple string upgrades
   -- bug (keith) Fix cyan first pixel on some models showing incorrectly
2022.02 January 15, 2022
   Note: macOS now requires macOS 10.14.  10.12 and 10.13 are no longer supported
   Note: The DMX models have changed from using world coordinates for various sizes (like beam lengths) to
         values relative to the size/scale parameters.  Thus, users will need adjust things to accomodate.
         Pixel sizes are also consistent between 2D and 3D and thus may also need adjusting.
   -- enh (gil)   Add multiple string support to PolyLine
   -- enh (dkulp) Move all UI views from OpenGL to Metal on macOS
   -- enh (dkulp) 3D and 2D layouts use same code paths (just flattend for 2D) so features like cyan starting
                  nodes will work on 3D layout.
   -- enh (dkulp) 2D layouts now use flattened 3D meshes for DMX models instead of a red X
   -- enh (scott) Automation: add getModels, getControllerNames and getControllerIPs
   -- enh (scott) Add LUA scripting engine for scripting Automations
   -- enh (keith) Show which panes are visible with checkmarks in the view menu
   -- enh (dkulp) Add Warp to Metal GPU rendering
   -- enh (gil)   Add multiple string support to PolyLine
   -- bug (keith) On some models, the cyan "first node" marker is in the wrong location
   -- bug (robfallone) Fix Per Model Per Preview doesn't include all Single Line models in group
   -- bug (Nick Krecklow) Use full LOR brightness encoding range in LOROutput
   -- bug (keith) Fix highlighting of nodes in submodels and states where nodes are reused across multiple rows
   -- bug (scott) Increase number of universes for v4 ESPixelStick
2022.01 January 6, 2022
   NOTE: This will be the last version to support macOS 10.12 and 10.13.   Future builds will
         require macOS 10.14 or newer.
   -- enh (keith) Small models in groups may use render buffers that are too small and cause pixelation.  Fixes #2866
   -- enh (scott) Change export of models/controller connections to xls file instead of csv
   -- enh (scott) Copy Smart Remote settings on Replace Model with model
   -- enh (sslupsky) controller: add support for default gamma
   -- enh (keith) Make model dimensions editable in real world units
   -- enh (scott) added packageSequence and packageLogFiles automation
   -- enh (scott) Added Remove All Models conformation dialog
   -- enh (cjd) Add ttyACM* devices (fixes #3016)
   -- enh (scott) download package sequence as zip file
   -- bug (keith) Fix FV3 does not reset zigzag when configuring a port
   -- bug (keith) Fix #3040 xlDo does not unescape some characters
   -- bug (keith) Fix #3045 ... add more sequence properties to getOpenSequence call
   -- bug (scott) regenerate timingList after importing timing tracks
2021.40 December 20, 2021
   -- enh (scott) Add alternate node support to horizontal matrix
   -- enh (keith) Add a selected count on FPP connect
   -- enh (keith) Detect when user tries to use AC mode without a suitable timing track
   -- enh (keith) Add ability to stretch custom model data to grid size
   -- enh (keith) Add Ctrl A-x + space to fast set smart remote of selected model in visualiser
   -- enh (keith) Add support for double height of waveform (without increasing display height)
   -- bug (scott) Fix hinks export on OSX
   -- bug (keith) Fix polyphonic transcription does not display notes
   -- bug (keith) Fix race condition sometimes experiences uploading to F48V4
   -- bug (keith) Fix ripple effect xcentre affects the ripple amount
   -- bug (keith) Fix a pixlite serial port upload error where there is a gap in used channels
   -- bug (keith) Fix xSchedule refire frequency fires immediately if started late
   -- bug (keith) Fix tooltip lingers after you start to drag model in visualiser
   -- bug (keith) Dont defaul matrix faces
   -- bug (keith) Fix xSchedule Remote Falcon API calls failing
   -- bug (keith) Fix sequences and lyric tracks download with wrong file extension on OSX
   -- bug (keith) Fix device naming is inconsistent between xScanner scans
   -- bug (keith) Fix an old value curve on Single strand reports upgrade issue
   -- bug (dkulp) Fix doublie clicking on xsq directory access issues
   -- bug (dkulp) Fix canvas layer changes not always detected   
2021.39 December 5, 2021
   -- enh (keith/dkulp) New REST/Scripting API for controlling xLights from scripts (work in progress)
   -- enh (scott) Basic script/automation runner dialog
   -- enh (keith) FPP Connect to remember column sort order
   -- enh (Camden Narzt) Add alternate nodes ability to vertical matrix model/tree
   -- enh (keith) Include sequence name when reporting excessive mmissing models
   -- enh (dkulp) Add "V2 ZSTD/Sparse" to default save type in Preferences panel.  Can be used to create
                  sparse sequences that only contain multiple specific model to act as effect sequences for FPP.
   -- enh (keith) Update to latest libcurl on Windows to fix some FPP Connect crashes
   -- enh (keith) Fix maximum generic serial payload size to 4096 bytes
   -- bug (keith) FV3 copies group count from first virtual string when it shouldnt
   -- bug (dkulp) Uploads to Beagles may not adjust to proper compression level
   -- bug (keith) Fix Fv4 sequence upload at level 9 causes playback issues, reduce compression level
   -- bug (keith) Fix some issues with LOR Optimised output would not send anything for long periods of time
   -- bug (scott) Fix HinksPix SR AC Upload
   -- bug (keith) Fix #id:sc not working for serial controllers
   -- bug (keith) Fix Falcon upload of Start Nulls
   -- bug (keith) After reconfiguring, give Fv4 controllers longer to reboot
   -- bug (dkulp) Fix various effects that are rendering improperly on multi-layer stars
   -- bug (dkulp) Fix layered arches to "Inside" layer is actually the inside layer
   -- bug (dkulp) OSX/Linux Preferences "Other" tab settings getting wiped out when opening preferences again
   -- bug (dkulp) Fix a semi-common crash in AudioManager when closing sequences
   -- bug (dkulp) OSX - if opening the hardware video encoder for exporting preview video fails, try dropping to
                        mpeg4 instead of h264/5
2021.38 November 20, 2021
   -- enh (keith) Add auto layout for J1Sys
   -- enh (keith) Add segment measures to Polyline
   -- enh (keith) Make state on faces and state effect bulk editable
   -- enh (dkulp) Use multi-cores for Lines effect to speed up rendering
   -- enh (dkulp) Add "Blur" to Experimental GPU/Metal Rendering Engine
   -- enh (dkulp) Bunch more optimizations to Butterfly for Experimental GPU/Metal Rendering Engine
   -- bug (dkulp) Fix v2 smart receivers not being selectable for FPP capes based on ttyS serial outputs
   -- bug (dkulp) Fix rare crash-inducing race condition in the JobPool
   -- bug (dkulp) M1Pro/Max fails to decode some mp4 files, detect and fallback to software decoding
   -- bug (dkulp) Fix vertical grid lines on non-Retina displays
   -- bug (dkulp) Revert back to OpenGL on OSX until all views can be converted to Metal to reduce vsync contention
   -- bug (scott) Fix some issues if a controller that a model is assigned to is no longer available preventing it 
                  from being assigned to a new controller
   -- bug (scott) Fix issues with new curl lib and some controllers on latest Linux/OSX
   -- bug (scott) Fix HinksPix port display for multistring models
   -- bug (keith) Fix auto layout models does not correctly handle model after one with start nulls if it has none.
   -- bug (keith) Invert layout when printing
   -- bug (keith) FV3 splits not calculated correctly if dummy pixels required.
   -- bug (keith) Display controller connection properties consistently.
   -- bug (keith) Have save buttons when linked show red if either layout or controllers need saving
2021.37 November 12, 2021
   -- enh (dkulp) Add LED Panel output to Pi based FPP controller types
   -- bug (dkulp) Fix Life effect not rendering past first frame
   -- bug (keith) Insert Prior seems to max out at 50 but the number box allows larger numbers
   -- bug (keith) Fix crash with Pictures Effect Assist panel
   -- bug (keith) Fix media upload repeats to FV4 whwn it shouldnt
2021.36 November 9, 2021
   -- enh (dkulp) Fix crashing on OSX 10.12
   -- enh (dkulp) Warn on OSX 10.12 and 10.13 that support will be dropped soon, update to 10.14 will be necessary soon
   -- enh (dkulp) More work moving to Metal on  macOS
   -- enh (dkulp) Esc to cancel drag/paste in Pictures assist
   -- enh (dkulp) Add Butterfly types 2-5 to "Experimental GPU Rendering" option
   -- enh (scott) Added Flip Horizontal and Flip Vertical to Layout Tab
   -- enh (scott) Various smart receiver type checks
   -- enh (scott) Increased ESPixelStick v4 ESP32 Max Universe size to 12
   -- enh (keith) Add the ability to use a state as a face outline when using node ranges
   -- enh (keith) Add entec controller properties (no upload yet)
   -- bug (keith) Prevent discovery duplicating controller names.
   -- bug (keith) Fix sequence being marked as dirty when it isnt really.
   -- bug (keith) Replace model with this model does not prompt if the model is on No Controller (#2910)
   -- bug (scott) Don't override ESPixelStick controller parameters if not set in xLights
   -- bug (scott) Workaround for ESP32 lockup issues
   -- bug (scott) Default HinksPix 16 Port AC to 0 start pixels
   -- bug (dkulp) Fix dragging of selection in Pictures assist window
   -- bug (dkulp) More of a workaround - CTRL-C/V copy/paste in Pictures assist.  Cmd-C/V  still not working.
   -- bug (Shelby) Upgraded libcurl on Monterey defaults to chunked transfers which breaks uploads to ESPixelStick
2021.35 November 2, 2021
   -- enh (keith) Allow setting a default Virtual Matrix/Video display location used when creating new playlist items
   -- enh (keith) Add magic words to SMS plugin that can trigger actions
   -- enh (keith) Update xSchedule FPP ping response to version 3
   -- enh (keith) Introduce the "No Controller" as the default when adding new models to reduce overlap situations even further
   -- enh (keith) Start introducing pixielink support
   -- enh (keith) Make shadow models/models being shadowed visually distinct in visualiser
   -- enh (scott) Make hinkspix sequence export weave universes like the upload config does
   -- enh (scott) Add support for smart remote types for controllers
   -- enh (scott) Add scale but keep aspect ratio to matrix singing faces
   -- enh (scott) Allow sequence import to load *.piz files without renaming them
   -- enh (dkulp) Begin introducing use of the Metal graphics API from some screen drawing and effect rendering
   -- bug (keith) Prevent blank web page being set for xSchedule which looks like web interface is broken
   -- bug (keith) Adjust Twilio plugin to work with updated API
   -- bug (keith) Fix error display when F48 is configured with 512+512+1 pixel
   -- bug (keith) Remove inner % property when star is 1 layer
   -- bug (keith) Fix layout wiring view does not correctly show missing nodes in custom models
   -- bug (keith) Fix faces effect wont render if nodes have been renamed after face effect was created with default names
   -- bug (scott) Fix crash due to _MACOSX folder in zip files
2021.34 October 23, 2021
   -- enh (billc) Add LED panel support for rpi-28D
   -- enh (keith) Add ability to set a default virtual matrix and video location
   -- enh (keith) Add to Alphapix and Pixlite/Pixcon property which forces DMX outputs to start at the beginning of a universe
   -- enh (keith) Make inactive controllers ping results look different
   -- enh (keith) Change some default settings for new users to be better options
   -- enh (keith) Add an output processor which can exclude channels from dimming and gamma
   -- enh (keith) Add to visualiser errors if smart remote and non-smart remote models are mixed on a group of 4 ports
   -- enh (keith) Add tooltips to string 2+ on multistring models
   -- bug (keith) Fix Hinks 4 port LR port 4, 8 etc show incorrect value
   -- bug (keith) When testing with suppress unused outputs xlights normal output breaks
   -- bug (keith) Fix calculation of first serial port channel when first model is not channel 1
   -- bug (scott) Only show FPP multisync flag on players
   -- bug (dkulp) Fix calculation of text length for non-OS fonts
2021.33 October 16, 2021
   -- enh (keith) Add in validations for null pixels and group maximums for controllers
   -- enh (keith) Add in a bunch of validations on the start channel dialog
   -- enh (keith) Add ability to save/load dmx effect mappings
   -- enh (keith) Double click on model removes it from the port.
   -- enh (keith) When double clicking to add model to port scroll the controller so it is visible
   -- enh (scott) Add detailed messages to multi controller upload
   -- enh (scott) Enhance visualiser export with start channel and pixel count properties
   -- bug (keith) Put in some protections for stupid long sequences in sequence settings. Also defer actually setting the sequence length until it is actually required.
   -- bug (keith) Fix batch render shows sequences in backup folders even when told not to
   -- bug (keith) Fix some artificial WS2811 protocol chips not showing in protocol list
   -- bug (keith) Fix fire more than once sequences repeat fire if less than 1 minute long
   -- bug (keith) Fix stars converting from old start location properties can end up corrupting rgbeffects file
   -- bug (keith) Force Falcon V3 smart remote uploads to be one pixel for unused ports
   -- bug (keith) Fix hide models on other controllers on visualiser does not always make sense
   -- bug (keith) Fix windows stack walks are broken
   -- bug (keith) Add filename checks to many effect properties to prevent filenames that wont save correctly
   -- bug (keith) Add exception handler to prevent .net plugins crashing xSchedule
   -- bug (keith) Fix hangs when zooming in on layout with grid visible
   -- bug (keith) Fix some rendered face issues. This likely breaks anyone still using xLights 3 face effect definitions ... but it is time to get with the times guys.
   -- bug (dkulp) Fix occasional corruption of FPP serial outputs
   -- bug (dkulp) Fix issues with single channel custom models in groups not rendering simple things like "On" effects properly if Horiz/Vert per model selected.
   -- bug (scott) Fix empty import view crash
   -- bug (scott) Fix check sequence crash if no controller port assigned on a model
2021.32 October 10, 2021
   -- enh (keith) Add an optional alignment grid to the 2D layout
   -- enh (keith) Increase 3D clipping depth on layout for larger displays
   -- ehh (keith) Increase tooltip duration and add a bunch of new tips
   -- enh (keith) Validate controller IP on controller tab
   -- bug (keith) Fix effect panels enabling/disabling broken after render all
   -- bug (keith) Fix crash rendering text effect for non-true type fonts
   -- bug (keith) Fix VU meter timing event random bar not lighting rightmost bar
   -- bug (keith) Fix import of timing track auto selected if existing instance of that timing track has no marks
   -- bug (keith) Fix DMX effect does not save correctly as a state
   -- bug (keith) Fix double click on effect does not work if scrolled horizontally
   -- bug (scott) Add back easylights 16 code until firmware fixes are released
2021.31 October 7, 2021
   -- enh (david) Linux set TCP_NODELAY for OpenPixelControl TCP Output
   -- enh (jason) Add virtual matrix to pi hat capabilities
   -- enh (scott) Added purple highlight to State Dialog, Selecting Grid Header will populate all nodes with force color if enabled or white
   -- enh (scott) Add ArtNet upload to ESPPixelStick V4
   -- enh (dkulp) Add multi-colour emojis to windows
   -- enh (keith) Report duplicate nodes in submodels in check sequence
   -- enh (keith) Add cyan first pixel to 2D as well - disable it for single node models
   -- enh (keith) Allow selection of gif files in movie effect
   -- enh (keith) More xScanner enhancements. Recognise more controllers. Faster execution.
   -- enh (keith) Warn every time xLights starts if the current show folder is in a backup folder
   -- enh (keith) In check sequence add validations to string start nodes highlighting common errors
   -- bug (keith) Fix double click on effects at maximum zoom can choose the wrong effect 
   -- bug (keith) For single colour custom models where all nodes only occupy one cell go back to old buffer style
   -- bug (keith) Fix select effects wont select effects that have no colour selected
   -- bug (keith) Fix universe per string not updating if the number of channels on controller does not change
   -- bug (keith) Fix XL font vertical and rotated 90 degree movement
   -- bug (keith) Fix a crash when loading FSEQ files
   -- bug (keith) Fix timing panel reverse checkboxes not being reset resulting in weird UI behaviour
   -- bug (keith) Prevent Pixlite upload sending a group count of 0
   -- bug (keith) Fix when channel block is first model on a Falcon V4 port an incorrect number of channels are configured
   -- bug (keith) Allow falcon to upload even when warnings are reported
   -- bug (scott) Fix layered arch start location
2021.30 September 28, 2021
   -- enh (dkulp) Add Mr/Mrs Claus, Christmas Tree, Gift emoji to popup list
   -- enh (dkulp) Add skin tone for Emoji's (ShapeEffect)
   -- enh (keith) Restructure xScanner to run without locking up the UI during the scan so you can access information while the scan runs
   -- enh (keith) When uploading to a controller with a FPP proxy check the FPP has that IP address proxied. Warn if it doesnt.
   -- enh (keith) In layout show the first pixel of every model in cyan when in 3D.
   -- enh (keith) Show groups and submodels in different colours in the model lists on the model groups panel
   -- enh (scott) Increase WLED maximum port number
   -- enh (scott) Add multiselect to view import
   -- bug (dkulp) Use the native emoji fonts on Linux/OSX and don't try to use the Windows specific font
   -- bug (keith) Fix grouped pixels uploading to Falcon V4
   -- bug (keith) When discovering E131 controllers dont set universe sizes to > 512 ... but add multiple universes
   -- bug (keith) Centre the close button on the keybindings dialog
   -- bug (keith) Fix hinkspix port number display in visualiser for ports > 4
   -- bug (keith) Force valid start channels when not using full xLights control on the Falcon V4
   -- bug (keith) Force gamma to valid value when uploading to Falcon V4
   -- bug (keith) Validate WAV files uploaded to Falcon V4
   -- bug (keith) Dont switch Falcon V4 from Player modes when uploading
   -- bug (scott) Fix duplicate models added to group on import/download
   -- bug (scott) Fix ESPixelStick v3 start channel upload
2021.29 September 16, 2021
   -- enh (dkulp) Update Twinkle effect to use different placement alogirithm if buffer is very sparse
   -- enh (dkulp) Keep Star screen coordinates as "floats" to provide smoother sizing/better display
   -- enh (dkulp) Add Inner Layer % field to star model to allow the inner models to be larger/smaller
   -- enh (dkulp) Add "Shaders on Background Thread" setting (Other panel in preferences) on Windows to enable running shaders 
                  on the render thread.  Defaults to off.
   -- enh (scott) Check firmware on the hinkspix is high enough to work with upload 
   -- enh (scott) Added Opening and Exporting xmaphint files
   -- enh (scott) Add export of submodels from one model to other models
   -- enh (keith) Add hinkpix LR port number hints to visualiser tool tips
   -- enh (keith) Default new controllers to auto size, auto layout and full control as this is more common than manual setup
   -- enh (keith) Add some validations on FPP proxy usage to catch some errors
   -- bug (dkulp) Twinkle effect text should be "Percent of lights" as its a percent, not hard number of lights.
   -- bug (dkulp) email entry field not being saved on OSX unless some other setting also is changed
   -- bug (dkulp) UNC style paths for Media/Resource directories had the first \ stripped off (and thus didn't work)
   -- bug (kevin) Hopefully reduce some crashing on Windows associated with shader effect
   -- bug (keith) Fix arch model import throws error and places arch in the wrong place
   -- bug (keith) Fix several upload configuration issues for the FxxV4 - requires firmware build 5 or higher
   -- bug (keith) Add drop hint validator to icicles and polyline model
   -- bug (keith) Update star render buffer for single node stars so it better reflects the actual number of controllable nodes
   -- bug (scott) Fix FSEQ upload to ESPixelStick
   -- bug (scott) Fix pressing delete after selecting a group deletes the models in the group
2021.28 September 7, 2021
   -- enh (keith) Add a check sequence warning about data layers which are uncommonly used
   -- enh (keith) Have face effect use the first assigned face on a model if the effect is calling for a face that is not on the model
   -- enh (scott) Highlight nodes used in other rows of submodel
   -- enh (scott) Move HinksPix upload to the upload API
   -- enh (scott) Added Layout Bulk Edit Options to set just protocol and Set Controller Port Number and Increment port number with each model.
   -- enh (dkulp) Allow FPP serial devices to be configured using USB controller
   -- bug (scott) Fix LOR S5 Start Layers
   -- bug (keith) Leave Pixlite in advanced mode after upload
   -- bug (keith) Catch attempts to upload to the wrong version of Falcon controller
   -- bug (keith) Don't switch falcon away from player modes when uploading
   -- bug (keith) Fix several issues using FPP connect to upload to Falcon V4
   -- bug (keith) Fix some Falcon V4 upload issues affecting long strings and serial output
   -- bug (keith) Fix falcon does not measure banks correctly if the number of ports is not divisible by 16
   -- bug (keith) Fix face effect image display not respecting image transparency
   -- bug (keith) Fix when switching between artnet and E131 don't allow universe to be left as 0
   -- bug (keith) Fix purged backups not consistently deleting files
   -- bug (keith) When converting from DDP to E131 with 1 channel we end up with universe size 1
2021.27 August 28, 2021
   -- enh (scott) Add new hinkspix long range support for visualiser and upload
   -- enh (keith) Strip out further code undermining users free rights to choose their models on windows
   -- enh (keith) Remove all model group missing models warnings as there is nothing a user can really do about it and several legitimate
                  ways to trigger it.
   -- enh (keith) Suppress model faces lead in/out was not working unless you chose to fade
   -- enh (keith) Show in model properties strands/string as an error if it does not divide evenly into nodes per string
   -- enh (keith) Don't let the fix files routine look into the backup folder.
   -- enh (keith) Don't have the backup process follow symbolic links
   -- bug (keith) Fix cant export presets with & in the name
   -- bug (keith) Fix channel block and single channel models not represented in visualiser correctly on pixel ports
   -- bug (keith) Fix multichannel 'single channel' custom models not rendering correctly (mostly missing face effects on channels)
   -- bug (keith) Fix music effect separate not working   
   -- bug (keith) Fix Falcon V4 upload fails for brightness settings the controller does not support
2021.26 August 25, 2021
   -- enh (dkulp) Add support for "Virtual Matrix" and "LED Panel Matrix" protocols for Models
   -- enh (dkulp) Add Virtual Matrix and LED Panel Matrix ports to Visualizer for controllers that support them
   -- enh (dkulp) Mark the various Pi based hats as being able to support Virtual Matrices
   -- enh (dkulp) Add a "Pi4 Virtual Matrix" FPP controller type that enables the second HDMI output port
   -- enh (dkulp) Update FPP output upload to fully configure virtual matrices
   -- enh (dkulp) Add "Full xLights Control" ability to FPP based controllers
   -- enh (dkulp) Support multiple virtual matrices per FPP output port (colorlight/linsn sender cards)
   -- enh (keith) Add check sequence warning for large custom models
   -- enh (keith) A better way to deal with deciding to map effects due to large numbers of missing models
   -- enh (keith) Add to check sequence a check for model groups where more than one of the contained models contains the same node
   -- enh (keith) Add open proxy button to controller tab
   -- enh (keith) When replacing a model with a model merge the group membership of both models
   -- enh (keith) Import Lyrix audactity like timing files ... xLights will autodetect the lack of tab delimiters and instead use space as a delimiter
   -- enh (keith) Add lead-in/out to the faces effect when using suppress when not singing.
   -- enh (keith) Add base support for auto map hints - there is no UI support for this in this release
   -- enh (keith) When dragging submodel or node effects in the mapping dialog if you hold down the shift key and drop the model onto
                  a submodel or node with the same node then map all matching submodels/nodes at once
   -- enh (keith) Add option to purge download cache on xLights startup
   -- enh (scott) Add start/end nulls to ESPixelStick
   -- bug (dkulp) If FPP hat/cape supports both pixels and panels, upload configs for both (actually all 3 with VirtualMatrix)
   -- bug (keith) Fix a falcon with no models but full xLights control does not actually clear all the outputs
   -- bug (keith) Fix crash in OSC where no path is specified
   -- bug (keith) Fix crash on startup when using a generic serial output
   -- bug (keith) Fix inactivating/activating a model does not add/remove <> around model name when applied
   -- bug (keith) Fix single channel custom model does not render twinkle correctly because the model buffer is not 1x1
   -- bug (scott) Fix DDP configuring of ESPixelStick V4 
2021.25 August 13, 2021
   -- enh (dkulp) If the model being imported can be mapped to a vendor supplied xmodel, prompt to use the vendor supplied xmodel.
   -- enh (keith) Reduce calls to lookup supplier from MAC address by using a local lookup file
   -- enh (keith) Redo the circle model default render buffer to better handle layers with different numbers of nodes
   -- enh (keith) Add option to emit MIDI and ArtNET timecode where hour = the step playing. This also works for SMPTE receive
   -- enh (keith) Show the current position (time) when scrubbing
   -- enh (scott) Add option to turn off group model missing warning
   -- bug (gil)   Fix Twinkle new render method bug that affected all Twinkles but the first one
   -- bug (keith) Fix VU Meter volume bars does not fill buffer horizontally
   -- bug (keith) Fix SMTPE timecode support hidden
   -- bug (keith) Fix breakdown of phrases/words does not undo correctly
   -- bug (keith) Fix single strand effect rainbow mode does not work if the first paletter colour is white or black
   -- bug (dkulp) Fix divide by zero crash in Single Strand effect
2021.24 August 3, 2021
   -- enh (scott) Added experimental Light-o-rama S5 Preview/Model Import
   -- enh (scott) Allow the multiple selection of groups with right click "Add Model to Group" option
   -- enh (scott) Add MAC address details to xScanner on Linux
   -- enh (keith) Add ability to move centre of the sub-buffer
   -- enh (keith) Add active and current view filters to model group panel
   -- enh (keith) Allow bulk conversion of effects on a model group to equivalent per model render style
   -- enh (keith) Default optional linked buttons to linked state
   -- bug (dkulp) Fix OpenGL check sequence reporting version issue incorrectly
   -- bug (keith) Fix Hanson controller upload fails
   -- bug (keith) Fix %lu in error message when opening a sequence with lots of missing models
   -- bug (keith) Fix show/hide all timing tracks does not always get them all
   -- bug (scott) Fix Linux build issues including adding xScanner
2021.23 July 24, 2021
   -- enh (scott) Enable Inputs on FPP 5.0 when starting output to lights
   -- enh (scott) Added multisync text to mode column in FPP Connect
   -- enh (keith) Optionally include model description in visualiser CSV export for those that have hookup instructions
                  in the model description field
   -- enh (keith) Prompt for confirmation before deleting controllers
   -- enh (keith) All column reordering of the model list on layout tab and persist it
   -- enh (keith) Detect when user is opening sequence with many mismatched models and give them a quick way to exit and 
                  try importing instead.
   -- enh (keith) Add quick show/hide timing tracks in master view (this will not work in other views)
   -- enh (keith) Show model amps in visualiser
   -- enh (keith) If using e131/artnet always show model universe/start channel in model tool tip
   -- bug (scott) Skip duplicate timing marks in vamp imports
   -- bug (keith) Fix custom models with > 65535 nodes don't work
   -- bug (keith) Fix Cube model Horizontal and Vertical Per Strand model group buffer broken.
   -- bug (keith) Fix visualiser move model between ports not working
   -- bug (keith) Fix the fix for OSX link button preference saving
   -- bug (keith) Fix polyline 2 point model shows in model preview as diagonal line unless perfectly level/vertical
   -- bug (dklup) Fix render progress bar not appearing
   -- bug (dkulp) Retain the output interface that FPP is using if the user has set it
2021.22 July 18, 2021
   -- enh (keith) Add -cs command line switch to run xlights, open sequence, run check sequence, close sequence
   -- enh (keith) Sort the saved subbuffers in dropdown
   -- enh (keith) Add xSchedule ability to trigger FPP v5 command slots
   -- enh (keith) Add the ability to save a default camera view for the layout panel which is used when xLights opens
   -- enh (dklup) Add xCapture to OSX xlights
   -- enh (scott) Added "Default New Sequence View" to Preferences
   -- bug (keith) Fix visualiser does not detect port too many pixels if all ports are used
   -- bug (keith) Fix zoom on node grids which can get confused
   -- bug (keith) Fix linked button preferences not saving on OSX
   -- bug (keith) Fix invalid characters breaks timing track display
   -- bug (keith) Reset windows to defaults does not reset test panel and visualiser
   -- bug (keith) Model/Row delete effects should not delete locked effects
2021.21 July 11, 2021
   -- enh (keith) Log obj file errors when loading
   -- enh (keith) Collapse model layer sizes by default in model properties
   -- enh (scott) Add buffer styles to submodels
   -- bug (scott) Include superstring properties in model exports
   -- bug (scott) Fix some issues with dmx model export
   -- bug (scott) Fix models being removed from other views when using make master
   -- bug (keith) Fix multi artnet discovery packet response handling
2021.20 June 28, 2021
   -- enh (billc) Added/change a few tool tips to help new users 
   -- enh (scott) Added the Ability to Import Views from RGBEffects File and Sequence Files
   -- enh (scott) Increase range of shockwave and single strand effect
   -- enh (dkulp) Allow DDP discovery to also work using the STATUS packet
   -- enh (keith) On display elements add button to move models to the bottom of the list
   -- bug (keith) Fix twinkle effect (new style) on really large render buffers
   -- bug (keith) Fix test dialog right click options dont work correctly
   -- bug (dkulp) Prevent crash using artnet with hostnames
   -- bug (dkulp) Ensure render progress bar goes away when done
   -- bug (dkulp) Fix FPP Virtual matrix not setting start channel
   -- bug (scott) Fix rgb effects file not marked dirty when importing models and views
2021.19 June 16, 2021
   -- enh (scott) Added Color Filter To Effect Select Panel
   -- enh (dkulp) Add end nulls to FPP boards
   -- enh (keith) Add option to sort views in the same order as the master view
   -- enh (keith) Add sequence/audio upload to Falcon V4 from FPP Connect
   -- enh (keith) Provide visual clues that audio device cannot be found as this prevents playback
   -- bug (scott) Fix Built-in Model to Custom Model Export to minimise missing nodes
   -- bug (dkulp) Fix custom model background image is black when adjusting brightness
   -- bug (keith) Further fix to s5 import for channel data
   -- bug (keith) Fix Pixlite sometimes wont upload configuration
   -- bug (keith) Fix drag model off port on visualiser does not update screen
   -- bug (keith) Fix node single colour does not work with custom models
   -- bug (keith) Fix some groups arches single strand effects not rendering correctly
2021.18 June 2, 2021
   -- enh (scott) Added F13-F24 Keybindings Support
   -- enh (scott) Add null pixel support to WLED
   -- enh (scott) Add Shift selected nodes to custom model editor
   -- enh (keith) Upgrade vamp links to v1.8.0 on windows
   -- enh (keith) Allow control over start point for layered arches to be inside or outside
   -- enh (keith) In sequences/lyrics download sort the items
   -- enh (keith) Add keybindings for prior and next audio tag
   -- enh (keith) Add keybindings for audio playback speed
   -- enh (keith) Remove default timing track when importing a sequence when only default exists, no timing marks exist and the import includes a timing track
   -- enh (keith) When adding vamp timing tracks default them to unique names
   -- enh (keith) When dragging effect onto grid highlight the model name we are on ... makes it easier to see what row you are on on super wide screens
   -- enh (keith) Add Falcon V4 support
   -- enh (keith) Add ability to support more than 3 smart remotes on a controller - this is enabled for Hinkspix and Falcon V4 at this time
				  *** If you use *A*->*B*->*C* or a->*B*->*C* smart remote styles then you will not be able to regress from this version unless you re-set up your smart remotes
   -- enh (keith) Rework the way we handle pixel protocols in the controller connection to try to be a lot smarter about compatible protocols
   -- bug (keith) Fix shader float and point2d parameters not working
   -- bug (keith) Fix shader variables can clash with other xlights settings
   -- bug (scott) Fix visualiser crash if pixel port is over the max pixel port count
2021.17 May 20, 2021
   -- bug (dkulp) Fix even more crashes in AudioManager (I hope)
   -- bug (dkulp) Fix Vendor Model images if the URL's the vendor provide are invalid/404
   -- bug (scott) Fix FPP upload of e1.31 inputs that are set for xLights only
   -- enh (scott) Add AppProgess Indicator for rendering
2021.16 May 17, 2021
   -- bug (dkulp) FPP Connect - add UDP output ranges to sparse fseq files
   -- bug (dkulp) Fix display issue in SubModels dialog where the list of ranges would not refresh properly
   -- bug (dkulp) Fix more crashes in AudioManager
   -- enh (scott) Added WLED Upload Support, requires WLED v0.12.1 Build 2105110 or newer
   -- enh (scott) Added the Ability To Join Submodels
2021.15 May 3, 2021
   -- enh (scott) Add resize to Media Import Options
   -- enh (scott) Add Pixie2 to LOR optimized output
   -- bug (keith) Fix some value curve dialogs not displaying on shader parameters
   -- bug (dkulp) Fix some crashes in AudioManager
   -- enh (keith) Add to visualiser concept of a selected model and then when you double click on a model on the models pane it chains to the end of the selected model
   -- enh (keith) Add "Multiply" blending mode
   -- enh (Old Salt) Added shader 'RGB to Greyscale_xL' to prepare layer for new 'Brightness' blending
2021.14 April 28 2021
   -- bug (keith) Fix new audio sequence hangs after you select the audio file in some circumstances
   -- bug (keith) Shader choice options containing commas don't display correctly
2021.13 April 27 2021
   -- enh (keith) Improve the visibility and playback of filtered waveforms
   -- enh (several) Bunch of new shaders added for download
   -- enh (dkulp) Move to wxWidgets 3.1.5 (plus fixes) on all platforms
   -- enh (scott) Add Pixie2 controller
   -- enh (keith) Add new F16V2R variants
   -- enh (keith) Add line numbers to shaders that are logged on compile failure
   -- enh (kevin) Allow audio shaders to respond to either VU data or intensity
   -- enh (scott) Load RGBEffects.xml File on XSQ Import for Group Names
   -- change (dkulp) Deprecate and start removing support for mingw/gcc builds on Windows
   -- bug (scott) Fix some issues with ctrl key shortcuts on windows
   -- bug (dkulp) Fix random crashes when add/remove headphones
   -- bug (keith) Fix broken pixlite upload
   -- bug (scott) Fix node numbers off by one on Cube Custom Model Export
   -- bug (keith) Fix problems starting xLights when double clicking on an xsq file.  Fixes #2611
2021.12 April 12 2021
   -- enh (dkulp) OSX - Key Binding editor dialog should use the OSX key names and icons
   -- enh (Old-Salt) Add bunch more shaders, restore some missing shader images
   -- enh (keith) Add the IMG_SIZE shader function
   -- bug (scott) Fix superstring model multistring channel count
   -- bug (scott) Fix key bindings that use CTRL on Windows
   -- bug (dkulp) Fix several instances of crashes when manipulating models on Layout panel while rendering (aborts rendering)
   -- bug (dkulp) Fix crash in path editor/Value Curves.  Occurs most often with lots of points. Fixes #2587
   -- bug (keith) Fix VUMeter does not work correctly with 40fps sequences
2021.11 April 3 2021
   -- enh (dkulp) Move to latest wxWidgets patches for Linux
   -- enh (dkulp) Allow using the Mac "raw control" key for key bindings. Fixes #2576
   -- enh (scott) Sort Buffer Presets
   -- bug (dpriem) fix memory/loop issue in effect preview gen
   -- bug (dkulp) sysctl is only needed on OSX.  Fixes #2563
   -- bug (dkulp) Fix default values for shaders with integer drop down selections.  Fixes #2569
   -- bug (dkulp) Reset shader value curves on effect selection.  Fixes #2575
   -- bug (dkulp) Don't use alpha channel for brightness, is VERY slow on Windows if double buffering used. Fixes #2573
   -- bug (scott) fix Morph swap button bulk edit
2021.10 March 31 2021
   -- enh (dpriem) Add Dark Mode support for Visualiser
   -- enh (scott)  Added the ability to Export Layout/Model/Wiring as a DXF File and Export Models as a STL/VRML File.
   -- bug (dkulp)  Fix decoding videos that don't have video frames at the start
   -- bug (dpriem) Fix drag/drop in import dialog
2021.09 March 23 2021
   -- enh          Add some more xLights optimized shaders to shaders.xml
   -- enh (dkulp)  Adjust some colors in the download dialogs for OSX Dark Mode.
   -- enh (dpriem) Allow mapping available to multiple selected models/submodels/groups in mapping dialog.
   -- enh (dkulp)  Change font color for timing tracks if not enough contrast with background.  Easier to read defaults in dark mode.
   -- bug (dkulp)  Fix rendering of first effect dropped on model group not actually rendering
   -- bug (scott)  Fix Multi word support in User Lyric Dialog
2021.08 March 14 2021
   -- enh (dpriem) Bunches of updates/enhancements for Import dialog
   -- enh (dpriem) Bunches of updates/enhancements for Effect Presets dialog
   -- enh (dkulp)  Point to new build of QM plugins on M1 macs
   -- bug (scott)  Fix custom model crash with super-strings
   -- bug (dkulp)  Fix crash if ico files are used instead of animated gifs
   -- bug (dkulp)  Fix corner icons for Morph effect assist panel
2021.07 March 2 2021
   -- bug (dkulp)  Fix potential hang with "Per Model" render styles
   -- bug (scott)  Fix crash in swap transition
2021.06 March 1 2021
   -- enh (kevin)  Add Circles and Shatter transitions
   -- enh (scott)  Add multi selection to submodel import in Faces dialog
   -- enh (scott)  Add BBB16 controller configuration
   -- enh (dkulp)  Add KiNet uploads to FPP output universes
   -- enh (kevin)  Allow reverse on star transition
   -- enh (kevin)  Add swap, shatter, and circles transitions
   -- bug (scott)  Prevent groups with blank name
   -- bug (scott)  Fix AC toolbar
   -- bug (scott)  Fix sequence import with spaces at the beginning of folder names
   -- bug (dkulp)  Fix Shader effect panel sliders/controls not triggering render
   -- bug (kevin)  Fix pinwheel and star transitions being inactive for some of the duration
   -- bug (dkulp)  Fix issues with effects that need to render on the main thread being rendered WAY too often if "Per Model" used.
2021.05 February 19 2021
   -- enh (dkulp)  Reduce timer usage which are preventing the CPU from sleeping (more work to go)
   -- enh (dpriem) Allow direct import from packaged sequence zip files
   -- bug (dkulp)  ESPixelStick websocket connection failes to handle fragemented messages
   -- bug (gil)    Fix Twinkle New Render Method not working with 1 pixel buffers
   -- bug (scott)  Fix brightness on ESPixelStick
   -- bug (scott)  Fix single icicle does not appear on screen
2021.04 February 8 2021
   -- enh (scott)  Look one level deep for missing files
   -- enh (scott)  When exporting models let user decide groups to include
   -- bug (dkulp)  Fix warp effect crash 
   -- bug (dkulp)  Fix issues with model handle sizes
   -- bug (dkulp)  Fix dirty ranges not detected correctly on nodes, strands and submodels
   -- bug (dpriem) Address preset preview size issues
2021.03 January 30 2021
   -- enh (dkulp)  Add menu item to open 2nd instance of xLights on OSX
   -- enh (keith)  Update the CMU dictionary
   -- enh (keith)  Add custom pre & postfixes to generic serial
   -- enh (keith)  Add a random option to the VU meter bars
   -- enh (keith)  Check when loading a sequence for any missing files and let the user know
   -- enh (keith)  Add the ability to create a state definition from a DMX effect
   -- bug (gil)    Fix Twinkle for Old Render Style broken with performance update
   -- bug (dkulp)  Fix border around toolbar buttons
   -- bug (dkulp)  Fix re-prompt for file permissions if show folder cannot be accessed
   -- bug (keith)  Fix backup folder not respected when backing up on start
   -- bug (keith)  Fix a number of issues with S5 spinner effect import
   -- bug (keith)  Fix import of S5 channel effects onto channel block model dont always work
   -- bug (keith)  Try to address some situations where timing track names could become invalid
   -- bug (keith)  Fix a crash when converting pre-visualiser networks file
   -- bug (keith)  Fix inconsistencies in the drawing of locked and disabled effects
   -- bug (keith)  Stop windows install updating path as this really should not be necessary and can break some software
   -- bug (keith)  Address a number of UI issues in the presets dialog
   -- bug (keith)  Fix crash removing all models from a controller in the visualiser
2021.02 January 21 2021
   -- enh (scott)  Added preset preview
   -- enh (scott)  Consolidate some submodel export buttons to a dropdown
   -- enh (keith)  Stop xScanner scanning public networks
   -- enh (keith)  Make visualiser work better with USB controllers
   -- enh (keith)  When custom model generator misses a node between 2 found nodes then optionally guess its location
   -- enh (keith)  Add ability to export a submodel as an xmodel - this can allow you to change a models node numbering
                   and then move across the submodel definitions that usually just break.
   -- bug (gil)    Fix issue with Display Elements sorting/drag/drop when there are multiple timing tracks
   -- bug (dkulp)  Fix ESPPixel sticks incorrectly sending grouping as 0
   -- bug (dkulp)  Change string handling where unicode might be important
   -- bug (dkulp)  Address some twinkle performance issues
   -- bug (dpriem) Ensure model measurement is shown by updating when zooming
   -- bug (keith)  Fix nodes for multi-layer arches don't update when layer nodes updated
   -- bug (scott)  Fix light count on arches and candycanes in model export
2021.01 January 14 2021
   -- enh (gil)    Improve handle selection when handles overlap to help with resizing tiny models
   -- enh (gil)    Allow sorting Timing tracks for Master View in Display Elements dialog.
   -- enh (gil)    Twinkle effect: add value curves
   -- enh (scott)  Change default model blending to drop down to make it less cryptic
   -- enh (keith)  Add -s command line option to run it single threaded (slow but errors can more easily be debugged)
   -- enh (keith)  Upgrade windows wxWidgets to 3.1.4
   -- enh (keith)  When input and outputs upload are linked disable the inputs as only one is needed
   -- enh (keith)  Add code to allow generation of a GIF for a preset (this is not user accessible at this time)
   -- bug (gil)    Prevent Node Select Grid zoom from being able to be reduced to 0 and recover if it has
   -- bug (gil)    Fix some drag/drop sorting issues in Display Elements dialog. (Hopefully didn't create new ones)
   -- bug (gil)    Fix new Bounce to Middle Singlestrand option when Cycles is not equal to 1.
   -- bug (keith)  Fix xFade does not allow 0 artnet universe
   -- bug (keith)  Fix decline on email does not save decision
   -- bug (dpriem) Fix models duplicated in group when importing from rgbeffects
   -- bug (dpriem) Fix replace model call in model list right click menu was not doing anything
2020.57 December 29 2020
   -- enh (gil)    Singlestrand Effect: Dual Bounce renamed to Dual Chase and new styles Bounce to Middle and Bounce from Middle added.
   -- enh (gil)    Twinkle effect has new algorithm that is more random to cover all pixels.  Old algorithm can still be selected.
   -- enh (gil)    Add Terrain Object functionality
   -- enh (keith)  Add FPP wifi strength to xScanner
   -- enh (keith)  Massively increase the xScanner parallelism to improve scan speed
   -- enh (keith)  Increase the maximum number of lines readable from a file by the text effect
   -- enh (keith)  Remove controller tab Delete All button ... not as useful as it used to be and you can always select all in the 
                   list instead and press delete
   -- enh (keith)  Add sort by start channel to display elements sorting menu
   -- enh (scott)  Added Flip/Shift/Reverse Nodes to the SubModel Dialog. Moved Generate and Import to dropdown menus under the new
                   Edit and Import Buttons. Added Shift and Reverse Nodes to Faces and States dialog. Added Copy and Rename to 
				   States dialog.
   -- enh (scott)  Add mode column to FPP connect
   -- bug (gil)    Fixed Mesh Objects drawing color only materials as grey.
   -- bug (gil)    Fixed Galaxy rounded ends not drawn correctly when start angle was non-zero.
   -- bug (keith)  Fix some situations where xSchedule does not automatically locate audio from FSEQ file
   -- bug (keith)  Allow # in state names
   -- bug (keith)  Fix xScanner crashing when encountering colorlite cards
2020.56 December 20 2020
   -- enh (keith)  Add xScanner - this is like an IP Scanner but with xLights/xSchedule/FPP awareness and thus helps to 
                   identify your network setup and spot issues. If you are joining the xLights zoom room with 'network issues'
				   I would recommend you run this first and be ready to show the results
   -- enh (keith)  Add a display elements sort option by controller/port
   -- bug (keith)  On import of models containing groups when you chose to merge into existing groups it didnt really add the 
                   submodels correctly
   -- bug (keith)  Fix F4/F5 pull push of custom model nodes not working
   -- bug (keith)  Fix crash due to long serial port name on windows
2020.55 December 14 2020
   -- enh (keith)  Show the groups a model (or its submodels) belongs to in the model properties
   -- enh (keith)  Show sequence channel count and last render date in playlist list
   -- enh (keith)  Show the state of serial controllers in the ping results list
   -- bug (dkulp)  Fix crash when opening some semi-corrupt sequences on M1 macs
   -- bug (dkulp)  Fix of fseq files to ESPixelStick 4.0 being off by one channel
   -- bug (keith)  Fix test panel does not correctly show channel colours for 4 channel pixels
   -- bug (keith)  Fix submodel filtering does not work correctly with case
   -- bug (keith)  Ensure irrelevant controller connection properties are removed
2020.54 December 8 2020
   -- enh (scott)  Added click-and-drag & double-click select to Model Faces and states Preview
   -- bug (keith)  Fix issues with remote falcon disable from xSchedule
   -- bug (keith)  Prevent outputs from being disabled when the controller is enabled ... this causes hard to detect issues
   -- bug (scott)  Fix HinksPix corruption issue
2020.53 December 6 2020
   -- enh (keith)  Show IP address in tooltip in xSchedule ping results, show web listening port
   -- enh (keith)  Add the model description to the model tooltip in visualiser if present
   -- bug (keith)  Fix opening custom model data looks like it hangs but really just runs really slowly
   -- bug (keith)  When double clicking ping results use the IP to open the controller web page
   -- bug (keith)  Fix model filter does not filter submodels
   -- bug (scott)  Fix model cut
2020.52 December 5 2020
   -- enh (scott)  Added click-and-drag & double-click select to SubModel Preview
   -- enh (scott)  Add sort of nodes to rows in submodel dialog
   -- enh (keith)  Added experimental KiNet support
   -- enh (keith)  Add F4/F5 and F7/F8 to move nodes forward/backward between layers in custom model dialog
   -- enh (keith)  Add option to disable sending of data to controllers that are not responding to ping to reduce ARP failure delays
   -- enh (keith)  Add support for !controller:startChannel format in xSchedule for defining matrix start channels
   -- enh (keith)  Add a filter to the model group panel to quickly find models to add into a group
   -- enh (keith)  On batch render dialog show the number of sequences selected
   -- enh (keith)  Add a right click option on custom model data grid to convert a pixel model to a single channel model
   -- enh (keith)  Increase the x axis granularity for custom value curves
   -- enh (keith)  Add support for SVG colour palettes
   -- enh (keith)  Further improve visulaliser bank error messages
   -- enh (dkulp)  Try to use the best GPU for shaders and support eGPUs on OSX
   -- bug (keith)  Fix backup fails when recovering an rgbeffects xbkp file
   -- bug (keith)  Fix serial port start channel is incorrect in visualiser where first model does not start on channel 1
   -- bug (keith)  Fix custom value curve editor does not allow editing of all points
   -- bug (keith)  Fix an LMS import crash
   -- bug (keith)  Fix opening of SCSS colour palettes
   -- bug (keith)  Fix freeze frame is not large enough and can limit length of effects
   -- bug (dkulp)  Fix some rendering crashes if you manipulate models
   -- bug (dkulp)  Fix FPP discovery fails on all but the first attempt
   -- bug (dkulp)  Fix model in group can move relative to the rest of the group in the model preview due to double scaling
   -- bug (pete)   Fix random number generator is not as random as it should be
2020.51 November 30 2020
   -- enh (scott)  Add white on all to RGBW handling
   -- bug (keith)  Fix custom model with indiv start nodes being incorrectly cleared
2020.50 November 29 2020
   -- enh (scott)  Added Export Controller Connections to Tool Menu. This exports a CSV file with all
					the ethernet type controllers and the models attached to each port.
   -- enh (scott)  Added "Scale Keep Aspect Ratio Crop" to the Picture Effect.
   -- enh (scott)  Added "Ctrl/Cmd + Scroll Wheel" Page Scrolling to Custom Model Dialog .
   -- enh (keith)  Add bulk move and bulk remove to visualiser right click menus
   -- enh (keith)  Add node find right click menu options to custom model dialog and submodel node selection dialog
   -- enh (keith)  Add option to set remote falcon into enable mode when not playing supported playlists
   -- bug (scott)  Fix Check Sequence Connection Error for Auto Configure Panel Controllers.
   -- bug (keith)  When a controller is in auto layout mode then recalc before saving controller tab and when opening visualiser
   -- bug (keith)  When switching between virtual strings falcon upload should reset overridden properties to the default
   -- bug (keith)  Fix circle model inside start location generates incorrect buffer/wiring
   -- bug (keith)  Fix incorrect controller rebooting message when uploading to old firmware on F48 controllers
   -- bug (keith)  When using auto layout remove individual start channels from models as these cause issues
   -- bug (keith)  Fix single channel window frames display as a single dot
2020.49 November 24 2020
   -- enh (scott)  Added Import Model Blend Mode to xLights Sequence Import
   -- enh (keith)  Allow remote falcon to only work while some playlists are playing
   -- enh (keith)  Add to port tooltip in visualiser an estimate of the current draw on the port
   -- enh (keith)  Add a brightness blend mode to xSchedule blend modes
   -- enh (keith)  Add a way to renumber a DMX channel of a prop in the visualiser and then cascade to the rest of the models
   -- enh (keith)  Add to visualiser colour bars to left of ports showing banks and smart remote groups
   -- enh (keith)  When controller port limits are broken on the falcon show the largest port in each bank to ease troubleshooting
   -- enh (dkulp)  Extend FSEQ file definition to support larger numbers of compression blocks
   -- bug (scott)  Fix Group Count for Falcon Upload
   -- bug (keith)  Allow xlights import to import from sequences that only contain timing tracks
   -- bug (keith)  Fix an issue with music fade value curve which can create insane render times
   -- bug (keith)  Fix unused smart remote throws off falcon upload setting of bank sizes
   -- bug (keith)  Fix cant change height of 3 point models in model properties
   -- bug (keith)  Fix unnecessary reboot of falcon on upload
   -- bug (keith)  Fix double click on model category in model download seems to hang mouse pointer
   -- bug (keith)  Fix window frame model sometimes loses a corner node
   -- bug (keith)  Fix ZCPP does not default to auto layout when discovered
   -- bug (keith)  Fix circle model wired from the inside   
   -- bug (keith)  When start of model is chosen default the channel offset to 1 as this is the most common choice
2020.48 November 17 2020
   -- enh (keith)  Add support for Falcon firmware 2.58
   -- enh (keith)  On visualiser freeze the port names on the left of the panel when scrolling right
   -- enh (keith)  On visualise models panel improve model sorting
   -- enh (keith)  Add a simpler generic DMX model
   -- enh (keith)  Show DMX channel number on main panel in visualiser
   -- bug (keith)  Fix issues with window frame model when top and bottom have different numbers of nodes
   -- bug (keith)  Fix issue removing model from a port in visualiser
   -- bug (keith)  Fix VU Meter Timing event pulse colour clamps max fade frames to model size
   -- bug (keith)  Fix xSchedule web site not defaulting correctly to index.html
   -- bug (scott)  Fix video/picture effect does not store filenames as xml safe
   -- bug (scott)  Fix FPPD neets restart on outputs upload
   -- bug (dkulp)  Limit render buffers to 2048 to prevent run away render times
2020.47 November 8 2020
   -- enh (keith)  Optionally suppress xbkp newer and model missing errors when batch rendering
   -- enh (keith)  Add option to minimise non essential xSchedule UI updates to squeeze out better performance
   -- bug (keith)  Prevent upload of 0 sized group to falcon controller
   -- bug (keith)  Force remove smart remote when dragging a model in visualiser onto a controller that does not support them
   -- bug (keith)  Fix P12S max channels per output
   -- bug (keith)  Fix model group default to vertical stack does not work 
   -- bug (keith)  When controller is set to autosize clear the individual universe sizes
   -- bug (keith)  If nothing is playing in xSchedule send blank to remote falcon
   -- bug (keith)  Fix bug in saving link controller/layout save
   -- bug (scott)  Fix default falcon brightness not working   
   -- bug (dkulp)  Delay check for updates to prevent some crashes on startup
   -- bug (dkulp)  Fix some crashes when sequence data buffers change while rendering   
2020.46 November 1 2020
   -- enh (keith)  When xLights has full control allow definition of the default brightness of unused ports
   -- enh (keith)  Optionally link controller tab and layout tab save buttons so both buttons save both files
   -- enh (keith)  Optionally link both upload inputs and upload outputs buttons
   -- bug (keith)  Fix double click on effect deselects effect
   -- bug (keith)  Fix model property change scrolls back to the top of the property list instead of holding position
   -- bug (keith)  Fix model group lists scroll back to the top instead of holding position
   -- bug (keith)  Fix Hanson boards should be opendmx
   -- bug (keith)  Fix clear model controller properties when copied
   -- bug (keith)  Handle material files where the name is not the same as the obj file correctly
   -- bug (keith)  Fix model groups panels lists don't resize with panel
   -- bug (keith)  Fix FPP connect duplicates sequences for upload in some scenarios
2020.45 October 26 2020
   -- enh (keith)  Add specific variants to falcon controllers where receivers are added to the serial port
   -- enh (pault)  Increase icicles maximum pixels to 2000
   -- bug (keith)  Fix internally wired layer star
   -- bug (keith)  Fix custom model does not work correctly with 4 channel pixels
   -- bug (keith)  Show serial port channels correctly in visualiser when channels are skipped
2020.44 October 21 2020
   -- enh (keith)  Add copy and mirror to the warp effect
   -- enh (keith)  When importing groups from other layouts optionally allow importing of empty groups
   -- enh (scott)  Enhance visualiser print to be multi page
   -- enh (scott)  Save visualiser window properties between openings
   -- enh (scott)  Separate some controller connection properties on the model export into their own columns
   -- enh (scott)  Add control over box size and font size in visualiser
   -- enh (dklup)  Add left/right scroll wheel support to visualiser
   -- enh (dkulp)  Add suppport to upload single channel pixel configs to FPP
   -- bug (scott)  Fix layered star isn't working as it used to
   -- bug (keith)  Fix 5 node star only shows 4 nodes
   -- bug (keith)  Fix timing track with quote does not open correctly
2020.43 October 12 2020
   -- enh (dkulp)  FPP - don't upload UDP Inputs file if there are no UDP inputs defined/needed (DDP)
   -- enh (dkulp)  FPP AutoConfigure on output - restore FPP mode when output disabled
   -- enh (keith)  Add the ability to tag shadow models and suppress overlapping channel warnings
   -- enh (keith)  Add some colour handling to Vixen 3 import
   -- bug (dkulp)  Move all "Turn on outputs" and "Turn off outputs" to common methods on xLightsFrame to keep the button/bitmaps in sync
   -- bug (dkulp)  OSX - when loading meshes, if mtl/texture cannot be accessed, query for access to the directory
   -- bug (dkulp)  Add extra tracing to perspective loading to hopefully help track issues with it
   -- bug (dkulp)  Fix obscure crash when changing away from Sequencer tab might cause recursive loop
   -- bug (dkulp)  Fix crash with CustomModelDialog if background image is not a valid image
   -- bug (dkulp)  OSX - Fix HinksPix export and FPP Connect not being able to write to mounted SD cards/volumes
   -- bug (dkulp)  OSX - Fix ESEQ export not working
   -- bug (keith)  Fix autosize does not work on LED panels and virtual matrix controllers - also stop visualiser
   -- bug (keith)  A bunch of fixes for LOR convert
   -- bug (keith)  Clear output data when clicking off an effect and outputting to lights
   -- bug (keith)  Fix inactive ruler stops it measuring
   -- bug (keith)  Fix star chops off top node
2020.42 October 5 2020
   -- enh (keith)  Add support for shader events tied to a timing track (see Lightning shader as an example)
   -- bug (keith)  Fix star where start is on the inside of the star is not working
                   The problem with this fix is it is NOT automatic. Changes will need to be manually made to the start location drop down
   -- bug (keith)  Star model for some sizes includes the wrong number of nodes in each layer
   -- bug (keith)  Fix file locations not working in xSchedule
   -- bug (keith)  Fix crash in FPP connect when FSEQ file is not valid
2020.41 October 4 2020
   -- enh (keith)  Rewrite star model so multiple layers of the same size actually work
   -- enh (keith)  Add option to start a star model on the left or right leg of the star closest to vertical at the bottom
   -- enh (keith)  On grid object in 3D display grid spacing in real world units if ruler is defined
   -- enh (keith)  Add ability to disable rendering of individual effects
   -- enh (keith)  Add ability to create a timing track from a row of effects
   -- enh (nickk)  Add cut effects to row and model
   -- enh (dkulp)  Have FPP look for media file using FSEQ headers if xsq not found
   -- bug (scott)  Fix HinksPix Controller Upload
   -- bug (scott)  Fix Controller Protocol Dropdown in Model Tab
   -- bug (scott)  Fix On Effect with DMX Models
   -- bug (dkulp)  Fix missing serial protocols prevent drag and drop on visualiser
   -- bug (dkulp)  Fix crash on OSX if 3D object is selected
   -- bug (dkulp)  Fix dragging 3D model it occasionally disappears
   -- bug (dkulp)  Fix RGBCape48 does not allow 48 outputs
   -- bug (dkulp)  Fix crash when dragging from outside xLights onto mapping dialog
   -- bug (dkulp)  Fix some shaders that wont compile
   -- bug (keith)  Display mesh models in grey if materials files are all missing
   -- bug (keith)  Support xtiming files containing lyrics with quotes and other xml illegal characters
   -- bug (keith)  Fix when changing controller protocol ID becomes really large
   -- bug (keith)  Fix cube model strands when left to right is chosen are not the same as when front to back is chosen
   -- bug (keith)  Fix crash with Horizontal and Vertical per strand/node
   -- bug (paular) Add DDP to WLED controllers
   -- bug (nickk)  Fix when no internet access check for updates hangs
2020.40 September 25 2020
   -- enh (keith)  Remember expanded property state on controller tab
   -- enh (dkulp)  Expand controllers (DDP/NULL) to support 2 Million channels
   -- bug (keith)  Fix arch model 3D match height not working
   -- bug (keith)  When changing serial controller type to a protocol with a fixed rate ensure that rate is displayed
   -- bug (keith)  Fix nutcracker123 only accepts https requests which broke model download (and others)
   -- bug (keith)  Fix xlights creates spurious directories in root folder on startup
   -- bug (dkulp)  Fix FPP brightness setting to match what FPP expects
   -- bug (dkulp)  Move more processing to multiprocessors
   -- bug (dkulp)  Fix some FPP discovery bugs which resulted in FPP addresses not showing
   -- bug (dkulp)  Fix some 404 errors in FPP connect that are not really errors
2020.39 September 20 2020
   -- bug (keith)  Fix bug which prevented some face and state effects rendering
   -- bug (keith)  Fix check sequence returns incorrect error total
2020.38 September 19 2020
   -- enh (keith)  Add colour dropper support to chromakey and sparkles
   -- enh (keith)  Add centre controls to ripple
   -- enh (keith)  Add real world dimensions in 3D layout
   -- enh (dkulp)  Bunch of render performance improvements
   -- bug (keith)  Fix keybinding should not change an effect if an effect is selected
   -- bug (keith)  Email addresses are valid if they start with numbers
   -- bug (keith)  xSchedule start process does not work if it contains \t or \n ... fix but users may need to change these to \\t and \\n
   -- bug (keith)  xSchedule e131 event does not listen to multicast universes
   -- bug (keith)  Bulk edit on morph slider does not work
   -- bug (dkulp)  Fix OSX app store build does not get serial port access permissions
   -- bug (dkulp)  Extend FPP timeout issues
2020.37 September 13 2020
   -- enh (keith)  Add a handle status function to guestbuttons to make it easier to customise based on xSchedule status
   -- bug (keith)  Fix occasional crashes when cloning groups and creating them
   -- bug (keith)  Fix error pasting phonemes where selected effect name is pasted
   -- bug (keith)  Optimise fix files to minimise slow file system operations which we have already tried
   -- bug (dkulp)  Fix board IDs for F40D-PB
   -- bug (scott)  Maybe fix vixen imports on OSX
2020.36 September 12 2020
   -- enh (keith)  Add a few timing track value curves
   -- enh (keith)  Add support for public xSchedule web pages while securing core web site
   -- enh (dkulp)  Add detection of ESPixelSticks with local FSEQs so FSEQs can be uploaded to them
   -- enh (dkulp)  Use sequence metadata and media files sizes to determine if they need to be sent to FPP
   -- enh (dkulp)  Add sort to FPP connect
   -- bug (dkulp)  Change when FPPD is rebooted to avoid some race conditions
   -- bug (scott)  Add back singing face support to DMX Skulls
   -- bug (keith)  Fix crash on layout due to no protocol options
   -- bug (keith)  Fix a number of S5 effect imports
   -- bug (keith)  Fix an issue with lyric track paste that prevented them being copied and pasted again
   -- bug (keith)  Fix snowflake effect settings are disabled when they shouldnt be
   -- bug (keith)  Fix some shape effect anomalies
2020.35 September 7 2020
   -- enh (keith)  Add ability to remove word and phoneme layers from timing tracks
   -- enh (scott)  Add PB16v2 controller
   -- bug (keith)  Fix xSchedule player window location when DPI per monitor being used
   -- bug (keith)  Fix test mode seems to hang in large shows - big performance change
   -- bug (keith)  Fix group creation related crash
   -- bug (keith)  Fix crash when using visualiser on controllers with no protocols
   -- bug (keith)  Fix sphere xmodel wont load
   -- bug (dpriem) Fix sort issue on windows with model/previews import
   -- bug (dkulp)  Fix overly aggressive warning on memory usage which was triggering on large shows
2020.34 August 30 2020
   -- enh (scott)  Added Generic Serial Output
   -- enh (scott)  Added export of submodel details
   -- enh (keith)  Enhance automap to try to find more maps
   -- enh (keith)  Add hint that #strings is typically the number of connections from controller to the model
   -- enh (dklup)  Start adding support for new Pixel Stick Firmware
   -- enh (dkulp)  Optimise controller discovery - much much faster
   -- enh (dklup)  Have FPP connect reuse known IP addresses reducing frequencing of missing FPP instances
   -- enh (priemd) Sort previews and models in import dialog
   -- bug (keith)  Clear force local IP if it isn't present - prevents failures when turning on output to lights
   -- bug (keith)  Add some additional effects to S5 import - these wont be great matches but at least they come in
   -- bug (keith)  Fix hang on sequence close/exit
   -- bug (keith)  Fix DDP discovery not returning IP address
   -- bug (keith)  Fix a bunch of extra check sequence checks
						face effects refering to faces not on the model
						video effect taken from song audio reports incorrectly that video file is missing
						multicast universe on multiple controllers incorrectly report as a clash
   -- bug (keith)  Fix some issues with ZCCP packet flags
   -- bug (dkulp)  Fix Smart Ts appear on controller types they shouldnt
2020.33 August 22 2020
   -- enh (keith)  Have check sequence flag dimming curves which will blank out a prop
   -- bug (scott)  Try to improve AC effect toolbar
   -- bug (keith)  Fix gamma value display in visualiser
   -- bug (keith)  Fix crash when rendering models due to missing controller connection node
2020.32 August 21 2020
   -- enh (scott)  Give an option to add imported models to existing groups or continue to create new groups when importing models containing submodel groups 
   -- enh (scott)  Lower arch minimum hollowness to zero
   -- enh (keith)  Add minleon controller upload support
   -- enh (dklup)  Add new Kulp controllers
   -- enh (dkulp)  Auto select first controller variant when available
   -- bug (scott)  Fixed Select Effects Window not finding effects on submodels/stands
   -- bug (scott)  Fix windows download link does not work
   -- bug (scott)  Fix spakles with a value curve at zero still displays sparkles
   -- bug (scott)  Model search should not be case sensitive
   -- bug (scott)  Fix video export required stereo audio
   -- bug (scott)  Fix select effects does not select effects on strands and nodes
   -- bug (priemd) Fix xmodel export includes empty groups
   -- bug (priemd) Fix 3D object model move rotate etc not working
   -- bug (keith)  Fix issue clicking and dragging out line model in 3D
   -- bug (keith)  Fix sunset and sunrise not saving in schedules
   -- bug (keith)  Fix a bunch of issues with transition window controls doing weird things. They now enable and disable more consistently
   -- bug (keith)  Auto handle really screwed up polyline models restoring them back to editable state
   -- bug (keith)  Fix old AC LMS import does not import twinkles as twinkles
   -- bug (keith)  Fix and rework the way visualiser drag and drop of serial output models work
   -- bug (dkulp)  Fix serial port config upload to FPP controllers 
2020.31 August 9 2020
   -- enh (dkulp)  Remove warning for files not in show directory if they are in a resources directory
   -- enh (scott)  Added Search Box to the Vendor Model Download Dialog
   -- enh (scott)  Added FLAC audio support
   -- enh (keith)  Add option to control if remote falcon deletes any pending queued items on startup
   -- enh (keith)  Add link to remote falcon visitor page for your show
   -- enh (keith)  Add option to remote falcon to try to gracefully interrupt playing sequence
   -- enh (keith)  Add ability for timecode to work with non sequence steps
   -- bug (dkulp)  Some dark mode updates for grid controls (custom model dialog, etc..)
   -- bug (dkulp)  Temporary workaround for tab font on Big Sur, other areas (property grids, grid control headers, etc..) will likely need fixes/updates in wxWidgets
   -- bug (dkulp)  Crash/hang on custom model dialog when using mouse to enter nodes (Linux/OSX)
   -- bug (dkulp)  Maximize buttons missing on ModelHouse preview (Linux/OSX)
   -- bug (keith)  Implement fixes to SMPTE and MIDI timecode
2020.30 August 2 2020 - Windows
   -- enh (scott)  Added LOR S5 Channel/Stand Importing
   -- enh (scott)  Added "bank" Controller Channel Checking
   -- bug (keith)  Fix windows version check fails due to website restore
   -- bug (keith)  Effect which should start/finish on a multiple of the frame time don't - leads to issue with vumeter not triggering on timing marks correctlty
   -- bug (keith)  Fix DDP controller ids show as in error is invalid
   -- bug (keith)  Fix some residual issues with protocol case
   -- bug (keith)  Fix remote falcon plugin wont send large schedules correctly
2020.30 July 30 2020 - OSX/Linux
   -- enh (dkulp)  Provide better error messages to user from VideoExporter if it fails to export
   -- enh (dkulp)  OSX - Move to latest SDL, FFMPEG, and libzstd and make sure they are compiled for both x86_64 and arm64
   -- enh (dkulp)  Support multiple directories for resources/media
   -- enh (nick)   Bunch of fixes and updates to LOR import
   -- enh (nick)   Improve resiliency of FSEQ parsing to detect corrupt FSEQ files
   -- enh (keith)  Add multiple eye settings to node range face allowing eye parts to be defined as different colours
   -- enh (keith)  Add xSchedule API to start/stop plugins
   -- enh (keith)  Add command support to xSchedule plugins including one to remote falcon to set the active playlist
   -- bug (scott)  RemoteFalcon crash if playlist is renamed/removed
   -- bug (scott)  Fix Model Blend Default Off Setting not working
   -- bug (priemd) Fix some resize issues with boxed models
   -- bug (dkulp)  Node selection SubModel dialog not allowing selection on OSX or Linux
   -- bug (dkulp)  Fixup filename doesn't work if anything in the path has non-xml safe characters
2020.29 July 18 2020
   -- enh (dkulp)  Respond to system color changes and update row header/waveform
   -- bug (dkulp)  Move email prompt until after version check and frame is up/displayed (occassionally hangs)
   -- bug (dkulp)  Fix problem with start channel calculation on reverse single color polylines
   -- bug (dkulp)  FPP Connect hanging on OSX
   -- bug (keith)  Fix duplicate layer star cuts off render buffer
   -- bug (keith)  Fix VU Meter and Music effect note range text boxes prematurely adjust Start vs End
   -- bug (scott)  Fix FPP Proxy output
2020.28 July 10 2020
   -- bug (keith)  Fix S5 import onto nodes does not work - this is a chance this will break import from multicolour 
                   models but I don't have any samples to test on.
   -- bug (dkulp)  More fixes for sandbox file access issues
   -- bug (dkulp)  Fix parsing custom value curves
2020.27 July 8 2020
   -- enh (dkulp)  Performance improvements with Video Preview window open, hardware video decoding
   -- enh (dkulp)  Various performance optimizations in render pipeline, thread pools
   -- enh (dkulp)  FPP Connect - restart fppd if outputs uploaded
   -- enh (keith)  Add a bulk edit option on file picker that lets you set the show folder
   -- enh (scott)  Added Right Click "Auto" Wiring Options to Custom Model Dialog
   -- enh (scott)  Added to Face Effect, Outline 2 uses 4th color pallet
   -- enh (scott)  Added DMX Skull Model Node Level Support for "On" Effect
   -- bug (keith)  "Black Cherry Cosmos" shader not working
   -- bug (keith)  Fix cube model with many strings does not take up the right number of visualiser ports
   -- bug (keith)  Fix some crashes if invalid IP addresses/hostnames are used for controllers
   -- bug (dkulp)  Video effects with "Keep Aspect Ratio" enabled take a long time to render
   -- bug (dkulp)  OSX Sandboxed builds not able to download models or shaders
   -- change(dkulp) OSX builds are now all "Sandboxed" applications.  This REQUIRES users to
                    go into preferences and reset all their preferences, select storage directories,
                    redownload shader/model caches, etc...
                    Expect some instability until the sandbox restrictions can all be found and
                    resolved.
   -- bug (scott)  Fixed DMX Skull Model Export
   -- bug (scott)  Better Scaling for "Scaled" Render Buffers
   -- bug (scott)  Fixed offset in PinWheel CW mode causing two arms
2020.26 June 23 2020
   -- enh (scott)  Add drop down for accessing buffer presets
   -- enh (scott)  Prevent users opening core xml files as sequences
   -- bug (dkulp)  Fix crash in OSX hardware video decoding
   -- bug (keith)  Try again to fix hangs relating to model list select dialog
2020.25 June 21 2020
   -- enh (dkulp)  Add "Virtual Matrix" as FPP controller type
   -- enh (dkulp)  "Discover" will now discover FPP instances used as Virtual Matrix
   -- enh (scott)  Add a GetMatrix xSchedule API call to get the matrix details
   -- enh (scott)  Remember node selection dialog size and position and zoom
   -- enh (priemd) Add ability to select models and add them to an existing group
   -- enh (keith)  Add a start angle to the spinner model
   -- enh (keith)  Add option on import to erase any existing effects on layers being imported
   -- bug (scott)  Fix AlphaPix Flex/Evolution upload for firmware 4.02, 4.04, and 4.05
   -- bug (scott)  Fix protocol case issues affect ability to chain a model
   -- bug (keith)  Fix visualiser hangs
   -- bug (keith)  Fix issues with serial model channel assignment and remove warning about model gaps
   -- bug (keith)  Erase colours when force colour option is turned off in faces and states
   -- bug (keith)  Fix remote falcon plugin seems to hang when token is invalid
   -- bug (keith)  Fix entering effect duration does not always take
   -- bug (keith)  Fix effect assist does not follow its preference setting
2020.24 June 13 2020
   -- enh (keith)  Change model export to xmodel to include all groups that only contain submodels/strands 
                   of the model being exported
   -- enh (keith)  Optionally allow xCapture to insert any missing frames in the generated FSEQ file due to frame dropping.
   -- enh (keith)  Be smarter about pre-selecting timing tracks when importing sequences if they are not already present in the sequence
   -- enh (keith)  Add right click select all/none to lyric track list in import dialog
   -- enh (keith)  Add the ability to drop fragment shaders onto the effects grid from explorer/finder
   -- enh (keith)  Add value curves to the video effect crop settings ... but beware this is slow
   -- enh (keith)  Increase xSchedule schedule priority range to 1-20. Priorities above 10 will beak anything in the queue allowing schedules
                   to trump remote falcon when required
   -- enh (keith)  Add advanced RGB-RGBW conversion
   -- enh (kevin)  Add support for audio shaders
   -- enh (priemd) Layout Panel multi select in Tree (#2071)
   -- enh (priemd) Additional options for Layout Panel Tree View popup menu
   -- enh (scott)  Add an optional second outline on face definitions
   -- enh (scott)  Add a rotation offset to the pinwheel effect
   -- enh (scott)  Add right click clear cells to custom model editor
   -- bug (keith)  Fix falcon full control of F48 with less than 32 outputs configured not uploading correctly
   -- bug (keith)  Fix a crash rendering effect on a DMX model
   -- bug (keith)  Fix some residual issues with models which had leading or trailing spaces. They will still throw an error
				   but that will go away on next save of sequence/layout
   -- bug (keith)  Fix opening sequence via FSEQ no longer finds XML if that is still the sequence file name
   -- bug (keith)  Fix windows setup to update controller config files on install
   -- bug (keith)  Fix an issue with face effect where it stops referring to the lyric track and uses the selected phoneme
   -- bug (keith)  Fix face node range order does not work if specified backwards
   -- bug (keith)  Fix reversed LOR models not importing from S5
   -- bug (keith)  Fix Advatek controller config did not allow for expanded mode
   -- bug (dkulp)  Fix some dark mode colour issues
   -- bug (dkulp)  Add xsq to "all" sequence import selection
   -- bug (scott)  Fix cut/copy/paste greyed out in custom model dialog
2020.23 June 5 2020
   -- enh (scott)  Add blend mode which is overwrite all but black to xSchedule blend modes
   -- enh (keith)  Add voting support to remote falcon plugin
   -- enh (keith)  Allow temporary change to show folder
   -- enh (keith)  Change sequence filename extension to xsq
   -- enh (keith)  Move all the subfolder settings into preferences
   -- enh (keith)  Add bulk edit support to morph quick set button
   -- enh (keith)  Add the ability to export and load sub-buffer definitions to simplify reuse
   -- bug (keith)  Fix crash when moving polyline between previews
   -- bug (keith)  Fix global proxy being ignored when outputting pixel data
   -- bug (keith)  Fix effect freeze does not freeze rotozoom and blur
   -- bug (keith)  Prevent crash on VSA import
   -- bug (dkulp)  Fix some overlap model upload issues to FPP
2020.22 May 26 2020
   -- enh (dkulp)  Faster refresh/startup on OSX with complicated group trees
   -- enh (scott)  Added model group to list right-click menu with the ability to Copy, Sort, and Clear models.
   -- enh (keith)  Add a setting for Falcon controllers which optionally allow you to let xLights fully control the falcon 
                   config erasing any other settings from string ports
   -- bug (dkulp)  Fix problems where changes to models nested deep within groups within groups would not
                   trigger rebuild of buffers resulting in effects rendered improperly or not at all
   -- bug (keith)  Fix model group centre cross disappears
   -- bug (keith)  Fix controller tab says it needs saving immediately on load
   -- bug (keith)  Fix crash when opening the visualiser
   -- bug (keith)  Fix crash when changing serial protocol to Pixelnet
   -- bug (keith)  Fix or add tracing for all crashes reported in crash log upload
2020.21 May 19 2020
   -- enh (scott)  Add model group import from other rgb effects files
   -- enh (chris)  Add auto build of linux builds
   -- enh (dkulp)  Merge virtual strings onto single line in visualiser on controllers that don't merge models
   -- enh (keith)  Enable download of Gilbert Engineering models
   -- bug (keith)  Fix play playlist step loop does not actually loop
   -- bug (keith)  Fix crash turning on MIDI remote mode
   -- bug (keith)  Fix escaping polyline before dropping a second point leaves an invalid single point polyline
   -- bug (keith)  Fix crash when finalising models - numerous scenarios
   -- bug (keith)  Fix handling of white channel on DMX floods
   -- bug (keith)  Fix a crash when shutter channel on a DMX flood is invalid
   -- bug (scott)  Fix a random effects crash
   -- bug (hearms) Fix MIDI and ARTNET timecode don't handle hours correctly
2020.20 May 17 2020
   -- enh (keith)  Add a plugin to xSchedule to work with Remote Falcon
   -- enh (keith)  Add commands to xSchedule to allow buttons to play and play looped a specific step
   -- enh (priemd) Add ability to rotate wiring view
   -- bug (scott)  Fix max serial port channels on FPP hats
   -- bug (scott)  Fix hinks pix controller upload crash due to non-ethernet controller type
   -- bug (keith)  Fix stars with multiple layers the same size generate overlapping nodes
2020.19 May 11 2020
   -- enh (keith)  Make it possible to clear the set model brightness from in the visualiser
   -- enh (keith)  Limit the list of vendors to those who have controllers that work with the chosen controller type
   -- enh (keith)  Add a hard stop option to xSchedule
   -- enh (keith)  Load models in parallel to speed up model load
   -- enh (keith)  Add port tooltips in visualiser to help manually configure controllers
   -- enh (priemd) Add grey option to wiring view
   -- bug (kevin)  Fix house-preview video export broken for odd-numbered heights
   -- bug (keith)  Fix issues with control tab convert not converting some !desc:sc entries correctly
   -- bug (keith)  Fix display export does not work for animations
   -- bug (keith)  Fix crash in servo effect if some skull servos are missing
   -- bug (keith)  Fix Falcon upload should refuse to upload invalid smart remote/non smart remotes on the same block of 4 ports
   -- bug (keith)  Fix custom model dimensions inflate by 1 when you edit the custom model data
   -- bug (keith)  Make scrubbing work in animations
   -- bug (keith)  Display picture effect as all red if the filename is blank
   -- bug (keith)  Fix scroll to model in visualiser
   -- bug (keith)  Fix every step is counting in sequence overall times
   -- bug (keith)  Change xSchedule to stop at the start of the end time rather than at the end of the end time minute
   -- bug (keith)  Fix crash adding models to groups
   -- bug (keith)  Fix models lose protocol in visualiser
   -- bug (keith)  Fix panels don't enable correctly when new sequence created
   -- bug (priemd) Fix display of submodel start channel
   -- bug (kevin)  Fix display export exports blank video if video is an odd number of pixels high
   -- bug (dkulp)  Fix matrix panel uploads incorrectly enabling BBB strings
2020.18 May 5 2020
   -- enh (keith)  Add 3d display of floodlights including beam length control
   -- enh (keith)  Add ability to span models over several smart remotes from the same 4 ports
   -- enh (keith)  Add ability to flip virtual matrices
   -- enh (keith)  When reporting missing face images include the name of the face in the error message
   -- enh (priemd) Optimise custom model loading
   -- enh (scott)  Updated HinksPix Export Dialog with Slave support and Master/Remote mode.
   -- enh (keith)  Added a new controller state ... active in xLights. This allows you to output to the controller in xLights but 
                   in xSchedule or when uploaded to a FPP master the output will be disabled.
   -- enh (keith)  Add option to fade to black or fade to zero in the set colour and set all playlist items in xSchedule
   -- enh (keith)  Add an offset to the schedule start end time when sunrise or sunset is used.
   -- bug (priemd) Fix submodel names don't update in model list when changed
2020.17 May 1 2020
   -- enh (keith)  Add native media player option for video in xSchedule. This slightly reduces control but improves performance
   -- enh (keith)  When using DMX effect on model group label the sliders using the node names from the first model in the group
   -- enh (keith)  Increase beam length of DMX moving heads and add visual control over beam width
   -- enh (keith)  Reorganise the tools menu to group related items
   -- enh (keith)  Add support for strings of single colour but individually controlled nodes
   -- enh (keith)  Make picture effect display red when picture is missing much like video and shader effects do
   -- enh (keith)  Scrub model preview along with the audio
   -- bug (keith)  Try to force dialogs that don't appear on any monitor onto a monitor
   -- bug (keith)  Fix picture effect tiling issues where tiling is not complete.
   -- bug (keith)  Fix effects on model group with horizontal or vertical scaled render styles but containing no models crashes when rendering
   -- bug (keith)  Fix some erroneous check sequence errors re subbuffers on per model render styles
   -- bug (keith)  Fix crash in visualiser when controller with no protocols is assigned to a port
   -- bug (keith)  Fix area flood beam length is anchored to z=0 rather than moving with the flood
   -- bug (keith)  Fix models with uneven strings don't show correctly in visualiser
   -- bug (keith)  Fix issue with unused smart remote port causing weird config on a falcon
   -- bug (keith)  Fix issue with loading downloaded DMX fixtures
   -- bug (keith)  Fix unable to map multiple channels from a single prop in the LMS import
   -- bug (scott)  Fixed AlphaPix Upload for Firmware 2.16+
   -- bug (scott)  Fix force ip not working on controller tab
2020.16 April 17 2020
   -- enh (dkulp)  Support for saving the password for FPP instances.
   -- enh (keith)  Add colour organ to xSchedule
   -- enh (keith)  Add time since start of playlist to schedule displays and in getplaying status api
   -- enh (keith)  When dragging handle on model show coordinates in the status bar
   -- enh (keith)  Add ability to append to file from a playlist item - great for logging what plays when
   -- bug (dkulp)  Fix FPP instances being discovered as PixLites.
   -- bug (dkulp)  Prevent FPP instances with duplicate hostname from being added
                   to the controller list with same name.
   -- bug (dkulp)  KulpLights F8-B's are mixing up ID's for the "No Serial" and "4 Serial" cases
   -- bug (keith)  Auto remote smart controller settings from models on controllers that don't support them in visualiser
   -- bug (keith)  Fix model group centre displays in wrong place
   -- bug (keith)  Fix setting start channel on model does not blank controller name
   -- bug (keith)  Fix eyeballs don't work on timing tracks in master view in display elements
   -- bug (keith)  Display polyline points in same coordinate system as other models
   -- bug (keith)  Fix renaming a model on sequence load removes the target model from all views
   -- bug (keith)  Fix a crash in text effect when % is entered
   -- bug (keith)  Fix cancel edit of custom model data zeros the z location
   -- bug (keith)  Fix crash due to render progress exceeding the limit
2020.15 April 10 2020
   -- enh (keith)  Add to visualiser ability to hide models assigned to other controllers
   -- enh (keith)  Add key binding editing
   -- enh (keith)  Add support for OPC (Open Pixel Control)
   -- enh (keith)  Update zoom room link
   -- enh (dkulp)  FPP set discover defaults to autosize and autoupload
   -- bug (keith)  Improve compliance of MIDI timecode sending
   -- bug (keith)  Improve compliance of ArtNet timecode sending
   -- bug (keith)  Fix state effect does not set channels on dmx models
   -- bug (keith)  Fix mix of individual universes and universe ranges don't convert correctly
   -- bug (keith)  Fix white wiring view does not zoom correctly
   -- bug (keith)  Fix auto upload broke ZCPP sending
   -- bug (keith)  Update shader download to work with new ISF URL format
   -- bug (keith)  Fix controller visualiser printing
   -- bug (scott)  Fix crash when changing custom model depth
   -- bug (scott)  Fix star model end channel not updating
   -- bug (dkulp)  Fix trackpad panning in 2D on OSX
2020.14 April 5 2020
   -- enh (keith)  Make xSchedule holding the screen on on windows optional
   -- bug (keith)  Fix FPP date column ordering is a text sort
   -- bug (keith)  Fix crash when rendering blur
2020.13 April 4 2020
   -- enh (keith)  Add visual indication of model group centre
   -- enh (keith)  Add effect suspend
   -- enh (keith)  Add effect suppress
   -- enh (keith)  Add colour control on sparkles
   -- enh (keith)  Add option to layer arches
   -- enh (keith)  Add SMPTE linear time code remote support to xSchedule
   -- enh (keith)  In visualiser pressing a key will scroll right hand list of models to 
                   the first model starting with that letter
   -- bug (keith)  Fix channel block model does not update correctly in model list
   -- bug (keith)  Give effects a bit more time to render a frame
   -- bug (keith)  Fix cancelling custom model editing screws up model location and size
   -- bug (keith)  Fix circle models dont seem to update when properties change
   -- bug (keith)  Fix ip address missing from getplayingstatus results
   -- bug (keith)  Fix when double clicking to remove node in ordered submodel move the 
                   insertion point to where the node was removed
   -- bug (keith)  Fix entering submodels with a space in the name is difficult
   -- bug (keith)  In visualiser when models overlap the visualiser acts weird
   -- bug (scott)  Fix model chain dropdown does not appear when port set
   -- bug (scott)  Escape illegal characters in CSV file names
2020.12 March 28 2020
   -- enh (dkulp)  FPP Connect - allow sorting the sequence list columns
   -- enh (dkulp)  Update FPP upload for new hardware, make sure both PRU's can be used
   -- enh (keith)  Add chroma key ... make any colour transparent
   -- enh (keith)  Add superstring support
   -- enh (keith)  Highlight selected node in the 3d preview of custom models when editing them
   -- enh (scott)  Add FSEQ date column to the FPP connect dialog
   -- bug (dkulp)  Fix issues of "DUMMY" virtual strings being uploaded to FPP
   -- bug (keith)  Fix more leading/trailing space issues on model names
   -- bug (keith)  When exporting GIF images of effects force almost blacks to black
   -- bug (keith)  Fix tree properties not enabling and disabling correctly
2020.11 March 21 2020
   -- enh (kevin)  Add star transition
   -- enh (kevin)  Extend options on the bowtie transition
   -- enh (keith)  Remember import and submmodels dialog size and position
   -- enh (dkulp)  Allow upload of config to PiHat serial port
   -- enh (scott)  Add brightness settings control in the visualiser dialog
   -- enh (keith)  Add WLED as a controller type
   -- bug (keith)  Fix reverse on out transitions disabled
   -- bug (keith)  Fix crash when model group name begins or starts with space
   -- bug (dkulp)  Use dark green for used universes under E1.31 and Artnet on OSX dark mode 
   -- bug (keith)  Fix cancel turning off of indiv universes does not re-set checkbox   
2020.10 March 17 2020
   -- enh (kevin)  Add five new transitions for matrices / high-density props
   -- enh (keith)  Add SanDevices E680 to controller options ... but no upload
   -- enh (keith)  Add ability to set the centre point for minimal grid model groups
   -- enh (keith)  Add the ability to generate submodels from 3D custom model slices
   -- enh (keith)  Add the ability to suppress model groups when defining model groups
   -- enh (keith)  Add ability to render and export models just for selected effect range
   -- bug (keith)  Fix controlle port range set to serial range when port is currently set to 0
   -- bug (keith)  More fixes to handle leading/trailing spaces in model names
2020.9 March 9 2020
   -- enh (keith)  Where model knows the controller it is on apply controller limitations to the model properties
   -- enh (keith)  Copy backup folder options into the preferences tab for backup
   -- enh (keith)  Prevent model names and controller names from having leading and trailing spaces ... as these cause hard to detect issues
   -- enh (keith)  Add protocol setting and constraints to the visualiser
   -- enh (dkulp)  Add e1.31 Auto upload for Falcons
   -- bug (gil)    Fix DmxMovingHead3D beam not showing up
   -- bug (dkulp)  Fix orientation and start corner of overlay models uploaded to FPP
   -- bug (dkulp)  Fix visualizer on HiDPI display on Windows
   -- bug (dkulp)  Fix cape detection for known FPP capes
   -- bug (scott)  Fix group count on falcon controllers
   -- bug (scott)  Fix SanDevice controller limits and auto layout
   -- bug (scott)  Fix Alphapix controller limits
   -- bug (keith)  Fix Falcon controller limits
   -- bug (keith)  Fix ESPixelStick does not show a pixel port
2020.8 March 4 2020
   -- enh (keith)  Add drag and drop model controller assignment capability to the visualiser
   -- enh (dkulp)  FPP based Pixel controllers can auto-upload string configuration, intput information, and
                   flip to bridge mode when Output to lights it turned on.
   -- bug (keith)  Keybinding effect should not change an existing effect
   -- bug (keith)  Fix paste bug which also impacted presets and random effects
2020.7 March 1 2020
   -- enh (keith)  Show used universes in controller properties by colouring in the cells when individual universes are shown
   -- enh (keith)  Add width to the spirograph effect
   -- bug (keith)  Fix preferences dialog crashes on windows
   -- bug (keith)  Fix more keyboard anomalies in AC mode
   -- bug (keith)  Fix focus not given to effects dropped with keybindings, paste and random effects
   -- bug (keith)  Fix value curve dropper does not disable slider and text box when applied
   -- bug (keith)  Fix split timing marks should not worked on fixed timing tracks
   -- bug (keith)  Prevent model export on groups as the code cant work
   -- bug (scott)  Fix bug in E131 autosize
   -- bug (scott)  Fix HC pixel channel counts problem
2020.6 February 24 2020
   -- bug (dkulp)  Fix FPP upload ... includes adding board variants which mean users may need to reselect the model for their controllers
   -- bug (scott)  Fix Artnet 0 universe not available
   -- bug (scott)  Fix E1.31 autosize not keeping start universe
2020.5 February 22 2020
   -- enh (keith)  Change halve timings to a more generic divide timings
   -- enh (keith)  Add undo support to divide timings
   -- enh (keith)  Convert setup tab to the new controller tab included in this are a huge number of 
                   smaller enhancements and standardisations of how the protocols work.
   -- enh (keith)  Add pixlite controller discovery
   -- enh (keith)  Add dominant frequency colour effect to VU meter which chooses the colour based on the dominant frequency
   -- enh (keith)  Increase maximum nodes on the circles model
   -- enh (keith)  Increase maximums on the custom model size
   -- enh (keith)  Add to xSchedule activate and deactivate all schedules API
   -- enh (keith)  Add ordered node selection to the submodel node selection dialog
   -- enh (keith)  In AC select mode when key is pressed to add an effect dont take it out of select mode ... this seems more natural
   -- enh (scott)  Added Export Model as GIF image
   -- enh (pete)   Have breakdown words and phrases respect locking
   -- bug (gil)    Fix too many node names created for 8-bit servos
   -- bug (keith)  Fix falcon upload of serial outputs incorrect if more than one port is uploaded
   -- bug (keith)  Fix change to submodel in group is not detected by sequencer until restart
2020.4 February 1 2020
   -- enh (dkulp)  Move all settings from Settings menu to new "Preferences" dialog
   -- enh (gil)    Servo effect can now be mapped to a Lyric track
   -- enh (gil)    Upgrade the DMX Servo model to be dynamic allowing up to 24 servos
   -- enh (gil)    Get the servo pivot point working for the DmxServo3D model
   -- enh (keith)  Add bulk edit to the active property on models and objects
   -- bug (scott)  Fix ZCPP max channel limit too low
2020.3 January 18 2020
   -- enh (gil)    Add new DMX Servo model options (Servo version uses images and Servo 3D version uses OBJ files)
   -- enh (gil)    Added a DMX 3 Axis Servo model option
   -- enh (gil)    Add a DMX Area Flood model for a rectangle instead of a circle
   -- enh (gil)    Update Import/Export capability of DMX models and provide ability to use model downloader
   -- enh (keith)  Prevent creation of show folder in program files folder on windows
   -- bug (keith)  Fix unable to chain model on a port which has a model on it that spanned multiple ports
   -- bug (dkulp)  Fix some trackpad/mouse scroll event scales
   -- bug (keith)  Dont show model overlaps when model group is selected
2020.2 January 11 2020
   -- enh (dkulp)  OpenGL - Allow 24bit depth buffer for OpenGL canvas.
   -- enh (dkulp)  OpenGL - For <=16bit depth buffer for OpenGL canvas, adjust the near clipping plane to provide more precision for z-depth to use.  This fixes most issues with Render Style 1 on Intel.
   -- enh (dkulp)  OpenGL - if problems creating context, fallback to using the shared context.  Mostly works fine to share it.  Should happen, but occasionally does with SideCar.
   -- enh (scott)  Rewrote Select Effect Panel. Effects can be selected by Effect Type. Removed Regex Searching.
   -- enh (gil)    Reorganized DMX models into separate classes.  Make sure you have backups of your show folder
                   just in case since it was a huge change.
   -- enh (dkulp)  Reduce number of wxIdleEvent are being processed to reduce CPU usage
   -- eng (keith)  Add support for the GDTF model file format in model import (DMX fixtures)
   -- bug (dkulp)  Videos with B frames may stop rendering.  Fixes #1902
   -- bug (dkulp)  OpenGL - Always create canvas with depth buffer.  This is required for SideCar.
   -- bug (gil)    View Objects were still saving with version 3 positioning instead of 4 causing rotation changes to keep reversing in X and Y
   -- bug (gil)    Fix lasso'ed effects not properly selecting or setting the selected row
   -- bug (gil)    Fix an unselected effect on top or bottom row could block a group effect move
   -- bug (gil)    Fix Circle model display when wiring starts in the center. Fixes #1905
   -- bug (gil)    Fix Circle model not redrawing when changing wiring start location
   -- bug (gil)    Fix rotation handle not working for 2D square models
   -- bug (keith)  Fix visualiser displays incorrect number of channels on unused serial output
   -- bug (keith)  Add phone blacklist file to windows install
   -- bug (scott)  Fix arches end channel not updating when node count changes
2020.1 January 3 2020
   -- enh (dkulp)  Change internal SequenceData storage to allow use of hugepages/superpages, not require a single gigantic storage blob
   -- enh (dkulp)  Upgrade Linux build to wxWidgets 3.1.3
   -- enh (gil)    Add group scaling
   -- bug (dkulp)  Fix a bunch of HiDPI issues on Windows that started with update to VS and wxWidgets 3.1.3
   -- bug (gil)    Fix 3D objects not refreshing when parameter fields are updated
   -- bug (gil)    Also needed to invert X rotation when upgrading model positions
   -- bug (gil)    Fix mesh object scaling that I broke in 2019.73
   -- bug (gil)    Fix control key not able to remove model from a group
   -- bug (keith)  Fix upload to falcon where a smart remote is skipped does not upload
   -- bug (keith)  Fix lights dont turn off when xLights exits while outputting to lights
2019.74 December 31 2019
   -- enh (scott)  Added HinksPix Export Dialog to generate stand alone mode files. No "Foreign" Slave support yet.
   -- enh (gil)    Allow all models and objects to be selected for rotation and translation
   -- enh (gil)    Allow 3D View Objects to be group rotated and translated
   -- enh (keith)  Add to check sequence some opinionated views on undesirable settings
   -- bug (gil)    Fix 3D View Objects rotation...i.e. mesh objects, grids, etc.
   -- bug (gil)    Model versions were not being saved with the new version number.  This is fixed now but if you
                   moved any of your rotations and saved in .73 you will need to fix them.
2019.73 December 30 2019
   -- enh (gil)    Improve 3D model rotate tool...single models rotate better and it works with groups
   -- enh (keith)  Add useful status details to buttons plugin web pages
   -- enh (keith)  Include the visual c++ runtimes in the main install file
   -- enh (keith)  Show current schedule end time in the web UI
   -- bug (keith)  Fix issue where more than one schedule can show as next
   -- bug (keith)  Fix issue where next scheduled time is not always calculated   
   -- bug (keith)  Allow PlayList MQTT item data to include xml special characters
   -- bug (keith)  Fix arches with > 1 light per node does not respect the gap property
   -- bug (keith)  Change apply setting key binding so it updates all selected effects ... not just the UI
   -- bug (keith)  Fix some value curves dont drop on value curve buttons
   -- bug (scott)  Fix hinkspix channel mapping error
   -- bug (gil)    Fix paste by cell for single effect does not use cell percentage
2019.72 December 28 2019
   -- enh (gil)    Zoom to mouse pointer in Layout
   -- enh (gil)    DMX Models should now work (they do something) for all effects
   -- enh (keith)  Add zoom to the wiring view
   -- enh (keith)  Lay the groundwork for a future where only xSchedule is released as 32 bit
   -- enh (keith)  Add an Any import type which shows all supported input types when importing sequences
   -- bug (gil)    Fix PolyLine Icicle drop channel numbers when Blue Square is selected
   -- bug (keith)  Fix xSchedule schedule clone does not work right
   -- bug (keith)  Bunch of cleanup to build process to eliminate unnecessary complexity
   -- bug (keith)  Fix hang when opening audio file fails
2019.71 December 21 2019
   -- enh (scott)  (beta) Added Hinkspix Pro/Easylights 16 Controller Upload
   -- enh (keith)  Show next schedule which will play in blue text on the UI
   -- enh (keith)  Add support for negative polyline drops ... sticks
   -- bug (keith)  Fix falcon split upload calc does not account for nulls and lots of virtual strings
   -- bug (keith)  Fix jump to step does not work in xSchedule correctly if steps have duplicate names
   -- bug (keith)  Fix turn off lights when xSchedule test mode is turned off
   -- bug (keith)  Fix S5 channel effects on multicolour models do not import
   -- bug (keith)  Prompt when cancelling custom model data edit if something changed
   -- bug (keith)  Fix windows stack walk not working when xLights crashes
   -- bug (dkulp)  Fix crash when model changes and we dont unselect any active effect
   -- bug (dkulp)  Fix crash when loading some sequences
   -- bug (dkulp)  Fix crash if effect is deleted while mouse is down
2019.70 December 13 2019
   -- enh (keith)  Allows state effect to work on groups of models containing the same states
   -- enh (scott)  Increase maximum star node count
   -- enh (chris)  Upgrade unix log3cpp to 1.1.3
   -- bug (keith)  Fix xSchedule test does not turn off test when reset pressed
   -- bug (dkulp)  Move email collection later to prevent some crashes 
   -- bug (scott)  Fix crash on FPP upload if media file has moved   
2019.69 December 7 2019
   -- enh (dkulp)  Increase SUP import time offset range
   -- bug (keith)  Fix xSchedule web displays incorrect next play time
   -- bug (keith)  Attempt to minimise log file rolling crash
   -- bug (keith)  Fix every step playlist items don't always play if they are delayed
   -- bug (keith)  Fix windows hardware video decoder memory leak
2019.68 December 3 2019
   -- enh (scott)  Added AlphaPix 16 and AlphaPix Flex Controller Upload Support.
   -- enh (dkulp)  Adjust fseq compression depending on Pi model
   -- enh (keith)  Add #universe/id:startchannel to model export even if model was not defined that way
   -- enh (keith)  Add "Play one random step in specified playlist" API
   -- enh (keith)  On windows have xSchedule bump its process priority when outputting to lights
   -- enh (keith)  On windows have xSchedule ask operating system not to sleep/screen blank etc
   -- bug (dkulp)  Fix crash where hardware video scaler does not scale image to right size
   -- bug (keith)  Fix crash which hung the whole machine when loading some sequences
   -- bug (keith)  Fix crash on exporting models
   -- bug (keith)  Fix xSchedule hang when playlist step changes active playlist
   -- bug (keith)  Fix On effect with 100% transparency does not show sparkles ... but it used to
2019.67 November 28 2019
   -- enh (scott)  Add maximise to import dialog
   -- enh (scott)  Add button to move selected models to top of the model list
   -- enh (scott)  Added Sort By Name But Groups At Top by Size, this should in theory move larger groups to the top
   -- bug (keith)  Fix xSchedule double sends data at times when playing 25ms sequences
   -- bug (keith)  Fix wxLogging causes lockups for some users
2019.66 November 25 2019
   -- enh (will)   Added Test Mode (BETA) plugin to xSchedule Web interface
   -- enh (scott)  Added APA102 pixel protocol support.
   -- enh (scott)  Added 'Alt + Left Click' panning to the Layout Tab - 2D only.
   -- enh (scott)  Added AlphaPix 4 Controller Upload Support.
   -- enh (keith)  Add bulk edit of smart remotes
   -- enh (dkulp)  Add uncompressed sparse v2 fseq file format for really low end controllers
   -- bug (dkulp)  Take a bunch of effects out of render cache that dont need to be there
   -- bug (dkulp)  Fix some bugs in setting the FPP restart flag when doing uploads
   -- bug (dkulp)  Fix a render and export model crash
   -- bug (keith)  Add bulk edit to warp effect choice fields
   -- bug (keith)  Fix xSchedule web UI password not working
   -- bug (keith)  Default J1SYS to 3200 speed
   -- bug (keith)  To fix errors like blank playlist dialog in xSchedule, memory leak in xSchedule I have moved   
                   the windows release to visual c++
   -- bug (keith)  Fix crash changing simple playlist step name
   -- bug (keith)  Fix 4 channel WRGB strings
   -- bug (keith)  Fix crash with single colour snowstorm effects
2019.65 November 18 2019
   -- enh (keith)  Support face effects on model group when all models have the same face definition
   -- enh (keith)  Warn about slow rendering due to certain pixel styles
   -- enh (keith)  Move to wxWidgets 3.1.3 and gcc 8.1.0 on windows   
   -- enh (keith)  Move to wxWidgets 3.1.3 on OSX   
   -- enh (keith)  Make snowstorm support colour curves
   -- enh (chris)  Move to wxWidgets 3.1.3 on Linux
   -- enh (scott)  Added J1Sys, EasyLights, and FPP Controllers to the Multi-Controller Upload Dialog.
   -- enh (cpinkham) Add upload of virtual matrixes to FPP
   -- bug (keith)  Handle some old anomalies with controller name not cleared from the model
   -- bug (keith)  Handle videos which don't use pts to tell us the frame timestamp
   -- bug (keith)  Fix old render style pinwheel does not follow centre offsets
   -- bug (keith)  Prevent node layout on model groups
   -- bug (keith)  Fix xSchedule incorrectly warns about sequence size
   -- bug (keith)  Fix cube model does not calculate string lengths correctly when uploading to controllers
   -- bug (keith)  Fix some submodel dialog issues with changing type of submodel
   -- bug (keith)  Fix papagayo does not breakdown words with just a single short phoneme
   -- bug (keith)  Fix twinkle AC ramps draw style is wrong if start/end brightness > 100
2019.64 November 9 2019
   -- enh (scott)  Added Pixlite and EasyLights Controllers to the Controller Selection Dropdown.
   -- enh (keith)  Make lyrics entry dialog resizable
   -- enh (dkulp)  Add zlib compressed FSEQ support
   -- enh (keith)  Add degrees setting to sphere model
   -- bug (gil)    Fix effect alignment bug if some effects are offscreen above visible area. Fixes #1192.
   -- bug (gil)    Fix panning so it's 1:1 in 2D.
   -- bug (keith)  Fix falcon upload virtual strings on smart remotes dont load in correct order
   -- bug (keith)  Fix falcon dmx upload wont take start channel greater than 512 if in absolute addressing mode
   -- bug (keith)  Fix crash loading mono audio files
   -- bug (keith)  Fix 3D model scaling issues for one high boxed models
   -- bug (keith)  Fix 2 colour LPE/LOREDIT colours not importing correctly
   -- bug (runion) Fix unicast does not send FPP media sync
2019.63 November 3 2019
   -- bug (dkulp)  Fix audio dropout
2019.62 November 2 2019
   -- enh (gil)    Prevent a zoom action from occurring while panning.  Fixes #1782.
   -- enh (scott)  Added 'Horizontal Stack - Scaled' & 'Vertical Stack - Scaled' Group Render Styles
   -- enh (scott)  Added Copy & Rename to Model Faces Dialog
   -- enh (keith)  Add some directx9 technical controls to special.options
   -- enh (keith)  Add an arch gap model property
   -- enh (keith)  Add option to double click an item to move between lists on model group panel and sequence elements panel
   -- enh (keith)  Add some optional render cache logging to help detect issues with the cache
   -- enh (keith)  Add quick seek to video reader for situations where the current position is well behind where it needs to be
   -- enh (keith)  Add an option to xSchedule so that on restart scheduled sequences which have missed their start times start
                   where they would have been up to - great for crash recovery situations
   -- bug (gil)    Fix preview windows flashing white while panning.
   -- bug (keith)  Improve stability of xFade under high load and improve throughput to over 300 universes
   -- bug (keith)  A number of tweaks to hardware video decoding to improve stability and fallback to software decoding.
                   Also default to software decoding until we have better reliability
   -- bug (dkulp)  Some tweaks to FPP connect to improve discovery
   -- bug (scott)  Fix importing submodels adds invalid data
   -- bug (scott)  Fix FPP connect not uploading panel start channel if port is defined
   -- bug (keith)  Models that fully overlap but are on different smart remotes should be uploaded on separate virtual strings
   -- bug (keith)  Fix set pixels api has wrong definition
   -- bug (keith)  Fix bulk edit dimming curves don't immediately apply to sequenced effects
   -- bug (keith)  Fix colour panel update does not apply to effects that are not visible
2019.61 October 26 2019
   -- enh (gil)    Add Redo option for the sequencer grid.  Fixes #555.
   -- enh (dkulp)  FPP/BBB DMX strings - set minimum length to 16 channels.  Several controllers have issues with less than that
   -- enh (dkulp)  FPP Connect - use a single progress dialog to avoid stealing focus for each thing transferred
   -- enh (keith)  Add special.options file for technical options
   -- enh (keith)  Add xSchedule API for querying pixels which have been set via the API
   -- rem (keith)  Remove Syncrolight support due to legal threats
   -- bug (dkulp)  FPP Connect - only remembered manually added FPP instance hostnames if they exactly matched (case sensitive)
   -- bug (kevin)  Fix certain shaders from ISF failing to compile
   -- bug (keith)  Fix loredit import crashes on strand mapping import
   -- bug (keith)  Fix loredit does not import all nodes on a strand
   -- bug (keith)  Fix reversed model should always be on a new virtual string
   -- bug (keith)  Put a bunch of extra defense in xFade against corrupt packets
   -- bug (keith)  Fix render cache purged when sequence closed
   -- bug (keith)  Fix some old rotozoom value curve errors
   -- bug (keith)  Bunch of windows hardware video decoding fixes
   -- bug (keith)  Fix submodels don't highlight when clicking on them in model list
   -- bug (keith)  Don't auto chain DMX models
   -- bug (keith)  Fix auto serial start channels not correct when models overlap
   -- bug (keith)  Fix boxed models generate illegal scale if x or y dimension is 1
   -- bug (keith)  Weird effects grid behaviour if effect starts and ends < 0ms
2019.60 October 20 2019
   -- bug (dkulp)  Attempt to fix crash on thread exit
   -- bug (gil)    Fix value curves for Single Strand effect.  Fixes #1355.
   -- bug (gil)    Fix PolyLine model rotation when it contains curves.  Fixes #1460.
2019.59 October 19 2019
   -- enh (keith)  Implement hardware video decoding on windows ... this is short of the OSX decoding as image
                   sizing is still done in software
   -- bug (keith)  Consistently show the dmx channel on models in the model list on the layout tab
   -- bug (keith)  Fix channel block does not respect user node display size wishes
   -- bug (keith)  Fix xcc files not loading on colour curve and colours panel
2019.58 October 13 2019
   -- bug (keith)  Fix dragging effects does not change effect to the right effect
2019.57 October 13 2019
   -- enh (dkulp)  Point Queen Mary for OSX to digitally signed versions for Catalina
   -- enh (dkulp)  Optional support to use VideoToolbox API's on OSX to use
                   hardware video decoders for Video effect.  Can be enabled/disabled
                   on the Settings menu
   -- enh (dkulp)  Upgrade to ffmpeg 4.2.1
   -- enh (scott)  Add shift-U and shift-D key bindings to add fade in/out for one second to selected effects
                   - mostly there as an example of how to set these up
   -- bug (keith)  Address effect selection lag
   -- bug (keith)  Restore circles fade behaviour
   -- bug (keith)  Fix corrupting xml by SMS options
   -- bug (keith)  Fix dmx channel issues with ZCPP (affects only serial outputs where first model was not on channel 1)
   -- bug (keith)  Fix multistring custom model without set indiv start nodes gets string lengths wrong
2019.56 October 9 2019
   -- enh (scott)  Added Folder/Filter Selection to the FPP Connect Dialog.
   -- enh (keith)  Add the ability to manually moderate SMS messages
   -- enh (keith)  Add an extensive SMS blacklist
   -- enh (keith)  Add model group clone
   -- enh (keith)  Add colour dropper panel
   -- bug (keith)  Fix submodels show start channels as invalid
   -- bug (keith)  Fix 3d preview latch is global causing jump when first moving a preview
   -- bug (keith)  Fix when previewing in 3d any rotation causes models to flash white
   -- bug (keith)  Fix when using keybindings to drop effects the panel reset is not respected on change in effect
   -- bug (keith)  Fix multi light per node candy canes don't display correctly
   -- bug (keith)  Fix timezone SMS adjust breaks test message feature
   -- bug (keith)  Fix blacklist filtered messages not sending SMS response but blacklisted phone was
   -- bug (keith)  When displaying default message when playing SMS suppress the text format
   -- bug (gil)    Fix servo effect value curves
2019.55 October 2 2019
   -- enh (dkulp)  Optimize FPP config upload to not upload file that is the same to avoid restart flag being set
   -- enh (keith)  Add value curve drag and drop mode for fast sequencing using saved value curves
   -- bug (keith)  Maybe fix some crash on sequence load problems
   -- bug (keith)  Possible fixes for some crashes and more logging to help isolate others
   -- bug (keith)  Fix poor default size of custom model wiring preview
   -- bug (keith)  Suppress shader file download error for image file
   -- bug (keith)  Improve the default sequencer panel layout
   -- bug (keith)  Fix a bunch of issues with the circles effect not displaying correctly
   -- bug (keith)  Fix single channel spinner model causes crashes when used in a model group
   -- bug (keith)  Prevent model group render buffers of different sizes causing crashes
   -- bug (dkulp)  In FPP upload use filename fixups to find media files
2019.54 September 27 2019
   -- enh (keith)  Add support for video files with an alpha channel to video effect
   -- bug (keith)  Fix double click output changing IP address (where last output with that IP address) does not update
                   models that refer to the IP address
   -- bug (keith)  Fix 3D preview of custom model data does not render correctly if no sequence is open
   -- bug (keith)  Make vu meter timing track effects work with multi-layer timing tracks (but it will only use the top layer)
   -- bug (keith)  Fix issues with single channel matrices not behaving correctly
   -- bug (keith)  Fix model group warning on startup when there is a blank model name in the list
   -- bug (dkulp)  Fix FPP upload of reverse setting on virtual strings
2019.53 September 23 2019
   -- bug (keith)  Fix crash moving models
2019.52 September 23 2019
   -- enh (keith)  Add support for additional vixen 3 timing tracks and lyric tracks
   -- enh (keith)  Add rudimentary vixen 3 sequence import
   -- enh (keith)  Add ability to suppress vendor downloads you don't want
   -- bug (scott)  Fix Node Select Dialog selection not saving in Faces and States Dialog.
   -- bug (keith)  Fix auto start channels all models on same port get the same start channel
   -- bug (keith)  Remove some check sequence warnings for serial protocols
   -- bug (keith)  Fix some quirks with submodel display
2019.51 September 21, 2019
   -- enh (keith)  Add 3D custom model preview with live wiring view when building custom models
   -- enh (keith)  Suppress version upgrade check when command line rendering
   -- enh (keith)  Add ability to adjust for timezone problems with SMS service responses
   -- bug (dkulp)  Fix problems of submodel rendering not honoring if "Blend between models" is turned off
   -- bug (dkulp)  Fix a bunch of undefined behavior warnings from Xcode 11's analyzers
   -- bug (gil)    Fix Marquee rendering on Single Lines to work like prior to 2019.12.
   -- bug (gil)    Fix after model preview reset next rotate jumps
   -- bug (keith)  Fix S5 import of archived models
   -- bug (keith)  When importing S5 single channel model import it at the model level rather than node level
   -- bug (keith)  Remove syncrolight 1024 pixel per output limitation
   -- bug (keith)  Get submodels in groups highlighting correctly in layout
   -- bug (keith)  Fix auto layout of DMX models not producing sensible results
2019.50 September 14, 2019
   -- bug (keith)  Fix issues with single channel candy canes and polylines
2019.49 September 13, 2019
   -- enh (scott)  Added Import From Model/File to Faces and States Dialog.
   -- enh (scott)  Added Import From Submodel to Faces and States Dialog.
   -- enh (scott)  Added Right Click Highlight Selected to Random Effect Selection Dialog
   -- enh (scott)  Added Autosize Channels to DDP output
   -- enh (keith)  Added Level & Level Note versions of the Jump and Jump 100 to VU Meter
   -- enh (keith)  Add WMV video file support
   -- enh (keith)  Add right click reset to all preview windows
   -- enh (keith)  Add latitude control to the sphere model
   -- bug (keith)  Fix maintaining 3D mode between sequencer and layout tabs
   -- bug (keith)  Fix icicles and candy cane alternate wiring not display correctly on preview
   -- bug (keith)  Fix check sequence check when files are not in show folder
   -- bug (dkulp)  Fix single line & polyline Blue/Green start rendering backwards
2019.48 September 9, 2019
   -- enh (scott)  Changed Node Select Dialog to select nodes on highlight.
   -- enh (keith)  Add alternate drop nodes to Icicles and Polyline Icicles models
   -- bug (keith)  Make polyline blue start behave consistently with other models.
   -- bug (keith)  Fix a crash in falcon upload where status.xml does not download
   -- bug (keith)  Fix bug with single line where start set to blue renders backwards
   -- bug (keith)  Fix ZCPP to E131 conversion leaves E131 in a really bad state
   -- bug (keith)  Fix syncrolight ethernet does not save id correctly
   -- bug (keith)  Fix syncrolight outputs with more than 300 pixels don't display correctly
2019.47 September 7, 2019
   -- enh (scott)  Added Random Effect Selection Dialog
   -- enh (scott)  Added SubModel/Phoneme/State name to Node Select dialog header text
   -- enh (scott)  Added shift scroll wheel horizontal scrolling to Node Select dialog
   -- enh (keith)  Add xSchedule API for speeding up/slowing down animation playback
   -- enh (keith)  Add option inclusion of user email in logs to help us reach out to people experiencing issues
                   where we need more information
   -- enh (keith)  Add progress while generating random effects as it runs slowly and it looks like it has hung
   -- enh (keith)  Add setup export to models export
   -- enh (keith)  Add sorting to controller connection column on layout tab
   -- enh (keith)  Add alternate wiring to candy cane model
   -- enh (keith)  Add the ability to display twitter account followers in xSchedule
   -- enh (keith)  Add MQTT user/password support
   -- bug (keith)  Make candy cane model consistent with arches model in the way it uploads to controller
   -- bug (keith)  Fix some model groups don't include bottom line of pixels
   -- bug (dkulp)  Fix layout import not working on OSX
   -- bug (dkulp)  Fix setup tab right click menu not working ... particularly on OSX
   -- bug (keith)  Fix image model shrinks to zero size when property changed
2019.46 August 31, 2019
   -- enh (keith)  Add option to hide the body of moving head lights
   -- bug (keith)  Fix bug where model does not show as you are drawing it
   -- bug (keith)  Fix bug where DMX models shrink to zero size when property changed
   -- bug (keith)  Fix bug where 3 point models change size when node count changes
2019.45 August 30, 2019
   -- bug (keith)  Fix Syncrolight ethernet controller flicker
   -- bug (keith)  Fix effects on strands, nodes and submodels dont render automatically
   -- bug (keith)  Fix a bunch of model sizing issues
   -- bug (keith)  Fix model preview model accessing invalid models crashes   )
2019.44 August 28, 2019
   -- enh (keith)  Add support for Syncrolight serial protocol
   -- enh (keith)  Add support for Syncrolight ethernet protocol
   -- enh (keith)  Move a number of xSchedule items to their own threads to minimise pauses in playback
   -- enh (keith)  Add extra variables to the xSchedule text playlist item so you can display things like song name
   -- enh (keith)  Add check sequence check for reuse of the show folder name in subdirectories
   -- enh (scott)  Add remote details to visualise exported csv file
   -- enh (scott)  Auto enable canvas mode for shaders that require it
   -- bug (keith)  Fix polyline model import imports them crazy large
   -- bug (keith)  Fix model group name does not change immediately on screen when renamed
   -- bug (keith)  Fix models disappear from layout screen but stay in model list
   -- bug (keith)  Fix model size matching ... particularly 2 and 3 point models
   -- bug (keith)  Fix some custom model nodes vertically in the wrong position
   -- bug (keith)  Fix xSchedule ping invalidly shows success when it has failed
   -- bug (keith)  Fix dialog positioning to keep them fully on a single display
2019.43 August 23, 2019
   -- enh (keith)  Have layout and house preview 3d state follow each other
   -- enh (scott)  FPP Connect Updates: Added Header Tool Tips and Right Click Option to Check/Uncheck Highlighted FSEQ Files
   -- bug (scott)  Fixed multi controller upload not showing Auto Start Channel E131 and DDP controllers
   -- bug (keith)  Fix multistring custom model returns incorrect string length
   -- bug (keith)  Fix e131 one output pressing visualise breaks start channel calcs
   -- bug (keith)  Fix model rename breaks all model chains
   -- bug (keith)  Fix clicking off an effect does not properly unselect it
   -- bug (dkulp)  Fix controller start channel issues when using proxy
   -- bug (keith)  Fix make models same size is slightly out
2019.42 August 20, 2019
   -- enh (keith)  When changing an outputs IP address if it is the last output with that IP address then change any models that specifically reference it
   -- bug (keith)  Fix model groups of groups dropping model groups
   -- bug (keith)  Fix hang in some invalid start channel situations
   -- bug (keith)  Fix model group panel does not clear when show folder changed
   -- bug (keith)  Fix invalid start channels don't show the invalid start channel in model list
   -- bug (keith)  Fix polyline nodes don't update on model properties when changed
   -- bug (keith)  Fix OSX xSchedule crash when web interface opened
2019.41 August 18, 2019
   -- enh (keith)  Add ability to load SRT subtitle files as timing tracks
   -- enh (keith)  Move xSchedule sync logging to its own logger to reduce impact on performance
   -- enh (keith)  Change xSchedule CURL playlist item to support https
   -- enh (keith)  Add arbitrary custom model data rotation
   -- enh (keith)  Rework start channel calculation routines. Some configurations should see better layout tab performance
   -- enh (dkulp)  Track and set the expansion board type on FPP bbb string instances
   -- enh (dkulp)  Added FPP PB16 and PB16-Exp support
   -- enh (scott)  Add controller IP and description to controller visualiser
   -- bug (keith)  Restore controller ping and enable through FPP proxy
   -- bug (keith)  Fix crash due to too many chained models blows the stack
   -- bug (keith)  Remove unnecessary discontiguous channel warnings
   -- bug (keith)  Bunch of auto layout models fixes for dmx and smart receivers
   -- bug (scott)  Fix cant upload to FPP model names containing quotes
2019.40 August 12, 2019
   -- enh (scott)  Added "APPLYSETTING" Keybinding Option
   -- enh (keith)  Add more comprehensive FPP remote/master support in xScheduler
   -- bug (keith)  Fix falcon smart string upload
   -- bug (keith)  Fix lyric import does not recognise fraction of seconds for start/end offsets
   -- bug (keith)  Fix some strings wont upload to falcon where virtual strings have large gaps
   -- bug (keith)  Fix cant change start channel because it is disabled
   -- bug (dkulp)  Fix arch start channel problem #1644
   -- bug (dkulp)  Fix slow startup when using multicast
2019.39 August 9, 2019
   -- enh (dkulp)  Discovery - match IP addresses to resolved hostnames as well
   -- enh (dkulp)  Add ability to convert ZCPP output to e1.31
   -- enh (keith)  Let user adjust maximum blank frames when generating custom model
   -- enh (keith)  Have the UI stop the user from setting up previews other than default in 3D as they cant actually be viewed in 3D in playback
   -- enh (keith)  Make SMS service test work even when no service has been signed up for
   -- enh (keith)  Add a F12 force keyboard focus in sequencer to allow keyboard recovery from entry fields stealing focus
   -- enh (pete)   Add logarithmic x Axis to music effect
   -- bug (dkulp)  Fix FPP connect upload of sequences that take longer than 10 seconds to transmit
   -- bug (keith)  Fix cant delete polyline point in 3D
   -- bug (keith)  Fix Metronome timing tracks not working
   -- bug (keith)  Fix changing sequence timing does not immediately change fseq file timing
   -- bug (keith)  Fix making 2 and 3 point models the same size does not work
   -- bug (kevin)  Fix broken transitions
2019.38 August 5, 2019
   -- enh (kevin)  Enable rotation of "From Middle" transition
   -- enh (kevin)  Add Dissolve and "Circular Swirl" transitions
   -- enh (keith)  Add pixel counts to controller visualiser
   -- enh (dkulp)  Support FPP upload when UI password set
   -- enh (dkulp)  Support FPP upload of SmartReceiver configuration to BBB capes
   -- enh (keith)  Add text & shapes effect font bulk edit
   -- enh (scott)  Add open node layout in browser for better printing
   -- bug (kevin)  Fix non-functional Reverse setting on Fold transition
   -- bug (scott)  Fix model chain shows red even though valid
   -- bug (keith)  Fix crash due to invalid model pointer in groups of groups
   -- bug (keith)  Fix single colour shockwave flashes white at the end
   -- bug (dkulp)  Fix weird behaviour of arches model and layers
   -- bug (dkulp)  Fix washed out videos and other matrix color issues introduced in .35
2019.37 July 30, 2019
   -- enh (keith)  Add option to SMS plugin to limit messages from a single number
   -- enh (keith)  Fix workflow around creation of new timing tracks
   -- enh (keith)  Enhance keyboard arrow key handling on effects grid - add autoscroll
   -- enh (keith)  Let user control RenderCache location
   -- bug (keith)  Fix ZCPP not applying additional string properties immediately
   -- bug (keith)  Fix ZCPP does not turn off bulbs when reducing bulb count
   -- bug (keith)  Fix blur crash
   -- bug (keith)  Address screen display over-brightness in transparency change
   -- bug (dkulp)  Fix Mojave time display turning black
2019.36 July 27, 2019
   -- enh (keith)  Enhance text display in xSchedule
   -- enh (keith)  Force valid show folder on xSchedule start
   -- enh (keith)  Enhance xSchedule ping to show failure count and only count 3 consecutive fails as an actual fail
   -- enh (keith)  Expand pixture effect frame rate adjustment upper limit to 20
   -- enh (keith)  Default saving xmap to same name as it was loaded from
   -- enh (scott)  Added Layer Blending Help Button
   -- enh (scott)  Added Import/Export Model to Circle Model
   -- enh (scott)  Added Toggle Effects Assistant Button to the Window Management Toolbar
   -- bug (scott)  Fixed buffer transform for "Overlay - Center" and "Overlay - Scaled"
   -- bug (scott)  Fixed "Blank" Star Model Import Bug
   -- bug (keith)  Fixed three point models don't save their RotateX setting
   -- bug (keith)  Fixed invalid OBJ crashes xLights
   -- bug (keith)  Fix SMS Daemon settings don't apply immediately
   -- bug (keith)  Fix SMS Daemon messages containing commas cause issues
   -- bug (keith)  Fix shader panel does not show scroll bars
   -- bug (keith)  Fix locked models resize when string properties change
   -- bug (keith)  Fix model groups don't highlight correctly when changed
   -- bug (keith)  Fix erroneous check sequence error for single channel matrices
   -- bug (keith)  Fix value curve should not allow 0.0 or 1.0 point to be deleted
   -- bug (keith)  Fix check sequence reports invalid error when text effect is drawing text from a lyric track
   -- bug (keith)  Fix crash importing uncompressed music xml file
   -- bug (keith)  Fix xSchedule white on white problem in dark mode for playlists list
   -- bug (keith)  Fix Twilio download of SMS messages
   -- bug (keith)  Fix deleting models when one has an effect on it
   -- bug (pete)   Fix crash if render state is dumped while not rendering
   -- bug (dkulp)  Fix OSX version does not show readme from help menu
   -- bug (dkulp)  More OpenGL work
2019.35 July 16, 2019
   -- enh (dkulp)  FPP Connect - don't expand multi-universe E1.31 outputs to multiple lines
   -- enh (keith)  Add some checks for slow storage and overly outsized images/video that drives rendering work
   -- enh (keith)  Add toggle presets dialog as a keybinding option
   -- bug (dkulp)  FPP Connect - hopefully fix crash on Windows.  If not, more logging added.
   -- bug (dkulp)  FPP Connect - Fix "Add FPP" button that stopped working a while ago
   -- bug (keith)  Fix crash when model is deleted while sequence is open
   -- bug (keith)  Model state dialog does not save selected nodes when using node selection dialog
2019.34 July 12, 2019
   -- enh (dkulp)  FPP Connect - Allow uploading of JUST the UDP outputs needed for that proxy instance
   -- enh (dkulp)  FPP Connect - Use pixel cape information and UDP output information to expand ranges used for v2 sparse files
   -- enh (keith)  Add keybinding for suspend render
   -- bug (dkulp)  FPP Connect - Typing new playlist name into FPP connect not working
   -- bug (dkulp)  AutoStart channels not working for DDP outputs
   -- bug (keith)  Now we are confident keybindings are not suffering ongoing corruption silently fix it
   -- bug (keith)  Fix crash deleting multiple layout models
   -- bug (keith)  Fix changing model group preview does not take effect until xlights restarted
2019.33 July 5, 2019
   -- enh (keith)  Add an alternate sweep to VU meter for timed bouncing arches
   -- enh (dkulp)  Use parallel discovery for FPP
   -- enh (keith)  Optimise the screen location of a bunch of dialog boxes
   -- enh (keith)  Defer papgayo dictionary load until you are breaking down words
   -- enh (keith)  Add finer grained control to midi event detection in xSchedule
   -- bug (gil)    Fix render buffers for 3D camera angles
   -- bug (keith)  Fix replace lost code which meant covert node data to effects does not work on strand and model
   -- bug (keith)  Move back to shaders rendering only on the main thread on windows due to excessive hanging
   -- bug (keith)  Fix 2/3 point models don't draw in 3D layout
   -- bug (keith)  Fix xSchedule plugins being called when they shouldn't be
   -- bug (keith)  Fix xSchedule does not always correct for show folders moved between machines
   -- bug (keith)  Add in shader bulk edit
   -- bug (keith)  Replace the audio band filter with a much better but slower one
   -- bug (keith)  Fix model bulk edit sometimes misses the focussed model
   -- bug (keith)  Fix cant see node numbers on custom model dialog on OSX dark mode
   -- bug (keith)  Fix bulk edit controller type not updating for all controller types
   -- bug (dkulp)  Fix bug with FPP not always finding FPP instances
2019.32 June 30, 2019
   -- enh (keith)  Add MQTT support to xSchedule ... both publish and subscribe
   -- enh (keith)  Add support for static and dynamic PS to xSchedule RDS
   -- enh (keith)  Add waveform band pass filters for partial frequency waveform visualisation in sequence waveform display
   -- enh (keith)  Warn users opening xbkp files that they will save as xml
   -- enh (keith)  Add more blend styles to sup import ... also change the default as the old one did not handle overlapping images correctly
   -- enh (scott)  Add uniqueness ip & description check to e131 where auto size is enabled
   -- enh (dkulp)  Start introducing FPP proxy configuration of controllers accessed via FPP
   -- bug (dkulp)  Fix backup on start on OSX fails
   -- bug (keith)  Fix arduino buttons code for driving xSchedule sending messages delayed by one button press
2019.31 June 26, 2019
   -- enh (keith)  Add bulk edit of controller type
   -- enh (keith)  Double click in morph effect assist sends corners to their maximums
   -- enh (keith)  Add ZCPP protocol support
   -- enh (keith)  In Model Faces, States and import dialogs remember any custom colours between accesses to the colour dialog
   -- enh (keith)  Add support for convert node data to effects at the strand and model level to reduce the tedium of converting
                   each node individually.
   -- enh (keith)  Consolidate custom model node missing errors to one where missing nodes are contiguous
   -- enh (keith)  Add a link to install vamp plugins when creating timing tracks on a musical sequence and vamp is not installed
   -- enh (keith)  Make it easier to open sequence backups directly
   -- enh (keith)  Optionally pull text from a lyric track for the text effect
   -- enh (dkulp)  Move shader rendering to background threads
   -- enh (keith)  Make the display of overlapping pixels in previews look more natural
   -- enh (keith)  Add lead in time and speed controls to Shader effect (includes ability to run time backwards)
   -- enh (keith)  Download shader if name in tree is double clicked on
   -- enh (keith)  Make transition hints optional and work even if effect backgrounds not being displayed
   -- bug (kevin)  Fix shader effects may fail to render on some versions of Intel Integrated Graphics
   -- bug (keith)  Fix crash where iseq has more channels than the sequence
   -- bug (keith)  Fix timeline zoomout results in timeline being positioned way past the end of the song
   -- bug (keith)  Fix house preview cant be manipulated when playback is paused
   -- bug (keith)  Fix MIDI responsiveness in xFade and xSchedule
   -- bug (keith)  Fix Bulge shader
   -- bug (keith)  Set minimum 3d movement tool size
   -- bug (keith)  Add bulk edit to pinwheel location value curves
2019.30 June 16, 2019
   -- enh (keith)  In MIDI events add the ability to discern non zero data 1/2
   -- enh (keith)  Add an active property to models so you can hide models from display
   -- bug (keith)  Fix when xSchedule is just outputting event playlists output buffer is not zeroed out
   -- bug (keith)  Fix broken model download
2019.29 June 15, 2019
   -- enh (keith)  Add a Shader download option for known good shaders
   -- enh (keith)  Add the ability to prevent some double midi events where the event is not using key on/off
   -- enh (keith)  Warn in check sequence if trying to use shaders on openGL less than v3
   -- enh (keith)  Add the ability to control gravity in liquid effect
   -- enh (keith)  Allow xFade to work with multiple MIDI controllers at once
   -- bug (keith)  Bunch of changes to try to prevent crashes due to delayed rendering on the main thread
   -- bug (keith)  Try to detect and handle gracefully corrupt FSEQ files
   -- bug (keith)  Change the generation of 2D render buffers where rotation leads to a non zero z coordinate
2019.28 June 8, 2019
   -- enh (scott)       Add filename tooltip to colour curve buttons
   -- enh (kevin/keith) Add shader effect
   -- enh (keith)       When importing a timing track from xLights sequence and the name clashes overwrite if there are no existing timing marks
   -- enh (keith)       Allow xSchedule to access more than one MIDI device at once
   -- enh (keith)       Add "Set step position" and "Set step position ms" xSchedule APIs
   -- enh (keith)       Add ability to clone xSchedule playlists, steps, items and schedules
   -- enh (keith)       Add the ability to set the Force Local IP in xSchedule
   -- enh (keith)       Add the delay setting to the text playlist item
   -- enh (keith)       Add check sequence check for matrices where strands/string does not divide evenly into string length
   -- bug (keith)       Fix autosave volume does not restore correctly
   -- bug (keith)       Fix xSchedule web page handling when plugins not running
   -- bug (keith)       Fix default web page not working
   -- bug (keith)       Some fixes to event playlists
   -- bug (dklup)       Fix handling of multiple FPP with same hostname
2019.27 June 1, 2019
   -- enh (kevin)   Add "Fold" transition type
   -- enh (kevin)   Add effects-grid representation of transitions
   -- enh (keith)   Add import of previews and models from another rgbeffects file
   -- enh (keith)   Add some more snowflake styles
   -- enh (keith)   Enhance controller connection bulk edit to allow editing just one property
   -- enh (keith)   When selecting waveform if less than 5 pixels are selected treat it like a click
   -- bug (keith)   Make it easier to grab end value curve handles
   -- enh (keith)   When using jukebox always show the effect properties
   -- enh (keith)   Add event support to xSchedule plugins
2019.26 May 24, 2019
   -- enh (scott)   When prompting for missing audio include the original audio filename to make it easier to find
   -- enh (scott)   When audio is missing look down 1 folder and grab it from there if found
   -- enh (keith)   Add plugin functionality to xSchedule. See "Plugin Documentation.txt" in github for the interface
					specification. Also in github is a C# plugin wrapper with simple UI.
   -- enh (keith)   Move SMS Daemon to be an xSchedule plugin
   -- enh (keith)   Suport a "-shimmer" suffix on phonemes which will shimmer the phoneme mouth. This needs to be manually
                    added where you want it
   -- enh (keith)   When dropping a video file and creating a video effect get smarter about the length of the effect created
   -- enh (keith)   Add a right click menu to the test dialog channel lists to quick select/deselect channels/models/outputs
   -- enh (keith)   When editing jukebox button definitions always pop up the dialog near the button
   -- enh (keith)   Fix #1554 add a set mode api to xschedule to turn on/off master and remote modes
   -- bug (keith)   Fix #1553 xschedule test mode disable does not restore xSchedule prior mode correctly
   -- enh (keith)   Fix #987 Add preset length to effect preset panel
   -- bug (keith)   Fix #959 Edit effect timing does not allow keyboard entry
   -- enh (keith)   Add a CTRL-A keyboard shortcut to layout panel to select all models
   -- bug (keith)   Remove a memory check in win32 which older versions of windows don't support
   -- bug (dkulp)   Better handling if other FPP discovery nodes are found that aren't FPP (like ESPixelSticks)
   -- bug (dkulp)   Try to detect if "null" hostnames would appear in FPP Connect and ignore
   -- enh (lee)     Save audio volume between xLights runs
2019.25 May 11, 2019
   -- enh (keith)   Add some extra modes to VU Meter that act like timing mark controlled sweeps for arches
   -- bug (keith)   Fix punctuation at word level of lyric tracks prevents phoneme breakdown
   -- enh (keith)   Add support for scss palette files
   -- enh (keith)   Allow user to control model handle sizes
   -- enh (scott)   Added Ellipse to Shapes Effect
   -- enh (scott)   Added Rotation to the Present, Tree, Heart, and Crucifix in the Shapes Effect
   -- bug (dkulp)   Change fseq compression to a more compatible version
2019.24 April 29, 2019
   -- bug (dkulp)   Fix FSEQ v2 hangs on saving ... sometimes
   -- bug (keith)   Fix corrupt FSEQ v2 file crashes on sequence load
   -- enh (keith)   Add xFade ability to control xLights on multiple machines
2019.23 April 27, 2019
   -- enh (scott)   Add manual link to help menu
   -- bug (keith)   Make custom model generator capable of detecting the start in very dark videos
   -- bug (keith)   Better fix for models don't track the mouse in 2D layout
   -- bug (keith)   Make the row header size scale with the row size on the effects grid
   -- bug (keith)   Remove a bunch of unnecessary processing when renaming model groups
   -- bug (keith)   Fix timing track label edit broken
   -- bug (keith)   Fix auto hide house does not work if house is maximised
   -- bug (dkulp)   Add logging and try to handle corrupt fseq files
   -- bug (dkulp)   Make sure FPP string upload doesn't generate configuration that won't work, make sure
                    it marks hasSerial as false if no serial is configured
   -- bug (dkulp)   When bulk editing path, if the original "looks" like a windows path ( : as second char),
                    then parse as a windows path instead of "native", Fixes #1532
   -- bug (dkulp)   When loading older show directories, it will sometimes go into a loop displaying error
                    message trying to calc start channels for most models
2019.22 April 21, 2019
   -- enh (scott)   Add right click print to controller visualiser
   -- enh (scott)   Add right click export to csv file to controller visualiser
   -- enh (scott)   Updated Sandevice upload to support color order, brightness, group count, null pixel, and
                    reverse controller settings on FIRST model.
   -- enh (scott)   Add Sandevice Controller types to e1.31 dialog.
   -- enh (keith)   Upgrade J1SYS P12 support to support a broader range of models
   -- enh (keith)   Add the ability to send ArtNET triggers to xSchedule
   -- enh (keith)   Add the ability to receive ArtNET triggers to xSchedule
   -- enh (keith)   Add a character map browser to the shape effect emoji
   -- bug (scott)   Explain to user why they cant enter text on a fixed timing track
2019.21 April 15, 2019
   -- bug (scott)   Falcon upload - Default to "Universe" mode instead of "Absolute" on Firmware 2.04 and lower.
   -- bug (keith)   Fix jukebox button size
   -- bug (keith)   Fix wiring dialog sometimes loses some nodes
   -- bug (keith)   Fix dont suppress upload to all controllers if controller type specified
2019.20 April 5, 2019
   -- enh (dkulp)   Falcon upload - will upload in "Absolute" mode if Falcon is configured that way,
                    or "Universe" mode if it's configured that way.
   -- enh (dkulp)   ESPixelStick upload will honor color order, brightness, gamma, group count controller settings on FIRST model
   -- enh (dkulp)   Add Controller type to e1.31 dialog, allow selection of Falcon/ESPixelStick.   More to come.
   -- enh (keith)   Add shift scroll wheel horizontal scrolling to custom model dialog
   -- enh (keith)   Add cut to effects grid right click menu
   -- enh (keith)   Add alternate pixel wiring to the spinner model
   -- bug (keith)   Fix if no timing element is selected and you try to split it it wont split
   -- bug (keith)   Fix dragging models in 2D don't track with mouse
   -- bug (keith)   Fix node tooltip in states/faces/submodels does not work if you zoom or pan
   -- bug (keith)   Fix a cause of crash when loading LMS to a data layer
   -- bug (keith)   Fix timing labels ae not clipped to the box
   -- bug (keith)   When using up down to move models in the display elements panel ensure the selected model is visible
2019.19 April 1, 2019
   -- bug (dkulp)   DDP upload menu optimization doesn't work on first output in list
   -- bug (keith)   Bunch of fixes to LOR S5 import
   -- bug (scott)   Fix some overlapping keybinding defaults
   -- bug (dkulp)   Fix discovery returns 1 too few channels
   -- bug (dkulp)   Fix DDP upload to controller doesn't filter controllers on first row
   -- bug (keith)   Fix issues with 3 point models resizing when properties changed
   -- bug (keith)   Fix polyline individual segments expansion state lost between refreshes
   -- enh (keith)   Increase maximum spirals on the tree model
   -- enh (keith)   Make e131 default to multiple universes ... single output
   -- enh (keith)   Add custom model import onto matrices/trees as submodels including the submodels, faces and states
   -- enh (keith)   Offset models when pasting them if same model in the same location
2019.18 March 25, 2019
   -- enh (dkulp)   DDP outputs can record the controller type (hat/cape) and upload menu item only shows it
   -- enh (dkulp)   "Discover" button on setup tab to discover and automatically add/configure FPP instances
   -- enh (dkulp/keith) Add "Visualise" option for DDP/FPP controllers to see layout of models on controller
                            (work in progress)
   -- enh (dkulp)   FPP Connect can update the start channel of matrices for LED panel capes
   -- enh (dkulp)   FPP Connect can detect LED Panel capes, display the matrix size
   -- enh (dkulp/keith) Ability to use "Controller description" for starting channel (DDP only right now)
   -- enh (dkulp)   xShcedule as FPP remote now works with Multicast protocol (maybe Linux/OSX only)
   -- enh (dkulp)   xShcedule as FPP remote now responds to FPP Ping's/Discovery
   -- enh (dkulp)   Much faster FPP Connect discovery
   -- enh (keith)   Add a LOR S5 LOREDIT file import. This is a highly speculative implementation at this time
                    Unless those who want this function lean in and help get it right it will likely never be
					quite right.
   -- enh (keith)   Use a dropdown for selection of Force IP address
   -- bug (dkulp)   Fix problem with FPP connect not finding models if using hostname instead of ip address
   -- bug (keith)   Fix slow render when using render cache
2019.17 March 15, 2019
   -- enh (keith)   Add falcon smart remote support
   -- enh (keith)   Add ability to split timing marks in half
   -- enh (keith)   Add playlist item for outputting a short MIDI message
   -- enh (keith)   Show node count in model list tooltip
   -- enh (keith)   Move SMS retrieval to a separate thread
   -- bug (keith)   Fix a number of issues with models resizing when parameters change
   -- bug (keith)   Fix #1510 Model preview does not always centre model group
   -- bug (keith)   Fix 3D model handles don't hold size while zooming
2019.16 March 9, 2019
   -- enh (scott)   Added Cut/Copy/Paste Support for 3D Objects in the Layout Tab
   -- enh (scott)   Added Arrow Key Support for 3D Objects in the Layout Tab
   -- enh (scott)   Added Right Click Menu for Canvas Layers Selection. Select All, De-select All, Select Layers With Effects
   -- enh (dkulp)   Add more parallelism to kaleidoscope render
   -- enh (dkulp)   Make SMS feature available on OSX ... although in this release it may cause playback pauses
   -- enh (priemd)  Add yet more wiring options to the cube model
   -- enh (keith)   Expand pixlite upload to support all pixlite and LOR pixicon versions
   -- bug (scott)   Pressing Delete Key in Layout Model List now deletes model
   -- bug (dkulp)   Add some screen rendering options to sort through some display issues on model preview
   -- bug (keith)   Fix LOR Optimised output allows you to exceed memory allocated
   -- bug (keith)   Fix issue with canvas layers dialog displaying crazy number of layers
   -- bug (keith)   Fix some render cache issues
   -- bug (keith)   Fix single note spectrogram doesn't render
2019.15 March 2, 2019
   -- enh (keith)   All screen map area to be manipulated using the mouse
   -- enh (keith)   Stop render range expansion to include immediately prior and immediately following effect
                    There is a small chance this will cause user to need to click render all in some scenarios. Please report and provide
					sample sequences if this happens
   -- enh (keith)   Include a default SMS work whitelist and blacklist
   -- enh (keith)   Add a frame audio waveform to the VUMeter
   -- bug (keith)   Fix rendering per preview from a 3D camera ... it just wasn't right
                    This may change how some rendering looks. I am happy to look into any issues but I will only change it again if it is
					clearly wrong.
   -- bug (keith)   Fix crash rendering per model per preview from 3D camera
   -- bug (keith)   Fix/change behaviour when inserting/deleting layers while rendering. Abort the rendering to take the action
   -- bug (keith)   Fix SMS tool sends too many rejection messages
   -- bug (keith)   Fix SMS tool displays sent messages
   -- bug (keith)   Fix a crash when breaking down phrases with no actual words
   -- bug (keith)   Fix lines effect value curve not disabling on reset
2019.14 February 23, 2019
   -- enh (keith)   Add logarithmic x axis to spectrogram
   -- enh (keith)   Add line spectrogram to vu meter
   -- enh (keith)   Add circle line spectrogram to vu meter
   -- enh (keith)   Add check sequence warning if firework is to fire with timing track but none is selected
   -- bug (dkulp)   Fix bug where 2d background image not honoring center is 0 setting
   -- bug (keith)   Fix bandwidth.com cant send SMS
   -- bug (keith)   Fix accumulating snowflake does not accumulate if flake count value curve is used
   -- bug (keith)   Fix DMX effect on a per model group does not work as expected
   -- bug (keith)   don't reset canvas mode when randomising effect settings
   -- bug (keith)   Fix crash when using large triangles in Kaleidoscope on some models
   -- bug (keith)   Fix xSchedule cant set text on a text item playing in every step
2019.13 February 21, 2019
   -- enh (keith)   Add the kaleidoscope effect
   -- enh (keith)   Add voip.ms support to SMS Daemon
   -- enh (keith)   Add Twilio support to SMS Daemon
   -- enh (keith)   Add ctrl/ctrl-shift constraints to subbuffer corner moves
   -- enh (keith)   Add ctrl/ctrl-shift constraints to morph assist panel corner moves
   -- enh (keith)   Allow fireworks x/y location to work independently. If only one is set only that axis is constrained
   -- bug (dkulp)   Fix an occasional crash when loading a mesh
2019.12 February 16, 2019
   -- enh (scott)   Timing Tracks will persist the collapsed flag
   -- enh (scott)   Added Subfolder Selection to Batch Render Dialog
   -- enh (keith)   Add lines effect
   -- enh (keith)   Make hold colour an option on shapes effect
   -- enh (keith)   Add xSMSDaemon ... a SMS bridge between bandwidth.com and xSchedule
   -- enh (keith)   Allow shuffle to be set on a playlist - allows a background shuffled playlist
   -- enh (keith)   Rework morph effect assist gestures to be more like the subbuffer panel
   -- enh (keith)   Add palettes to the install
   -- bug (keith)   Fix shapes effect does not support all emoji characters
   -- bug (keith)   Fix xSchedule text wont display emojis
   -- bug (keith)   Fix some face effect settings don't reset when another effect is chosen
   -- bug (keith)   A bunch of tweaks to xSchedule remote mode
   -- bug (keith)   Fix fireworks fire on music/timing marks still fires randomly as well
   -- bug (keith)   Fix crash when rightclicking on box handle of polyline and trying to delete node
   -- bug (keith)   Fix cant open xml from command line
   -- bug (fbond)   Fix cant rescan midi in xSchedule event setup
   -- bug (dkulp)   Fix transparent image display problem on layout
   -- bug (dkulp)   Fix load issue on vixen eseq files
2019.11 February 9, 2019
   -- enh (keith)   Add shift move to morph effect assist
   -- enh (keith)   Upgrade pixlite controller upload for the 16 Long Range
   -- enh (keith)   Allow material files to be left in a subfolder under obj file location as long as it is referenced in the material file
   -- enh (keith)   Add bulk edit to the morph swap points button
   -- enh (keith)   Make the row header resizable
   -- enh (smorgan) For timecode slightly outside jitter limit nudge it back into time
   -- enh (smorgan) Add support for E131 prioritisation
   -- bug (scott)   Fixed Custom Model Dialog Shrink Crash
   -- bug (keith)   Fix 3d obj with space don't load
   -- bug (keith)   Fix custom models don't draw at z=0 when first created
2019.10 February 5, 2019
   -- enh (keith)   Add an <ALL> state to state effect that activates all states. Also recognise * in timing track as well
   -- enh (keith)   If models on a port are not using contiguous channels put them on virtual strings
   -- enh (keith)   Add an xSchedule API to change/reload showfolder
   -- enh (keith)   Include xmap files in the backup
   -- enh (keith)   Increase maximum lines read from file to 20 for text effect
   -- enh (gil)     Add option to convert tool for creating LEDBlinky animation files.
   -- enh (kevin)   Enable timeline icons for fan and galaxy effects
   -- bug (keith)   Fix custom models load with zero layers if depth not set correctly in the file
   -- bug (keith)   Fix falcon upload with virtual strings miscalculates number of nodes on a port
   -- bug (keith)   Fix disabled effect options with valuecurves don't disable correctly
   -- bug (keith)   Fix shape effect movement defaults to on when loading old sequences
   -- bug (kevin)   Fix model/row copy effects from one sequence and pasting in another. Fixes #1450
2019.09 February 2, 2019
   -- enh (keith)   Add movement to shapes effect
   -- enh (keith)   Add tool for applying reaper or xAudio edits to one or more audio files and generate a new audio file
   -- enh (keith)   Refactor the fireworks effect including new settings and new value curves
   -- enh (keith)   Add gain to VUMeter
   -- enh (keith)   Add an inverted music value curve
   -- enh (keith)   Add gain to music value curve
   -- enh (keith)   Render cache effects which have blur or rotation applied
   -- enh (dkulp)   Update OpenGL 1.1/2.1 render routines to perform better with Mesh objects
2019.08 January 30, 2019
   -- enh (keith)   Add to check sequence the listing of faces, states and view points a sequence relies
			        upon to help preparing for imports
   -- enh (keith)   In layout mode adapt the axis of movement using the keyboard based on camera position
   -- enh (keith)   Add channel remapping to the DMX effect
   -- enh (keith)   Add the selected effect time to the waveform for easier alignment
   -- enh (dkulp)   Add an option for 2D preview to move the 0.0 location to the middle bottom
   -- enh (dkulp)   Draw solid parts of mesh before lights, allows lights to blend onto them. (OpenGL 3.3 only)
   -- enh (scott)   Added rotation to the Shape Effect
   -- enh (scott)   Added Value Curve to Ripple Rotation Setting
   -- bug (keith)   Fix model per strand render buffers when group contains a custom model
   -- bug (keith)   Fix node layout on custom models when not square
   -- bug (keith)   Fix effect type bulk edit doesn't always initialise effect settings
   -- bug (dkulp)   Fix crashes with PolyLine w/ icicles where strand lengths did not return enough nodes for
                    various buffer styles
   -- bug (dkulp)   In 3d, if a grid or image object wasn't also present, mesh would not appear.
2019.07 January 27, 2019
   -- enh (tyler)   Add inside/outside ratio to star model
   -- enh (dkulp)   Add zoom gesture to Preview Panels.
   -- bug (dkulp)   OpenGL 2.1 - texture/image brightness was always 100%
   -- enh (dkulp)   Add Mac TouchPad/MagicMouse "wheel" event handling to Preview Panels.
   -- enh (dkulp)   OpenGL 3.3 only - pass mesh data to Video memory once, share data - much faster display
   -- enh (dkulp)   Share mesh textures across contexts (reduces memory, faster loads)
   -- bug (gil)     Fix PolyLine crash when older version 1 positioning is upgraded.  Fixes #1436.
   -- enh (keith)   Add 3D Custom Model
   -- enh (keith)   Make locked models more obvious in 3D
   -- enh (keith)   Allow cube and sphere to be exported as 3D custom models
   -- enh (keith)   Make green box on 2D layout optional
   -- enh (keith)   Add right click reset camera position to house preview and layout
   -- enh (keith)   Update J1SYS P2 upload to support more model configurations
   -- enh (keith)   Add a latency setting when running xSchedule in remote to adjust for network delays
   -- enh (keith)   Add a jitter setting when running xSchedule in remote to reduce adjustments not
                    required due to small variations in packet delivery time
   -- enh (keith)   Add bulk edit to effect type
   -- enh (keith)   Add support for \n and \t in playlist item text
   -- bug (keith)   Fix crash changing number of lights on single channel polyline
   -- bug (keith)   Fix z coordinates on a polyline when typed don't work
   -- bug (keith)   Fix issues with switching between effects using dropdown on effect settings dialog
   -- bug (keith)   Fix issues with Artnet & MIDI timecode when in remote mode
2019.06 January 19, 2019
   -- bug (dkulp)   Fix sub-buffers ending up with two rows/columns identical.  Fixes #1421
   -- bug (dkulp)   Fix FPP connect dialog not working correctly on windows
   -- bug (keith)   Fix 2D pan Y is reversed
   -- bug (keith)   Fix model move in 2D not working in Y direction
   -- bug (keith)   Fix grid axis not drawing
   -- bug (keith)   Fix xSchedule not reading FSEQ correctly
   -- enh (keith)   Make model move accelerate when using keyboard when repeatedly moving models using keyboard
   -- enh (keith)   Add sphere model
   -- enh (keith)   Increase maximum hollow size on spinner
2019.05 January 15, 2019
   -- bug (keith)   Fix controller upload picks up USB outputs
   -- bug (keith)   Fix #1418 effects don't work correctly on strands
   -- bug (keith)   Fix #1411 moving a model group containing a submodel in 3D crashes
   -- bug (keith)   Fix #1400 drawing an effect reactivate inactive objects
   -- bug (keith)   Fix FSEQ files not loading correctly
   -- bug (keith)   Fix check sequence overlapping channel false positive if first model node is not the lowest channel
   -- bug (keith)   Fix node layout dialog for cube model
   -- bug (keith)   Fix submodel generation on cube models
   -- bug (keith)   Fix strand and node names should not be duplicated on a model
   -- enh (jhinkle) Add Easylights controller upload
2019.04 January 11, 2019
   -- enh (gil)    Add pan based on viewing angle in 3d
   -- enh (dkulp)  Implement advanced FPP connect dialog including
						Discovery of FPP instances
						Instance by instance settings controls over what is uploaded
						Sparse FSEQ uploading
						Compressed FSEQs
   -- enh (scott)  Add keyboard shortcuts to sequence save dialog
   -- enh (keith)  Add cube model
   -- enh (keith)  Add value curve flipping
   -- enh (keith)  Include 3d objects in package sequence and cleanup file locations
   -- bug (keith)  Fix multistring custom model start channel
   -- bug (keith)  Fix xSchedule test settings dialog does not open
   -- bug (keith)  Fix anomalies when creating a sequence with a specified timing track
2019.03 January 6, 2019
   -- bug (scott)  don't persist some not useful FPP connect settings
   -- bug (dkulp)  Fix xLights does not handle system audio device change on OSX
   -- bug (dkulp)  Remove non-functioning scroll bars on layout
   -- bug (keith)  Fix model group per model default is not rendering correctly
   -- bug (keith)  Fix cant rename objects
   -- bug (keith)  Fix xSchedule not reading FSEQ correctly
   -- bug (keith)  Fix erroneous check sequence warning if a group is in All Previews
   -- bug (keith)  Fix some problems with lyric breakdown and improve user experience while dictionaries load
   -- enh (keith)  Exclude faces not used in a sequence from the package sequence file
   -- enh (keith)  Add a 2D value curve generator
2019.02 January 2, 2019
   -- enh (keith)  Add xSchedule test mode
   -- bug (keith)  Fix 3D undo crash and loss of objects
   -- bug (keith)  More keybindings work to try to isolate the problem
   -- bug (keith)  Fix cant change model to single channel
   -- bug (scott)  Timing track rename does not show you current name
2019.01 January 1, 2019
   -- enh (gil)    Add 3d
   -- enh (keith)  Test for out of order universes on a controller in check sequence
   -- bug (dan)    Fix more issues with FSEQ saving and loading
2018.59 December 30, 2018
   -- bug (keith)  Fix Single colour models not working correctly
2018.58 December 28, 2018
   -- bug (keith)  Fix FSEQ files not being saved with correct format
2018.57 December 27, 2018
   -- enh (keith)  Add full support for model connection properties for Falcon controllers. Create virtual strings
                   where necessary. Virtual strings will now be deleted on ports we upload to.
   -- enh (keith)  Add support for multi-string custom models
   -- bug (keith)  Fix change in model properties does not always update other models start channels in the model list
   -- bug (keith)  Fix display of @ model start channels
2018.56 December 16, 2018
   -- bug (keith)  Fix LOR import so dmx intensity and intensity come in correctly
   -- bug (keith)  Fix artnet not outputting
   -- enh (keith)  Add visual indication of slowness to the xSchedule UI
   -- enh (keith)  Add control over what happens when a remote xSchedule gets told to stop playing a sequence ... blank or not blank lights
   -- enh (scott)  Added Rotate 90 option to Custom Model Dialog
2018.55 December 10, 2018
   -- bug (keith)  Fix scheduler crash that was way more common than I thought it would be [windows + linux only]
2018.54 December 9, 2018
   -- bug (keith)  Fix scheduler timer does not start correctly when xSchedule starts [windows + linux only]
2018.53 December 8, 2018
   -- bug (keith)  Fix test dialog hangs on exit
   -- bug (keith)  Controller property changes don't update immediately in model list
   -- bug (keith)  When xSchedule is a FPP remote all sequences play as if they were 50ms timing
   -- bug (keith)  When xSchedule is FPP master it sends more sync packets than it should
   -- bug (keith)  Fix xSchedule hang - hard to be certain i got all causes but i likely did.
   -- bug (keith)  Fix xSchedule corrupts sequence file name leading to skipping of step - possible fix.
                   This may also address looping problems but it is hard to be sure as I don't have those problems.
2018.52 December 5, 2018
   *** CAUTION: This release on windows/linux include some changes which have a small risk of leading to xlights/xSchedule hanging
                so please test before using in your show. If you have issues it is safe to go back to a prior release.
   -- bug (keith)  Fix some memory leaks in xlights/xSchedule - this also may help with some lag
   -- bug (keith)  Fix prevent 32bit from trying to load render cache if it is there
   -- bug (keith)  Fix some xSchedule options not applied until restart
   -- bug (keith)  Fix absolute model start channels off by one in model list
   -- enh (keith)  Add "Close xSchedule" api
   -- enh (keith)  Add "Add n Seconds To Current Step Position" api
   -- enh (keith)  Add option in xSchedule to continue to try to connect outputs that fail to connect ... and when it fails do it silently
   -- enh (keith)  Add a state event to xSchedule which fires when it starts, lights go on, lights go off and when an output fails to start
   -- enh (keith)  Force LOR outputs to occasionally output a channel even if it hasnt changed
2018.51 December 3, 2018
   -- bug (keith)  More work on xSchedule audio load failure ... maybe a fix
   -- bug (keith)  Reduce frequency of model property reload
   -- bug (keith)  Keep sort in models when things are changed
   -- bug (keith)  Speed up virtual matrices by reducing unnecessary work
   -- bug (keith)  Default xSchedule to more efficient scaling algorithm
   -- bug (keith)  Fix chained model start channel displayed off by 1
   -- bug (chrisP) Fix FPP models file formatting error
   -- bug (keith)  Fix FPP sync when xSchedule is the remote
   -- bug (keith)  Fix face node select node saving
   -- enh (keith)  Add render cancel to keybindings
2018.50 December 1, 2018
   -- bug (scott)  Fix Submodel Dialog missing buttons and scaling issues
   -- enh (scott)  FPP Connect - save/load previous file selections on dialog open and close
   -- bug (keith)  Fix test dialog cant multiselect to change testing state
   -- bug (keith)  More work on xSchedule audio load failure ... maybe a fix
   -- bug (keith)  Fix some more old value curve errors
   -- bug (keith)  Fix xl Font move left and invalid chars not rendering correctly. This may impact some sequences if you have been working around this bug
   -- bug (keith)  Add xSchedule performance logging which can be turned on
   -- enh (keith)  Add optional parallel output of data in xSchedule
   -- enh (keith)  Add ability to skip media upload to FPP
   -- bug (keith)  Fix xSchedule crash if you schedule a stop command
   -- bug (keith)  Fixes for node effects on dmx models and some effect selection issues. Also fixes some LMS import problems
   -- enh (keith)  Allow opt out of using gzip for FPP upload
2018.49 November 27, 2018
   -- bug (keith)  Maybe one fix for xSchedule crashes loading audio + lots of extra logging to try and work out why it crashes
   -- bug (keith)  Fix model download progress dialog does not hide
   -- bug (keith)  Fix cant delete an every step step
2018.48 November 26, 2018
   -- bug (keith)  Add extra logging to try to identify xSchedule audio load crash
   -- bug (keith)  Fix bars effect expand on large model loops around
   -- bug (keith)  Fix single strand skips renders in correctly on odd width models
   -- bug (keith)  Fix vendor model download failing
   -- bug (keith)  Fix playlist loops loops once more than it should
   -- bug (keith)  Fix indiv start channels default incorrectly if using #ip:u:sc
   -- bug (keith)  Fix make view master crashes
   -- bug (keith)  Fix output processor colour order not saving
   -- bug (keith)  Fix test model channel for dis-contiguous channel model is incorrect:
   -- bug (keith)  Fix crash in submodel dialog
   -- enh (keith)  Add ability to define a step in a playlist that plays as part of every step in the playlist
2018.47 November 22, 2018
   -- bug (keith)  Fix a crash passing in an xml file to xlights which is not in the show folder
   -- bug (keith)  Ensure a city is always set
   -- bug (keith)  Fix a possible crash on sequence close
   -- bug (keith)  Fix schedule hang if pre-load audio and audio is shorter than fseq
   -- bug (keith)  Only reverse offset of a value curve if the offset is not zero
   -- bug (keith)  Allow default FPP version to be set for when FPP Connect cant determine the version
   -- enh (keith)  Add the ability to dim a range of channels as a playlist item
   -- enh (keith)  Add a tool to cleanup file locations ... bringing them all under the show folder
   -- enh (keith)  Display the range of controller outputs in the model list
2018.46 November 18, 2018
   -- bug (dkulp)  Fix node layout crash for custom models
   -- bug (dkulp)  Add some fallbacks for FPP upload if UI security is on
   -- bug (keith)  Fix some old value curve warnings ... hopefully for good
   -- enh (keith)  Warn in check sequence if videos are being rendered without render cache
   -- enh (keith)  Update the falcon upload to recognise the new connection properties ... these are
                   taken from the first model on each output
   -- enh (keith)  Add keybindings support for the effect and colour update buttons. F5 defaults for
                   effect update but you need to delete keybindings to have the new defaults added
2018.45 November 15, 2018
   -- bug (dkulp)  FPP Connect - failure to unzip fseq if name contains certain special characters
   -- enh (dkulp)  FPP Connect via HTTP/FTP - option to create/append to playlist whatever is uploaded
   -- enh (dkulp)  Model controller connection can store various properties including
                   gamma/direction/RGB order/brightness/null pixels which is also used for the FPP
                   controller upload to configure those strings if set
   -- bug (dkulp)  FPP controller upload DMX channels are off by 1
   -- bug (dkulp)  ModelPreview window not refreshing if model not selected
   -- enh (keith)  Prevent model chains from breaking when chained models are deleted
   -- bug (keith)  Fix xSchedule pauses when web connection interrupted
   -- enh (keith)  Make fixed timing tracks more obvious and make them convertible to unfixed
   -- enh (keith)  Adjust xSchedule FPP sync packets to match v2.x new send pattern
   -- bug (keith)  Fix deleting a model containing a submodel which is in a group generates non-sensical
                   errors and removes all the submodels from the group.
   -- bug (keith)  Fix polyline arc last segment doesn't arc
2018.44 November 11, 2018
   -- bug (keith)  Fix a crash when deleting models
   -- bug (keith)  Fix value curve dialog text entry not working correctly #1314
   -- bug (keith)  Fix fire growth cycles value curve not right
2018.43 November 10, 2018
   -- bug (dkulp)  Default DDP packet size should be 1440
   -- bug (keith)  Fix test doesn't work on non-contiguous submodels
   -- bug (keith)  Fix a number of crashes
   -- enh (dkulp)  If uploading to FPP 2.4+, gzip the fseq files prior to transmission
   -- enh (dkulp)  Use HTTP upload method for transferring fseq/mp3 files to FPP instead of FTP
   -- enh (keith)  Allow manually adding IP addresses for xSchedule to monitor
2018.42 November 6, 2018
   -- enh (keith)  Add text effect grabbing text from a file ... so you can have multiple effects changed from one
                   location even in multiple sequences
   -- enh (keith)  Add MIDI timecode offsets so you can use +1hr offsets
   -- enh (keith)  Add delay to the Screen Map playlist item
   -- enh (keith)  Make common mode/config issues even more obvious in xSchedule
   -- enh (keith)  Add check schedule check for submodels with nodes not on the parent model
   -- bug (keith)  Fix xSchedule web server 404 if ? parameter after a file request
   -- bug (keith)  Fix OSX crash calling FPP event
   -- bug (keith)  Warn user if removing an element that has effects at the strand, node and submodel level
   -- bug (keith)  Fix not reclaiming effect memory when they are deleted - this may be a risky change
   -- bug (keith)  Fix a bunch of crashes from crash reports
2018.41 November 2, 2018
   -- enh (scott)  Added Cut, Copy, Paste, Zoom In/Out, Background Image, and Model Select to the Node Select Dialog
   -- enh (keith)  Add music trigger fade value curve
   -- enh (keith)  Improve the apparent randomness of random playlists
   -- enh (keith)  Add a video effect duration mode that ends transparent rather than blue
   -- bug (keith)  Protect user from causing crash by taking actions while models download
   -- bug (keith)  Fix next time shows never if last day is today and show starts later.
   -- bug (keith)  Fix some colour anomalies in LMS import due to invalid effects.
   -- bug (keith)  Fix crash checking falcon firmware version
   -- bug (keith)  Fix bulk controller upload buttons not visible if you have lots of controllers
2018.40 October 29, 2018
   -- enh (keith)  Allow user to change row header text colour and fix time/frame rate text unreadable on OSX
   -- enh (keith)  Increase maximum DMX channels
   -- bug (keith)  Fix crash caused by exporting small model ESEQ files
   -- bug (keith)  Add tracing of frames intervals that are twice as long as they should be
   -- bug (keith)  Try again to fix OSX schedule time issues
   -- bug (keith)  Fix crash creating LCB files
   -- bug (keith)  More fixes to rendered faces
   -- bug (keith)  Fixes to Falcon F4 upload
2018.39 October 27, 2018
   -- enh (keith)  Add Not So Big Buttons plugin to xSchedule
   -- bug (keith)  Fix face effect not rendering the mouth
2018.38 October 26, 2018
   -- enh (gil)    Resize FSEQ data during save if sequence timing has changed.
   -- bug (dkulp)  Fix dimming/gamma curves on single channel models
   -- enh (keith)  Add music value curves that track music intensity
   -- enh (keith)  #1297 Add a to text playlist item ability to count down a number of seconds
   -- bug (keith)  #1299 Fix start channel cancel button does not work
   -- bug (keith)  #1300 Crash on initialising single channel spinner model
   -- bug (keith)  #1287/#1288 Fix controller connection not updating into model list
   -- bug (keith)  #1289 xSchedule eseq off by one problem
   -- enh (keith)  xCapture allow user to set frame time to specific MS
   -- enh (keith)  Add bulk edit of filename and pathname on picture, video and glediator effect file picker
   -- enh (keith)  Add xSchedule API to "Clear background playlist"
   -- bug (keith)  #1286 switching musical sequence to animation does not completely remove audio
   -- bug (keith)  Fix xSchedule looks like it needs saving immediately after it is loaded
   -- enh (keith)  Add to model list right click menu ability to remove all empty groups
   -- bug (keith)  Fix crash when deleting models on layout panel
   -- bug (keith)  Address a bunch of issues with the rendered face effect not behaving like other face effects
   -- bug (keith)  Address start/end schedule time not saving correctly on OSX
2018.37 October 21, 2018
   -- bug (keith)  Redevelop EDM RDS playlistitem to use radiotext
   -- bug (keith)  Face and state effect don't recognise backward ranges
   -- bug (keith)  Force xSchedule to only recognise HH:MM:SS time format
   -- enh (keith)  Add export to an really large image file the wiring diagram ... useful for really dense models
   -- enh (keith)  Add bulk controller upload ... upload to multiple controllers (of the same type).
   -- bug (keith)  Fix cant add groups into groups
   -- enh (keith)  Allow subbuffer to be moved (use ctrl/shift-ctrl to constrain axis of movement)
   -- enh (keith)  Add ability to apply a subbuffer to selected effects - kind of like bulk edit
   -- bug (keith)  Effects copied/imported from groups using group only buffer styles don't render on models when placed there
2018.36 October 12, 2018
   -- end (will)   Ability to hide header/footer on buttons plugin. Add &args=noheader to URL.
   -- enh (gil)    Increase the packet sized for the LOROptimisedOutput.
   -- enh (gil)    Fix horizontal and vertical distribute in Layout panel.  Fixes #1219.
   -- enh (gil)    Prevent creating LOR data unless sequence timing is a multiple of 10ms.  Add warnings.
   -- enh (keith)  Add Vixen 3 timing mark import
   -- enh (keith)  Add higher priority schedules optional graceful interruption of lower priority schedules
   -- enh (keith)  Add to check sequence check on matrix face effect files
   -- bug (keith)  Fix F48 upload where controller in as new state and attempting to put 1074 or more pixels on bank 1 & 2.
   -- bug (keith)  Fix audio controls don't work properly on tabs other than sequencer
   -- enh (keith)  Add to face effect the ability to auto hide when no phoneme to single
   -- enh (keith)  Add more bulk edits to the face effect
   -- enh (keith)  Add more rotozoom and subbuffer checks to face effect ... these produce unexpected results that are hard to troubleshoot
   -- enh (keith)  Make it easier to define virtual matrices 1:1 sized with pixel data and even multiples thereof
   -- enh (keith)  Add a fade in/out play list item
   -- enh (keith)  Add manual video speed control to video effect including the ability to play it backwards
   -- enh (keith)  Add a "Set playlist as background" api to xSchedule
   -- bug (keith)  Handle overlapping models better in falcon/esp pixel stick uploads
   -- enh (keith)  Smart entry of start channel and controller connection. #1 -> #1:1, 1 -> WS2811:1
   -- enh (keith)  In start channel dialog allow user to filter to just models in current preview
   -- enh (keith)  Get ESC working as shortcut to cancel most dialogs
2018.35 October 6, 2018
   -- enh (gil)    Add New Timing Track from row header right-click now provides all timing options. Fixes #377.
   -- bug (gil)    Fix multi-arch nodes incorrectly ordered when using Blue Square start.  Fixes #1162 and Fixes #454.
   -- bug (gil)    Fix WiringDialog scaling for some Windows machines.
   -- bug (gil)    Prevent Arches and Icicles from drawing upside down when drawn right to left.
   -- enh (gil)    LOROptimised output nows skips frames when not keeping up.  Prevents blue screen crash when turning off output to lights.
   -- bug (gil)    Fix LOROptimised output to work in Test dialog. Fixes #1262.
   -- bug (dkulp)  Fix issues if rgbeffects.xml does not contain ALL the colors we need to things get mapped to black
   -- bug (dkulp)  When clicking/modifying a single effect, the restricted node range was not being used correctly
                   (all channels above the max node in the model were not considered restricted)
                   Fixes #1253
   -- bug (keith)  Flag as an error attempts to define too many input universes on a falcon controller
   -- bug (keith)  Prevent crash in animation sequence if length is changes to zero
   -- bug (keith)  Fix ctrl-shift-right arrow doesn't always stretch effects
   -- bug (keith)  Guarantee minimum size of the display elements panel when it is shown
   -- bug (keith)  Fix vixen export generates illegal sequence timing
   -- enh (keith)  Add new xSchedule APIs for stopping event playlists: Stop event playlist; Stop event playlist if playing step
2018.34 3D Beta October 1, 2018
   -- enh (gil)    Add obj file import
   -- bug (gil)    Lots of fixes
2018.34 October 1, 2018
   -- enh (dkulp)  On OSX, detect if default audio device changes and move audio to new device
   -- enh (dkulp)  Update to newer standard dictionary
   -- bug (dkulp)  Loading dictionaries did not properly use utf-8.  Fixes #1247
   -- enh (scott)  Added Start and End Time to the Lyric Dialog
   -- bug (scott)  Fixed Missing Nodes on Node Select Dialog
   -- bug (keith)  Fix row paste always pastes to top row on model
   -- bug (keith)  Get artnet timecode working correctly proven with TimeLord and Madrix
   -- enh (keith)  Add MIDI timecode support
   -- bug (keith)  Fix crash on right click after undoing an effect add
   -- bug (keith)  Fix start channel dialog does not list outputs which use ids rather than universes
   -- bug (keith)  Fix on effect cycles should be 0-100 not 0-1000 and value of 10 acted like 1
   -- bug (keith)  Fix models list export gets end channel on model groups wrong
   -- bug (keith)  Fix xSchedule rapid restart on linux unable to listen on http port
   -- bug (keith)  Fix custom model background image does weird colour things when you dim it down
   -- enh (keith)  Add 7 pixel fonts to text effect
   -- bug (keith)  Allow blink for rest and off static phonemes
   -- bug (keith)  Fix some quirks in xSchedule date handling that only show up in months with less than 31 days
2018.33 September 7, 2018
   -- enh (dkulp)  Bunch of performance updates for Test dialog on OSX
   -- bug (keith)  FPP Upload extra logging and delete any existing file before upload
   -- bug (keith)  Change the way we detect falcon model/version as 2.01 firmware broke it
   -- bug (keith)  Fix hang selecting model group on layout panel
2018.32 September 6, 2018
   -- enh (scott)  Added Node Select Dialog to the SubModel, Faces, and State Dialogs
   -- enh (scott)  Added import/export to colour manager dialog
   -- bug (keith)  Fix output to lights incorrect for grouped universes
   -- bug (keith)  Some fixes for text effect
   -- bug (keith)  Fix value curves not resetting on some effects
   -- bug (keith)  Fix falcon upload serial validation incorrect
   -- bug (keith)  Fix screen wont update after exporting preview to video
   -- bug (keith)  Fix some number pad keys don't work
   -- enh (keith)  Consolidate render cache folders
   -- enh (dkulp)  Bunch of performance updates for Test dialog on OSX
2018.31 3D Beta August 30, 2018
   -- enh (gil)    Add images and grids to layout
   -- enh (gil)    Add easier way to invert trees
   -- bug (gil)    Disable some 3d items when in 2d
2018.31 August 30, 2018
   -- bug (dkulp)  Fix performance regression for "Sparkles reflect music"
   -- bug (gil)    Fix timings not snapping when not zoomed in. Fixes #1221.
   -- enh (scott)  Added Submodel Draw Dialog
   -- enh (keith)  Rewrite falcon and espixel stick upload to reduce edge cases which cannot be supported for upload
   -- bug (keith)  Fix window frame model wont accept anti-clockwise setting
   -- bug (keith)  Insert gap for node in custom model renumbering all nodes
   -- bug (keith)  When start channel on model changed models in groups can become invalid causing crashes on layout panel
   -- bug (keith)  xCapture does not handle properly when it cant listen for e131/artnet
   -- bug (keith)  Fix xSchedule lags every second on Linux ... these changes will also improve all platforms when the on second processing runs
   -- enh (keith)  Replace model prompts to copy replaced model start channel if it doesn't look like you have set the start channel on the replacement model
   -- bug (keith)  Fix zoom link
   -- bug (keith)  Fix non ip outputs not listed in the universe dropdown on start channel dialog
   -- bug (dkulp)  Fix esc to stop render on osx
   -- bug (dkulp)  Fix occasional crash when changing show directories
2018.30 August 22, 2018
   -- bug (kevin)  Fix house preview video export on Windows with high DPI settings
   -- enh (keith)  Change the colour panel update button to purely apply the colour palette. All other colour settings are ignored.
   -- enh (keith)  Add the ability to change custom model background image from within the model data dialog
   -- enh (keith)  Strip out some buttons and replace them with a right click menu with enhancements to create
                   gaps in node numbers, compress node numbers to remove gaps, trim blank rows and columns from the edges of the
				   model and a shrink function that helps reduce the model size.
   -- enh (keith)  Give user some ability to influence minimum model size generated by Generate Custom Model and prevent some over
                   optimised custom models being generated
   -- fix (keith)  Crash if use home/end while editing a field
   -- fix (keith)  Hang or slowness opening a new sequence after previously rendering another sequence
2018.29 August 19, 2018
   -- enh (keith)  Add key binding support for presets
   -- enh (keith)  Add a link to zoom in the help menu
   -- bug (keith)  Unable to enter text in controls if key is assigned in keybindings
   -- enh (keith)  Add warning about setting which stops FSEQ save as that setting is undesirable for most users
   -- bug (keith)  Fix a bunch of minor errors in xSchedule - play random step in specified playlist, some play list item names not saving, high midi cpu
   -- enh (keith)  Allow timing mark creation after pausing playback
   -- bug (keith)  Fix timing tags don't load properly
   -- bug (keith)  Fix morph text control is not editable
2018.28 3D Beta August 12, 2018
   -- bug (gil)    Fix 3D buffer for nested groups
   -- bug (gil)    Fix pan to account for grid rotations in house view
   -- enh (dkulp)  Enhance mouse/trackpad on OSX
2018.28 August 18, 2018
   -- bug (gil)    Fix VSA Imports with multiple event types and treat DMX relays as 8bit effects.
   -- enh (keith)  Extend significantly the keybindings available functions
   -- bug (keith)  Fix text boxes with sliders where zero is not valid are hard to edit
   -- bug (keith)  Fix single segment polyline allows individual start channels
   -- bug (keith)  Fix video drift in generate custom model video watcher
   -- bug (keith)  Yet another tweak to the animated GIF reader
   -- bug (keith)  Fix right click after selecting models to create a group omits selected model
   -- bug (keith)  Minimise render events when bulk editing
   -- bug (keith)  Direct user in check sequence how to fix legacy value curves
   -- bug (dkulp)  Fix reading back from render cache changes pixel colour
2018.27 3D Beta August 12, 2018
   -- enh (gil)    xLights 3D
2018.27 August 11, 2018
   -- bug (dkulp)  Fix some issues with subbuffers
   -- bug (dkulp)  don't double up the windows menu
   -- enh (keith)  Allow matrix faces to be dropped on submodels
2018.26 August 10, 2018
   -- enh (dkulp)  Use parallel algorithms in Twinkle/Butterfly/Plasma/Warp/Meteors effects to speed them up
   -- bug (keith)  Fix variable subbuffers not calculating correctly
   -- bug (keith)  Fix yet more animated gif issues
   -- bug (keith)  Fix some effects are wrapping pixels (eg picture, pinwheel)
   -- bug (keith)  Add back start parameter to sine based curves because timeoffset was not quite equivalent
   -- enh (kevinp) New save icons
2018.25 August 8, 2018
   -- bug (keith)  Prevent a crash due to value curves on sub-buffers. This stops the crash but the subbuffer is wrong ... further fixes to come.
   -- enh (keith)  Remove flag that prevents effects grid paste until a copy has been performed
   -- bug (keith)  Fix non default perspective with floating windows does not load correctly
   -- bug (keith)  Fix a problem with transparent gifs
   -- bug (keith)  Fix a few bugs with render cache
   -- bug (keith)  Fix check sequence flags controller upload issue on multicast universes ... these should be ignored
   -- bug (keith/scott)  FPP Connect unable to find/upload sequences if not saving FSEQ files in show folder
   -- enh (scott)  Added Refresh Button to FPP Connect USB Upload
2018.24 August 5, 2018
   -- enh (keith)  Allow exclusion of channels in xFade from blending/brightness control
   -- enh (keith)  Add a sample on warp mode which samples a pixel from the underlying layer and sets the whole model to that colour
   -- enh (keith)  Add optional render cache ... likely to improve average render times by 30-50%.
   -- bug (keith)  Fix initialisation of model individual start channels
   -- bug (keith)  Rework unmask again as the last rework only handled pure black ... this version restores the old unmask which doesn't really unmask and adds a true unmask.
   -- enh (keith)  Add ESC key to cancel of render all
   -- bug (keith)  Fix transparent GIFs not rendering correctly
   -- enh (keith)  Add "Play specified playlist if not playing <playlist name>" and "Play specified playlist if nothing playing <playlist name>" commands to xSchedule
   -- enh (dkulp)  Introduce multithreading to layer blending
2018.23 July 27, 2018
   -- enh (gil)    Increase DMX Effect/Panel to 40 channels.
   -- enh (gil)    Changed the default channel assignments for the Skulltronix Skull option so that you can set
                   the start channel to 1 and the Power channel will start at channel 8, Jaw at 9, etc.
   -- enh (gil)    Added the default Node Names for the Skulltronix Skull option.
   -- enh (gil)    Timing duration is now read from VSA file instead of a hard-coded 33ms.
   -- bug (gil)    Corrected VSA import issue when there are inactive tracks mixed in with active tracks.  Fixes #1190.
   -- bug (keith)  Add in bulk edit for canvas mode
   -- bug (keith)  Fix bug which crashed house video export when writing last frame
   -- bug (keith)  Fix audio/video sync for some video files
   -- bug (keith)  Fix crash due to some GIF files where first frame was not the full frame size. Include support for GIF backgrounds
   -- bug (keith)  Fix check sequence Canvas mode checks
   -- enh (keith)  Add Y offset to wave effect
   -- bug (keith)  Fix square wave to make it recognise thickness and mirror properties
   -- bug (keith)  Fix strobe and meteors incorrectly drawing when responding to music but no music present
   -- bug (keith)  Fix Add DDP not disabled when outputting to lights
   -- bug (keith)  Fix align models does not save positions correctly.
   -- bug (keith)  Fix audio frame data not always prepared correctly leading to audio dependent effects not working as expected
   -- enh (keith)  Add MIDI scan to setup of MIDI triggered events in xSchedule
   -- bug (scott)  Fix crash if new group name is blank
   -- bug (cjd)    Revert to GTK2 due to ongoing instability in GTK3/wxWidgets support
2018.22 July 8, 2018
   -- bug (scott)  Fix FSEQ not being saved by default in same folder as xml
   -- enh (scott)  Add custom papagayo dictionary editor
   -- enh (keith)  Remove empty vendor model categories
   -- bug (keith)  Fix memory leak impacting xSchedule and xlights
   -- enh (keith)  Allow user to decide how interactively crashes are handled
   -- bug (keith)  Fix windows upgrade detection because xLights website page it was using is broken
   -- enh (keith)  Add a library of DMX fixtures
   -- bug (keith)  Fix bug in spirals effect rotation value curve
   -- enh (keith)  Add checksequence check for spurious use of canvas blend style which slows down rendering
   -- bug (gil)    Fix VSA import.
2018.21 July 4, 2018
   -- bug (scott)  Fix Roto-Zoom reset bug
2018.20 July 2, 2018
   -- bug (keith)  Fix warp treatment flips to constant
   -- bug (keith)  Fix bulk edit crash
   -- bug (keith)  Fix changing audio speed starts audio playback
   -- enh (scott)  Enhance submodel copy to prompt about overwriting like named submodels
   -- bug (scott)  Prevent blank submodel names
2018.19 June 30, 2018
   -- bug (keith)  Fix warp treatment flips to constant
   -- bug (keith)  Fix bulk edit crash
   -- bug (keith)  Fix submodel dialog crash
   -- bug (keith)  Fix mapping to strands does not work after a model is renamed
   -- bug (keith)  Fix unmask blend style so it doesn't turn black areas white
   -- bug (keith)  Change the way rendering is triggered to try to stop some crashes
   -- enh (keith)  Add value curves to marquee
   -- enh (keith)  Fully script the windows release process
   -- enh (keith)  Optimise xSchedule performance
2018.18 June 25, 2018
   -- bug (keith)  Fix audio wont stop playing
   -- enh (keith)  Add support for Falcon F48
2018.17 June 24, 2018
   -- enh (kevin)  Add 'circular swirl' to warp effect
   -- enh (kevin)  Add 'drop' to warp effect
   -- enh (kevin)  Add 'wavy' to warp effect
   -- bug (kevin)  Fix issues with warp effect panel
   -- bug (keith)  Some xFade fixes and improved UI information
   -- bug (keith)  Fix layered RGB star doesn't render more than 1 node
   -- enh (keith)  Add display of % location in morph effect assist
   -- bug (keith)  Some tweaks to wiring view display
   -- enh (keith)  Some jukebox configuration improvements
   -- enh (keith)  Add advance button to xFade for shows on rails
   -- bug (keith)  Fix blank city name sunrise/set crash
   -- enh (keith)  Recognise sunup and sundown and similies for sunset and sunrise
   -- enh (scott)  Submodel import should skip submodels that already exist in the model
   -- enh (keith)  For multiple firing schedule make the fire time relative the the schedule start time.
   -- bug (keith)  Fix some issues with multi-firing schedules.
   -- enh (keith)  Add activate and deactivate a schedule via an API call to xSchedule
   -- enh (keith)  Add a microphone playlist item to xSchedule to set pixel colors in real time
   -- enh (keith)  Add ability to make black transparent in video, pictures and matrix faces
   -- bug (keith)  Fix servo effect values incorrect
   -- bug (keith)  Fix bulk edit doesn't work on effects with two word names
   -- bug (keith)  Loading sequence to non existing model wont map to strands or submodels
2018.16 June 17, 2018
   -- enh (kevin)  Added warp effect for use with canvas blend style
   -- enh (scott)  Added Vixen FSEQ File Support to xSchedule
   -- enh (dkulp)  Scale bitmap buttons for locks/value curves, color panels on HiDPI screens
   -- enh (dkulp)  New high-res lock/unlock icons
   -- enh (dkulp)  Initial support for using system background colors
   -- bug (dkulp)  TouchBar forward/back buttons reversed
   -- enh (scott)  Added "Copy", "Import from Model", and "Import from File" Buttons to the Submodel Dialog.
					- Copy The Currently Selected SubModel.
					- Import SubModels from another Model.
					- Import SubModels from a xmodel files.
   -- enh (keith)  Add xFade
   -- bug (keith)  Windows update does not directly download zip file
   -- bug (keith)  xCapture doesn't filter out non ArtDMX packets
2018.15 June 10, 2018
   -- enh (dkulp)  Big speedup for Canvas mode
   -- enh (keith)  Add PixelEditor import
   -- bug (keith)  Fix LMS CCR imports not working correctly
   -- bug (keith)  Fix delete and select on a node row not working
   -- enh (keith)  Add ability for xSchedule to trigger a jukebox item
   -- enh (keith)  Add the ability to shift a set of selected effects by a given time interval
   -- enh (keith)  Add collapse all layers to row heading right click menu
   -- enh (keith)  Add parent model to submodel names when they are in groups and the group is expanded in sequencer
   -- enh (keith)  Add the ability to time shift value curves
   -- enh (keith)  Add -o command line option to turn on output to lights when xlights starts
   -- bug (keith)  Fixes for crashes in xLights, xSchedule and xCapture
2018.14 May 23, 2018
   -- enh (keith)  Add select/copy/paste all effects on a model
   -- enh (keith)  Add windows check for newer version
   -- enh (keith)  Add search to sequences/lyrics download
   -- enh (keith)  Add CSV format to Glediator effect. Each line is a frame, each column is a channel.
   -- bug (keith)  Fix image model does not work in a group correctly
   -- enh (keith)  Add ability to syncronise shape effects
   -- bug (keith)  Add fixes to keep available render buffers correct
   -- enh (keith)  Add white channel support to the DMX model
   -- enh (keith)  Add video render caching to eliminate the need to continually shrink large videos
   -- enh (keith)  Add support for sunrise and sunset to schedule
   -- bug (keith)  Fix some anomalies with resetting schedules
   -- bug (scott)  Fix for new sequence files saving in the FSEQ folder not the show folder
   -- enh (scott)  Added File Read option to Text Playlist Item in xSchedule
2018.13 May 12, 2018
   -- bug (scott)  Fix an issue with xml/fseq saving
   -- bug (keith)  Fix music download doesn't show artist tag
   -- enh (keith)  Add ability to periodically schedule a playlist within a time interval - hourly, half hourly, etc
2018.12 May 11, 2018 - withdrawn
   -- enh (dkulp)  Support for 4 channel pixel strings (RGBW variants)
   -- enh (dkulp)  Allow specifying how to handle mapping of RGB -> RGBW
   -- bug (dkulp)  Hostnames were not usable in several places, had to use IP addresses
   -- enh (dkulp)  FPP Connect - upload output settings for DDP outputs
   -- bug (dkulp)  Fix crash if uploading zig zag settings to FPP controller
   -- enh (scott)  Added Folder Setting dialog for Media, FSEQ, Backup, and Alt Backup Directories.
   -- bug (keith)  Fix tendril crash when movement was set to zero
   -- bug (keith)  Upgrade some check sequence warnings to errors
   -- bug (keith)  More fixes for images on resizing buffers
   -- bug (keith)  Fix test dialog issues for model groups that contain submodels
   -- bug (keith)  Fix import from sequence with nothing to import causes crash
   -- enh (keith)  Add value curves to snowflakes and meteors
   -- bug (keith)  Fix discard on exit prompts for recovery when sequence next opened
   -- bug (keith)  Fix bulk edit not working on some effects
   -- enh (keith)  Add the ability to drop an image, video or glediator files onto grid and create the right effects
   -- enh (keith)  Rework canvas blend style to support canvas and blend at the same time
   -- bug (keith)  Fix a bunch of problems with output id's such as not saving them, missing on LOR etc
   -- enh (keith)  Add the ability to save and load face mappings
   -- bug (keith)  Get xCapture working correctly with multicast data
   -- enh (keith)  Rework PJLink. Existing PJLinks will stop working replaced by a new projector play list item which
                   knows how to control many more projector models
   -- enh (keith)  Add support for sparse submodel buffers and label rows more clearly
   -- bug (keith)  Some fixes to auto custom model that significantly improve the model generated ... reduce the
                   number of missed pixels and the ability to process poor video
   -- enh (keith)  Add jukebox
   -- enh (keith)  Add lyric track and sequence download
   -- enh (keith)  Add random value curve setting. Convert to custom once you are happy with your curve or it will
				   keep changing every time the effect is rendered.
2018.11 Apr 14, 2018
   -- enh (dkulp)  Add ability to upload string/dmx configs for various FPP v2 Capes/Hats
   -- enh (dkulp)  Update FPP to support v2 channel input and output configs
   -- enh (keith)  Add the ability to select the layers included in Canvas blend mode
   -- bug (keith)  Rework the value curve fix
   -- bug (keith)  Some fixes to FPP event sending in xSchedule
   -- bug (keith)  Fix test mode RGB modes
   -- bug (keith)  Fix some xSchedule crashes
2018.10 Apr 9, 2018
   -- enh (cjd)    Add -c/-x options to run xCapture/xSchedule via main binary on linux
   -- bug (dkulp)  The "skips" in the Marquee effect should be "clear", not black
   -- enh (keith)  Add xSchedule event which fires when a controller becomes unreachable
   -- bug (keith)  Some fixes to value curves with divisors
   -- bug (keith)  Fixes to x/y shape position value curves
   -- bug (keith)  Fix for some issues with scaled pictures in resizing buffers
   -- enh (keith)  Add the ability to delete timeline tags
   -- enh (keith)  Add the ability to schedule a playlist every n days
   -- enh (keith)  Add a play list item that sets a range of pixels to a particular colour
   -- enh (keith)  Add controller upload support for ECG-P12S firmware 3.3
   -- enh (keith)  Add controller upload for ESPixelStick firmware 3.0
2018.9 Apr 1, 2018
   -- enh (gil/dkulp)  Added ability to render a waveform range selection.  Fixes #489.
   -- enh (keith)  Add xCapture for capturing E131/ArtNET data from the network and save it to a FSEQ or ESEQ file
   -- bug (scott)  Added 3D fade to Single Strand Effect for Rainbow Pallet
   -- enh (scott)  Added Search Box to Effect Preset Dialog
   -- bug (dkulp)  Fix some window resizing issues
   -- bug (gil)    Fix OpenGL not rendering correctly on some windows installs
   -- bug (keith)  Fix fast start audio does not look like it saved
   -- enh (keith)  Add xCapture tool for capturing e131/Artnet network data and save it to a FSEQ
   -- enh (keith)  Optimise matrix face rendering
   -- enh (cjd)    Move Linux to GTK 3
2018.8 Mar 16, 2018
   -- bug (dkulp)  "Black" static text labels on Windows
   -- enh (dkulp)  Initial support for HiDPI displays on Windows
   -- enh (gil)    Improve phoneme placement when breaking down words.
   -- enh (keith)  Add a canvas blend mode and a transparent option to the off effect which will often be used together
   -- bug (keith)  Fix FPP event not being recognised by FPP
   -- enh (keith)  Add the option to loop event playlists when you start them
   -- bug (keith)  Stop all doesn't stop event playlists
   -- bug (keith)  Log illegal key bindings
2018.7 Mar 11, 2018
   -- enh (k.mojek) Add display of video when sequencing using a video track for the audio
   -- enh (all)    Move to wxWidgets 3.1.1 on all platforms ... also codeblocks and gcc version upgrades on windows
   -- enh (keith)  Rework test dialog to better support large shows
   -- enh (keith)  Add submodels to test dialog
   -- enh (keith)  Add option to suppress outputs not under test in the test dialog
   -- enh (keith)  Add to xSchedule MIDI events
   -- enh (Keith)  Add to xSchedule an event playlist list. This is an extra list of simultaneously running playlists
				   which when triggered play one step once ... typically in response to an event. These are applied on
				   top of any running playlist or background playlist. Great for on demand elements in your show without
				   interrupting your show.
   -- enh (keith)  Add an Id attribute to DDP/NULL and all the USB output types which can be used with the
				   #universe:startchannel addressing mode as a substitute for the universe ... as long as you keep the id unique
   -- enh (keith)  Add emojies to the shape effect ... in reality these are just characters from fonts ... not full colour emojies
   -- enh (keith)  Add sustain output process ... when a channel goes to zero ... overwrite it with the last non zero value
   -- enh (keith)  Add the ability to control the order in which the x, y and z + zoom rotozoom translations are applied.
   -- bug (keith)  Fix some issues with effect selection
   -- bug (keith)  Some performance fixes which impacted selecting effects and moving effects
   -- enh (keith)  Enhance mapping lots of channels sequentially
   -- enh (keith)  Add spectrogram Peak to VUMeter
   -- enh (keith)  Provide an easy way to replace a model without breaking all the groups it is a member of
2018.6 Feb 06, 2018
   -- enh (gil)    Add LOR event to xSchedule which should work with the Input Pup board.
   -- enh (gil)    Update LOR events to work with multiple cards on one port.
   -- bug (gil)    Fix model list in layout freezing.  Fixes #1090.
   -- enh (gil)    Don't draw DMX head beams as white when color is black unless in Layout.
   -- bug (gil)    Fix DMX Flood not drawing in correct location.  Fixes #1092.
   -- enh (dkulp)  Bunch of updates to DDP output to allow future enhanced output to FPP
   -- bug (dkulp)  Fix some render issues and crashes that occur after making changes to setup tab
   -- enh (scott)  Add Effect Select Panel.
   -- enh (keith)  xSchedule will now stop at the end of the FSEQ if it is shorter than the audio file
   -- bug (keith)  Fix vendor model download only allows model to be used once
   -- bug (keith)  When layout panel first opened models wont select
   -- bug (keith)  Fix 172.17.*.* - 172.31.*.* IP addresses incorrectly reported as internet routable
   -- enh (keith)  When configuring screen map in xSchedule show the area being mapped
   -- enh (keith)  Increase Value Curve/Colour curve resolution from 40 to 100
   -- enh (keith)  Improve the speed of number aware sort
   -- enh (keith)  Rework the window frame model to allow it to go CW and CCW. This may break some existing models
   -- enh (keith)  Check sequence to detect effects dropped on strands, submodels and models with no nodes
   -- bug (scott)  Fixed FPP Connect USB Drive Upload
2018.5 Feb 05, 2018
   -- enh (gil)    Added a new LOR output that allows xLights to control CCRs, Pixies, and other
                   existing controllers to run on a LOR network (LOR USB dongle).
   -- bug (gil)    Protect against crash if user sets DMX flood light channel values higher than total changes.
   -- bug (dkulp)  Fix brightness if contrast isn't set
   -- enh (keith)  Add matrix face downloader
   -- enh (keith)  Add the ability to lock effects. This will prevent moving and resizing only.
   -- bug (keith)  Fix face centred not working all the time
   -- bug (keith)  Fix brightness slider does not work with pictures effect
   -- enh (keith)  Add some course grained volume controls to Audio menu
   -- enh (keith)  Add waveform colours to the colour manager
   -- enh (keith)  Add a screen mapper playlist item ... in theory this lets you do real time video on an element ... amongst other things.
2018.4 Jan 30, 2018
   -- bug (will)  Fixed xScheduleWeb to display proper version number
   -- bug (dkulp)  Fix convert to effects not recognizing down ramps that hit the last frame
   -- bug (dkulp)  Fix problems with LOR v4 lcb only outputting 100% effect values
   -- bug (dkulp)  Convert To Effects only working for first node in model
   -- bug (dkulp)  CandyCane tops are chopped off in Model preview
   -- bug (dkulp)  CheckSequence erroneously flags 172.16 networks as broadcast. Fixes #1084
   -- bug (dkulp)  Fix problem with individual start channels on strings cannot self reference
   -- enh (craig)  Randomize only effect settings with a lock available.
   -- bug (keith)  Fix the arches model when start is set to the blue square
   -- enh (keith)  Force xtimging import to round down timing mark times if they occur between frames
   -- enh (keith)  Add the ability to include multiple timings in a single xTiming file
   -- enh (keith)  Add event triggers to scheduler which trigger command execution. Triggers available:
						- On a channel value in the running sequence
						- On a channel in a received E131 packet
						- On a channel in a received ArtNet packet
						- On a channel in a received DMX packet from a serial connection
						- On a channel in a received OpenDMX packet from a serial connection - this looks like it will be less reliable than DMXModel
						- On a OSC message
						- On a FPP event
	-- enh (keith) Add ability to send a FPP Event in a playlist step
	-- enh (keith) Add ability to send to a serial device in a playlist step
	-- enh (keith) Rework FPP/OSC remote so it works correctly with new event model and add ARTNet timecode support
	-- enh (keith) Add magic fixes for overlapping channels and illegal start channels
2018.3 Jan 21, 2018
   -- enh (dkulp)  Bunch of render optimizations, particularly for groups with large number of models
   -- enh (dkulp)  Bunch of startup optimizations - don't load rgbeffects.xml twice
   -- enh (scott)  Added Ctrl-A to Selects all Effects in a Sequence.
   -- enh (scott)  Add Select Row Effects to Sequencer Row Right Click Menu.
   -- enh (scott)  Added Support for Sandevice Firmware 5
   -- enh (gil)    Added a DMX Flood option to the DMX Model.
   -- enh (gil)    Allow the rotate handle to Spin the DMX Model when in 3D mode.
   -- bug (gil)    Fixed a LOR S4 clipboard export model issue.
   -- enh (keith)  Add an image model
   -- bug (keith)  Fix listbox column sizing in the model group panel
   -- bug (keith)  Fix issues with mapping dialog
   -- bug (keith)  Fix performance issue with setup tab when contains large number of universes
   -- enh (keith)  Fix default model start channel to always be 1 channel after the model with the highest ending channel
   -- bug (keith)  Fix some text controls not updating associated sliders
   -- enh (keith)  Display in tooltip current effect being rendered in render progress dialog ... if it gets stuck you will know
                   which effect is to blame
   -- enh (keith)  Fix #1010 - If user cancels nominating a valid show directory xLights should exit.
   -- bug (keith)  Fix xSchedule incorrectly calculates virtual model end when checking if it fits within channel range
   -- bug (keith)  More improvements to number aware sorting which was not handling number sorts where more than the number
                   component were different
   -- bug (keith)  Circular model start channel references was hanging xLights
2018.2 Jan 14, 2018
   -- enh (dkulp)  Add House/Model preview and play controls to TouchBar
   -- update (dkulp) Change minimum OSX requirement to 10.10
   -- enh (dkulp)  Support exporting model to LOR S5 clipboard file
   -- enh (k.mojek) Add "Export House Preview Video" to File menu
   -- enh (gil)    Add Snap to Timing Marks feature.  Turn this on in Settings.  Only works for single effect resizing.  Fixes #132.
   -- enh (gil)    Make snap to timing mark feature temporary toggle using control key.
   -- bug (gil)    Prevent error message when importing ISEQ data layer not in show directory.  Fixes #1047.
   -- bug (gil)    Make sure all models are available when creating new Views.  Fixes #1049.
   -- bug (gil)    Fix Bulk Edit crash when right-clicking a TextEdit box.
   -- enh (gil)    Correct certain scenarios of right/left arrow move/stretch operations and simplify Single vs Multiple effect movement.
   -- bug (gil)    Fix PolyLine crash for case of 3 Channel RGB with Individual Start Channels set.
   -- bug (keith)  Fix eseq frame size rounded by 4 when it shouldn't be.
   -- bug (keith)  Newly created schedule doesn't play immediately
   -- bug (keith)  Reset schedule does not reset
   -- enh (keith)  Add a reset all schedules function to xSchedule
   -- bug (keith)  Fix cant double click on first song in a playlist to jump to it
   -- enh (keith)  Add number aware model name sorting (layout, display elements, import effects)
   -- enh (keith)  Enhance handling of invalid mappings loaded from mapping files so mappings are not lost when saved and invalid mappings are highlighted
   -- enh (keith)  Add download of vendor custom models from licensed vendors
   -- enh (will)  Added Controller Status to xSchedule WebUI
   -- enh (scott)  Added Right Click Ping controller Option To Setup Tab
2018.1 Jan 04, 2018
   -- enh (k.mojek) Add "Print Layout" option to Layout popup menu
   -- enh (dkulp)  Add WRGB 4 channel node type
   -- enh (keith)  Add xSchedule output process which can scale pixels from 3 channel to 4 channels per pixel
   -- bug (keith)  Fix xSchedule output processors not re-enabling
   -- enh (keith)  Add bulk edit of mulitple model controller connection, dimming curve and preview
   -- enh (keith)  Add create group of models by right clicking on a layout after selecting the models to be in the group
   -- bug (keith)  Display node number relative to string in node layout rather that absolute node number within model
   -- enh (keith)  Add to matrix face dialog attempt to fill in all the rows based on the pattern of an entered row
   -- bug (scott)  Fix bars effect rounding problem
   -- enh (k.mojek) Add ability to save the layout as an image via right click menu
   -- bug (gil)    Last release broke DMX effect panel label updating.
   -- bug (craig)  Fix crash of randomize effect after effect is deleted. Fixes #1052
   -- bug (will)  Fixed xSchedule webui timeout, it will now reconnect automatically.
2017.45  Dec 29, 2017
   -- enh (craig)  Added a randomize current selected effect button
   -- bug (craig)  Fixed the create random effects to generate random settings
   -- enh (keith)  Add Open Sound Control (OSC) Master and Remote sync
   -- enh (keith)  Add Open Sound Control (OSC) sender as a play list item
   -- enh (keith)  Show layers and layer number in row headings
   -- enh (keith)  Expand the sizes of matrices xSchedule supports
   -- bug (keith)  Fix audio override checkbox not set correctly when loaded
   -- enh (keith)  Show controller status in xSchedule
   -- bug (keith)  Fix audio send to specified device
   -- enh (keith)  Add effect setting bulk editing to almost all settings
   -- enh (keith)  Add horizontal and vertical stack buffer styles for groups ... makes text on groups possible.
   -- enh (keith)  Show schedule end time when a schedule is running (including any extensions)
   -- enh (keith)  In xSchedule text optionally suppress text rendering if the text to be rendered is blank ... letting the underlying effect through
   -- enh (keith)  Add to VU meter "Level Color" and "Timing Event Pulse Color"
   -- bug (gil)    Fix Shift Effects not shifting effects in collapsed layers. Fixes #1046.
   -- bug (gil)    Fix Show All Effects not expanding collapsed model layers or strand layers.  Fixes #1045.
2017.44  Dec 19, 2017
   -- bug (dkulp)  Fix keyboard handling of numbers
   -- enh (keith)  Add visual clue that xSchedule is serving web requests
   -- bug (keith)  Add handling of mixed audio rates on windows/linux
2017.43  Dec 18, 2017
   -- bug (gil)    Fix bars gradient option when not alpha blending.  Fixes #1032.
   -- bug (keith)  Fix xSchedule text on single string vertical matrices
   -- bug (keith)  Fix xmap node level mappings not loading from file correctly
   -- enh (keith)  Add option to turn model blending off by default in new sequences
   -- enh (keith)  Detect low proportion duplicate nodes in custom models indicating possible accidental duplication of nodes
   -- enh (keith)  Ctrl-S works on all tabs as a shortcut to the relevant save button
   -- enh (keith)  Warn users when they switch to a backup show folder. Also highlight in check sequence.
2017.42  Dec 14, 2017
   -- enh (chris)   Enable automatic application of patch for wxwidgets on linux
   -- bug (chris)   Separate patches for wxwidgets on linux so can handle fixes for http://trac.wxwidgets.org/ticket/17683
					and https://trac.wxwidgets.org/ticket/18021
   -- bug (dkulp)  Fix crash when actions generate a ton of render events for the same models (like mouse resize of effect)
					This fixes #1024
   -- bug (keith)  Reverse a change which may have broken some picture effects
   -- bug (keith)  Fix reopen audio device when it changes
   -- bug (keith)  Fix restricted read from FSEQ doesn't work for offsets
   -- enh (keith)  Add a way to manually force log render state
2017.41  Dec 11, 2017
   -- bug (dkulp)  Delay actually deleting effects and layers until the end of render to avoid potential crashes.
   -- bug (dkulp)  Work on making sure selections are cleared before effects are deleted
   -- enh (dkulp)  Add undo to Delete Row Effects
   -- bug (keith)  Fix some problems with vertical matrices in xSchedule
   -- enh (keith)  Add some fast deleting of effects on a layer/model
   -- bug (keith)  Fix a possible race condition when updating effect settings
   -- enh (keith/dkulp) Add model as a pixel render style for model groups ... treats models within the group as a single pixel
   -- bug (keith)  Prevent warnings about autosave newer than sequence after batch render
   -- enh (keith)  Save selected batch render items between runs of batch render
   -- enh (keith)  Add selectable audio devices on windows for xSchedule
   -- bug (craig)  Fix the output of the xScheduleData/GetModels.dat file format as json
   -- enh (scott)  Added Tree, Crucifix, Present, Candy Cane, Snowflake, and Heart Shapes to the VU Effect
   -- bug (dkulp)  Fix dimming curves that are different for R/G/B
   -- bug (dkulp)  Fix debug build popups on rendering on Linux
   -- bug (dkulp)  Fix crash on text effect on Linux (fonts still look crappy)
   -- enh (scott)  Warn windows 64 bit computer users using 32 bit that they should move to 64 bit
   -- bug (scott)  Fix FPP connect wont upload sequences in subdirectories
   -- bug (keith)  Fix xSchedule text properties not working in API
   -- bug (keith)  Fix a number of problems with xSchedule channel constraints not saving
   -- bug (keith)  Fix xSchedule fast start audio does not respect per song volume setting
   -- bug (keith)  Fix virtual matrices not working reliably or correctly in xSchedule
   -- enh (keith)  Allow play list buttons like play schedule to work even if a schedule for that playlist is selected
   -- enh (keith)  Add delete all effects on a layer or a model
   -- bug (keith)  Fix some bugs in calculating pixels for some vertical matrices in xSchedule
2017.40  Dec 05, 2017
   -- enh (keith)  Add support for #universe:startchannel and output:startchannel to all xSchedule fields that define a start channel
   -- bug (keith)  Fix test dialog tooltips don't display correct models
   -- bug (keith)  Remove Master View from the new sequence default views
   -- enh (dkulp)  Add undo to update effect and update palette actions
   -- enh (rich)   Fix xSchedule web doesn't like spaces in some url parameters #1000
   -- bug (scott)  Fix spinner calculate incorrect number of nodes #1008
   -- bug (scott)  Fix bulk render not working with xml files in subdirectories
   -- bug (keith)  Ensure all lights are off when xSchedule closes
   -- enh (keith)  Add active timing marks to Value Curve and Colour Curve dialogs so you can line things up easier
   -- bug (keith)  Fix xSchedule test mode does not display A-B-C-ALL and some other patterns correctly
2017.39  Nov 28, 2017
   -- bug (dkulp) Fix some memory leaks when open/close sequences as well as switching show directories
   -- enh (dkulp) Import Superstar files onto a submodel
   -- bug (dkulp) Fix a render bug which could leave artifacts on models
   -- enh (keith) Begin to introduce output:startchannel and #universe:startchannel addressing into xSchedule.
                  Initially in Matrices and Virtual matrices.
   -- enh (keith) Display window position details to xSchedule for video and image display. Allows simple
                  comparison of values without opening the positioning window.
2017.38  Nov 25, 2017
   -- bug (dkulp) Send render event for effect moves, previously not always sent
   -- enh (dkulp) Add left/right starting locations for candy canes
   -- enh (gil)   Add new Single Color Intensity string type that will react to any color.
   -- bug (keith) F4V3 with expansion board should not limit pixels on any port
   -- enh (keith) Add present shape effect
   -- bug (keith) xSchedule should take duration of a step from the highest priority FSEQ over any audio or video item
   -- bug (keith) xSchedule fpp master was sending too many media sync packets for audio/video only steps
   -- enh (keith) Add timing event bars to vu meter ... like timing event bar but all bars are displayed and colours cycle
   -- bug (keith) Erase background before rendering xyzzy so when playing on top of your show you can see the game
   -- bug (keith) Prevent right click delete of submodels in model list on layout tab
   -- bug (keith) Fix bug with remap and reverse in xSchedule output processors
   -- bug (keith) Flag schedule as dirty if matrices/virtual matrices are edited
   -- bug (keith) Fix crash with single colour spinners and the single line render style
   -- bug (keith) Fix display elements window gets really small and hard to find
   -- bug (keith) Fix problem where FSEQ data is dumped on load due to mismatch in size ... roundto4 problem
   -- bug (keith) Attempt to fix lights not going off when background sequence stops
   -- enh (keith) Allow video in scheduler to fade in and fade out
   -- enh (keith) Allow video looping in scheduler
   -- enh (keith) Add right click on setup to deactivate all unused outputs
   -- enh (keith) Remove xseq as a target for conversions ... there is no obvious reason why people would still be creating them
   -- bug (keith) Fix x/y text location mixed up when it is saved
   -- bug (keith) Fix crash when loading xLight v3.x.x animation sequences
   -- bug (keith) Fix audio preload in xSchedule
   -- enh (keith) Add the ability to abort Generate Custom Model processes by holding down ESC key
2017.37  Nov 14, 2017
   -- enh (dkulp) If dimming curve would map to identity, remove it to avoid the extra processing it entails
   -- enh (keith) Add ESCAPE key abort generate custom model scanning.
   -- enh (keith) Prevent corruption of face info in rgbeffects
   -- enh (keith) Add artnet to fpp connect upload
   -- enh (dkulp) Fix Text effect crash on Windows only if multiple colors are selected.  Fixes #982
   -- enh (dkulp) If updating the palette of settings of a LOT of effects, aggregate the render events.
                  Speeds it up a lot.
2017.36  Nov 13, 2017
   -- enh (dkulp)   Add multicolor support for non-rotated text
   -- enh (dkulp)   Allow user to change number of universes in an e131 multi universe output
   -- bug (scott)   Remove layer count from group lines in preset list
   -- bug (gil)     Fix motion positions for XL fonts in Text effect.
   -- enh (keith)   Move xSchedule video reading to a separate thread ... should make video performance more reliable but will use
                    more of memory
   -- enh (keith)   Add packets per second display to scheduler UI
   -- bug (keith)   Strip duplicate test presets on load ... not sure how they got there but don't load them.
   -- enh (keith)   Add to check sequence check for overlapping effects ... these should not be possible but I have seen some sequences
                    where is has happened.
2017.35  Nov 05, 2017
   -- enh (cjd)     Update checker for Linux and OSX
   -- enh (dkulp)   Set the thread name to the Model name it is rendering, makes it easier to find the right thread in the debugger
   -- enh (dkulp)   Add a warning when loading/creating sequences that would result in massive amounts of memory being used
   -- enh (dkulp)   Add batch render to tools menu
   -- bug (keith)   Fix missing distribute horizontally and vertically in layout
   -- bug (keith)   Force recreation of the render buffer if the frame size changes ... this will then require a full render
   -- bug (keith)   Fix PJLink
   -- enh (keith)   Add the ability to suppress virtual matrices if showing video or images.
   -- bug (keith)   Fix FPP connect not enabling upload when IP address is entered.
   -- enh (keith)   Add an option to shrink the waveform display
   -- enh (keith)   Improve positioning in printing wiring view to avoid cutoff.
   -- enh (keith)   Add keyboard cut & paste to custom model dialog
   -- bug (keith)   Colour curve does not display position tooltip unless dragging
   -- bug (keith)   Fix pixelnet should support 4096 channels.
   -- enh (keith)   Add crucifix to shape effect
   -- bug (keith)   Fix submodel start channels displaying incorrectly as invalid.
2017.34  Oct 27, 2017
   -- enh (keith)   Add a dead pixel colour output process to xSchedule which only blanks a pixel if the colour would be wrong
   -- enh (keith)   Add the ability to disable output processes
   -- bug (keith)   Fix multilight wiring dialog not displaying correctly
   -- enh (keith)   Set model start channel back to one when user selects the end of model model as that is the most common desired resilt
   -- enh (keith)   Some checks to warn if xSchedule and xLights are outputting to lights at once
   -- enh (keith)   Add level pulse colour to VU Meter ... pulses a colour when music reaches specified intensity and changes colour each time it does
   -- bug (keith)   Fix piano timing track setting not loading properly
   -- bug (keith)   Fix xSchedule doesn't respect force local IP
   -- enh (keith)   Add the ability to share xSchedule button definitions
   -- enh (keith)   Add tooltip help to button parameters
2017.33  Oct 15, 2017
   -- bug (gil)     Fix Galaxy not blending when reverse is checked.  Fixes #949.
   -- enh (keith)   Add xSchedule gamma processing on output
   -- enh (keith)   Add FPP master support for pixel sticks and Falcons
   -- bug (keith)   Fix xSchedule play last item only once plays repeatedly
   -- bug (keith)   Fix scrolling sideways using Shift+scroll wheel stops paste using right click menu working
   -- bug (keith)   xSchedule FSEQ, Video and audio don't respect the delay setting
   -- bug (keith)   Fix upload to falcon hang if multistring element goes past configured string ports
   -- enh (keith)   Add check sequence check for non contiguous models on the same controller connection
   -- enh (keith)   Add xSchedule can act as a remote using the csv sync packets (which are routable)
   -- enh (keith)   Add cropping to the video effect
2017.32  Oct 09, 2017 - PRODUCTION RELEASE
   -- bug (keith)   xSchedule crashes clearing the matrix when closing easter egg
   -- bug (keith)   xSchedule does not turn off lights when sequence ends
   -- bug (keith)   Falcon upload incorrect for multi-string elements
   -- bug (keith)   Make splash screen not topmost
   -- bug (keith)   Fix crash importing Vixen 2.x sequences
   -- enh (keith)   Enhancements to check sequence
						- empty model groups
						- submodels with no nodes
						- referenced files outside show directory
2017.31  Oct 06, 2017
   -- bug     (dkulp)  For HLS convert, it was failing to map channel names properly.
   -- enh     (dkulp)  Reset OSX release mode flags for wxWidgets to disable assert popups
   -- bug/enh (dkulp)  On OSX, resample audio to 48000 and use smaller buffers to reduce audio latency (I hope)
   -- bug     (dkulp)  Fix wxAssert warnings when quitting from menu on OSX and using debug build
   -- enh     (keith)  Add more check sequence checks ... checks of effects on strands, submodels and nodes, checks for effects that shouldnt be on nodes
   -- enh     (keith)  Add more check schedule checks ... FSEQ files which don't match networks
   -- bug     (keith)  Fix a stack of crashes and bugs
   -- bug     (keith)  Fix #universe:start channel flagging as invalid
   -- bug     (keith)  Fix rotozoom presets
   -- enh     (keith)  Add remember custom colours in colour selection dialog
2017.30  Sep  29, 2017
   -- enh (scott)   Added Swap Start and End Points to Morph Effect.
   -- enh (keith)   Add printing of wiring view
   -- enh (keith)   Add display of actual time in value curve and color curve dialogs.
   -- bug (dkulp)   Prevent crash when perspective is chosen from menu prior to flipping to the sequencer tab
   -- bug (dkulp)   Fix memory leak in JobPool that was leaking thread objects consuming resources
   -- enh (keith)   Add X and Y axis rotations to rotozoom
   -- bug (keith)   Fix xlights not opening fseq files
   -- bug (keith)   Fix zoom value curve not converting correctly
   -- enh (keith)   Add splash screen courtesy Kevin Pankratz
   -- enh (keith)   Add ctrl-. and ctrl-/ save and restore position in the sequencer
   -- enh (keith)   Add crtl-shift 0-9 which jump to 0%-90% through the song in the sequencer
   -- enh (keith)   Add tags using right click on the timeline ... then use ctrl-0-9 to jump to the tags
   -- enh (gil)     Add built-in bitmap Fonts that match SuperStar.  Not every option works on these fonts but they
                    do allow the color palette to be used.
   -- bug (gil)     Fix Convert for LOR files being offset by 1.  Fixes #916.
   -- enh (dkulp)   Optimize loading of test presets to make xLights start quicker
   -- bug (dkulp)   Hopefully fix crash in CheckSequence if hostname doesn't resolve to local ip address
   -- enh (keith)   Add new Ctrl left and right arrow and Ctrl Shift left and right arrow shortcuts in sequencer to move and expand effects to timing marks or next effect
   -- bug (keith)   Fix problem with allocating too few channels if invalid start channels and not enough channels on setup tab
2017.29  Sep  19, 2017
   -- bug (keith)   Fix regular crash due to slow rendering. This bug was introduced in 2017.28
   -- enh (cjd)     Add option to run xSchedule via xlights on linux (for AppImage usage)
   -- enh (scott)   Custom Model Renumber (#917)
   -- bug (dkulp)   Fix crash when changing show directories
   -- bug (dkulp)   OSX - flipping back and forth between tabs sometimes results in hidden panels appearing and moving to a docked state.
2017.28  Sep  18, 2017
   -- bug (dkulp)   Fix crash on startup when ViesModelsPanel is not visible
   -- bug (scott)   Fixed Divide by zero error if main window is re-sized too small. (#913)
   -- enh (keith)   Add ability to import LAS files ... actually this existed ... it just wasn't selectable
   -- bug (keith)   Add warning if i am importing a datalayer that is bigger than the number of xlights channels
   -- enh (keith)   Add a xSchedule file play list item which can write information about current song etc to a file
   -- bug (dkulp)   OSX - Popup menu in "WiringView" dialog is completely disabled
   -- bug (keith)   Fix crash when change setup and then switch to layout
   -- bug (keith)   Fix crash when FPP master xSchedule step finishes
   -- enh (keith)   Add creation of a phoneme track from channel data
   -- enh (keith)   Add model locking
   -- bug (keith)   Force value curves to update to real values ... it wasn't doing this which may have masked some issues
   -- enh (keith)   Add row heading tooltips where the model names are too wide to display
   -- enh (keith)   Add ability to display wiring view from the front
   -- enh (keith)   Add wiring views for most inbuilt models
   -- bug (keith)   Validate serial controller number of channels
   -- enh (keith)   Add ability to extract Phonemes from imported single channel face data
   -- bug (keith)   Fix timing track labels don't save if added to a fixed timing track
   -- bug (keith)   Generated submodels leave gaps on large models - overlaps are better than gaps
   -- bug (keith)   Rename "Output #" column on setup tab to just "Number" to try to address some new user confusion
   -- enh (keith)   Add optional auto hide show of house preview when playing
   -- enh (keith)   Add Make master capability to display elements which can take any view and copy its definition to the master view
   -- enh (keith)   Tag start channels with leading *** where the number has not calculated correctly
   -- bug (keith)   Prevent duplicate model start calculation errors from displaying
   -- enh (keith)   Add time display to play controls toolbar on house preview
2017.27  Sep  6, 2017
   -- bug (keith)   Fix falcon upload only uploading enough pixels for first model on a chained output
   -- enh (keith)   Add LMS timing import
   -- bug (keith)   Fix value curves not sticking
2017.26  Sep  4, 2017
   -- bug (gil)     Fix issue where playback can hang when selecting an effect.
   -- bug (gil)     Fix some picture effects not displaying in the correct place
   -- enh (keith)   Add optional output frame deduplication with key frames if a frames data does not change
   -- bug (keith)   Fix falcon upload occasionally losing descriptions or crashing the controller and failing on V2 controllers
   -- bug (keith)   Change AC shortcut key Twinkle from 't' to 'k' so it doesn't clash with setting timing marks
   -- enh (keith)   Add optional play controls to the house preview
   -- enh (keith)   Add fast play speeds
2017.25  Aug 29, 2017
   -- enh (gil)     Improve effect selection so it selects effects with boundaries outside both edges of the selection.
   -- enh (keith)   Add option to auto save your perspectives and keep them between show directories on the same machine
   -- bug (keith)   Prevent invalid pixel split on falcon upload
   -- enh (keith)   Remember submodel dialog size and position
   -- enh (keith)   Add a simple snowflake shape
   -- bug (chris)   Fix #900 ogg files not loading correctly
   -- enh (keith)   Add ability to import LOR prop files as custom models
   -- enh (keith)   Make scheduler slowy adjust synch with an FPP master instead of jumping
2017.24  Aug 21, 2017
   -- bug (gil)     Fix custom previews not showing correct background in House Preview window.
   -- bug (gil)     Fix custom previews not initialized with correct brightness. Fixes #691.
   -- enh (gil)     Make double-click in open grid work like it used to.
   -- enh (gil)     Allow sequence timing to be changed in the GUI.
   -- enh (craig)   xSchedule Add a tooltip for where the default root web directory is located
   -- bug (dkulp)   Fix problem writing to fseq files that are larger than 2GB
   -- enh (keith)   Add the Shape Effect
   -- enh (keith)   Change value curves to use the true values of the setting they control.
                    This is major heart surgery once you save a sequence with a value curve
                    in it in this version we strongly recommend you don't load it again in an
                    older version of xlights.
   -- enh (keith)   Increase maximum number of nodes per arch
   -- enh (keith)   Increase maximum file size backed up
2017.23  Aug 15, 2017
   -- enh (dkulp)   Add ability to specify starting location and direction for circle model
   -- enh (keith)   Add ability to manual edit effect timing - right click on an effect to access it
   -- bug (keith)   Fix model group panel no longer to able to repeatedly add/remove models using button
   -- bug (keith)   Fix scheduler does not properly restore from an interrupted schedule
   -- bug (keith)   When adding timing tracks only add them to master + current view
   -- bug (keith)   Fix some drag and drop anomalies on display elements
   -- bug (keith)   Add some SDL logging to track down OSX audio not playing bug
   -- enh (keith)   Fine tune AC UI ... try to make it more intuitive
   -- bug (keith)   Fix handling of expansion boards on the falcon
   -- enh (keith)   Allow AC sequencing on nodes
   -- enh (keith)   Some check sequence additions
   -- bug (keith)   Fix model rename breaks submodels
2017.22  Aug 06, 2017
   -- bug (gil)     Fix timeline display for sequences greater than an hour.  Fixes #811.
   -- bug (dan)     Prevent a GIF with 0 frame time hanging xlights
   -- enh (keith)   Test tab auto turn on
   -- bug (keith)   Fix on exit test tab doesn't turn back on output to lights correctly
   -- enh (keith)   Test tab remembers test settings between sessions
   -- bug (keith)   Fix change name of model causes loss of model selection
   -- enh (keith)   Improve model naming when copying or adding new models
   -- bug (keith)   Fix cascade again ... it will work someday
   -- bug (keith)   Fix failed backup in some subfolders
   -- enh (keith)   Add remove unused layers to row heading right click menu
   -- bug (keith)   Prevent errors pasting models with illegal data
   -- bug (keith)   Fix backup not working for directories with . in the name
   -- bug (keith)   Fix AC cascade ... again
   -- enh (keith)   Add controller and connection details to node layout
   -- enh (keith)   Add ability to create a custom model from an inbuilt model
   -- bug (keith)   Fix string count on export models csv ... make it our best guess at the number of physical light strings
   -- enh (keith)   Make model group panel drag and drop
2017.21  Jul 28, 2017
   -- enh (keith)   Add the block of channels model
   -- enh (keith)   Add option to display fades as ramps on the On effect and Twinkle effect ...
                    mainly for use in AC mode
   -- bug (keith)   Fix a bunch of bugs with AC mode
   -- bug (keith)   Fix #882
   -- bug (keith)   Fix problem with oversize buffers crashing xLights
2017.20  Jul 26, 2017
   -- bug (dkulp)   Old keybindings files can result in U/D/O problems when defaults are removed.
   -- enh (keith)   Add value curves to sub-buffers
   -- enh (keith)   Move audio file loading to its own thread to improve audio load time in scheduler. Eliminates the need to load and convert the entire file before the audio starts to play.
   -- bug (keith)   Fix crash when importing a LOR timing track a second time.
   -- bug (keith)   Fix check sequence erroneously flagging invalid start channel where universes are grouped as a single output.
   -- bug (keith)   Fix value curve dialog wont accept keyboard input in text boxes and sliders
   -- bug (keith)   Fix export models bulb counting
   -- enh (keith)   Add simple current estimation to model export ... assumes 0.06A per node which would be normal for a typical white pixel at full power.
   -- enh (keith)   Add a collapse all function to the row heading right click menu
   -- bug (keith)   Fix schedule not playing past midnight
   -- bug (keith)   Fix schedules not showing next time after they are edited
   -- bug (keith)   Fix #876 renaming a model involved in start channel chaining breaks the chain
   -- enh (keith)   Expand E6804 upload to also support the E682.
   -- enh (keith)   Add decaying sine value curve
   -- enh (keith & Gil) Add AC Sequencing mode
   -- enh (keith)   Add a reverse nodes button to submodel dialog
   -- bug (keith)   Fix picture effect position text boxes don't allow -100 entry
   -- enh (keith)   Include some standard value curves in the distribution
   -- bug (keith)   fix serial configuration upload to the falcon controllers. Also address expansion boards better.
   -- enh (gil)     Add new option via menu setting so that double-click on a timing effect plays the audio for that duration.
                    Useful for creating/adjusting lyric timings.
2017.19  Jul 08, 2017s
   -- bug (dkulp)   Guard against submodels that result in 0 nodes that could
                    create gigantic buffers. Fixes #879
   -- bug (dkulp)   Fix crash with exporting of model group.  Fixes # 878
   -- bug (keith)   Put in place protections against extreme gamma values.
   -- bug (keith)   Fix show all effects crash because dynamic cast failed
2017.18  Jun 29, 2017
   -- bug (dkulp)   Vixen import was not reading channel names/colors from profile files.
   -- enh (keith)   Add a show all effects row header right click menu option which opens up all groups/models/strands
                    to show all effects ... but only those that are necessary to see them all
   -- bug (keith)   Fix some crashes when mapping due to empty lists
   -- bug (keith)   Fix hue vc not working on layers other than zero
   -- enh (keith)   Add splitter to mapping dialog
   -- enh (keith)   Add splitter to sequence elements dialog.
   -- bug (keith)   Fix test tab throws error if output to lights was already on
   -- enh (keith)   Connect up menu items for controller upload on OSX
   -- enh (dkulp)   Update vixen import to try and find the profile file and use the channel names from there
   -- bug (keith)   Fix color panel value curves not setting correctly as you move from effect to effect
2017.17  Jun 19, 2017
   -- bug (gil)     Fix individual start channels not displaying correct numbers.
   -- enh (gil)     Use freeform selection if selected timing track has no timing marks.
   -- bug (gil)     Prevent overlaps of Spiral and Fan effects imported from SuperStar.
   -- bug (keith)   Fix issue highlighted in this thread http://nutcracker123.com/forum/index.php?topic=4407.0
   -- bug (keith)   Fix back to back glediator effects not rendering correctly.
2017.16  Jun 13, 2017
   -- bug (dkulp)   Fix some resize issues in import dialog
   -- bug (dkulp)   Fix drag/drop in import dialog (requires rebuild of wxWidgets)
   -- bug (dkulp)   Fix sorting in available models on import dialog
   -- bug (gil)     Correct 3D DMX model rotating opposite of other view types.
   -- bug (gil)     Fix crash when deleting last view in the list.  Fixes #869.
   -- enh (gil)     Add ability to right-click House Preview and switch to custom previews you've created.
   -- enh (gil)     Bars Effect: add new directions and VC to center point to allow precise motions.
   -- bug (gil)     Fix effect preset import bug that caused old effect upgrade code to run.  Fixes #863.
   -- enh (keith)   Get Pixlite 4 controller upload working
   -- enh (keith)   Get J1Sys P2 controller upload working
   -- enh (keith)   Show models already used in mapping dialog
   -- enh (keith)   Add more submodel generation options
   -- enh (keith)   Add auto mapping for xlights/lor/vixen import ... automatically maps identical model/group names
   -- bug (keith)   Force model start channels to be up to date if accessing functions from setup tab
   -- enh (keith)   Add Model State to DMX model so you can use them to represent DMX channels where specific values
                    represent specific behaviour
   -- bug (keith)   Force new models/groups to appear immediately in sequence elements available list for master view
   -- bug (keith)   Prevent xlights import overwriting existing effects
   -- bug (keith)   Fix if no effects present render never ends
2017.15  May 31, 2017
   -- bug (dkulp)   Fix crash if submodel is part of a preview
   -- enh (keith)   Add E6804 controller configuration support
   -- bug (keith)   Fix errors with FPP config
   -- bug (keith)   Fix xyzzy on matrixs on start channels other than 1
   -- bug (keith)   Fix triangle wave effect crash
   -- enh (keith)   Enhance wiring view and add light background option
   -- bug (keith)   Fix hang in blend transition
   -- enh (keith)   Add documentation of xSchedule variables into the UI
   -- bug (keith)   Fix some issues with layer counts in preset window
   -- bug (keith)   Fix end channel display if multiple universes are grouped under one output
   -- bug (keith)   Fix 64 bit vamp performance on windows
2017.14  May 17, 2017
   -- enh (gil)     Add additional colors to color manager.
   -- bug (gil)     Fix Align Both Times case that was failing.
   -- bug (gil)     Fix effect panel not updating icon when using drop-down combo.
   -- bug (dkulp)   Fix crash if model using #x:y notation has end channel beyond end of setup. Fixes #859
   -- bug (dkulp)   Fix crash if expand group with submodels and model doesn't exist in sequence
   -- bug (dkulp)   Hook up the renderDirty event to the align options and undo to render the changes
   -- bug (keith)   Fix 64 bit stack walking on windows.
   -- enh (keith)   Allow entering of a description for a FPP instance in the connection dialog
   -- enh (keith)   Allow bulk uploading of sequences to all known FPP instances
   -- enh (keith)   Add select used/unused models to Display Elements model list right click menu
   -- enh (keith)   Add a per node option to candle effect
   -- bug (keith)   Fix some drag and drop issues in the master view in display elements panel
2017.13  May 10, 2017
   -- bug (dkulp)   Only do the "outside -> in" render for the Tree model.  Fixes #573.
   -- enh (cjd)     Output status messages to commandline when run in render mode
   -- enh (gil)     New Color Manager Dialog so user can customize colors in the program.  Fixes #363.
   -- enh (gil)     Add right-click option to correct aspect ratio of a Matrix model (equal width vs height spacing). Fixes #606.
   -- enh (gil)     Custom model import will now keep submodels in original order instead of backwards.  Fixes: 787.
   -- enh (gil)     Allow boxed models to be sized past the window boundary. Fixes #853.
   -- enh (gil)     Add values curves to Galaxy Effect
   -- bug (gil)     Fix effect slider not responding after effect was moved up or down a row.
   -- bug (gil)     Fix bug where effect align start times or end times failed to work.
   -- enh (gil)     Add 2 new effect alignment options
   -- enh (dkulp)   Allow reverse order for SubModel node ranges (10-5 instead of 5-10). Fixes #852
   -- enh (keith)   Rework sequence elements dialog box to get rid of the add window and make everything drag and
   -- enh (keith)   Rework xlights/LOR/Vixen import mapping dialog to use drag and drop mapping
   -- bug (keith)   Fix out of order universe handling in falcon controller upload
   -- enh (keith)   Display model end channel in layout tab in similar format to start channel (along with absolute
   -- enh (keith)   Add source size parameter to Liquid effect
   -- bug (keith)   Fix group rename not fixing group names in open sequence
   -- bug (keith)   Add back LOR import CCR level import
   -- bug (keith)   Fix some potential crashes in FPP upload
   -- enh (keith)   A bit of a speculative change ... optimise LOR output to only output changed data in each frame
                    Looking at FPP and the protocol it looks like it will use and should make it run better when not
					every channel changes every frame.
   -- bug (keith)   Fix crash when audio files are opened in quick succession
   -- enh (keith)   Allow multiple import of xpresets
2017.12  Apr  30, 2017
   -- bug (sean)    Fix for Vamp plugins for 64 bit xLights. Web page updated to remove direct download of Vamp
2017.11  Apr  29, 2017
   -- enh (sean)    Include 64 bit xSchedule.exe besides 32 bit xScheduler.exe
   -- bug (sean)    Fixed install directory for 32 bit xLights. Now files go into ../Program Files(x86)/xlights
   -- bug (dkulp)   "Old Style" pinwheel performance degredation since 2017.05
   -- bug (dkulp)   ColorWash/On cycle calcs resulted in extra full on frame at end
   -- bug (keith)   Picture effect tile down not displaying correctly
   -- enh (keith)   Get Vamp 64 bit working on all computers
   -- enh (keith)   Save and recall buffer, timing and color panel reset setting
   -- enh (keith)   Add 64 bit xSchedule for windows
2017.10  Apr  25, 2017
   -- enh (keith,gil,dan) New 64bit version of xLights released
   -- enh (gil)     Add pan and tilt slew limits to DMX model.
   -- bug (gil)     Fix default Skulltronix dmx model values not being saved in XML.
   -- enh (gil)     Add DMX model parameter to control displayed beam length
   -- enh (gil/keith) Get windows 64 bit compiling reliably
   -- enh (keith)   Add setting to exclude presets from packaged sequences
   -- enh (keith)   Add setting to exclude audio from packaged sequences
   -- enh (keith)   Add xmodel export of star models
   -- enh (keith)   Add Pixlite 16 controller configuration upload
   -- enh (keith)   Add submodel generation
   -- bug (keith)   Make crash log filenames easier to read
   -- bug (keith)   Fix issues with some VU meter effects on single line models
   -- bug (dkulp)   Use buffering to write xseq files.  Fixes #845
2017.9  Apr  9, 2017
   -- bug (dkulp)   Per Model/Strand render buffer styles could crash with Groups within Groups
   -- bug (dkulp)   Effects added on submodels/strands would not trigger "dirty" flag
   -- bug (dkulp)   Render after Paste will clear data in models unaffected by paste
   -- bug (keith)   Guard against trying to access a polyline layer that does not exist.
   -- bug (dkulp)   RenderAll will not render the very last frame (modifying the effect likely would)
                    This fixes #842
   -- bug (dkulp)   Node display extends the color in the last frame out an extra frame width
2017.8  Apr  3, 2017
   -- bug (dkulp)   Fix excessive rendering when pasting effects
   -- enh (cjd)     Add direction setting for lightning effect. Fixes #830
   -- enh (keith)   Add candle effect
   -- bug (keith)   Fix superstar import picture scaling
   -- bug (keith)   Fix LMS import into data layer off by 1 channel error
   -- bug (keith)   Fix blanking between steps in scheduler
   -- enh (keith)   Add timing event pulse and timing event jump 100 VUMeter effects
2017.7  Mar 28, 2017
   -- enh (keith)   Add prompted recovery of unsaved changes to rgbeffects
   -- bug (keith)   Disable import effects if no sequence is open
   -- enh (keith)   Add liquid effect
   -- enh (keith)   Add ability for picture effect to play GIF files with their encoded timing and loop it
   -- bug (keith)   Fix ArtNET universe 0 not saving
   -- bug (keith)   Fix Glediator effect causing field errors
   -- enh (keith)   Get x64 build working for Visual Studio.
   -- enh (keith)   Get Vamp 64 bit working
   -- enh (keith)   Scheduler - Add support for animated GIFs to image playlist item
   -- bug (keith)   Scheduler - Fix crash on exit when using PJLink
   -- bug (keith)   Scheduler - Fix matrix start channels not saving or able to set to > 100
   -- enh (keith)   Add export/import of matrix, tree and dmx models
   -- bug (keith)   Fix render progress bar not pressing properly
   -- enh (keith)   Add value curve displaying value of point under mouse even when model isnt custom
   -- bug (keith)   Fix schedule queueing
   -- bug (keith)   Fix schedule blend modes
   -- enh (keith)   Code SSE instructions for scheduler blend mode. Currently disabled.
   -- enh (keith)   Enhance FPP connect to remember multiple FPP connection details
   -- bug (keith)   Fix bug in model state corrupting rgbeffects file
   -- enh (keith)   Enhance schedule api to allow playlists, steps, items, schedules and buttons to be identified by id
   -- enh (keith)   All multiple model delete on layout tab. Only prompt to confirm if sequence is open with effects on that model
   -- enh (keith)   Add support for xSchedule web API using web sockets
   -- enh (keith)   Let user decide if background playlist should play if nothing else is playing
   -- enh (keith)   Add spatial color curve to shimmer effect
   -- enh (keith)   Add model size matching to layout panel model right click menu
   -- bug (keith)   Add missing text controls to fireworks and strobe effect settings.
   -- enh (keith)   Add new implementation of shimmer effect algorithm which is more controllable and predicatable. The old algorithm is also available.
   -- enh (keith)   Add support for the DDP protocol http://www.3waylabs.com/ddp/ - a very light weight ethernet protocol
   -- enh (keith)   Add model export to Minleon Network Effects Controller file which can be loaded onto SD card
   -- bug (keith)   Fix video display in xSchedule on OSX
   -- enh (keith)   Add custom frame rates to new sequence setup
   -- bug (keith)   Fix triangle wave left to right not working
   -- bug (keith)   Fix some rounding issues that was throwing video timing off when aligned to audio track.
   -- enh (will)   Enhance scheduler web interface. More functionality. More responsive.
2017.6  Feb 27, 2017
   -- bug (dkulp)   Fix xScheduler logging on OSX
   -- enh (keith)   Add duration treatments to glediator effect
   -- bug (keith)   Include iseq files in package sequence
   -- enh (keith)   Add a scale but keep aspect ration option to Pictures effect
   -- enh (keith)   Add HSV adjustment to the colour panel ... this is applied after the effect is rendered.
   -- enh (keith)   Let the user turn on/off color panel reset when changing effects. This setting is not saved when xlights is closed. It also does not control the pallette itself
   -- bug (keith)   Fix spatial colour curves on pinwheel using old render style
   -- enh (keith)   Add a new sweep 3d style to pinwheel
   -- bug (keith)   Fix projector details not saving correctly in xSchedule options
   -- enh (keith)   Default color curves to the colour of the swatch (instead of black)
2017.5  Feb 23, 2017
   -- bug (dkulp)   Fix Pinwheel effects created before June 2016
   -- bug (dkulp)   Re-allow use of hostnames instead of IP addresses for outputs
   -- bug (dkulp)   Fix nullptr crash if master view has models that don't exist
   -- bug (keith)   Fix incorrect Property value count in e131 packets
   -- bug (keith)   Fix not handling inactive controllers correctly
   -- bug (keith)   Fix handling of Not Connected serial outputs
   -- bug (keith)   Fix xSchedule not applying global brightness correctly
   -- enh (keith)   Add arduino code for physical button control of xSchedule - this is available from GitHub only ... it isnt installed.
   -- enh (keith)   Add a run process to the scheduler that does not interrupt current playlist
2017.4  Feb 18, 2017
   -- bug (sean)    Include latest xSchedule.exe in release. 2017.3 had old version.
   -- enh (dkulp)   Add a couple "Per Model" render buffer styles to the ModelGroup render styles
   -- enh (gil)     Fix effect backgrounds so that they default to first color of a gradient instead of black.  Fixes #823.
   -- enh (keith)   Let the user turn on/off buffer and timing panel reset when changing effects. This setting is not saved when xlights is closed.
   -- enh (keith)   Add option to video effect to automatically synchronise with a video file that has been used for the sequence audio track.
   -- enh (keith)   Remove the old scheduler
   -- bug (keith)   Fix universe dropdown in start channel dialog did not show all universes available on an ip address if they were grouped under a single output
   -- enh (keith)   On value curve dialog show a tooltip when manipulating a custom curve showing the real value that will be returned by the curve at the chosen point.
   -- enh (keith)   Complete FPP Master mode in xScheduler
   -- enh (keith)   Complete FPP Remote mode in xScheduler
2017.3  Feb 13, 2017
   -- enh (gil)     Add new Skulltronix skull display option for the DMX model.
   -- enh (gil)     Add new 3D DMX head display option for the DMX model.
   -- enh (gil)     Add new Servo Effect mainly for 16 bit servos but also works with 8 bit channels.
   -- enh (gil)     Add capability to import a VSA file.
   -- bug (dkulp)   Don't stop the timer when flipping pages. Fixes #817
   -- enh (keith)   Add list of referenced files to export effects
   -- enh (keith)   Add text output to xSchedule allowing run time countdowns and text overlays
   -- bug (keith)   Fix #814 Make video start time editable
   -- bug (keith)   Fix #820 Fix triangle wave form in wave effect
   -- bug (keith)   Fix end channel not updating on layout tab
   -- enh (keith)   Add documentation of xSchedule web api to the source repository
   -- enh (keith)   Add colour reordering as an output process in the scheduler.
   -- enh (keith)   Add pixel reversing as an output process in the scheduler
   -- enh (keith)   Add import to xSchedule to allow importing of xLights 4 schedules.
   -- enh (keith)   Make LOR import more like xLights import ... using a tree rather than a grid. There is some risk this breaks "CCR"
                    import mode in which case someone needs to send me a sample file.
   -- enh (keith)   Make Vixen 2 import more like xLights import ... using a tree rather than a grid.
   -- bug (keith)   Fix erasure of outputs on setup when changing to a new empty folder
   -- enh (keith)   Give mapping files an extension (*.xmap) to make them easier to locate
   -- enh (keith)   Allow user to specify the local IP address to use for e131/Artnet output
   -- bug (keith)   Fix #753 Preset not showing the correct number of layers
2017.2  Jan 31, 2017
   -- bug (sean)   Bundle up package so that xSchedule has the properties files needed to run
2017.1  Jan 31, 2017
   -- enh (dkulp)   Support groups within groups. Preview should be correct with groups within groups
   -- enh (dkulp)   Add sequence option for allowing blending of effects between models/groups
   -- bug (dkulp)   Fix problem in JobPool where not enough threads may be created
   -- bug (gil)     Fix import of LOR timings during new sequence wizard.  Fixes #805.
   -- enh (gil)     Increase DMXModel selection area hotspot.  Fixes #795.
   -- bug (gil)     Fix blue screen crash when turning off outputs while using LOR USB dongle.  Fixes #797.
   -- bug (gil)     Fix forcing of color for RGB nodes in LMS import. Red and Blue were swapped.
   -- enh (keith)   Add xSchedule
   -- enh (keith)   Refactor output processing. This should generally be invisible other than some performance enhancements and
                    useability enhancements on the setup tab. This includes Fix #789, #808 and #812
   -- bug (keith)   Fix #760 - ESEQ elispses doesn't open dialog
   -- enh (keith)   Fix #796 - disable export effects if no sequence open
   -- bug (keith)   Fix #785 - mac crash due to UI update on background thread in Fireworks effect.
   -- enh (keith)   Optionally allow backup to backup xml files in subdirectories for users that don't keep their xml files in the show
                    directory itself but a folder underneath it
2016.56  Dec 31, 2016
   -- enh (gil)     Rounder ends for the Galaxy effect.
   -- enh (gil)     Use dynamic step angle to prevent holes in Galaxy effect for large radii.
   -- enh (gil)     Update Fan to support spatial colors and not leave any holes.
   -- enh (gil)     Update Shockwave to support spatial colors and not leave any holes.
   -- bug (gil)     Fix cell highlight movement that was broken by other fixes in August.  Fixes #755.
   -- enh (keith)   Add sequence packaging to the tools menu 	"Tools,Package Sequence"
   -- bug (keith)   Make xbkp auto recovery smarter. don't trigger after a sequence is discarded. don't trigger when doing batch rendering.
   -- enh (keith)   Add more sub buffer panel right click menu options for fast setting sub buffers
   -- enh (keith)   Add manual sub buffer editing allowing more precise control over sub buffer sizing
   -- enh (keith)   Add home (start of song) /end (end of song) /pause (pause playing song) shortcut keys in sequencer
   -- enh (keith)   Add spatial color curves to single strand
   -- enh (keith)   Add spatial color curves to pinwheel
   -- enh (keith)   Add reset to default on remaining effects when you select a different effect to the one currently selected (fan/morph/shockwave/dmx/fill/galaxy)
2016.55  Dec 20, 2016
   -- enh (keith)   Add spatial color curves to spirals
   -- enh (keith)   Add spatial color curves to bars
   -- enh (cjd)     Rewrite pinwheel effect to handle larger buffer sizes cleanly
   -- enh (keith)   Reset buffer panel when selecting a new effect
   -- enh (keith)   Reset buffer panel when selecting a new effect
   -- enh (keith)   Add spatial colour curves on the On effect
   -- enh (keith)   When opening sequence check for a newer xbkp file. If it exists prompt the user to use it instead.
   -- enh (keith)   Add -w command line switch which wipes all saved settings restoring machine settings back to a true fresh install
   -- enh (keith)   Add checks for large blocks of unused channels between models
   -- enh (keith)   Add fireworks fire with timing mark
   -- enh (keith)   Add flip to colour curve
   -- enh (Keith)   Detect multiple outputs sending data to the same place in check sequence
   -- enh (keith)   Input config upload to FPP for bridge mode
   -- enh (keith)   Input config upload to falcon controllers
   -- enh (keith)   Output config upload to falcon controllers
2016.54  Dec 14, 2016
   -- bug (dkulp)   Make sure we have a valid show directory at startup... too many crashes occur without one
   -- bug (dkulp)   Fix bugs with file based dimming curves
   -- enh (gjones)  Add ability to time shift all effects in a sequence under Edit menu.
   -- enh (dkulp)   Basic TouchBar support on latest MacBookPros
   -- bug (dkulp)   Don't crash if an invalid model or submodel causes a buffer to be size 0
   -- bug (dkulp)   Don't crash if special characters are using timing track labels. Fixes #762
   -- bug (dkulp)   Fix crash if sequence has % in name
   -- bug (keith)   Fix video effect crash on Mac
   -- bug (keith)   Strip leading zeroes from ip addresses to prevent issues
   -- bug (keith)   Fix video looping
   -- bug (keith)   On piano effect fix reset of timing track when note range changed
   -- bug (keith)   Fix updating of start/end time in the status bar when an effect is moved
   -- bug (keith)   Fix export models to export all models not just those in current preview
2016.53  Nov 17, 2016
   -- bug (dkulp)   Fix overlap of icicle drops when using multiple strings
   -- bug (dkulp)   Fix crash when Undo is used after closing a sequence and opening another
   -- bug (dkulp)   Fix crash on OSX if numpad enter is used in certain text boxes
   -- enh (gil)     Add wrap option to Fill effect to allow turning off wrap feature.
   -- enh (gil)     Add ability to flip the channel numbers in custom model horizontally and vertically.
   -- enh (dkulp)   If the OpenGL driver didn't double-buffer, try again.  Should help on GDI driver
   -- bug (dkulp)   Fix crash if right click on setup tab when no controllers are defined
   -- bug (dkulp)   May not be able to import xLights effects onto submodels without first viewing the
                    submodel layers on the grid
2016.52  Nov 09, 2016
   -- bug (dkulp)   Allow using @ModelName:# for individual strands where ModelName is the current model
   -- bug (keith)   Fix picture effect image scaling
   -- bug (keith)   Set default FPP user to fpp instead of pi
   -- bug (dkulp)   Fix crash if node effects are on nodes that don't exist
   -- bug (dkulp)   Fix crash on scheduler page on OSX due to obsolete code
   -- enh (jwylie)  changed minute : second counter to accept prepended and appended text
   -- enh (dkulp)   Add ability to import custom timing tracks (and lyric tracks) from other xLights sequences
   -- bug (dkulp)   Fix crash on drag/drop if no sequence loaded
   -- bug (dkulp)   CreateCustomModel dialog would not allow selecting node type on OSX
2016.51  Nov 04, 2016
   -- enh (cjd)     Update sequence checker to handle start channels in #IP:UNIVERSE:CHANNEL format
   -- bug (keith)   Performance fix for animated GIFs. Example: "This is Halloween" went from 300 second save to 65 seconds.
   -- bug (dkulp)   Fix mac<-->windows pictures effect filename mapping
   -- bug (dkulp)   Fix mapping onto nodes if nothing is mapped to the strand
   -- bug (dkulp)   Fix "Create Random Effects" taking forever to complete
   -- enh (dkulp)   Get the FPP Connect via FTP stuff working on the Mac, also allow named based FPP
   -- bug (keith)   Fix GIF scaling not working.
   -- bug (keith)   Sequence Checker issue for Stars layers #721
   -- bug (dkulp)   Don't crash if polyline is set to singlechannel and start at right
   -- bug (keith)   Fix #612 ... missing effect settings scroll bars
   -- enh (keith)   Optimise logging in render loop ... means you cant turn on logging
   -- bug (cjd)     Avoid crash if pasted data is empty
   -- bug (cjd)     Fix github issue #709 "Fire effect height textbox entry"
   -- enh (keith)   fixed change the link button image and hover text in the Setup screen #342
   -- bug (dkulp)   Make sure picture effect is re-initted at start of effect
   -- bug (dkulp)   Fix crash on toggle strands when clicked on a strand instead of model
   -- enh (gil)     Add shutter support to DMX model.
   -- enh (dkulp)   Allow 180 trees to rotate and make sure they display as 180
   -- enh (dkulp)   Start all jobs that don't depend on anything above them first, then start the rest. 30% performance improvement
   -- enh (dkulp)   Make sure the list box is growable
   -- bug (dkulp)   Fix crash on render on some sequences
   -- enh (Jennifer)added tool tips for countdowns
   -- enh (Jennifer)New enhancements for countdown timer in text effect
   -- bug (keith)   Fix GIF handling of   animated gifs where first frame is not full size ... ie it has an offset.
   -- bug (keith)   Fix video effect start time bug.
   -- enh (keith)   Allow VU Meter to use Papagayo layer 0 as a timing track
   -- bug (keith)   Fix GIF handling of animated gifs where first frame is not full size ... ie it has an offset.
   -- bug (cjd)     Fix rounding issue leading to unwanted gap between spirals
   -- bug (gil)     Fix model staying highlighted when removed from group.  Fixes #707.
   -- bug (gil)     Prevent creation of unnecessary layer when promoting effects from only 1 strand.
   -- bug (gil)     Fix exception when opening Lyric Dialog.  Fixes #710.
   -- bug (keith)   More animated GIF fixes ... all should work now
   -- enh (keith)   Add right click menu to controller list on setup tab including:
						- Inserts controller(s) at a point in the list
						- Bulk update IP address, channel count and description
						- Bulk delete
						- delete key and control-A to select all
						- activate and deactivate are now available as right click menu items. Double click or ENTER key open the controller to edit it
						- grey out inactive controllers
						- open controller in your browser
   -- enh (keith)   Allow rubber band selection of models on layout tab without holding down shift key
   -- bug (keith)   keyboard nudging of multiple selected models now works
   -- enh (keith)   Add multi artnet universe adding
   -- bug (keith)   Fix seven segment states not sticking
   -- enh (keith)   Add FPP Connect which will upload universe configuration, fseq and media files.
   -- enh (keith)   Add start channel checks to check sequence
   -- enh (keith)   Fix #342 - add a tooltip to show dir/media dir link button to make it clearer what the button does in each state
2016.50  Sep 27, 2016
   -- enh (gil)     Allow arrow key to be used to scroll in model list in Layout.  Fixes #582.
   -- enh (dkulp)   Add -r command line switch to render and save the fseq file and then exit
   -- enh (gil)     Modify value curve icon so its clear when its not selected.  Fixes #675.
   -- bug (gil)     Fix crash when pasting bad data.  Fixes #693 and fixes #694.
   -- bug (gil)     Fix extra preview windows not holding their saved position.
   -- bug (dkulp)   Fix node names used for HLS imports truncating the last character
   -- bug (keith)   Fix animated GIF files which were not rendering correctly
   -- enh (keith)   Make import and export of effects remember the last type used under assumption user is likely to want to use it again next time
   -- bug (keith)   Fix contrast on oversized sub-buffer. Yellow on light grey
   -- enh (keith)   Add links to common web resources to the help menu
   -- enh (keith)   Add palette reversal and deletion
   -- enh (keith)   Add multilayer insertion
   -- bug (keith)   Make output and model descriptions XML safe
   -- enh (keith)   Make garland lights on tree effect optional
   -- bug (keith)   Fix start channel check in check sequence calculating start channel incorrectly
   -- enh (keith)   Add custom model checks to check sequence
   -- enh (keith)   Add model checks to star and circles models to ensure all nodes correctly allocated
   -- bug (keith)   Fix some preset layer counting issues - string v3 presets from display as they cant be applied
   -- enh (keith)   Fix #340 - Add key model stats to model export
   -- enh (keith)   Add effect decriptions and effect export to get easy list of your effects in sequence and analysis of effect frequency.
   -- enh (keith)   Add check sequence check to highlight the network adapter we will output lights data to.
   -- bug (keith)   Increase maximum time offset for effects import from 10 seconds to 10 minutes
   -- bug (keith)   Fix model channel overlap checking in layout panel
2016.49  Sep 13, 2016
   -- bug (dkulp)   Strand/Submodel/Node effects were not clearing their buffer area on control updates resulting in ghosted data until a full render
   -- bug (dkulp)   DimmingCurve dialog not initialized properly for models that don't already have a dimming curve
   -- enh (dkulp)   Optimise blur - speeds rendering
   -- enh (dkulp)   Add additive/subtractive/min/max blend modes to on effect
   -- bug (dkulp)   Fix "promote node effects" not appearing when it should
   -- bug (dkulp)   Fix crash when select show models/nodes/strands from menus
   -- bug (gil)     Fix align both times boundary problem.  Fixes #669.
   -- bug (gil)     Fix layout group change to model group not taking effect till restart.
   -- enh (gil)     Change default model group back to minimal grid.
   -- bug (gil)     Prevent effects from being copied or moved twice when they exist multiple times in the grid.
   -- bug (gil)     Fix names for DMX model types in property grid.  Fixes #674.
   -- bug (gil)     Fix single timing effect copy/paste not working.  Fixes #677.
   -- enh (gil)     Add value curves to Shockwave effect.
   -- enh (gil)     Add value curves to the Fan effect.
   -- enh (gil)     Add value curves to Morph effect.
   -- enh (keith)   Implement e1.31 sync
   -- enh (keith)   Add more checks in check sequence
   -- enh (keith)   Show single colour string colour in sequencer row heading
   -- enh (keith)   Add mouth image hints to faces dialog
   -- enh (keith)   Show waveform selected duration under Time/FPS display
   -- bug (keith)   Fix dropping of ranged timing tracks between two existing ranges. Also prevent spanning ranges.
   -- enh (keith)   Add star to ripple effect and VUMeter
   -- enh (keith)   Add a free model description property which is included in model export. Useful for documenting things like which controller output it should be on or the cable tag or anything else that you may currently record in a spreadsheet.
   -- bug (keith)   Fix some problems with timing mark creation
   -- enh (keith)   Shift + Double click on effect highlights corresponding portion of the waveform
   -- enh (keith)   Add a random color curve type ... randomly sets the color to a color between the colors of the color stops
   -- enh (keith)   Add palette quick sets
   -- enh (keith)   Add ArtNET support - DMX and Sync packets only
2016.48  Aug 29, 2016
   -- enh (keith)   Add sequence checker, "Tools,Check Sequence". This scans your Setup/Layout/Open sequence and highlights any
                    items that might cause issues or unexpected behaviour in xLights.
   -- bug (dkulp)   Fix WaveForm rendering beyond the end of the sequence
   -- bug (dkulp)   Add error check for faces effect with non-default buffer style
   -- bug (dkulp) ColorWash needs to remove lower level defaults as well
   -- bug (dkulp)   Fix crash in SubModel dialog flipping notebook pages when no submodels defined
   -- enh (gil)     Add match duration effect alignment option.  Fixes #670.
2016.47  Aug 22, 2016
   -- bug (dkulp)   Update the EffectAssist when the effect settings change, EffectAssist considers submodel buffer sizes
   -- enh (dkulp)   Add submodels to Tree on LayoutPage, allow renaming
   -- enh (dkulp)   Put the currently open sequence filename in titlebar
   -- enh (dkulp)   Add standard OSX "Window" menu item
   -- bug (dkulp)   Fix prompt of modified sequence when closing an unmodified sequence
   -- bug (gil)     Fix import of Superstar Spirals spin direction.
   -- bug (gil)     Fix PolyLine when controller is position is right.
   -- bug (keith)   Fix seven segment bitmap
   -- bug (keith)   Fix video export of model for non matrices
   -- enh (keith)   Add value curve user definable presets
2016.46  Aug 17, 2016
   -- enh (dkulp)   Add SubModels for defining effects for specific parts of a model
   -- bug (dkulp)   Check for invalid models before adding the models associated with the current view so those models can be selected for mapping
   -- bug (dkulp)   Fix node level effect broken in .45
   -- bug (dkulp)   Add some null ptr checks and logs to try and prevent/diagnose various crashes
   -- enh (dkulp)   More work on enabling sandboxing on OSX
   -- enh (dkulp)   Update ffmpeg to new version that doesn't make calls to private API's
   -- enh (gil)     Text effect is now 1 line only.  Old effects have been split with new layers added when necessary.
   -- bug (gil)     Fix window frame rendering when top and bottom sizes don't match.  Fixes #599.
   -- bug (gil)     Fix arch selection when rotated upside down. Fixes #645.
   -- enh (gil)     Support mask settings for Text effect imported from SuperStar.
   -- enh (gil)     Add Shimmer option to Pictures effect to support Scene imports with shimmer from SuperStar.
   -- enh (gil)     Fix SuperStar import that created upside-down color wash effects.
   -- bug (gil)     Fix scene import from SuperStar.  Scenes should never be flipped vertically.  Also remove FlipY option.
   -- bug (gil)     Fix the flipping issue for vertical sequences for SuperStar import.  Got broke when fixing horizontal sequences.
   -- bug (gil)     Fix copy/paste row that was broken by 2016.43.
   -- enh (gil)     Modify row copy/paste to work cross sequence and when Paste by Cell is active.  Fixes #650.
   -- bug (gil)     Fix Pictures WrapX to wrap at model size instead of image size.
   -- enh (gil)     Make the transition smooth if you cancel a model Import.  Fixes #647.
   -- enh (gil)     Enhance the Layout page by combining the Model Groups and Models.
   -- enh (keith)   Add model preview to face and state dialogs to make setting channel ranges easier
   -- bug (keith)   Fix crash when importing fseq when creating a new sequence
   -- bug (keith)   Fix oversize LCB export. Fixes #648
   -- bug (keith)   Fix display of tooltips on test tab to display models against channels.
   -- enh (keith)   Remove all but timing track option for piano effect
   -- bug (keith)   Strip out video export for models that don't make sense or model groups. Fix colour handling for non RGB colour orders
   -- enh (keith)   Add simple way to add 7 segment labels to model state definition
   -- enh (keith)   Add a view from rear option on custom model dialog to help you insert pixels correctly into your coro models Fix #646
   -- bug (keith)   Fix #u:ch format calc when non e131 controllers are present.
   -- bug (keith)   Fix accidentally disabled prompt for missing media file.
   -- enh (keith)   Add state and submodel settings to xmodel files.
   -- bug (keith)   Fix channel count on models export when channels are not contiguous
   -- bug (keith)   Trigger start channel recalculation if setup tab is changed ... this helps if you are using o:c or #u:c formats for start channel
   -- enh (keith)   Add model wiring dialog for custom models ... shows you a reverse view of the model with the wiring.
2016.45  Aug 09, 2016
   -- bug (keith)   Fix colour curve preset scanning.
   -- bug (dkulp)   Prevent crash at startup if preview background image doesn't exist
   -- bug (gil)     Set selected effect when it gets selected by a cell selection.  Fixes #623.
2016.44  Aug 08, 2016
   -- enh (keith)   Add Import Notes to right click menu on timing tracks ... this will create a timing track containing notes from Polyphonic Transcription, MIDI, Audacity and Music XML files. All sources other than 'Timing Track' will be removed from the piano effect in a future release.
   -- enh (keith)   Enhance autosave to be smarter and only save when it needs to
   -- enh (keith)   Add metronome arbitrary fixed timing timing tracks
   -- enh (keith)   Add color curves
   -- enh (keith)   Add some built in color curves
   -- enh (keith)   Add Timing Event Jump effect to VU Meter
   -- enh (keith)   Make fireworks follow the colour curve
   -- bug (keith)   Fix crashes in music and piano effects
   -- enh (gil)     Add ability to import/export PolyLine models to .xmodel files.
   -- enh (gil)     Add ability to PolyLine to create curved segments.
   -- enh (gil)     Shift+MouseWheel will now scroll grid right/left when in Waveform or Grid.  Fixes #644.
   -- bug (gil)     Fix crashes when switching show directories that contain custom previews.
   -- bug (gil)     Fix default layer mode not saving for Model Group properties.  Fixes #643.
   -- enh (gil)     Allow multiple selected effects to be moved up/down and nudged left/right.  Fixes #642.
   -- bug (gil)     Fix offscreen effects not moving when dragged or nudged left/right.  Fixes #637.
   -- enh (gil)     Change from Lights/String to Nodes/String based on string type selected.  Fixes #633.
   -- enh (gil)     Allow playback speed to slow down playback for Animation sequences.
2016.43  Aug 04, 2016
   -- bug (gil)     Fix PolyLine not working with single channel nodes.
   -- enh (gil)     Increase Fan radius parameters limits.  Fixes #640.
   -- enh (gil)     Improve effect selection and updates when vertical scrolling.  Fixes #638.
   -- bug (gil)     Prevent crash when custom model background image does not exist. Fixes #639.
   -- bug (gil)     Remove Previews from Menus when switching show directories.
   -- enh (gil)     Add ability to scale PolyLine models.
   -- enh (gil)     Add effect alignment options.
   -- bug (gil)     Prevent PolyLine point deletion unless there are 3 or more points.
   -- bug (gil)     Fix PolyLine selection hotspots and selection with yellow bounding box.
   -- enh (gil)     Account for malformed XML in the new version 6 format of SuperStar files.
   -- bug (dkulp)   Fix crash when matrix type models have a single node and a Model/Strand render style is used
   -- enh (dkulp)   Allow multiple layers for Strands, strands blend onto the model level effects
2016.42  Jul 30, 2016
   -- bug (gil)     Fix typo that prevented the Layer Star render style from working for Star Model.
   -- enh (gil)     Add PolyLine model.
   -- bug (gil)     Fix crash when trying to paste with active timing track and no cell selected. Fixes #634.
   -- bug (keith)   Corrected by allowing a 1 second target to be selected when outside of a cell.
   -- bug (keith)   Fix music effect crash when dragged past end of song
   -- bug (keith)   Reduce errors when video effect cant find video file to just one on first frame
   -- bug (keith)   Add support for model start channel in form #ip:universe:channel
   -- bug (keith)   Update start channel dialog for new format + also make it smarter about which outputs and universes exist.
   -- bug (dkulp)   Don't add/remove layers if the element is being rendered
   -- bug (dkulp)   Fix a crash when loading an invalid sequence that had effect names of ""
2016.41  Jul 22, 2016
   -- enh (dkulp)   Add pinch-to-zoom on Mac trackpad
   -- enh (gil)     Add page to new sequence wizard to allow selection of a view or all models.
   -- enh (gil)     Allow effects to be dropped without timing marks while a timing track is active.
   -- enh (gil)     Add new options to the Fill Effect: Band Size, Skip Size, and Offset.
   -- bug (dkulp)   When applying effect presets or key bindings, adjust for the xLights version that was used to create that effect/binding
   -- bug (dkulp)   Crash when trying to delete the background image
   -- bug (dkulp)   Strobe node types not working correctly
   -- enh (dkulp)   Add Left/Right starting locations for Icicles
   -- enh (dkulp)   Write all data in fseq file direcly in one call instead of copied by frame
   -- enh (dkulp)   Read from fseq files full frames at a time directly into data buffer instead of by channel
   -- bug (keith)   Fix video drift in auto custom model generation
   -- enh (keith)   Add video export of a models effects
   -- enh (keith)   Add new #x.y format for specifying start channel x = universe, y = channel within universe
   -- enh (keith)   Include xbkp files in log packaging and crash dumps
   -- bug (keith)   Fix handling of multiple e131 universes in a single output
   -- enh (keith)   Add timing track renaming on sequencer window and make it automatically update any effects that were using the timing track
   -- bug (keith)   Prevent illegal characters in model and model group names
   -- bug (keith)   Handle duplicate timing tracks in XML file without crashing
   -- enh (keith)   Add additional columns to models export
   -- bug (keith)   Fix test tab handling of models which are single node but multiple nodes such as DMX model
   -- enh (keith)   Add count of effect layers on presets window
2016.40  Jul 10, 2016
   -- bug (gil)   Make the initial pane size larger for Model Preview, House Preview, and Effect Assist windows.  Fixes #617
   -- bug (dkulp) Fix dotted lines in model preview selection stuff
   -- enh (keith) Add end channel to model export
   -- enh (keith) Log very slow rendered frames
   -- bug (keith) Fix value curve problems with some waveforms
   -- bug (keith) Stop custom model leading to crashes if non numerics are entered in grid cells
   -- enh (keith) Add view log to the tools menu so users can easily access the log file
   -- bug (keith) Stop tab dragging on main window
   -- bug (keith) Make video Effect render red if video file does not exist or single line buffer style is used
   -- enh (keith) Add value curve to sparkles
   -- enh (keith) Add preset export/import *.xpreset files
   -- bug (keith) Fix crash in piano when start note > end note
   -- bug (keith) Fix tendril music line effect that was broken in v39
   -- bug (keith) Fix blur effect that was broken in v39
   -- enh (keith) Add model face and state data to xmodel files
   -- bug (gil)   Fix conversion to LOR format.
   -- enh (gil)   Modify On and Colorwash effects to support DMX models.
   -- enh (gil)   New DMX model added with 5 types so far: Top view, Side view, Bars view, Top w/Bars, and Side w/Bars.
   -- bug (gil)   Prevent scenario where model groups could show up to be added to a model group.
   -- bug (gil)   Delete model and all effects from grid and views when a model is deleted. Fixes #611
   -- enh (gil)   More improvements to SuperStar imports of images.
   -- enh (gil)   Better placement of Text Effects imported from SuperStar.
   -- enh (gil)   Add option to average colors for SuperStar import.  More closely matches it's blending method.
   -- enh (gil)   Improve SuperStar import settings for Fan effect.  This effect is close but not an exact duplicate.
   -- enh (gil)   Fix proper start/end angles for flowys imported from Superstar for horizontal layout.
   -- bug (gil)   Fix potential crashes with right-click popup menus in Layout page.
   -- enh (gil)   Make the initial pane size larger for Model Preview, House Preview, and Effect Assist windows.  Fixes #617.
2016.39  Jun 30, 2016
   -- enh (keith) Make network setup save button work like the layout panel ... red when it needs saving.
   -- bug (dkulp) Fix background brightness setting with OpenGL 1.1
   -- enh (keith) Add backup when xlights started or show directory changed
   -- bug (keith) Fix value curve enabling/disabling
   -- bug (keith) Fix piano and VUMeter reaction to changes in timing marks.
   -- enh (keith) Add model state settings and effect to control states.
   -- enh (keith) Add autosave to layout
   -- bug (keith) Ensure effect settings reset when first selected for dropping if they are different to the current effect. Most effects done.
   -- enh (keith) Add value curves to Pinwheel, Shimmer, Spirograph, Ripple effects
   -- enh (gil) DMX effect automatically determines number of channels to use
   -- enh (gil) DMX effect will show node names if defined in place of channel numbers
   -- enh (gil) DMX effect now uses value curves to do ramps and works with single color channels
   -- bug (gil) Fix SuperStar import for Scenes, ImageActions, and Text when layout is horizontal.
   -- bug (gil) Update last preview in effects file when preview is deleted.  Also protect against a bad named stored in the file.
2016.38  Jun 24, 2016
   -- bug (gil) Make sure new previews are added to the model groups preview combo box.
   -- bug (gil) Remove items from View menu when a preview is deleted.
   -- bug (cjd) Don't crash when loading perspectives menu if current perspective not defined
   -- bug (cjd) Update default perspective if it is renamed
   -- bug (cjd) Reload default perspective on show directory change - fixes issue #601
   -- bug (keith) Fix matrix faces not able to be changed to scaled. It always resets to centred.
   -- bug (keith) For text effect with no movement ... x start slider was being ignored. Fixed.
   -- bug (keith) Fix vertical fade on colorwash effect. It only worked if horizontal fade was also selected.
   -- bug (dkulp) Check the file extension based on the lowercase version of the extension, so *.lms and *.LMS both work
   -- enh (gil) NEW: Multiple Previews
   -- bug (gil) Fix SuperStar import for Scenes to account for scaling changes to the Pictures effect.
   -- bug (cjd) Don't crash when loading perspectives menu if current perspective not defined
   -- bug (cjd) Update default perspective if it is renamed
   -- bug (cjd) Reload default perspective on show directory change - fixes issue #601
   -- enh (dkulp) Optimizations for OpenGL for WaveView and model/house previews to pass all vertices/colors/textures in one shot
                          (helps performance, particularly when multiple previews are shown)
   -- enh (dkulp) Fix warnings/crash if background image doesn't exist
   -- bug (dkulp) Fix crash on mac when swithing show directories.
   -- bug (keith) Fix circles effect not changing colour.
   -- bug (keith) Fix #589 - guard against nulll ptr
   -- enh (keith) Implement alternate backup F11 allows the user to specify an alternate backup location and
           backs up there instead. There is no auto backup to this location.
   -- bug (keith) Make video more resilient to missing files
   -- enh (keith) Add models export - now with controller details as well
   -- enh (keith) Add optional backup on save ... this is by default disabled.
   -- enh (keith) Add value curves to: Music, Piano, Tendrils, Lightning, VUMeter, Wave
   -- enh (keith) Allow custom value curve points to be moved sideways as well as vertically.
   -- bug (keith) Fix bugs in piano effect: Piano not drawn without notes source, Polyphonic Transcription broken.
   -- bug (keith) Fix problem with fire effect when buffer size changes
   -- enh (keith) Add square wave to value effect presets
   -- enh (keith) Add multicolour support to Tendrils ... should also fix bug with Tendril not holding colour change
   -- bug (keith) Disable bitmap brightness slider and show/hide button if no bitmap defined on custom model.
2016.37  Jun 10, 2016
   -- enh (keith) Extra logging on startup to isolate problems ... also logs config so we can understand settings.
   -- enh (dkulp) Allow 10K nodes/string for SingleLineModel.  Fixes #592
   -- enh (keith) Allow bars effect centre for compress/expand to be moved
   -- enh (keith) Add spinner model
   -- bug (keith) Fix a possible crash with perspectives.
   -- enh (keith) Make image files in faces and assist easier to load by not having separate filter for each file type.
   -- bug (dkulp) Fix problems with "Single Strand" buffer mode and custom models
   -- bug (dkulp) fix "Per Preview" mode cutting off right most pixels of custom models
2016.36  Jun 06, 2016
   -- enh (keith) Change backup files to *.xbkp extension to minimise risk of users opening them accidentally. Include them in F10 backup.
   -- bug (dkulp) Don't use OpenGL 2.1 if context is 2.0
			Use glMapBufferRange instead of glBufferSubData (latter is SLOOOW on ATI cards)
   -- bug (dkulp) fix the background image button not working
   -- bug (dkulp) Update borders for singlestrand
   -- enh (dkulp) Strip out the defaults for the On/ColorWash and buffer/layer/color panels at load time.   Reduces file size
   -- bug (dkulp) Use an icon bundle with different sized icons to hopefully fix the volume meter issue on windows
   -- bug (keith) Fix custom value curves not being editable when you re-open them
   -- enh (keith) Value curves added to single strand, garlands, fire, plasma, curtain. color wash
   -- bug (keith) Fix tendril effect
   -- bug (keith) Fix custom value curves not being editable when you re-open them
   -- bug (keith) When changing value curves use a sensible set of defaults.
   -- enh (cjd)  Add menu items to load/save perspectives directly
   -- enh (gil)     Add new effect Fill.
   -- enh Improve several effect toolbar icons.
2016.35  Jun 02, 2016
   -- bug (dkulp) Fix colorwash stripping out stuff it shouldn't
   -- bug (dkulp) Minor layout things (removing borders on value curve buttons) and hopefully fix subbuffer panel
   -- bug (dkulp)  If using pixel style "Solid Circle" or "Blended Circle", performance with OpenGL 2/3 was really bad.
   -- bug (dkulp) Ability to disable/change the autosave interval.  Move settings query to its own timer (that doesn't need to be precise)
   -- bug (dkulp)  Fix subbufferpanel not clearing the lines
   -- bug (cjd)  Update makefile for linux
   -- enh (keith) Add value curves to butterfly, bars, spirals, circles
   -- enh (keith) Add a reset to roto-zoom
   -- bug (keith) Fix lights off button. Ensure I send a lights off packet to all lights when it is turned off.
   -- enh (keith) Add value curve to brightness
2016.34  May 31, 2016
   -- bug (gil)     Fix crash when delete key was hit in Layout with no model selected.  Fixes #533.
   -- enh (gil)     Restructure how model groups are handled by providing a panel for them and allowing the
                    members of the group to be managed in the property grid area.
   -- enh (gil)     Add tilt parameter to the Arch model to allow better 3D perspective.  Fixes #250.
   -- enh (keith) Add starfield option to meteors implode/explode look more like a starfield
   -- enh (keith) Add autosave to .working.xml files ... saves every 3 minutes as long as the sequence is not playing
   -- enh (keith) Zoom and rotate added to buffer panel including value curves for most parameters
   -- enh (keith) Blur Value Curve Added
   -- bug (dkulp)   DOn't corrupt rgb-effects if mouth set to (off).  Fixes #577
   -- bug (dkulp)   Refresh the start/end channels when various properties (string type, strands, etc...) are changed. Fixes #575
2016.33  May 25, 2016
   -- bug (dkulp) Maybe fix #571?   Don't post the event, call directly.
   -- bug (dkulp) don't draw the grid and then immediately refresh anyway.
   -- bug (dkulp) Log a few more opengl things, Force GDI driver down to 1.x
   -- enh (keith) General import improvements: filename in title bar, warnings
             if you have not saved mapping and the ability to suppress excessive error messages.
   -- enh (keith) Improved xLights sequence import.
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
   -- enh (keith) VideoReader ... more protection for possible divide by zero errors.
            Includes logging of any problems found.
   -- bug (keith) Make logs 1MB as originally intended. These should zip to well under 50% when sent to us.
   -- bug (cjd) Force TendrilEffect and TextEffect on foreground thread under Linux
2016.31  May 16, 2016
   -- bug (dkulp)   Fix for Model window not displaying
2016.30  May 16, 2016
   -- enh (keith) PGO export for those LOR etc users that would like to use xlights to create their PGO files.
   -- enh (keith) Add scrubbing to audio (hold control key and left mouse button on waveform)
   -- bug (gil)     Correction to FSEQ data layer import.  Should only reset sequence data on import not on render.
   -- bug (dkulp)   Fix for OpenGL on Intel graphics card. Intel 2.1 has issues with POINT_SPRITE
2016.29  May 14, 2016
   -- bug (keith) Fix prompt for custom model import so it happens after you size the custom model
   -- enh (keith) Remove Papagayo tab
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
   -- enh (keith) Add Manual Custom Model building assisted by image/video.
   -- enh (keith) Add LSP timing track import
   -- bug (gil)     Mark the sequence dirty when sequence duration is changed to prompt for save on exit.  Fixes #563.
   -- bug (gil)     Fix jumping back to start briefly after an unpause.  Fixes #565.
2016.28  May 10, 2016
   -- bug (gil) Better fix for effects dragging past zero. This fixes dragging a timing mark to zero seconds.
   -- enh (keith) Add papagayo timing track import via Timing Track Import context menu
   -- enh (keith) Add progress display during rendering all
2016.27  May 09, 2016
   -- bug (dkulp) Fix problems of the model groups that are set to the old default of grid
                  as per preview using a buffer significantly smaller than they used to
   -- enh (keith) Add FPS display when playing the sequence
   -- bug (keith) Fix a bug with filename fixups
   -- enh (keith) Cleanup scaling options on Music Effect ... now via a choice - None/Individual Notes/All Notes
   -- enh (keith) Sparkles now can optionally grow with music intensity
   -- enh (keith) Strobe effect now can optionally grow with music intensity
   -- enh (keith) Fire effect now can optionally grow with music intensity
   -- enh (keith) Add timing track export/import. Includes support for Papagayo and Piano timing tracks.
   -- bug (keith) Lots of fixes to auto custom model generation
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
   -- enh (keith) New option "Tools,Log". This will create a zip file of your xlights log file. No longer need to
           search the hidden appdata directory to find it. Keith described it like this: Add "Package Problem Files"
		   to tools menu which lets the user package current config/sequence/logs when reporting a problem.
   -- enh (dkulp) Sub-Buffers. Add ability to specify what section of a buffer you want the effect applied to
   -- enh (keith) Block manual switching between wizard tabs in auto generation of custom model.
2016.24  May 04, 2016
   -- bug (dkulp) Fix OPENGL calls so that Sound wavefile shows on Intel Graphics cards
2016.23  May 03, 2016
   -- enh (keith) Added new option "Tools, Generate Custom Model"
   -- enh (dkulp) Upgrade to newer OpenGL library.
                  The OpenGL rendering has been updated from the "fixed pipeline" code
				  (which was state of the art in 1998) to using modern OpenGL shaders (>2010).
				  This allows us to better use the capabilities of modern video cards.
				  However, to support older machines, we can drop down to OpenGL 2.1 shaders
				  or even to the old 1.3 fixed pipeline. Modern machines should be faster,
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
   -- bug (keith) Fix bug in music effect not working unless at the start of the song
   -- enh (keith) Add video acceleration and slow down so video fully plays in time periods
                      different to their actual length
   -- bug (keith) Fix sensitivity bug in music effect
   -- bug (keith) Fix spectrograms wider than the model
   -- bug (keith) Similar fix for video
   -- bug (gil)     Fix Pictures effect broken by last release.
2016.21  Apr 22, 2016
   -- enh (keith) Add x/y offset and some new movements to tendril
   -- enh (keith) Custom model export and import allowing simple sharing
   -- enh (keith) Add x offset to piano
   -- enh (keith) Add x and y offsets to select meteor effects
   -- enh (keith) Add x and y offsets to select vumeter effects
   -- enh (keith) Add timing track option for defining piano notes
   -- bug (gil)     Fix first frame and last frame buttons so the grid will scroll.  Fixes #544.
   -- enh (keith) Add Music Effect
   -- enh (keith) Add Note On sub effect to VU Meter - Like On but you can filter the notes it responds to
   -- enh (keith) Add Note Level Pulse sub effect to VU Meter - Like Level Pulse but you can
                      filter the notes it responds to
   -- enh (keith) Add Timing Event Colour sub effect to VU Meter - This changes colour on timing events
   -- enh (keith) Add option to trigger firework effects based on the audio intensity
   -- enh (keith) Add the option to vary meteor effect meteor count based on the audio intensity
   -- enh (keith) Add Colour On sub effect to VU Meter - Selects a colour based on the intensity of the audio
   -- enh (keith) Add the ability to constrain the frequency range of the spectrogram. Columns in
                     this effect are now aligned with MIDI notes.
   -- bug (keith) Ensure pictures, piano and video effect move when moving folders around
   -- enh (keith) Move convert tab into a dialog under TOOLS. Now go to convert "TOOLS,CONVERT"
2016.20  Apr 13, 2016
   -- bug (gil)   Fix right-click Paste not active after a Control-C copy.
   -- bug (gil)   Fix single copied effect not pasting to multiple cell selection.
   -- enh (gil)   Enable right-click Copy, Paste, Delete options for timing section of grid.
   -- enh (gil)   Enable Copy Row and Paste Row for Timing tracks.
   -- bug (gil)   Fix bug preventing some timing effects from being pasted.
   -- bug (gil)   Fix copy/paste bug where single effect copy was not pasting with original duration.
   -- bug (cjd) Install dictionaries/phoneme files on Linux
   -- bug (dkulp) disable the popup as it causes a crash
2016.19  Apr 10, 2016
   -- bug (dkulp) Fix Text not showing
   -- bug (dkulp) Fix icicles buffer width
   -- enh (gil)     Add EditToolBar with Paste By Cell and Paste By Time options.  Fixes #418 and Fixes #421.
   -- enh (gil)     Add ability to select, copy, and paste timing effects. Several restrictions in place.  Fixes #366.
   -- bug (keith) Fix implemented ripple missing triangle option #524
   -- bug (keith) Cleaned up docking windows toolbar icons #519
2016.18  Apr 7, 2016
   -- bug (gil)   Fix bug when Papagayo timing was collapsed and selected grid cell selection wasn't working.
   -- bug (gil)   Fix Delete key operation so it doesn't do a Cut unless Shift is pressed.
   -- bug (dkulp) Fix shift-del/ins on LayoutPanel as well
   -- bug (dkulp) Fix "individual start channel" calculations
   -- bug (dkulp) Set the buffer size correctly for candycanes.  Fixes #521
   -- bug (dkulp) Find a solution to some crashing in release builds
   -- bug (dkulp) Remove model properties dialog and model list dialog
   -- bug (dkulp) Fix crash in editing groups
   -- enh (keith) Add writing of FPP universes file when saving networks
   -- bug (keith) Fix logging on windows ... log file is named xlights_l4cpp.log
         found typically in c:\users\<user>\AppData\Roaming
2016.17  Apr 3, 2016
   -- enh (keith) Add writing of FPP universes file when saving networks
   -- bug (keith) Fix logging on windows ... log file is named xlights_l4cpp.log
          found typically in c:\users\<user>\AppData\Roaming
   -- bug (dkulp) Fix text effect on Windows
   -- bug (dkulp) Fix single lines that are 1 channel/string with multiple lights
   -- bug (dkulp) Fix circle radius
   -- bug (keith) Add true cancel when mis-loading a sequence when in wrong folder ...
                 what it actually does is just delete all models that don't exist ...
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
   -- bug (keith) Fix test dialog handling of multi-universe E131
   -- bug (keith) Fix test dialog handling of different start pixel specifications on models
   -- enh (keith) Move windows version to wxWidgets 3.1
2016.14  Mar 23, 2016
   -- enh (keith) Piano support for audacity file reading.
   -- bug (dkulp) Fix crash if mapping maps a model that isn't used in the source sequence. This fixes #505
   -- enh (dkulp) Change how the group add/remove/editing is handled in Layout
   -- enh (dkulp) New model type "CandyCanes". Added cane rotation
   -- enh (keith) log capture on crash in debug report.
   -- bug (keith) Fix bulbs flashing in test mode.
   -- enh (keith) log4cpp added. These are library routines that allow enhanced logging of errors.
   -- bug (keith) Add back in output control in test mode.
   -- enh (keith) Redevelop Piano effect ... this is a ground up rewrite and is not compatible with the old piano effect.
   -- enh (keith) Add logging as a runtime capability for better debugability
   -- bug (keith) Fix LOR controllers in test mode Fix #502
   -- bug (keith) Add back in multiselection in test mode
   -- bug (gil)     Fix timings not draw correctly when window is resized. Fixes #504.
   -- bug (gil)     Fix Pictures scaling issues.  Fixes #501.
2016.13  Mar 15, 2016
   -- enh (gil) Added gradient option to bars effect
   -- enh (keith) Add a colour aware test mode ... RGBW.
   -- enh (dkulp)   Optimize the redraws on timer events a bit.
                      Drops CPU usage from 95% to about 60% on my machine.
   -- bug (gil)     Send render event for effects pasted to multiple cells.  Fixes #497.
   -- enh (keith) Move test tab to a separate dialog ... add new ways of selecting channels for testing.
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
   -- enh (keith) Add video looping
   -- bug (keith) Fix some video formats that were not playing
   -- bug (keith) Fix audio controls on the toolbar
   -- bug (gil)     Fix bug moving effects that aren't fully visible.  Fixes #488.
   -- bug (gil)     Allow min period effects to move with mouse.  Fixes #479.
   -- bug (dkulp)   Imports from other formats were generating invalid key ID's
   -- bug (dkulp)   Pictures imported from SS were not displaying properly
   -- enh (dkulp)   Update On/ColorWash to only output non-defaults in XML
2016.10  Mar 3, 2016
   -- enh (keith) Support old-school CTRL-INSERT, SHIFT-INSERT and SHIFT-DELETE (windows only)
   -- bug (dkulp)   Fix problem with first pixel with grouped arches in SingleStrand . Fixes #474
   -- bug (dkulp)   Fix crash on close when preview running
   -- bug (dkulp)   Fix color/blur sliders not updating to defaults.  Fixes #477.  Fixes #476.
   -- enh (dkulp)   Split Layer Timing panel into two panels (Layer Blending and Layer Settings)
   -- bug (dkulp)   Fix duplicate effect data problems if seq data exceeds 4GB (64bit only)
                  This failure was found when testing a sequence of 900K channels on a MAC
   -- enh (dkulp)   Add begin/end effect transitions
   -- enh (gil)     Update Morph to use all 8 colors allowing 6 color tails.
   -- enh (keith) Add video effect. Now supports *.mp4, *.mpg, *.avi, *.mov, *.flv, *.mkv, *.asf
   -- bug (gil)     Fix crash when moving effect left or right with arrow keys and not timing track active.  Fixes #473.
   -- enh (keith) Replace media handling with ffmpeg and SDL eliminating the VBR problem and adding new media support.
   -- enh (keith) Add tooltips to explain controller and channel a xLights channel maps to
   -- enh (keith) Handle movement of show directory (as long as it isnt renamed) automatically
2016.9  Feb 24, 2016
   -- enh (gil)   Allow single effect to be moved left and right using arrow keys if timing grid is active.
   -- enh (gil)   Allow single effect to be moved up and down on grid.  No longer restricted to a single model.
   -- enh (keith) Add description against rows on the setup tab Issue #422
   -- enh (keith) Add View->Windows->Dock All menu item to wrangle all free windows back into their docked position
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
   -- enh (keith) Added blur capability in the layer blending window
   -- bug (gil)  Fix crash if down arrow is hit on a timing track.
   -- bug (keith) Fix crash if two 2016 versions are compared
   -- regr(dkulp) If media from XML doesn't exist but is in the Media directory, use it from there.
   -- bug (keith) Fix false positive VBR file detection
2016.8  Feb 16, 2016
   -- bug(gil)   Allow media sequence to play to defined duration and not stop at media length. Fixes #138.
   -- enh(gil)   Add Layered description to tooltip.  Fixes #179.
   -- bug(gil)   Remove incorrect hover tips from main menus. Fixes #242.
   -- bug(gil)   Rebuild sequence data length when media file is changed.
   -- enh(gil)   Model groups are now indicated with an icon on sequencer grid.
   -- enh(gil)   Morph corner images now have a minimum size so they are always easily seen.
   -- enh(keith) Add music suppport to Tendril effect.
   -- bug(dkulp) Fix faces not blinking properly. Fixes #458
   -- bug(dkulp) DisplayElements panel default display is not sized correctly on Mac.  Fixes #466
   -- bug(dkulp) Check to make sure a model exists before adding it to a sequence
   -- enh(keith) Add VUMeter effect (very much beta in this release. Appearance may change in near future releases. Please do not rely on it at this stage)
   -- bug(gil)   Fix selected cells moving into timing area when vertically scrolling.  Fixes #419.
   -- bug(gil)   Fix arrow keys not moving selection properly which could cause hotkey paste in wrong locations. Fixes #432.
   -- bug(gil)   Fix effect duplicating when using arrow keys on grid.  Fixes #424.
   -- enh(gil)   Show mouse position in waveform.
2016.7  Feb 08, 2016
   -- bug(dkulp) Fix crashes on animations
   -- bug(dkulp) Fix crashes caused by old Whole House Models
2016.6  Feb 07, 2016
   -- enh(keith) Add warning when using variable bit rate audio files
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
   -- enh(keith) Add tendrils effect. Also includes introduction of GraphicsContext.
                 Builders please see README.Windows for rebuild instructions.
   -- enh(keith) Add natural snowfall without accumulating to the snowflakes effect
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
   -- enh (dpitts) Made output/start channel in models have correct end output/end channel.
   -- enh (dkulp)  Add some options to SuperStar import for doing some simple image scaling
   -- enh (dkulp)  don't scale image in effect assist to allow editing images smaller than the model
   -- enh (dkulp)  Start adding ability to resize image from effect assist
   -- enh (dkulp)  Able to parse .sup files that have configuration elements (that aren't valid XML) (fixes #394)
   -- bug (dkulp)  Prompt for image directory for SUP import up front so Cancel can work. (fixes #393)
   -- enh (dkulp)  Make sure user_dictionary can load from show dir and make sure it can override stuff in the standard dictionaries.
   -- enh (sean)   Removed built in fade from lightning effect
   -- enh (sean)   changed the backup (F10) so that it saves files <20mbytes. Before it was 10mbytes.
   -- bug (gil)    Prevent magnetically connected effects from being crushed to zero duration.
   -- bug (dkulp)  Fix problem where numChannels*numFrames > 2G
   -- bug (gil)    Fix Marquee not rendering smooth when reverse is checked.
4.2.16  Oct 13, 2015
   -- bug (keith) Fix broken picture wiggle style
   -- enh (sean)  Added slider to give horizontal movement to lightning bolts
   -- enh (sean)  Added check box to give chose to have forked lightning.
4.2.15  Oct 08, 2015
   -- bug (gil) Fixed the labels showing "off"
4.2.14  Oct 07, 2015
   -- enh (ChrisD) Add application launcher for linux
   -- bug (ChrisD) New timing dialog should be titled as such (partial fix for issue #377)
   -- bug (gil)    Buttons will now re-enable after an LSP export model operation.
   -- enh (Keith)  New feature in pictures, tiled pictures
   -- bug (gil)    Fix potential crashes for all imports where models may be hidden.
   -- bug (gil)    Fix LSP import effects when a target model is hidden.
   -- enh (sean)   New effect class Lightning
   -- bug (gil)    Sequence SaveAs was not saving XML in new directory.
   -- bug (gil)    Remove fixed attribute for timing track converted to papagayo timing.
   -- enh (gil)    Add LOR conversion option to show verbose channel mapping.
   -- enh (Keith)  Updated tile code to support x/y offsets in Picture effect
   -- bug (dkulp)  After deleting an effect, if you hit Up/Down, crash will result
   -- bug (dkulp)  Use native ObjectC code for clipboard paste for custom models to support pasting from Numbers
   -- bug (dkulp)  Click on a "node" may not hit test properly if an effect exists on the model or strand layers
   -- bug (dkulp)  Prevent effect from being resized so start is < 0ms which prevent effect updates/rendering/etc..
   -- enh (dkulp)  Add option to twinkle to re-randomize the color after each twinkle
   -- enh (keith)  Added tiling options to picture effect
   -- bug (sean)   Fixed bottom row of plasma. Bug was the bottom row was copied from top row
   -- bug (ChrisD) Fix issue #368 - missing OK/Cancel on Schedule Playlist
4.2.13  Sep 27, 2015
   -- enh (sean) Added new Plasma effect. Now you don't need to use Butterfly to get Plasma effects
   -- bug(dkulp) Fix crash when rendering models that have channels that extend beyond the configured range
   -- bug(dkulp) The "Enable" flag is only honored when changed while outputting to lights
   -- enh(cjd) Add application launcher for linux systems
4.2.12  Sep 22, 2015
   -- enh (sean) Added a new style to Butterfly, Style=10. This allows setting color of Plasma
         also all plasma effects (Styles 6,7,8,9,10 in Butterfly), will use
         Background chunks to change the plasma effect.
   -- bug (gil) * Fix crash if deleting a timing in new sequence wizards.  Fixes #372.
         this bug was actually fixed in 3.2.11
   -- enh (cjd) * Implement issue #237 - Enhancement request: Media file missing,
         prompt for user input
   -- enh (cjd) * Implement issue #367 - Enhancement: Add .out file type
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
   -- bug(cjd) Don't crash when collecting a crash report on linux
   -- bug(cjd) Fix text effect on linux
4.2.8  Sep 08, 2015
   -- bug(gil) Fix bug where play marker wouldn't work if pressing play was the first thing you do and also
                fixed dropping an effect while model playback is paused such that it executes a stop and
                switches to effect play.  Fixes #322.
   -- enh(gil) Use standard File SaveAs dialog for saving sequences. Fixes #336.
   -- enh(gil) Improved dialog when prompting to save sequence changes.
   -- bug(gil) Fix the single cell selection caused by undocumented logic enhancements last release.  Fixes #359.
4.2.7  Sep 06, 2015
   -- bug (cjd) Fixed a number of bugs in Linux version of xLights
   -- bug (cjd) Set colour of multi-selected items to yellow. This fixes the multi model select
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
   -- bug (cjd) Fixed popup error box when selecting models in LAYOUT
   -- enh(cjd) New checkbox for channel overlap check in LAYOUT.
   -- enh(cjd) in LAYOUT fixed flashing when selecting models
   -- bug(gil)  Presets will now populate correctly when switching to a new Show directory.
   -- bug(gil)  Presets created at top level will now show up when program is reopened.
   -- bug(chrisP) Fix export of Falcon .eseq Effects files
4.2.3  Aug 22, 2015
   -- bug(gil)   Version older detection code was wrong....this caused morphs to be converted when they shouldn't.
                  Also fixed morph corner 2b point graphic.
4.2.2  Aug 20, 2015
   -- enh(cjd) in the Layout screen starting and ending channels are now shown. Name,Start and End are clickable columns to sort
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
    -enh (dkulp) Star model can now have multiple star size "layers" to model exploding star type things
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
     --  enh (dkulp) If an empty LMS channel is part of an RGB group, don't consider it empty.
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
   - Enhancement:(Frankr) enhanced the right click process for protecting a cell. Before you had to get focus on the cell by left clicking (turned it yellow), then you could right click. Now just ricght click any cell, you don't need left click first
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
   -- enh (sean) new feature in Meteors effect. Meteors can now fall up
   -- bug (sean) Added check to make it so empty picture selection does not cause hundreds of pop up windows
   -- bug (sean) fixed LOR lms and lcb export
   -- bug (sean) added check to make sure model's have at least 1 pixel wide and high
3.0.14 Jun 13,2013
   -- enh New movie files added to Picture Effect
          Use web page to change your movies into jpg files. http://image.online-convert.com/convert-to-jpg
   -- enh Strobe in Twinkle Effect is now random lights
3.0.13 May 30,2013
   -- enh New Strobe Effect now available in the Twinkle Effect Class
   -- enh Twinkle Effect Class now allows different steps of twinkle.
3.0.12 May 26,2013
   -- enh New type of vertical text where letters are not rotated but stacked on
          top of each other. Finally we have Line 1 one color, line 2 second color.
          Each line of text can have a different font, a different direction.
          There is a position bar for each line of text.
3.0.11 May 15,2013
   -- enh Text can now be rotated 0, 90, 180 or 270 degrees
3.0.10 May 12,2013
   -- enh Changed 32 character limit in the Text effect to be 256 characters
3.0.9  May 11,2013
   -- enh Animated Gifs added to the Pictures effect
3.0.8  Apr 25,2013
   -- enh Added export for LSP to create UserPatterns.xml
          The actual file created will have a *.user extension. rename it to use it
   -- enh (dpitts) New Effect, Fireworks!. Dave Pitts coded up this new effect.
3.0.7  Apr 23,2013
   -- bug Fixed Pallet 3 button. It was not picking up color when it was clicked.
   -- bug Fixed export of Meteors. This effected all sequencers
   -- bug Fixed export of Life. This effected all sequencers
   -- bug Fixed export of Snowstorm. This effected all sequencers
3.0.6  Mar 11,2013
   -- enh Modifed Nutcracker tab to keep a minimum size screen on left window.
   -- enh Added color selection based on distance from center of object for Spirograph.
   -- enh Added the ability to animate the d parameter in spirograph
   -- enh Moved all effects into their own source file.
          RGBEffects.cpp is now smaller and there are 20 new cpp
          files. RenderBars.cpp, RenderMeteors.cpp, .etc.
   -- bug Corrected another bug in LOR lcb export.
3.0.5  Mar 08,2013
   -- bug Corrected color swap in lcb files. 
   -- bug Corrected bug in HLS export.
3.0.4  Mar 04,2013
   -- Added new effects, Tree and Spirograph.
3.0.3  Mar 02,2013
   -- enh Added new effect, Twinkle.
3.0.2  Feb 28,2013
   -- bug Corrected unsigned 24bit value for HLS *.hlsnc export.
3.0.1  Feb 26,2013
   -- enh Added wxGauge to show process for exporting sequence files.
3.0.0  Feb 22,2013
   -- enh Initial Release. This is Matt Browns original porting of Nutcracker into xLights.
