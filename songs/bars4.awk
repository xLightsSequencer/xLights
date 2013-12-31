#
#	This awk script will create a DOS Batch file.
#	Batch file is feed with a list of all files that end with _bars.
#	for each bars file, base filename is calculated.
#	a mkdir is created for the basename
#	a move of all files is done from the songs directory to the newly created song subdirectory
#
#	gawk -f ../bars4.bat -v filename="01 carol of bells_bars.txt"

BEGIN {
barsbat="bars.bat";
n=0;
line=0;
}
{
print $0;
split (filename,a,"_bars");
directory2=a[1] "_2bars.txt";
directory4=a[1] "_4bars.txt";
n++;

if(n%2==1) print $0 >> directory2;
if(n%4==1) print $0 >> directory4;
}
#dir /b /w *_bars.txt* | gawk -f bar.awk