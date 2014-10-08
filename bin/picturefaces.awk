#
#	edit these next lines and put in what your voice dat files were from Papagayo
#
#	Syntax: gawk -f picturefaces.awk -v m1{model} -v v1={lastFrame} -v v2={lastFrame}  {file1} {file2} > {output}
#
#		-v scale=y
#  gawk -f picturefaces.awk -v m1=D -v v1=3649 -v v2=910 -v scale=y voice1.dat voice1b.dat > picture1.xml
#  gawk -f picturefaces.awk -v m1=C  -v v1=3649 -v v2=910 voice2.dat voice2b.dat > picture2.xml
#  gawk -f picturefaces.awk -v m1=B  -v v1=3649 -v v2=910 voice3.dat voice3b.dat > picture3.xml



BEGIN {
	# find model
	model="face0";
	if(length(m1)>1) model=m1;

	fps=20;
	v1+=0;
	v2+=0;
	v3+=0;
	v4+=0;




	imagefiles["O"]="TREE_O.PNG";
	imagefiles["U"]="TREE_U.PNG";
	imagefiles["AI"]="TREE_AI.PNG";
	imagefiles["E"]="TREE_E.PNG";
	imagefiles["FV"]="TREE_FV.PNG";
	imagefiles["L"]="TREE_L.PNG";
	imagefiles["MBP"]="TREE_MBP.PNG";
	imagefiles["rest"]="TREE_REST.PNG";
	imagefiles["etc"]="TREE_ETC.PNG";
	imagefiles["WQ"]="TREE_WQ.PNG";

	imagefiles["O"]="HC1_O.jpg";
	imagefiles["U"]="HC1_U.jpg";
	imagefiles["AI"]="HC1_AI.jpg";
	imagefiles["E"]="HC1_E.jpg";
	imagefiles["FV"]="HC1_FV.jpg";
	imagefiles["L"]="HC1_L.jpg";
	imagefiles["MBP"]="HC1_MBP.jpg";
	imagefiles["rest"]="HC1_REST.jpg";
	imagefiles["etc"]="HC1_ETC.jpg";
	imagefiles["WQ"]="HC1_WQ.jpg";


	xml_header(model);

	file=0;
	line=0;
	maxFrames=0;
	"CD" | getline; # Get current directory and parse it using getline

	current_directory = $0;

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
		phoneme=$2;

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

		xml_data(seconds, phoneme,imagefiles,current_directory,scale);
	}
}
END {


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

function xml_data(seconds, phoneme,imagefiles,current_directory,scale)
{
	print "<tr>";
	printf ( "<td Protected=\"0\">%7.3f</td>\n",seconds);
	printf ( "<td Protected=\"0\">%s</td>\n",phoneme);
	printf ( "<td Protected=\"0\">Pictures,None,Effect 1,ID_CHECKBOX_LayerMorph=0,ID_SLIDER_SparkleFrequency=200,ID_SLIDER_Brightness=100,ID_SLIDER_Contrast=0,ID_SLIDER_EffectLayerMix=0,E1_SLIDER_Speed=10,E1_TEXTCTRL_Fadein=0.00,E1_TEXTCTRL_Fadeout=0.00,E1_CHECKBOX_FitToTime=0,E1_CHECKBOX_OverlayBkg=0,");

	filename = current_directory "\\" imagefiles[phoneme];

	gsub("My Documents","Documents",filename);
	direction="none";
	if(scale=="y") direction="scaled";

	printf ("E1_TEXTCTRL_Pictures_Filename=%s,E1_CHOICE_Pictures_Direction=%s,E1_SLIDER_Pictures_GifSpeed=20,",filename,direction);

	printf("E1_BUTTON_Palette1=#FF0000,E1_CHECKBOX_Palette1=1,E1_BUTTON_Palette2=#00FF00,E1_CHECKBOX_Palette2=1,E1_BUTTON_Palette3=#0000FF,E1_CHECKBOX_Palette3=0,E1_BUTTON_Palette4=#FFFF00,E1_CHECKBOX_Palette4=0,E1_BUTTON_Palette5=#FFFFFF,E1_CHECKBOX_Palette5=0,E1_BUTTON_Palette6=#000000,E1_CHECKBOX_Palette6=0,E2_SLIDER_Speed=10,E2_TEXTCTRL_Fadein=0.00,E2_TEXTCTRL_Fadeout=0.00,E2_CHECKBOX_FitToTime=0,E2_CHECKBOX_OverlayBkg=0,E2_BUTTON_Palette1=#FF0000,E2_CHECKBOX_Palette1=1,E2_BUTTON_Palette2=#00FF00,E2_CHECKBOX_Palette2=1,E2_BUTTON_Palette3=#0000FF,E2_CHECKBOX_Palette3=0,E2_BUTTON_Palette4=#FFFF00,E2_CHECKBOX_Palette4=0,E2_BUTTON_Palette5=#FFFFFF,E2_CHECKBOX_Palette5=0,E2_BUTTON_Palette6=#000000,E2_CHECKBOX_Palette6=0</td>\n");

	print "</tr>";
}

function xml_footer()
{
	print "</xsequence>";
}
