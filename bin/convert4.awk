#
#	convert4.awk
#	Description: This program will take multiple xml files and join them
#			together.
#
#	arguments
#		pass in the awk command file "-f convert4.awk"
#		pass in an additional two audacity timing file to add into converted file
#			-v t1="file name with spaces.txt"
#			-v t2="file name with space22s.txt"

#	Command:	gawk -f convert4.awk wizards.xml > wixzards.xl4

#	with one additional audacit label file
#       	gawk -f convert4.awk  -v t1="04 - Wizards In Winter (Instrumental)_bars.txt" -v t2="04 - Wizards In Winter (Instrumental)_beats.txt"  w.xml > w4.xml
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
			if(tr==1) # Seconds row
			{
				seconds=parse_td($0);
				if(index(seconds,".")>0)
				{
					n++; # counter of lines that have a seconds value
					file_array[n]=tr-2;
					seconds_array[n]=seconds " " n;
					#	print "file " file " seconds_array[" n "]=" seconds;
				}
				else
					print "ERROR, no seconds. " $0;;
			}
			if(tr==2) # Label row
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
						effectarray[n,model]=parse_td($0);
						file_tr[n]=file " " model;
						#	print $0;
						#	print "effect[" n "]=" seconds_array[n] " " label_array[n] " " 
						#	"file_tr[n] = " file_tr[n] " "
						effect[n] " " substr($0,1,50);
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
	for(m=1;m<=models;m++)
	{
		#	model = model_array[m];
		xml_body(m,model,maxN,seconds_array,effect,file_tr,file_array,maxModel,maxFile,models);
	}
	xml_footer(maxN,seconds_array,t1,t2);

}
function xml_header(models,model_array)

{
	print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	print "<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\">";
	print "<DisplayElements>";
	print "\t<Element type =\"timing\" name=\"Song Timing\"  visible=\"1\" active=\"1\"/>";
	if(length(t1)>0) print "\t<Element type =\"timing\" name=\"t1\"  visible=\"1\" active=\"0\"/>";
	if(length(t2)>0) print "\t<Element type =\"timing\" name=\"t2\"  visible=\"1\" active=\"0\"/>";
	for(m=1;m<=models;m++)
		printf "\t<Element type =\"model\" name=\"%s\"  visible=\"1\"/>\n",model_array[m];
	print "</DisplayElements>";
	print "\n<ElementEffects>";



	# <Element type ="timing" name="Beattrack" collapsed="0" visible="1" active="1"/>
	# <Element type ="timing" name="Singers" collapsed="0" visible="1" active="0"/>
	# <Element type ="view" name="Minitrees" collapsed="0" visible="1"/>
	# <Element type ="view" name="North Poles" collapsed="0" visible="1"/>
	# <Element type ="model" name="Megatree" collapsed="0" visible="1"/>
	#  <Element type ="model" name="Center Mega" collapsed="0" visible="1"/>
	# <Element type ="view" name="Verticals" collapsed="0" visible="1"/>
	# <Element type ="model" name="Left Mega" collapsed="0" visible="1"/>
	# </DisplayElements>

	# <Element type='model' name = "Megatree">
	#        <Effect protected="0" startTime="0.000" endTime="0.500">Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</Effect>
	#        <Effect protected="0" startTime="1.000" endTime="1.500">Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</Effect>
	#        <Effect protected="0" startTime="2.000" endTime="'2.500">Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</Effect>
	#        <Effect protected="0" startTime="3.000" endTime="5.500">Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</Effect>
	#    </Element>


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

function xml_body(m,model,maxN,seconds_array,effect,file_tr,file_array,maxModel,maxFile,models)
{



	printf "<Element type='model' name = \"%s\">\n",model_array[m];

	for(i=1;i<=maxN;i++)
	{
		#	print "i,seconds_array[i] ",i,seconds_array[i];
		#	print "file_tr[n] file_array[n]",file_tr[i],file_array[i];
		#	print " effectarray[n][m]= " 	effectarray[n,m];
		split(seconds_array[i],a," ");
		seconds = a[1];
		n=a[2];

		split(seconds_array[i+1],a," ");
		seconds_future = a[1];
		row=calculate_row(n,file_tr,maxModel,maxFile);
		effect_array[row]=effect[n];
		if(seconds!=seconds_future)
		{
			if(seconds_future<seconds) seconds_future=seconds;
			id = (m-1)*maxN + i;
			printf "\t<Effect id=\"%d\" Protected=\"0\" startTime=\"%7.3f\" endTime=\"%7.3f\">%s</Effect>\n",
			       id,seconds,seconds_future,	effectarray[n,m];
		}


	}
	print "</Element>";	
}

function xml_footer(maxN,seconds_array,t1,t2)
{

	print "<Element type='timing' name=\"Song Timing\">";
	for(i=1;i<=maxN;i++)
	{
		split(seconds_array[i],a," ");
		seconds = a[1];
		n=a[2];

		split(seconds_array[i+1],a," ");
		seconds_future = a[1];
		if(seconds_future<seconds) seconds_future=seconds;

		printf ("\t<Effect protected=\"0\" startTime=\"%7.3f\" endTime=\"%7.3f\"/>\n",seconds,seconds_future);
	}
	print "</Element>";


	if(length(t1)>0)
	{
		print "<Element type='timing' name=\"t1\">";
		while (getline<t1 > 0)
		{
			printf ("\t<Effect protected=\"0\" startTime=\"%7.3f\" endTime=\"%7.3f\"/>\n",$1,$2);
		}

		print "</Element>";
	}

		if(length(t2)>0)
	{
		print "<Element type='timing' name=\"t2\">";
		while (getline<t2 > 0)
		{
			printf ("\t<Effect protected=\"0\" startTime=\"%7.3f\" endTime=\"%7.3f\"/>\n",$1,$2);
		}

		print "</Element>";
	}

	print "</ElementEffects>";

	id++;
	print "<nextid>" id "</nextid>";
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

