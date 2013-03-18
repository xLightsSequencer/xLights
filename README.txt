xLights is a program that allows you to play the sequence files from Vixen and
LOR. It has the ability to drive USB DMX controllers, E1.31 ethernet
controllers.

xLights has a extensive scheduler.

Nutcracker is a program that generates animated effects on massive RGB devices
such as megatrees, matrices, and arches. 

Nutcracker is a pre generator before you get into your sequencer. You define
the target model. You specify whether you have a Megatree, Arch, eave, Star,
Matrix. After modeling your target, Nutcracker allows you to next pick effects
from the effect class library. Spirals, Meteors, Bars, Text are all available. 

Finally, if you like the effect you have created , you pick a sequencer for
Nutcracker to export to.

Currently Nutcracker Supports 4 sequencers

Vixen 2.1, 2.5: Produces *.vir and *.vix files
LOR S2 and S3: Produces *.lcba nd *.lms files
HLS: Produces hlsnc file
LSP: Produces UserPatterns.xml

xLights version History:
   2010 Original xLights released
   2012e Latest version of xLights from December 2012 that is used to enhance 
      Nutcracker tab.

Nutcracker Version History:

   PHP, web page Versions
   --------------------
      1.0 Released Feb 2012, one effect spirals. 
      2.0 Released Aug 2012, 18 effects and projects and songs
   C++, Integrated into xLights
   ----------------------------
      3.0 Pre-release Jan 2013: Complete rewrite of Nutcracker into C++ plus
          wxWidgets by Matt Brown. 
	
It is our goal to remove the need for WAMP or XAMPP

Nutcracker 3.0 Releases

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
