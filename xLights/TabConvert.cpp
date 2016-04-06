/***************************************************************
 * Name:      xLightsMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#define FRAMECLASS xLightsFrame::

#include "xLightsMain.h"
#include "ConvertDialog.h"
#include "FileConverter.h"

#include <wx/msgdlg.h>

void FRAMECLASS ConversionError(const wxString& msg)
{
    wxMessageBox(msg, wxString("Error"), wxOK | wxICON_EXCLAMATION);
}
void FRAMECLASS SetStatusText(const wxString &msg) {
    StatusBar1->SetStatusText(msg);
}

void FRAMECLASS ConversionInit()
{
    long TotChannels=NetInfo.GetTotChannels();
    mediaFilename.clear();
    ChannelColors.clear();
    ChannelNames.clear();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    SeqData.init(0, 0, 50);
}

void FRAMECLASS SetMediaFilename(const wxString& filename)
{
#ifndef NDEBUG
    log4cpp::Category& logger = log4cpp::Category::getRoot();
    logger.debug("Setting media file to: " + filename + "\n");
#endif

    mediaFilename=filename;
    if (mediaFilename.size() == 0)
    {
        mMediaLengthMS = 0;
        return;
    }

    wxPathFormat PathFmt = mediaFilename.Contains(wxString("\\")) ? wxPATH_DOS : wxPATH_NATIVE;
    wxFileName fn1(mediaFilename, PathFmt);
    if (!fn1.FileExists())
    {
        wxFileName fn2(CurrentDir,fn1.GetFullName());
        mediaFilename=fn2.GetFullPath();
    }
}

void FRAMECLASS ClearLastPeriod()
{
    int LastPer = SeqData.NumFrames()-1;
    for (size_t ch=0; ch < SeqData.NumChannels(); ch++)
    {
        SeqData[LastPer][ch] = 0;
    }
}

#define string_format wxString::Format

void FRAMECLASS WriteVirFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf)
{
    wxString buff;

    int ch, p;
    wxFile f;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }

    for (ch = 0; ch < numChans; ch++)
    {
        SetStatusText(wxString("Status: ") + string_format(" Channel %ld ", ch));

        buff = "";
        for (p = 0; p < numPeriods; p++)
        {
            buff += string_format("%d ", (*dataBuf)[p][ch]);
        }
        buff += string_format("\n");
        f.Write(buff);
    }
    f.Close();
}

void FRAMECLASS WriteLSPFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf, int cpn)
{
    /*  MrChristnas2000 (from DLA forum) investigated the lsp xml file for LSP 2.8

    Here are some of his notes

    eff="1" -- effect is Ramp Up
    eff="2" -- effect is Ramp Down
    eff="3" -- effect is ON
    eff="4" -- effect is OFF
    eff="5" -- effect is Twinkle
    eff="6" -- effect is Shimmer
    eff="7" -- effect is NO Change

    One second is = 88200
    Note pos="2000" is the TimeInterval timing mark point.

    First observation
    Each track allways ends with the line
    <TimeInterval eff="7" dat="" gui="" in="1" out="1" pos="100000000" sin="-1" att="0" />
    It is the only line with the value of att="0"


    Second pattern created is a 5 second with 1 ch Red, 1 ch Blue, 1 ch Green and last Ch OFF
    Group Name = Test Patterns
    Effect Name = Ch1R.Ch2B.Ch3G.Ch4Off

    Next observation is that the two entries are the color of the RGB effect
    bst="-16711936" ben="-16711936" have to do with the color in and the color out.

    Effect Name = Ch1RB.Ch2BG.Ch3GW.Ch4Off

    Red to Blue
    bst="-65536" ben="-16776961"
    Blue to Green
    bst="-16776961" ben="-16711936"
    Green to White
    bst="-16711936" ben="-1"

    Off
    bst="-1" ben="-1"

    Next obversation
    is that the first and last line in every track has the value
    att="0"
    As well as any line that has a effect value

    Next obversation
    Lines with continuation of an effect has the value
    eff="7" dat="" gui="" in="1" out="1" pos="2000" sin="-1" att="2"
    The only changing value is the time position 'pos'

    Effect Name = Ch1Ronoff.Ch2Bonoff.Ch3Gonoff.Ch4Off

    Another observation is when an ON effect is added/changed that the line contains the following full data block
    dat="&lt;?xml version=&quot;1.0&quot;
    encoding=&quot;utf-16&quot;?&gt;&#xD;&#xA;&lt;ec&gt;&#xD;&#xA;
    &lt;in&gt;100&lt;/in&gt;&#xD;&#xA;
    &lt;out&gt;100&lt;/out&gt;&#xD;&#xA;&lt;/ec&gt;"
    gui="{DA98BD5D-9C00-40fe-A11C-AD3242573443}"
    This does not seem change from pattern to pattern save.
    I also removed it from a saved pattern and it didn't seem to make any difference with or without it.

    Another observation is that an OFF line is allways:
    <TimeInterval eff="4" dat="" gui="{09A9DFBE-9833-413c-95FA-4FFDFEBF896F}" in="1" out="1" pos="4410" sin="-1" att="0" bst="-1" ben="-1" />
    The only changing value is the time position 'pos'

    Effect Name = Ch1RrDn.Ch2BRu.Ch3GRd.Ch4Shmr

    Effect Name = Ch1RrDn.Ch2BRu.Ch3GRd.Ch4Twnkl


    Last obversation is that only when a change in effect type is a value in the gui="" inserted


    Reference info.
    Mili Sec	Tim Mk Val
    1	4410
    2	8820
    3	13230
    4	17640
    5	22050
    6	26460
    7	30870
    8	35280
    9	39690
    10	44100
    11	48510
    12	52920
    13	57330
    14	61740
    15	66150
    16	70560
    17	74970
    18	79380
    19	83790
    20	88200


    This table seems to hold from save to save of effects.
    Effect 2 gui value
    49E1F143-321A-4f5b-9F39-32984FF12410
    Effect 1 gui value
    1B0F1B59-7161-4782-B068-98E021A6E048
    Effect 3 gui value
    DA98BD5D-9C00-40fe-A11C-AD3242573443
    Effect 4 gui value
    09A9DFBE-9833-413c-95FA-4FFDFEBF896F


    for (ch=0; ch+2 < numChans; ch+=3 ) // since we want to combine 3 channels into one 24 bit rgb value, we jump by 3
    {
    >SetStatusText(wxString("Status: " )+string_format(" Channel %ld ",ch));

    buff="";

    for (p=0; p < numPeriods; p++, seqidx++)
    {
    rgb = ((*dataBuf)[(ch*numPeriods)+p]& 0xff) << 16 |
    ((*dataBuf)[((ch+1)*numPeriods)+p]& 0xff) << 8 |
    ((*dataBuf)[((ch+2)*numPeriods)+p]& 0xff); // we want a 24bit value for HLS

    */

    wxString ChannelName, TestName, xmlString, guiString;
    int ch, p, channels_exported = 0;
    int pos, bst, old_bst, ben;
    unsigned long rgb;
    float seconds;
    wxFile f;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");

    f.Write("<ArrayOfPattern xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n");
    f.Write("\t<Pattern>\n");
    f.Write("\t<GroupName>Nutcracker</GroupName>\n");


    wxString m_Path, m_Name, m_Ext;
    wxFileName::SplitPath(filename, &m_Path, &m_Name, &m_Ext);

    f.Write("\t<Name>" + m_Name + "</Name>\n");
    f.Write("\t<Image>\n");
    f.Write("\t\t<Width>999</Width>\n");
    f.Write("\t\t<Height>200</Height>\n");
    f.Write("\t\t<BMPBytes>/9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAgGBgcGBQgHBwcJCQgKDBQNDAsLDBkSEw8UHRofHh0aHBwgJC4nICIsIxwcKDcpLDAxNDQ0Hyc5PTgyPC4zNDL/2wBDAQkJCQwLDBgNDRgyIRwhMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjL/wAARCAAUAFADASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwDvrm5nFxcgTyACQ4+c+tMN1cb3/fy9P75ptz/x83X/AF0P86Yfvv8A7te6kv6+RS2X9dyeK5n81f38n3f75q3HcTean76T7w/iNZ8X+tX/AHauRf61P94V4mZaVFbt/mephEvYS+f5GuZZMn943T1oEsmR+8bp60w9T9KB1H0rxbs+Uu+T5GfJcTea/wC+k+8f4jVSW5n81v38n3f75qeX/Wv/ALxqnL/rW/3a9rLdajv2/wAj6vFpewj8vyHi6uN6fv5en980+2uZzcWwM8hBkGfnPrVcffT/AHafbf8AHza/9dB/OvbaX9fM8t7P+uxsTaVA00zF5Pmck8j1+lM/smDcx3y8j1H+FFFeJ7Wp/M/vLS0/rzFGlQKciSXIXHUf4U9dOjBUiWXI+n+FFFZybnrLUuM5R0T0JPs7f8/Ev5L/AIUfZ2/5+JfyX/Ciio9nDsjD2cOXZEbadGSxMsuT9P8ACmHSoGOTJLkrjqP8KKKuDcNY6G8pylo3oJ/ZMG5Tvl4HqP8ACnw6VAs0LB5PlcEcj1+lFFae2qfzP7yGtP68j//Z</BMPBytes>\n");

    f.Write("\t</Image>\n");
    f.Write("\t<Tracks>\n");


    old_bst = 999;   // pick a value to gaurantee we will use a eff=3 line on the next pass
    for (ch = 0; ch + (cpn - 1) < numChans; ch += cpn)   // since we want to combine 3 channels into one 24 bit rgb value, we jump by 3
    {
        old_bst = 999;   // pick a value to gaurantee we will use a eff=3 line on the next pass

        if (ch % 9 == 0)
        {
            SetStatusText(wxString("Status: ") + string_format(" Channel %ld. ", ch));
        }
        f.Write("\t<Track>\n");
        f.Write("\t\t<TrackGuid>60cc0c76-f458-4e67-abb4-5d56a9c1d97c</TrackGuid>\n");
        f.Write("\t\t<IsHidden>false</IsHidden>\n");
        f.Write("\t\t<IsPrimaryTrack>false</IsPrimaryTrack>\n");
        f.Write("\t\t<TrackColorName>Gainsboro</TrackColorName>\n");
        f.Write("\t\t<TrackColorARGB>-2302756</TrackColorARGB>\n");
        f.Write("\t\t<TrackID>0</TrackID>\n");
        f.Write("\t\t<TrackType>0</TrackType>\n");
        //    f.Write("\t\t<WiiMapping inv=\"0\" ibn=\"\" inbn=\"\" ani=\"0\" ain=\"\" hty=\"-1\" fed=\"0"\ wind=\"-1\" wibt=\"0\" cint=\"False\" ceff=\"False\" hefsd=\"True\" lef=\"3\" lefl=\"1\" intb=\"0\" efd=\"0\" />\n");
        f.Write("\t\t<Name />\n");

        /*
        <Intervals>
        <TimeInterval eff="1" dat="" gui="{1B0F1B59-7161-4782-B068-98E021A6E048}" a="128" b="128" in="1" out="100" pos="88200" sin="-1" att="0"/>
        <TimeInterval eff="2" dat="" gui="{49E1F143-321A-4f5b-9F39-32984FF12410}" a="128" b="128" in="100" out="1" pos="176400" sin="-1" att="0"/>
        <TimeInterval eff="7" dat="" gui="{49E1F143-321A-4f5b-9F39-32984FF12410}" a="128" b="128" pos="264600" sin="-1" att="0"/>
        <TimeInterval eff="4" dat="" gui="" a="128" b="128" in="1" out="1" pos="352800" sin="-1" att="0"/>
        <TimeInterval eff="4" dat="" gui="" a="128" b="128" in="1" out="1" pos="441000" sin="-1" att="0"/>
        </Intervals>
        */
        xmlString = string_format("&lt;?xml version=&quot;1.0&quot; encoding=&quot;utf-16&quot;?&gt;&#xD;&#xA;&lt;ec&gt;&#xD;&#xA;  &lt;in&gt;100&lt;/in&gt;&#xD;&#xA;  &lt;out&gt;100&lt;/out&gt;&#xD;&#xA;&lt;/ec&gt;");
        xmlString = string_format("");

        if (ch == 156)
        {
            rgb = 0;
        }

        guiString = string_format("{DA98BD5D-9C00-40fe-A11C-AD3242573443}");
        f.Write("\t\t<Intervals>\n");
        //  for (p=0,csec=0; p < numPeriods; p++, csec+=interval, seqidx++)

        channels_exported += cpn;

        for (p = 0; p < numPeriods; p++)
        {
            seconds = (p*dataBuf->FrameTime()) / 1000.0;
            //  SetStatusText(wxString("Status: " )+string_format(" Channel %4d. %4d out of %4d ",ch,p,numPeriods));
            pos = seconds * 88200;
            //   SetStatusText(wxString("Status: " )+string_format(" Channel %ld. p=%ld (%ld). Sizeof %ld . seqid %ld",ch,p,numPeriods,sizeof(dataBuf),seqidx));


            /*
            byte = (*dataBuf)[seqidx];
            r_idx = g_idx= b_idx = (ch*numPeriods)+p;
            // if(ch < numChans-1)
            {
            g_idx=(ch+1)*numPeriods+p;
            }
            //  if(ch < numChans-2)
            {
            b_idx=(ch+2)*numPeriods+p;
            }
            rgb = ((*dataBuf)[r_idx]& 0xff) << 16 | ((*dataBuf)[g_idx]& 0xff) << 8 | ((*dataBuf)[b_idx]& 0xff); // we want a 24bit value for HLS
            */
            if (cpn == 1)  // cpn (Channels per Node. if non rgb, we only use one byte
                rgb = ((*dataBuf)[p][ch] & 0xff) << 16;
            else
                rgb = ((*dataBuf)[p][ch] & 0xff) << 16 |
                ((*dataBuf)[p][ch + 1] & 0xff) << 8 |
                ((*dataBuf)[p][ch + 2] & 0xff); // we want a 24bit value for HLS

                                                //  if(rgb>0 or rgb<0)
            {
                bst = rgb;
                ben = rgb;
                // 4410 = 1/20th of a second. 88200/20
                if (rgb == 0)
                {
                    if (cpn == 1)
                        f.Write(string_format("\t\t\t<TimeInterval eff=\"4\" dat=\"\" gui=\"\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\"/>\n", pos));
                    else
                        f.Write(string_format("\t\t\t<TimeInterval eff=\"4\" dat=\"\" gui=\"\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\"  bst=\"-1\" ben=\"-1\"/>\n", pos));

                }
                else if (bst == old_bst)
                {
                    f.Write(string_format("\t\t\t<TimeInterval eff=\"7\" dat=\"\" gui=\"\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"2\"  />\n", pos));
                }
                else
                {
                    if (cpn == 1)
                        f.Write(string_format("\t\t\t<TimeInterval eff=\"3\" dat=\"%s\" gui=\"%s\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\" />\n", xmlString, guiString, pos));
                    else
                        f.Write(string_format("\t\t\t<TimeInterval eff=\"3\" dat=\"%s\" gui=\"%s\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\" bst=\"%ld\" ben=\"%ld\" />\n", xmlString, guiString, pos, bst, ben));

                }
                old_bst = bst;
            }
            //  old_bst=999;   // pick a value to gaurantee we will use a eff=3 line on the next pass
        }
        //  f.Write(string_format("\t\t\t<TimeInterval eff=\"4\" dat=\"\" gui=\"\" a=\"128\" b=\"128\" in=\"1\" out=\"1\" pos=\"100000000\" sin=\"-1\" att=\"1\"/>\n"));
        f.Write("\t\t</Intervals>\n");
        f.Write("\t\t</Track>\n");
    }
    f.Write("\t\t</Tracks>\n");
    f.Write("\t</Pattern>\n");
    f.Write("</ArrayOfPattern>\n");
    f.Close();
    SetStatusText(wxString("Status: Export Complete. ") + string_format(" Channels exported=%4d ", channels_exported));

}

void FRAMECLASS WriteHLSFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf)
{
    wxString ChannelName, TestName, buff;
    int ch, p;
    unsigned long rgb;
    int seqidx = 0;

    wxFile f;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }

    for (ch = 0; ch + 2 < numChans; ch += 3)   // since we want to combine 3 channels into one 24 bit rgb value, we jump by 3
    {
        SetStatusText(wxString("Status: ") + string_format(" Channel %ld ", ch));

        buff = "";

        for (p = 0; p < numPeriods; p++, seqidx++)
        {
            rgb = ((*dataBuf)[p][ch] & 0xff) << 16 |
                ((*dataBuf)[p][ch + 1] & 0xff) << 8 |
                ((*dataBuf)[p][ch + 2] & 0xff); // we want a 24bit value for HLS
            if (p<numPeriods - 1)
            {
                buff += string_format("%d ", rgb);
            }
            else
            {
                buff += string_format("%d", rgb);
            }
        }
        buff += string_format("\n");
        f.Write(buff);
    }

    f.Close();
}

void FRAMECLASS WriteLcbFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf)
{
    wxString ChannelName, TestName;
    int ch, p, csec, StartCSec;
    int intensity, LastIntensity;
    wxFile f;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }


    wxString m_Path, m_Name, m_Ext;
    wxFileName::SplitPath(filename, &m_Path, &m_Name, &m_Ext);
    //  printf("'%s' is split as '%s', '%s', '%s'\n", m_FileName, m_Path,
    //  m_Name, m_Ext);

    int interval = SeqData.FrameTime() / 10;  // in centiseconds
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    f.Write("<channelsClipboard version=\"1\" name=\"" + m_Name + "\">\n");

    //  <channels>
    //  <channel>
    //  <effect type="intensity" startCentisecond="0" endCentisecond="10" intensity="83" />
    f.Write("<cellDemarcations>\n");
    for (p = 0, csec = 0; p < numPeriods; p++, csec += interval)
    {
        f.Write(string_format("\t<cellDemarcation centisecond=\"%d\"/>\n", csec));
    }
    f.Write("</cellDemarcations>\n");
    //
    // LOR is BGR with high bits=0
    // Vix is RGB with high bits=1
    f.Write("<channels>\n");
    for (ch = 0; ch < numChans; ch++)
    {
        SetStatusText(wxString("Status: ") + string_format(" Channel %ld ", ch));

        f.Write("\t<channel>\n");
        LastIntensity = 0;
        for (p = 0, csec = 0; p < numPeriods; p++, csec += interval)
        {
            intensity = (*dataBuf)[p][ch] * 100 / 255;
            if (intensity != LastIntensity)
            {
                if (LastIntensity != 0)
                {
                    f.Write(string_format("\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n",
                        StartCSec, csec, LastIntensity));
                }
                StartCSec = csec;
            }
            LastIntensity = intensity;
        }
        if (LastIntensity != 0)
        {
            f.Write(string_format("\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n",
                StartCSec, csec, LastIntensity));
        }
        f.Write("\t</channel>\n");
    }
    f.Write("</channels>\n");
    f.Write("</channelsClipboard>\n");
    f.Close();
}

/*
base64.cpp and base64.h

Copyright (C) 2004-2008 Rene Nyffenegger

This source code is provided 'as-is', without any express or implied
warranty. In no event will the author be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this source code must not be misrepresented; you must not
claim that you wrote the original source code. If you use this source code
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original source code.

3. This notice may not be removed or altered from any source distribution.

Rene Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

// encodes contents of SeqData in channel order
wxString FRAMECLASS base64_encode()
{
    wxString ret;
    int i = 0;
    int j = 0;

    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (int channel = 0; channel < SeqData.NumChannels(); channel++) {
        for (int frame = 0; frame < SeqData.NumFrames(); frame++) {
            char_array_3[i++] = SeqData[frame][channel];
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i <4); i++)
                {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
        {
            ret += base64_chars[char_array_4[j]];
        }

        while ((i++ < 3))
        {
            ret += '=';
        }

    }
    return ret;
}

//returns number of chars at the end that couldn't be decoded
int FRAMECLASS base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data)
{
    size_t in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i <4; i++)
            {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
            {
                data.resize(data.size() + 1);
                data[data.size() - 1] = char_array_3[i];
            }
            i = 0;
        }
    }

    if (i && encoded_string[in_] == '=')
    {
        for (j = i; j <4; j++)
        {
            char_array_4[j] = 0;
        }

        for (j = 0; j <4; j++)
        {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
        {
            data.resize(data.size() + 1);
            data[data.size() - 1] = char_array_3[j];
        }
    }
    return i;
}

int rountTo4(int i) {
    int remainder = i % 4;
    if (remainder == 0) {
        return i;
    }
    return i + 4 - remainder;
}

void FRAMECLASS WriteFalconPiModelFile(const wxString& filename, long numChans, long numPeriods,
    SeqDataType *dataBuf, int startAddr, int modelSize)
{
    wxUint16 fixedHeaderLength = 20;
    wxUint32 stepSize = rountTo4(numChans);
    wxFile f;

    size_t ch;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }

    wxUint8* buf;
    buf = (wxUint8 *)calloc(sizeof(wxUint8), stepSize);

    // Header Information
    // Format Identifier
    buf[0] = 'E';
    buf[1] = 'S';
    buf[2] = 'E';
    buf[3] = 'Q';
    // Data offset
    buf[4] = (wxUint8)1; //Hard coded to export a single model for now
    buf[5] = 0; //Pad byte
    buf[6] = 0; //Pad byte
    buf[7] = 0; //Pad byte
                // Step Size
    buf[8] = (wxUint8)(stepSize & 0xFF);
    buf[9] = (wxUint8)((stepSize >> 8) & 0xFF);
    buf[10] = (wxUint8)((stepSize >> 16) & 0xFF);
    buf[11] = (wxUint8)((stepSize >> 24) & 0xFF);
    //Model Start address
    buf[12] = (wxUint8)(startAddr & 0xFF);
    buf[13] = (wxUint8)((startAddr >> 8) & 0xFF);
    buf[14] = (wxUint8)((startAddr >> 16) & 0xFF);
    buf[15] = (wxUint8)((startAddr >> 24) & 0xFF);
    // Model Size
    buf[16] = (wxUint8)(modelSize & 0xFF);
    buf[17] = (wxUint8)((modelSize >> 8) & 0xFF);
    buf[18] = (wxUint8)((modelSize >> 16) & 0xFF);
    buf[19] = (wxUint8)((modelSize >> 24) & 0xFF);
    f.Write(buf, fixedHeaderLength);

    for (long period = 0; period < numPeriods; period++)
    {
        //if (period % 500 == 499) AppendConvertStatus (string_format("Writing time period %ld\n",period+1));
        wxYield();
        for (ch = 0; ch<stepSize; ch++)
        {
            buf[ch] = (*dataBuf)[period][ch];
        }
        f.Write(buf, stepSize);
    }
    f.Close();
}

void FRAMECLASS ReadFalconFile(const wxString& FileName, ConvertDialog* convertdlg)
{
    ConvertParameters read_params(FileName,                                     // input filename
        SeqData,                                      // sequence data object
        NetInfo, //GetNetInfo(),                                 // global network info
        ConvertParameters::READ_MODE_LOAD_MAIN,       // file read mode
        this,                                         // xLights main frame
        convertdlg,
        &mediaFilename);                             // media filename

    FileConverter::ReadFalconFile(read_params);
}

wxString FromAscii(const char *val) {
    return wxString::FromAscii(val);
}

void FRAMECLASS ReadXlightsFile(const wxString& FileName, wxString *mediaFilename)
{
    wxFile f;
    char hdr[512], filetype[10];
    int fileversion, numch, numper, scancnt;
    size_t readcnt;

    ConversionInit();
    if (!f.Open(FileName.c_str()))
    {
        PlayerError(wxString("Unable to load sequence:\n") + FileName);
        return;
    }
    f.Read(hdr, 512);
    scancnt = sscanf(hdr, "%8s %2d %8d %8d", filetype, &fileversion, &numch, &numper);
    if (scancnt != 4 || strncmp(filetype, "xLights", 7) != 0 || numch <= 0 || numper <= 0)
    {
        PlayerError(wxString("Invalid file header:\n") + FileName);
    }
    else
    {
        SeqData.init(numch, numper, 50);
        char * buf = new char[numper];
        wxString filename = FromAscii(hdr + 32);
        if (mediaFilename) {
            *mediaFilename = filename;
        }
        else {
            SetMediaFilename(filename);
        }
        for (int x = 0; x < numch; x++) {
            readcnt = f.Read(buf, numper);
            if (readcnt < numper)
            {
                PlayerError(wxString("Unable to read all event data from:\n") + FileName);
            }
            for (int p = 0; p < numper; p++) {
                SeqData[p][x] = buf[p];
            }
        }
        delete[] buf;
#ifndef NDEBUG
        log4cpp::Category& logger = log4cpp::Category::getRoot();
        logger.debug(string_format(wxString("ReadXlightsFile SeqData.NumFrames()=%ld SeqData.NumChannels()=%ld\n"), SeqData.NumFrames(), SeqData.NumChannels()));
#endif
    }
    f.Close();
}

void FRAMECLASS WriteFalconPiFile(const wxString& filename)
{
    wxUint8 vMinor = 0;
    wxUint8 vMajor = 1;
    wxUint16 fixedHeaderLength = 28;
    wxUint32 stepSize = rountTo4(SeqData.NumChannels());

    wxUint16 stepTime = SeqData.FrameTime();
    // Ignored by Pi Player
    wxUint16 numUniverses = 0;
    // Ignored by Pi Player
    wxUint16 universeSize = 0;
    // Gamma 0=encoded 1=linear
    wxUint8 gamma = 1;
    // Gamma 0=unknown 1=mono 2=RGB
    wxUint8 colorEncoding = 2;
    wxFile f;
    // Step Size must be multiple of 4
    //wxUint8 buf[stepSize];

    size_t ch;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }

    wxUint8* buf;
    buf = (wxUint8 *)calloc(sizeof(wxUint8), stepSize < 1024 ? 1024 : stepSize);

    // Header Information
    // Format Identifier
    buf[0] = 'P';
    buf[1] = 'S';
    buf[2] = 'E';
    buf[3] = 'Q';

    buf[6] = vMinor;
    buf[7] = vMajor;
    // Fixed header length
    buf[8] = (wxUint8)(fixedHeaderLength % 256);
    buf[9] = (wxUint8)(fixedHeaderLength / 256);
    // Step Size
    buf[10] = (wxUint8)(stepSize & 0xFF);
    buf[11] = (wxUint8)((stepSize >> 8) & 0xFF);
    buf[12] = (wxUint8)((stepSize >> 16) & 0xFF);
    buf[13] = (wxUint8)((stepSize >> 24) & 0xFF);
    // Number of Steps
    buf[14] = (wxUint8)(SeqData.NumFrames() & 0xFF);
    buf[15] = (wxUint8)((SeqData.NumFrames() >> 8) & 0xFF);
    buf[16] = (wxUint8)((SeqData.NumFrames() >> 16) & 0xFF);
    buf[17] = (wxUint8)((SeqData.NumFrames() >> 24) & 0xFF);
    // Step time in ms
    buf[18] = (wxUint8)(stepTime & 0xFF);
    buf[19] = (wxUint8)((stepTime >> 8) & 0xFF);
    // universe count
    buf[20] = (wxUint8)(numUniverses & 0xFF);
    buf[21] = (wxUint8)((numUniverses >> 8) & 0xFF);
    // universe Size
    buf[22] = (wxUint8)(universeSize & 0xFF);
    buf[23] = (wxUint8)((universeSize >> 8) & 0xFF);
    // universe Size
    buf[24] = gamma;
    // universe Size
    buf[25] = colorEncoding;
    buf[26] = 0;
    buf[27] = 0;

    if (mediaFilename.length() > 0) {
        int len = strlen(mediaFilename.c_str()) + 5;
        buf[28] = (wxUint8)(len & 0xFF);
        buf[29] = (wxUint8)((len >> 8) & 0xFF);
        buf[30] = 'm';
        buf[31] = 'f';
        strcpy((char *)&buf[32], mediaFilename.c_str());
        fixedHeaderLength += len;
        fixedHeaderLength = rountTo4(fixedHeaderLength);
    }
    // Data offset
    buf[4] = (wxUint8)(fixedHeaderLength % 256);
    buf[5] = (wxUint8)(fixedHeaderLength / 256);
    f.Write(buf, fixedHeaderLength);


    for (long period = 0; period < SeqData.NumFrames(); period++)
    {
        for (ch = 0; ch<stepSize; ch++)
        {
            buf[ch] = SeqData[period][ch];
        }
        f.Write(buf, stepSize);
    }
    f.Close();
    free(buf);
}

