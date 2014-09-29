#
#	merge_xml.awk
#	Description: This program will take multiple xml files and join them
#			together.
#
#	Command:	gawk -f merge_xml.awk voice11.xml
#
BEGIN {
	line=0;
	h=0;
	header=2;
	file=models=0;
	n=0; tr=-1;
	for(i=1;i<=100;i++)
		maxModel[i]=0;
}
{
	line++;
	h++;	# header line counter
	#print header,h,n, tr,$0;

	if($1=="<td>Label</td>")
	{
		header=1;
		file++;
		if(file>maxFile) maxFile=file;
		file_array[n]=file;
		h=0;
	}
	if($1=="</tr>") header=0;
	if(header==1 && h>0)
	{
		if(index($0,"<td>")>0) 
		{
			split($0,a,"<td>");
			split(a[2],b,"</td>");
			#print "MODELS: " b[1],$0;
			models++;
			model_array[models]=b[1];

		}
		header_seen=1;
	}
	else if(header==0 && substr($1,1,1)=="<")
	{


		if($1=="<tr>")
		{
			tr=0;
			seconds="0";
		}
		else
		{
			tr++;
			if(tr==1) 
			{
				seconds=parse_td($0);
				if(index(seconds,".")>0)
				{
					n++;
					file_array[n]=file;
					seconds_array[n]=seconds " " n;
					##	print "file " file " seconds_array[" n "]=" seconds;
				}
			}
			if(tr==2) 
			{
				if(index(seconds,".")>0)
				{
					label=parse_td($0);
					label_array[n]=label;
				}
			}
			if(tr>=3)
			{
			
				if(index(seconds,".")>0)
				{
					model=tr-2;
					if(length($0)>100) 
					{
						if(model>maxModel[file]) maxModel[file]=model;
						effect[n]=parse_td($0);
						file_tr[n]=file " " model;
						#	print "effect[" n "]=" seconds_array[n] " " label_array[n] " " effect[n];
					}
				}
			}
		}
	}
}
END {
	maxN=n;


	sort(seconds_array);


	#print "MaxN,maxModel = " file,maxN,maxModel[file],maxFile;



	xml_header(models,model_array);
	xml_body(maxN,seconds_array,label_array,effect,file_tr,maxModel,maxFile,models);
	xml_footer();

}
function xml_header(models,model_array)

{
	print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	print "<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\">";
	print "<tr>";
	print "\t<td>Start Time</td>";
	print "\t<td>Label</td>";
	for(m=1;m<=models;m++)
		printf "\t<td>%s</td>\n",model_array[m];
	print "</tr>";
}

function calculate_row(n,file_tr,maxModel,maxFile)
{
	split(file_tr[n],b," "); # gives us the file number and the tr number
	file=b[1];
	tr=b[2];

	if(file==1) model_row=tr;
	else
	{
		model_row = 0;
		for(f=2;f<=file;f++)
			model_row += maxModel[f-1];
		model_row += tr;
	}

	return model_row;
}

function color_wash()
{
	return "Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=1,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0";

}

function xml_body(maxN,seconds_array,label_array,effect,file_tr,maxModel,maxFile,models)
{
	total_models=0;
	for(f=1;f<=maxFile;f++)
		total_models += 	maxModel[f];

	for (m=1;m<=total_models;m++)
	{
	#	effect_array[m]=color_wash();
		effect_array[m]="";
	}

	#	model_row has the sum of all teh models from the various files passed in.
	for(i=1;i<=maxN;i++)
	{
		split(seconds_array[i],a," ");
		seconds = a[1];
		n=a[2];

		split(seconds_array[i+1],a," ");
		seconds_future = a[1];
	row=calculate_row(n,file_tr,maxModel,maxFile);
		effect_array[row]=effect[n];
		if(seconds!=seconds_future)
		{
			printf "<tr>\n";

			printf "\t<td Protected=\"0\">%s</td>\n", seconds;
			printf "\t<td Protected=\"0\">%s|n=%d</td>\n", label_array[n],n;
			for (m=1;m<=total_models;m++)
			{
				if((seconds_future-seconds)>1) gsub("ID_CHECKBOX_LayerMorph=0","ID_CHECKBOX_LayerMorph=1",effect_array[m]);
				printf "\t<td Protected=\"0\">%s</td>\n",effect_array[m];
				if((seconds_future-seconds)>3) # if next phoneme is 3 or more seconds, then blank
					effect_array[m]=color_wash();
				else
					effect_array[m]="";
			}
			printf "</tr>\n";
		}
	
	}	
}

function xml_footer()
{
	print "</xsequence>";
}

function parse_td(buff)
{
	split(buff,a,"<td Protected=\"0\">");
	split(a[2],b,"</td>");
	#print "parse_td " buff "b1 " b[1] " b2 " b[2];
	return b[1];
}

function alen(array,   dummy, len) {   # get length of an array
	for (dummy in array)
		len++;

	return len;
}

function sort(array,   haschanged, len, tmp, i)
{
	len = alen(array);
	haschanged = 1;

	while ( haschanged == 1 )
	{
		haschanged = 0;

		for (i = 1; i <= len - 1; i++)
		{
			# tmp array has values like "1.123 11", "12.222 144"
			#	we want to sort the first token and treat it as a number. force cast by adding 0.0
			split(array[i],a," ");
			split(array[i+1],b," ");
			v1=a[1]+0.0;
			v2=b[1]+0.0;
			if (v1 > v2)
			{
				tmp = array[i];
				array[i] = array[i + 1];
				array[i + 1] = tmp;
				haschanged = 1;
			}
		}
	}
}

