#
#
#    fix_audacity_timing.awk
#    Description: This program will fix audacity label files if the start and end times are the same
#
#    Command:    gawk -f fix_audacity_timing.awk "01 - Carol of the Bells.txt" > "01 - Carol of the Bells2.txt"

{                 # first read in file and load all values into some arrays
	line++;
	start[line]=$1;
	end[line]=$2;
	label[line]=$3;
}
END {
	for (i=1;i<line;i++) # process all entries except for the last one
	{
		if(start[i]==end[i])
			end[i]=start[i+1];
		printf("%s\t%s\t%s\n",start[i],end[i],label[i]);
	}
	if(start[line]==end[line]) end[line]=start[line]+0.05; # Add 50ms to last start time to make a timing mark that is about one frame in size.
	printf("%s\t%s\t%s\n",start[line],end[line],label[line]);
}

