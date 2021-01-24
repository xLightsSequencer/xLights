This folder is used to prepare the ../bin/standard_dictionary file from the CMUDict distribution

CMU Dict information can be found:

https://en.wikipedia.org/wiki/CMU_Pronouncing_Dictionary
http://svn.code.sf.net/p/cmusphinx/code/trunk/cmudict/

https://github.com/cmusphinx/cmudict <= This is the repo I have pulled the data from as it seems to have more words and be occasionally updated

To prepare the file download the following files into this folder:

- README - this is added into standard_dictionary as a comment header meeting our attributiion requirements
- cmudict.dict - this is the actual dictionary
- cmudict.vp - this is some symbols

The utility here will then read these files and generate the standard_dictionary file which should then be placed in the /bin folder. It assumes it is run inside the folder containing these files

The actions this program takes:

1. It inserts the readme first into the output file with each line preceeded by ";;; # "
2. It reads each line from dict and vp files
	- converts to all upper case
	- it adds an extra space after the first word
	- if a line contains a # then the # and everything after it is removed
	- leading and trailing spaces on the line are removed
3. The lines are then sorted alphabetically
