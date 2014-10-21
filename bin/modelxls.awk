#
#	Program: modelxls.awk
#	Purpose: This script will parse your xlights_rgbeffects.xml file and make a csv file ready for Excel
#	Output: xlights_rgbeffects.csv
#	Run Command: gawk -f modelxls.awk xlights_rgbeffects.xml > xlights_rgbeffects.csv
#	Author: Sean Meighan (sean@meighan.net)
#
#	
#
BEGIN {

	## <model name="MEGA" DisplayAs="Tree 270" StringType="RGB Nodes" parm1="24" parm2="110" parm3="2" StartChannel="97" StartSide="B" Dir="L" Antialias="0" ModelBrightness="0" MyDisplay="1" offsetXpct="-0.4291" offsetYpct="-0.0132" PreviewScale="0.4800" PreviewRotation="0"/>
#<model name="Arches" DisplayAs="Arches" StringType="RGB Nodes" parm1="6" parm2="28" parm3="1" StartChannel="8017" StartSide="B" Dir="L" Antialias="0" ModelBrightness="0" MyDisplay="1" offsetXpct="-0.1068" offsetYpct="0.3992" PreviewScale="0.4000" PreviewRotation="0"/>

	line=0;
	print "Model,Start_Channel,Start_Node,\"Strings x Nodes\",DisplayAs,StringType,\"Part of My Display\"";

}
{
	if($1=="<model")
	{
		line++;
		model = parse("name=");
		DisplayAs=parse("DisplayAs=");
		StringType=parse("StringType=");
		strings = parse(" parm1=");
		nodes = parse(" parm2=");
		strands = parse(" parm3=");
		start_channel = parse(" StartChannel=");
		MyDisplay = parse(" MyDisplay=");



		#print  model "|" DisplayAs "|" StringType " " strings "x" nodes "Strands=" strands " channel=" start_channel;
		#models[line]=model;
		start_node = int ((start_channel+2)/3);
		MyDisplayYesNo="N";
		if(MyDisplay==1) MyDisplayYesNo="Y";
		print "\"" model "\"," start_channel "," start_node ",\"" strings "x" nodes "\",\"" DisplayAs "\",\"" StringType "\"," MyDisplayYesNo;
	}
}

function parse(token)
{
	split($0,a,token);
	split(a[2],b,"=");
	split(b[1],c,"\"");
	return c[2];
}

END {
	}

