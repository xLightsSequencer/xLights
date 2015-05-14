#
#	This awk script will create a DOS Batch file.
#	Batch file is feed with a list of all files that end with _bars.
#	for each bars file, base filename is calculated.
#	a mkdir is created for the basename
#	a move of all files is done from the songs directory to the newly created song subdirectory
#
BEGIN {
barsbat="bars.bat";
print "# bars.bat" > barsbat;
}
{
print $0;
split ($0,a,"_bars");
directory=a[1];
print "mkdir \"" a[1] "\"" > barsbat;

print "gawk -f bars4.awk -v filename=\"" $0 "\" \"" $0 "\"" > barsbat;

print "move /Y \"" a[1] "*.*\" \"" directory "\"" > barsbat;
}
#dir /b /w *_bars.txt* | gawk -f bars.awk
#ls *_bars.txt | gawk -f bars.awk