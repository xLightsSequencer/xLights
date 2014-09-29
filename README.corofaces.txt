Corofaces using awk script to produce xml files

1) Go to Papagayo and export each voice you have
2) Build a model for each face. Example face1, face2. pumpkin1,pumpkin2 .etc.
3) Copy these files from C:\Program Files (x86)\xLights and put them into your show directory. Example show directory: C:\Users\sean\Documents\xmas2014
coroface.awk  # This program takes a Papgayo voice file and changes itno a xlights Nutcracker command
coroface.bat  # This is a batch file that can be used to automate making the files
merge_xml.awk # This script will combine multiple xml files into one.

4) run the following command for each voice file you have.
	a) pass in the model name. In the following example the modelname was "face1"
	b) Pick the name of the newly created xlights xml file as the last argument. In this example we are creating voice1.xml
	
gawk -f coroface.awk -v m1=face1 voice1.dat > voice1.xml

5) Go into xlights
6) Do a create a new sequence from a sound file.
Click that you have an xml file.


If you need to combine multiple xml files into one
Syntax: gawk -f merge_xml.awk {firstfile} {secondfile} {thirdfile} {.etc.}  > {output xml file)

Example: combine 3 xml files into one big xml
gawk -f merge_xml.awk voice1.xml voice2.xml voice3.xml > voice.xml

----------------------------------------------------------------------
-             COROFACE Batch file, coroface.bat                    -
-----------------------------------------------------------------------
rem	First copy coroface.awk, coroface.bat and merge_xml.awk from the 
rem
rem		Syntax" gawk -f coroface.awk
rem	edit these next lines and put in what your voice dat files were from Papagayo
rem arguments
rem		"-v m1=face1". -v means you will define a variable. m1 is the variable. it contains
rem						the model name for voice1
rem		"-v v1=3649". This is the last frame in the first pgo file. Look on line 4 of
rem						your pgo file. it will show the last frame.
rem

gawk -f coroface.awk -v m1=face1 -v v1=3649 -v v2=910  voice1.dat voice1b.dat > voice1.xml
gawk -f coroface.awk -v m1=face2  -v v1=3649 -v v2=910 voice2.dat voice2b.dat > voice2.xml
gawk -f coroface.awk -v m1=face3  -v v1=3649 -v v2=910 voice3.dat voice3b.dat > voice3.xml

rem edit this command to contain all of the xml files you need to combine
rem Syntax: gawk -f merge_xml.awk {firstfile} {secondfile} {thirdfile} {.etc.}  > {output xml file)

gawk -f merge_xml.awk voice1.xml voice2.xml voice3.xml > voice.xml
-----------------------------------------------------------------------
------------- End of coroface.bat -------------------------------------

7) Play your sequence