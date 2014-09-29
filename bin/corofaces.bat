rem
rem	edit these next lines and put in what your voice dat files were from Papagayo
rem	First copy coroface.awk, coroface.bat and merge_xml.awk from the 
rem
rem		Syntax" gawk -f coroface.awk -v m1=xxx -v v1=nnn voice1.dat voice1b.dat > voice1.xml
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