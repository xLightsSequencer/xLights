#
#	edit these next lines and put in what your voice dat files were from Papagayo

#gawk -f coroface.awk -v m1=face1 -v v1=3649 -v v2=910  voice1.dat voice1b.dat > voice1.xml
#gawk -f coroface.awk -v m1=face2  -v v1=3649 -v v2=910 voice2.dat voice2b.dat > voice2.xml
#gawk -f coroface.awk -v m1=face3  -v v1=3649 -v v2=910 voice3.dat voice3b.dat > voice3.xml



BEGIN {
	# find model
	model="face0";
	if(length(m1)>1) model=m1;
	while (getline < "xlights_rgbeffects.xml" > 0)
	{
		if($1=="<model" && $2=="name=\"" model "\"") 
		{
			#print $0; 
			split($0,a,"CustomModel=");
			split(a[2],b,"\"");
			#print b[2];
			n=split(b[2],c,";");

			for (i=1;i<=n;i++)
			{
				#print n-i "(" i ")> " c[i];
				k=split(c[i],d,",");
				numb="";
				for (j=1;j<=k;j++)
				{
					#printf ("%2d",d[j]);
					x=d[j]+0;
					if(index(number[x],":")>0)
						;
					else
					{
						number[x] = numb j-1 ":" n-i ;
					}
				}
				#print " ";
			}
		}
	}
	fps=20;
	v1+=0;
	v2+=0;
	v3+=0;
	v4+=0;


	p[1]="O"; pindex["O"]=1;
	p[2]="U";  pindex["U"]=2;
	p[3]="AI";  pindex["AI"]=3;
	p[4]="E";  pindex["E"]=4;
	p[5]="FV";  pindex["FV"]=5;
	p[6]="L";  pindex["L"]=6;
	p[7]="MBP";  pindex["MBP"]=7;
	p[8]="rest";  pindex["rest"]=8;
	p[9]="etc";  pindex["etc"]=9;
	p[10]="WQ";  pindex["WQ"]=10;

	channels["O"]=4;
	channels["U"]=4;
	channels["AI"]="2:3";
	channels["E"]="2:6";
	channels["FV"]=6;
	channels["L"]=6;
	channels["MBP"]=6;
	channels["rest"]=6;
	channels["etc"]=5;
	channels["WQ"]=5;
	xml_header(model);

	file=0;
	line=0;
	maxFrames=0;

}


#	sample voice file
#MohoSwitch1
#1 rest
#4 rest
#5 AI
#15 etc
#30 L
#35 O
#40 etc
#50 U
#55 MBP



{
	if($1=="MohoSwitch1") file++;
	if(NF==2)
	{
		frame_timing = 1.0/fps;

		frame=$1;

		if(frame>maxFrame[file]) maxFrame[file]=frame;
		sum_previous_frames=0;

		if(file==1) 
			calculated_frame = frame 
		else if(file==2)
			calculated_frame = frame + v1;
		else if(file==3)
			calculated_frame = frame + v1+v2;
		else if(file==4)
			calculated_frame = frame + v1+v2+v3;

		seconds = calculated_frame*frame_timing;
		#printf "seconds=%d,file=%d,frame=%d,calculated_frame=%d,v1=%d,v2=%d,$0=%s\n",seconds,file,frame,calculated_frame,v1,v2,$0;
		ch = channels[$2];
		n=split(ch,a,":");
		x=pindex[$2];
		x_y="";
		for(i=1;i<=n;i++)
		{

			x=a[i];
			#printf ("%d %7.3f %s ch=%s %s\n",i ,secs,$2,a[i],number[x]);
			phoneme = $2;
			if(i==1)
				x_y = number[x];
			else if (i>=2)  # We have nultiple channels for this phoneme
				x_y = x_y ":" number[x];
			Outline_x_y = number[1];
			Eyes_x_y = number[8];
		}
		xml_data(seconds, phoneme,x_y,Outline_x_y,Eyes_x_y)
	}
}
END {

	print "<x_y_locations>\n";
	for (x=0;x<=10;x++)
	{
		phoneme=p[x];
		ch = channels[phoneme];
		print x ">>  " number[x];
	}
	print "</x_y_locations>\n";
	xml_footer();
}
function xml_header(model)
{
	print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	print "<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\">";
	print "<tr>";
	print "<td>Start Time</td>";
	print "<td>Label</td>";
	print "<td>" model "</td>";
	print "</tr>";

	print "<tr>";
	printf("    <td Protected=\"0\">0.000</td>\n");
	printf("    <td Protected=\"0\">Blank</td>\n");
	printf("    <td Protected=\"0\">Color Wash,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,E1_SLIDER_ColorWash_Count=1,E1_CHECKBOX_ColorWash_HFade=0,E1_CHECKBOX_ColorWash_VFade=0,E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=0,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=0,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=1,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n");

	print "</tr>";
}		

function xml_data(seconds, phoneme,x_y,Outline_x_y,Eyes_x_y)
{
	print "<tr>";
	printf ( "<td Protected=\"0\">%7.3f</td>\n",seconds);
	printf ( "<td Protected=\"0\">%s:%s</td>\n",phoneme,x_y);
	printf "<td Protected=\"0\">CoroFaces,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0";



	printf ",E1_CHOICE_CoroFaces_Phoneme=%s,E1_TEXTCTRL_X_Y=%s,E1_TEXTCTRL_Outline_X_Y=%s,E1_TEXTCTRL_Eyes_X_Y=%s,",phoneme,x_y,Outline_x_y,Eyes_x_y;
	printf "E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=1,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=1,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=0";

	printf ",E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n";

	print "</tr>";
}

function xml_footer()
{
	print "</xsequence>";
}
