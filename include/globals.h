#ifndef XLIGHTS_GLOBALS_H_INCLUDED_
#define XLIGHTS_GLOBALS_H_INCLUDED_

// not needed, now it si defined in xLightsMain.h, line 242
// #define XLIGHTS_VERSION "4.0.4"
#define XLIGHTS_CONFIG_ID "xLights"
#define XLIGHTS_HELP_URL        "http://sourceforge.net/apps/mediawiki/xlights/index.php?title=Main_Page"
#define XLIGHTS_SCRIPT_HELP_URL "http://sourceforge.net/apps/mediawiki/xlights/index.php?title=XScheduler_Script_Reference"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define XLIGHTS_ANIMATION_FILE     "xlights_animation.xml"
#define XLIGHTS_NETWORK_FILE       "xlights_networks.xml"
#define XLIGHTS_CHANNEL_FILE       "xlights_channels.csv"
#define XLIGHTS_SCHEDULE_FILE      "xlights_schedule.xml"
#define XLIGHTS_RGBEFFECTS_FILE    "xlights_rgbeffects.xml"
#define XLIGHTS_PGOFACES_FILE      "xlights_papagayo.xml"
#define XLIGHTS_RGBEFFECTS_VERSION "0006"
#define XLIGHTS_SEQUENCE_EXT       "xseq"
#define XLIGHTS_SEQ_STATIC_COLUMNS 2

#define XLIGHTS_DESCRIPTION "Use xLights to control you holiday light display"
#define XLIGHTS_LICENSE "xLights Copyright 2010-2013 Matt Brown\nNutcracker Copyright 2012-2013 Sean Meighan\n\nThis file is part of xLights.\n\nxLights-Nutcracker is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. xLights-Nutcracker is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with xLights.  If not, see <http://www.gnu.org/licenses/>.\n\nNutcracker website is http://nutcracker123.com/nutcracker\n\nAcknowledgements\n\nxLights relies on 2 open-source libraries: wxWidgets and irrXML.\n\nCode for serial port communication was inspired by the ctb library written by Joachim Buermann. Code for base64 encoding and decoding was written by Rene Nyffenegger."

#endif
