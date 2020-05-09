/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <stdio.h>
#include <sstream>
#include <iomanip>

#include <wx/wx.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/imaggif.h>
#include <wx/anidecod.h>
#include <wx/quantize.h>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
}

#include "ConvertDialog.h"
#include "FileConverter.h"
#include "UtilFunctions.h"
#include "models/ModelGroup.h"
#include "outputs/OutputManager.h"
#include "sequencer/EffectLayer.h"
#include "xLightsMain.h"
#include "FSEQFile.h"
#include <log4cpp/Category.hh>

#ifndef CODEC_FLAG_GLOBAL_HEADER /* add compatibility for ffmpeg 3+ */
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER
#endif


void xLightsFrame::ConversionError(const wxString& msg)
{
    DisplayError(msg.ToStdString());
}

void xLightsFrame::SetStatusText(const wxString &msg, int filename) {
    if (_renderMode) {
        printf("%s\n", (const char *)msg.c_str());
    } else {
        if (filename) {
            FileNameText->SetLabel(msg);
            FileNameText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
        } else {
            StatusText->SetLabel(msg);
            StatusText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
        }
        StatusText->Refresh(); // Draw now so even when we are in CPU bound loops status updates
    }
}

void xLightsFrame::SetStatusTextColor(const wxString &msg, const wxColor& color) {
    if (_renderMode) {
        printf("%s\n", (const char *)msg.c_str());
    } else {
        StatusText->SetLabel(msg);
        StatusText->SetForegroundColour(color);
    }
}

void xLightsFrame::ConversionInit()
{
    long TotChannels=_outputManager.GetTotalChannels();
    mediaFilename.clear();
    ChannelColors.clear();
    ChannelNames.clear();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    SeqData.init(0, 0, 50);
}

void xLightsFrame:: SetMediaFilename(const wxString& filename)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Setting media file to: %s.", (const char *)filename.c_str());

    mediaFilename = filename;
    if (mediaFilename.size() == 0)
    {
        mMediaLengthMS = 0;
        return;
    }

    wxPathFormat PathFmt = mediaFilename.Contains(wxString("\\")) ? wxPATH_DOS : wxPATH_NATIVE;
    wxFileName fn1(mediaFilename, PathFmt);
    if (!fn1.FileExists())
    {
        wxFileName fn2(CurrentDir, fn1.GetFullName());
        mediaFilename = fn2.GetFullPath();
    }
}

void xLightsFrame:: ClearLastPeriod()
{
    int LastPer = SeqData.NumFrames()-1;
    for (size_t ch=0; ch < SeqData.NumChannels(); ch++)
    {
        SeqData[LastPer][ch] = 0;
    }
}

#define string_format wxString::Format

void xLightsFrame:: WriteVirFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame, SeqDataType *dataBuf)
{
    wxFile f;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }

    for (int ch = 0; ch < numChans; ch++)
    {
        SetStatusText(wxString("Status: ") + string_format(" Channel %ld ", ch));

        wxString buff = "";
        for (unsigned int p = startFrame; p < endFrame; p++)
        {
            buff += string_format("%d ", (*dataBuf)[p][ch]);
        }
        buff += string_format("\n");
        f.Write(buff);
    }
    f.Close();
}

void xLightsFrame:: WriteLSPFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame, SeqDataType *dataBuf, int cpn)
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

    wxString ChannelName, TestName;
    int channels_exported = 0;
    unsigned long rgb;
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


    int old_bst = 999;   // pick a value to gaurantee we will use a eff=3 line on the next pass
    for (int ch = 0; ch + (cpn - 1) < numChans; ch += cpn)   // since we want to combine 3 channels into one 24 bit rgb value, we jump by 3
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
        wxString xmlString = string_format("&lt;?xml version=&quot;1.0&quot; encoding=&quot;utf-16&quot;?&gt;&#xD;&#xA;&lt;ec&gt;&#xD;&#xA;  &lt;in&gt;100&lt;/in&gt;&#xD;&#xA;  &lt;out&gt;100&lt;/out&gt;&#xD;&#xA;&lt;/ec&gt;");
        xmlString = string_format("");

        wxString guiString = string_format("{DA98BD5D-9C00-40fe-A11C-AD3242573443}");
        f.Write("\t\t<Intervals>\n");
        //  for (p=0,csec=0; p < numPeriods; p++, csec+=interval, seqidx++)

        channels_exported += cpn;

        for (unsigned int p = startFrame; p < endFrame; p++)
        {
            float seconds = ((p-startFrame)*dataBuf->FrameTime()) / 1000.0;
            //  SetStatusText(wxString("Status: " )+string_format(" Channel %4d. %4d out of %4d ",ch,p,numPeriods));
            int pos = seconds * 88200;
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
                int bst = rgb;
                int ben = rgb;
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

void xLightsFrame::WriteHLSFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame, SeqDataType* dataBuf)
{
    wxString ChannelName, TestName;
    int seqidx = 0;

    wxFile f;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }

    for (int ch = 0; ch + 2 < numChans; ch += 3)   // since we want to combine 3 channels into one 24 bit rgb value, we jump by 3
    {
        SetStatusText(wxString("Status: ") + string_format(" Channel %ld ", ch));

        wxString buff = "";

        for (unsigned int p = startFrame; p < endFrame; p++, seqidx++)
        {
            unsigned long rgb = ((*dataBuf)[p][ch] & 0xff) << 16 |
                ((*dataBuf)[p][ch + 1] & 0xff) << 8 |
                ((*dataBuf)[p][ch + 2] & 0xff); // we want a 24bit value for HLS
            if (p < endFrame - 1)
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

void xLightsFrame:: WriteLcbFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame, SeqDataType *dataBuf, int ver, int cpn)
{
    int interval = SeqData.FrameTime() / 10;  // in centiseconds
    if( interval * 10 != SeqData.FrameTime() ) {
        DisplayError("Cannot export to LOR unless the sequence timing is evenly divisible by 10ms");
        return;
    }

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

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    f.Write(string_format("<channelsClipboard version=\"%d\" name=\"%s\">\n", ver, (const char *)m_Name.c_str()));

    if (ver == 1) {
        //old version only supports single channels
        cpn = 1;
    }

    //  <channels>
    //  <channel>
    //  <effect type="intensity" startCentisecond="0" endCentisecond="10" intensity="83" />
    int maxCell = 0;
    f.Write("  <cellDemarcations>\n");
    int csec = 0;
    if (ver == 1) {
        for (unsigned int p = startFrame, csec = 0; p < endFrame; p++, csec += interval)
        {
            f.Write(string_format("    <cellDemarcation centisecond=\"%d\" />\n", csec));
            maxCell = csec * 10;
        }
    } else {
        f.Write("    <cellDemarcation centisecond=\"0\"/>\n");
        csec = (endFrame - startFrame) * interval;
        maxCell = csec * 10;
        f.Write(string_format("    <cellDemarcation centisecond=\"%d\" />\n", csec));
    }
    f.Write("  </cellDemarcations>\n");
    //
    // LOR is BGR with high bits=0
    // Vix is RGB with high bits=1
    f.Write("  <channels>\n");
    for (int ch = 0; ch < numChans; ch += cpn)
    {
        SetStatusText(wxString("Status: ") + string_format(" Channel %d ", ch));

        f.Write("    <channel>\n");
        xlColorVector colors;
        colors.resize(endFrame - startFrame);
        for (unsigned int p = startFrame; p < endFrame; p++)
        {
            if (cpn == 1) {
                colors[p].Set((*dataBuf)[p][ch], (*dataBuf)[p][ch], (*dataBuf)[p][ch]);
            } else {
                colors[p].Set((*dataBuf)[p][ch], (*dataBuf)[p][ch + 1], (*dataBuf)[p][ch + 2]);
            }
        }
        EffectLayer layer(nullptr);
        DoConvertDataRowToEffects(&layer, colors, dataBuf->FrameTime());

        int lastEndTime = 0;
        for (int eidx = 0; eidx < layer.GetEffectCount(); eidx++) {
            Effect *eff = layer.GetEffect(eidx);
            if (eff->GetStartTimeMS() != lastEndTime) {
                //off from last effect to start of this effect
                if (ver == 1) {
                    f.Write(string_format("      <effect startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\" type=\"intensity\" />\n",
                                          lastEndTime / 10, eff->GetStartTimeMS() / 10, 0));
                } else {
                    f.Write(string_format("      <effect startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\" />\n",
                                          lastEndTime / 10, eff->GetStartTimeMS() / 10, 0));
                }
            }

            f.Write(string_format("      <effect startCentisecond=\"%d\" endCentisecond=\"%d\"",
                                  eff->GetStartTimeMS() / 10, eff->GetEndTimeMS() / 10));
            if (eff->GetEffectName() == "On") {
                int starti = eff->GetSettings().GetInt("E_TEXTCTRL_Eff_On_Start", 100);
                int endi = eff->GetSettings().GetInt("E_TEXTCTRL_Eff_On_End", 100);
                xlColor c = eff->GetPalette()[0];

                if (cpn == 3) {
                    std::stringstream stream;
                    stream << " startColor=\"FF"
                        << std::setfill ('0') << std::setw(6)
                        << std::hex << std::uppercase << c.GetRGB(false)
                        << "\" endColor=\"FF"
                        << std::setfill ('0') << std::setw(6)
                        << std::hex << std::uppercase << c.GetRGB(false)
                        << "\"";
                    f.Write(stream.str());
                } else {
                    starti *= c.red;
                    starti /= 255;
                    endi *= c.red;
                    endi /= 255;
                }
                if (starti == endi) {
                    f.Write(string_format(" intensity=\"%d\"", starti));
                } else {
                    f.Write(string_format(" startIntensity=\"%d\" endIntensity=\"%d\"", starti, endi));
                }
            } else if (eff->GetEffectName() == "Color Wash") {
                xlColor c1 = eff->GetPalette()[0];
                xlColor c2 = eff->GetPalette()[1];
                if (cpn == 1) {
                    int starti = c1.asHSV().value * 100.0;
                    int endi = c1.asHSV().value * 100.0;
                    f.Write(string_format(" startIntensity=\"%d\" endIntensity=\"%d\"", starti, endi));
                } else {
                    std::stringstream stream;
                    stream << " startColor=\"FF"
                            << std::setfill ('0') << std::setw(6)
                            << std::hex << std::uppercase << c1.GetRGB(false)
                            << "\" endColor=\"FF"
                            << std::setfill ('0') << std::setw(6)
                            << std::hex << std::uppercase << c2.GetRGB(false)
                            << "\" intensity=\"100\"";
                    f.Write(stream.str());
                }
            }
            if (ver == 1) {
                f.Write(" type=\"intensity\" />\n");
            } else {
                f.Write(" type=\"INTENSITY\" />\n");
            }
            lastEndTime = eff->GetEndTimeMS();
        }

        if (lastEndTime < maxCell) {
            f.Write(string_format("      <effect startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"0\"",
                                  lastEndTime / 10, maxCell / 10));
            if (ver == 1) {
                f.Write(" type=\"intensity\"");
            }
            if (ver == 2) {
                f.Write(" type=\"INTENSITY\"");
            }
            f.Write(" />\n");
        }
        f.Write("    </channel>\n");
    }
    f.Write("  </channels>\n");
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

#define ESEQ_HEADER_LENGTH 20

void xLightsFrame::WriteFalconPiModelFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame,
    SeqDataType* dataBuf, int startAddr, int modelSize,
    bool v2)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (v2) {
        V2FSEQFile* file = (V2FSEQFile*)FSEQFile::createFSEQFile(filename, 2);
        file->setNumFrames(endFrame - startFrame);
        file->setStepTime(dataBuf->FrameTime());
        file->setChannelCount(startAddr + modelSize);

        //add a sparse range so the header is correct,
        file->m_sparseRanges.push_back(std::pair<uint32_t, uint32_t>(startAddr - 1, modelSize));
        file->writeHeader();
        //now reset the sparse range to channel 0 since we don't have all the data in the dataBuf
        file->m_sparseRanges[0] = std::pair<uint32_t, uint32_t>(0, modelSize);
        for (unsigned int x = startFrame; x < endFrame; x++) {
            file->addFrame(x - startFrame, &(*dataBuf)[x][0]);
        }
        file->finalize();
        delete file;
    }
    else {
        wxUint32 stepSize = roundTo4(numChans);
        wxFile f;
        logger_base.debug("Creating file %s. Channels: %ld Frames %ld, Start Channel %d, Model Size %d.",
            (const char*)filename.c_str(),
            numChans, endFrame - startFrame, startAddr, modelSize);

        if (!f.Create(filename, true)) {
            ConversionError(wxString("Unable to create file: ") + filename);
            logger_base.error("Unable to create file %s.", (const char*)filename.c_str());
            return;
        }

        wxUint8 buf[ESEQ_HEADER_LENGTH];

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
        f.Write(buf, ESEQ_HEADER_LENGTH);

        size_t size = dataBuf->NumFrames();
        size *= stepSize;

        f.Write(&(*dataBuf)[0][0], size);

        f.Close();
    }
}

// Log messages from libav*
void my_av_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
    static char message[8192];

    // Create the actual message
    vsnprintf(message, sizeof(message), fmt, vargs);

    // strip off carriage return
    if (strlen(message) > 0)
    {
        if (message[strlen(message) - 1] == '\n')
        {
            message[strlen(message) - 1] = 0x00;
        }
    }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("WriteVideoModelFile: lvl: %d msg: %s.", level, static_cast<const char *>(message));
}

// Render a model into our image
void RenderModelOnImage(wxImage& image, Model* model, uint8_t* framedata, int startAddr, int x = 0, int y = 0, bool invert = false)
{
    int outheight = image.GetHeight();
    int outwidth = image.GetWidth();

    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("Writing model frame. Model=%s, startAddr=%d, x=%d, y=%d, w=%d, h=%d, ow=%d, oh=%d", (const char *)model->name.c_str(), startAddr, x, y, width, height, outwidth, outheight);

    uint8_t* imagedata = image.GetData();

    // model should be simpler as all channels should be together ... but are they
    int chs = model->GetChanCountPerNode();
    uint8_t* ps = framedata + startAddr; // pointer to the channel data in the frame

    // Work out the colour order
    wxByte r = model->GetChannelColorLetter(0);
    int rr = 0;
    int gg = 1;
    int bb = 2;
    if (r == 'G')
    {
        gg = 0;
    }
    else if (r == 'B')
    {
        bb = 0;
    }
    wxByte g = model->GetChannelColorLetter(1);
    if (g == 'R')
    {
        rr = 1;
    }
    else if (g == 'B')
    {
        bb = 1;
    }
    wxByte b = model->GetChannelColorLetter(2);
    if (b == 'R')
    {
        rr = 2;
    }
    else if (b == 'G')
    {
        gg = 2;
    }

    // Now process each node
    for (size_t i = 0; i < model->GetNodeCount(); i++)
    {
        xlColor c = model->GetNodeColor(i);

        // Get all the bulbs attached to these nodes
        std::vector<wxPoint> pts;
        model->GetNodeCoords(i, pts);

        // for each bulb
        for (const auto& it : pts) {
            // work out where we should display it in the output image
            int xx = x + it.x;
            int yy = y + it.y;

            if (invert) {
                yy = outheight - yy - 1;
            }

            // make sure it is within the image bounds
            if (xx >= 0 && xx < outwidth && yy >= 0 && yy < outheight)
            {
                // calculate a pointer to the pixal in the image
                uint8_t* p = imagedata + (yy * outwidth + xx) * 3;

                if (chs == 1) {
                    // for single channels we use the node colour
                    *p = c.Red();
                    *(p + 1) = c.Green();
                    *(p + 2) = c.Blue();
                }
                else {
                    // set the bulb colour to the pixel ... taking into account colour layout
                    *(p) = *(ps + rr);
                    *(p + 1) = *(ps + gg);
                    *(p + 2) = *(ps + bb);
                }
            }
            else
            {
                // this shouldnt happen
                wxASSERT(false);
            }
        }

        ps += chs;
    }
}

void FillImage(wxImage& image, Model* model, uint8_t* framedata, int startAddr, bool invert)
{
    if (model->GetDisplayAs() == "ModelGroup") {
        ModelGroup* mg = static_cast<ModelGroup*>(model);

        // Render each model
        for (auto m = mg->Models().begin(); m != mg->Models().end(); ++m) {
            // work out this models start channel relative to the buffer
            int start = (*m)->GetFirstChannel() - startAddr;

            // Work out where the zero point is for this model
            // FIXME:  Models are no longer fixed percentages on the screen
            //         We can use msl.GetScreenOffset(preview) but it needs a pointer to the ModelPreview that contains the model to be rendered.
            int x = 0, y = 0;
            //ModelScreenLocation& msl = (*m)->GetModelScreenLocation();
            //int width = image.GetWidth();
            //int height = image.GetHeight();
            //int x = ((float)width * msl.GetHcenterOffset());
            //int y = ((float)height * (1.0 - msl.GetVcenterOffset()));

            RenderModelOnImage(image, *m, framedata, start, x, y, invert);
        }
    } else {
        // Render the model
        RenderModelOnImage(image, model, framedata, model->GetFirstChannel() - startAddr + 1, 0, 0, invert);
    }
}

void xLightsFrame:: WriteVideoModelFile(const wxString& filenames, long numChans, unsigned int startFrame, unsigned int endFrame,
    SeqDataType *dataBuf, int startAddr, int modelSize, Model* model, bool compressed)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Writing model video.");

    int origwidth;
    int origheight;
    model->GetBufferSize("Default", "2D", "None", origwidth, origheight);

    int width = origwidth;
    int height = origheight;

    // must be a multiple of 2
    if (width % 2 > 0) width++;
    if (height % 2 > 0) height++;
    logger_base.debug("   Video dimensions %dx%d => %dx%d.", origwidth, origheight, width, height);
    logger_base.debug("   Video frames %ld.", endFrame - startFrame);
    unsigned framesPerSec = 1000u / dataBuf->FrameTime();

    av_log_set_callback(my_av_log_callback);

    #if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
    #endif

    const char *filename = filenames.c_str();
    AVOutputFormat* fmt = av_guess_format(nullptr, filename, nullptr);
    if (!fmt)
    {
        logger_base.warn("   Could not deduce output format from file extension : using MPEG.");
        fmt = av_guess_format("mpeg", filename, nullptr);
    }
    if (!fmt)
    {
        logger_base.error("   Could not find suitable output format.");
        return;
    }

    // Choose the output video formate
    if (compressed)
    {
        fmt->video_codec = AV_CODEC_ID_H264;
    }
    else
    {
        fmt->video_codec = AV_CODEC_ID_RAWVIDEO;
    }
    //fmt->video_codec = AV_CODEC_ID_MPEG4; // this is the default for AVI

    // Create the codec context that will configure the codec
    AVFormatContext* oc = nullptr;
    avformat_alloc_output_context2(&oc, fmt, nullptr, filename);
    if (oc == nullptr)
    {
        logger_base.warn("   Could not create output context.");
        return;
    }

    // Find the output codec
    AVCodec* codec = avcodec_find_encoder(fmt->video_codec);
    if (codec == nullptr)
    {
        logger_base.error("   Cannot find codec %d.", fmt->video_codec);
        return;
    }

    // Create a video stream
    AVStream* video_st = avformat_new_stream(oc, nullptr);
    if (video_st == nullptr)
    {
        logger_base.error("   Cannot allocate stream.");
        return;
    }
    video_st->time_base.num = 1;
    video_st->time_base.den = framesPerSec;
    video_st->id = oc->nb_streams - 1;

    // Configure the codec
    AVCodecContext* codecContext = avcodec_alloc_context3( codec );
    codecContext->bit_rate = 400000;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->time_base.num = 1;
    codecContext->time_base.den = framesPerSec;
    codecContext->gop_size = 12; // key frame gap ... 1 is all key frames
    codecContext->max_b_frames = 1;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    if (fmt->video_codec == AV_CODEC_ID_H264)
    {
        codecContext->pix_fmt = AV_PIX_FMT_YUV444P;
        av_opt_set(codecContext->priv_data, "preset", "ultrafast", 0);
        av_opt_set(codecContext->priv_data, "qp", "0", 0);
        av_opt_set(codecContext->priv_data, "crf", "0", AV_OPT_SEARCH_CHILDREN);
    }
    else if (codecContext->codec_id == AV_CODEC_ID_MPEG4) {
        av_opt_set(codecContext->priv_data, "qp", "0", 0);
        av_opt_set(codecContext->priv_data, "crf", "0", AV_OPT_SEARCH_CHILDREN);
    }
    else if (codecContext->codec_id == AV_CODEC_ID_RAWVIDEO)
    {
        codecContext->pix_fmt = AV_PIX_FMT_BGR24;
        av_opt_set(codecContext->priv_data, "qp", "0", 0);
        av_opt_set(codecContext->priv_data, "crf", "0", AV_OPT_SEARCH_CHILDREN);
    }
    else if (codecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /* Needed to avoid using macroblocks in which some coeffs overflow.
         * This does not happen with normal video, it just happens here as
         * the motion of the chroma plane does not match the luma plane. */
        codecContext->mb_decision = 2;
    }

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;

    int ret = avcodec_open2(codecContext, nullptr, nullptr);
    if (ret < 0)
    {
        logger_base.error("   Cannot open codec context %d.", ret);
        return;
    }

    ret = avcodec_parameters_from_context( video_st->codecpar, codecContext );
    if ( ret != 0 )
    {
        logger_base.error( "   Cannot init video stream from codec context" );
        return;
    }

    ret = avformat_init_output(oc, nullptr);
    if (ret < 0)
    {
        logger_base.error("   Cannot init output %d.", ret);
        return;
    }

    // Create the frame object which will be placed in the packet
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        logger_base.error("   Cannot not allocate frame.");
        return;
    }
    frame->format = codecContext->pix_fmt;
    frame->width = codecContext->width;
    frame->height = codecContext->height;

    // Initialise image converter
    int sws_flags = SWS_BICUBIC;
    AVPixelFormat informat = AV_PIX_FMT_RGB24;
    struct SwsContext *sws_ctx = sws_getContext(origwidth, origheight, informat,
        codecContext->width, codecContext->height, codecContext->pix_fmt,
        sws_flags, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        logger_base.error("   Could not create image conversion context.");
        return;
    }

    // Create source and final image frames
    AVFrame src_picture;
    ret = av_image_alloc(src_picture.data, src_picture.linesize, origwidth, origheight, informat, 1);
    if (ret < 0) {
        logger_base.error("   Could not allocate picture buffer.");
        return;
    }

    ret = av_image_alloc(frame->data, frame->linesize, codecContext->width, codecContext->height, codecContext->pix_fmt, 1);
    if (ret < 0) {
        logger_base.error("   Could not allocate picture buffer.");
        return;
    }

    // Dump to the log the video format
    av_dump_format(oc, 0, filename, 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0) {
            logger_base.error("   Could open file %s.", static_cast<const char *>(filename));
            return;
        }
    }

    /* Write the stream header, if any. */
    if (avformat_write_header(oc, nullptr) < 0) {
        logger_base.error("   Could not write video file header.");
        return;
    }

    frame->pts = 0;

    for (size_t i = startFrame; i < endFrame; i++)
    {
        // Create a bitmap with the current frame
        wxImage image(origwidth, origheight, true);
        FillImage(image, model, (uint8_t*)&(*dataBuf)[i][0], startAddr, false);

        // place it in a frame
        ret = av_image_fill_arrays(src_picture.data, src_picture.linesize, image.GetData(), informat, origwidth, origheight, 1);
        if (ret < 0) {
            logger_base.error("   Error filling source image data %d.", ret);
            return;
        }

        // invert the image, scale image and convert colour space
        uint8_t * data = src_picture.data[0] + (origwidth * 3 * (origheight - 1));
        uint8_t* tmp[4] = { data, nullptr, nullptr, nullptr };
        int stride[4] = { - 1 * src_picture.linesize[0], 0, 0, 0 };
        ret = sws_scale(sws_ctx, tmp, stride,
            0, origheight, frame->data, frame->linesize);
        if (ret != codecContext->height) {
            logger_base.error("   Error resizing frame %d.", ret);
            return;
        }

        AVPacket* pkt = av_packet_alloc();
        av_init_packet(pkt);

        ret = avcodec_send_frame(codecContext, frame);
        if ( ret < 0 )
        {
            logger_base.error("   Error encoding frame %d.", ret);
            return;
        }

        while (1)
        {
            ret = avcodec_receive_packet(codecContext, pkt);
            if (ret == 0)
            {
                pkt->duration = 1LL;
                ret = av_interleaved_write_frame(oc, pkt);
            }
            else if (ret == AVERROR(EAGAIN))
                break;
        }

        frame->pts += 1LL;

        av_packet_free(&pkt);
    }

    // render out any buffered data
    {
        AVPacket* pkt = av_packet_alloc();
        av_init_packet(pkt);
        ret = avcodec_send_frame(codecContext, nullptr);
        while (1)
        {
            ret = avcodec_receive_packet(codecContext, pkt);
            if (ret == 0)
            {
                ret = av_interleaved_write_frame(oc, pkt);
            }
            else if (ret == AVERROR_EOF)
                break;
        }
        av_packet_free(&pkt);
    }

    // Write the video trailer
    av_write_trailer(oc);

    // Close the output file
    if (!(fmt->flags & AVFMT_NOFILE))
        avio_closep( &oc->pb );

    // Free and close everything
    sws_freeContext(sws_ctx);

    avcodec_free_context(&codecContext);
    avformat_free_context(oc);

    // Remove the log function
    av_log_set_callback(nullptr);

    logger_base.debug("Model video written successfully.");
}

void xLightsFrame::WriteGIFModelFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame,
    SeqDataType* dataBuf, int startAddr, int modelSize, Model* model, unsigned int frameTime)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Writing model GIF.");

    int width;
    int height;
    model->GetBufferSize("Default", "2D", "None", width, height);

    // must be a multiple of 2
    logger_base.debug("   GIF dimensions %dx%d.",  width, height);
    logger_base.debug("   GIF frames %ld.", endFrame - startFrame);
           
    wxImageArray imgArray;

    for (size_t i = startFrame; i < endFrame; i++)
    {
        // Create a bitmap with the current frame
        wxImage image(width, height, true);
        FillImage(image, model, (uint8_t*)&(*dataBuf)[i][0], startAddr, true);

        wxImage image2;
        wxQuantize::Quantize(image, image2);

        // Black out the almost blacks
        const unsigned char BLACK_THRESHOLD = 5;
        auto pal = image2.GetPalette();

        unsigned char* red = (unsigned char*)malloc(pal.GetColoursCount());
        unsigned char* green = (unsigned char*)malloc(pal.GetColoursCount());
        unsigned char* blue = (unsigned char*)malloc(pal.GetColoursCount());
        for (int i = 0; i < pal.GetColoursCount(); i++)
        {
            pal.GetRGB(i, red + i, green + i, blue + i);
            if (*(red + i) != 0 && (*(green + i) != 0) && *(blue + i) != 0)
            {
                if (*(red + i) < BLACK_THRESHOLD && *(green + i) < BLACK_THRESHOLD && *(blue + i) < BLACK_THRESHOLD)
                {
                    image2.Replace(*(red + i), *(green + i), *(blue + i), 0, 0, 0);
                    *(red + i) = 0;
                    *(green + i) = 0;
                    *(blue + i) = 0;
                }
            }
        }
        wxPalette newpal = wxPalette(pal.GetColoursCount(), red, green, blue);
        image2.SetPalette(newpal);

        free(red);
        free(green);
        free(blue);

        imgArray.Add(image2);
    }

    wxGIFHandler gif = wxGIFHandler();

    wxFileOutputStream outStream(filename);

    bool ret = gif.SaveAnimation(imgArray, &outStream, true, frameTime);
    if (ret)
    {
        logger_base.debug("Model GIF written successfully.");
    }
    else
    {
        logger_base.debug("Model GIF written successfully.");
    }   
}

void xLightsFrame:: WriteMinleonNECModelFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame,
    SeqDataType *dataBuf, int startAddr, int modelSize, Model* model)
{
    // this writes out at the sequence frame rate ... this may be a problem as samples I have seen seem to use 30fps

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Writing model Minleon Network Effects Controller File.");

    wxLogNull logNo;  // suppress popups from png images. See http://trac.wxwidgets.org/ticket/15331

    int width;
    int height;
    model->GetBufferSize("Default", "2D", "None", width, height);

    wxFile f;
    if (!f.Create(filename, true))
    {
        ConversionError(wxString("Unable to create file: ") + filename);
        return;
    }

    logger_base.debug("   Model dimensions %dx%d => %ld channels %ld frames %ld.", width, height, (long)width * (long)height * 3, numChans, endFrame - startFrame);

    unsigned char header[512];
    memset(header, 0x00, sizeof(header));

    header[0] = 0xCC;
    header[1] = 0x33;
    header[5] = 0x01;
    header[8] = 0x9C;
    header[9] = 0x40;
    header[11] = 0x03;
    auto framems = SeqData.FrameTime();
    if (framems == 0x21) // 30 fps
    {
        header[16] = 0x55; // I dont know what this value represents but it seems to be present in the samples I have seen
    }
    else if (framems == 0x19) // 40 fps
    {
        header[16] = 0xA4; // I dont know what this value represents but it seems to be present in the samples I have seen
    }
    else
    {
        // leave it as zero ... particularly for 20fps
    }
    header[17] = framems & 0xFF;
    header[19] = framems & 0xFF; // some times this value is one less than the one above but i dont know why
    header[20] = (numChans & 0xFF00) >> 8;
    header[21] = (numChans & 0x00FF);
    header[22] = 0x0A;
    header[25] = 0x64;
    header[26] = 0xFF;

    f.Write(header, sizeof(header));

    // every sample i have seen has one extra zero frame at the front ... so adding it
    wxByte zero = 0x00;
    for(int i = 0; i < numChans; i++)
    {
        f.Write(&zero, sizeof(zero));
    }

    for (unsigned int i = startFrame; i < endFrame; ++i)
    {
        f.Write(&(*dataBuf)[i][0], numChans);
    }
    f.Close();

    logger_base.debug("Model Minleon Network Effects Controller file written successfully.");
}

void xLightsFrame:: ReadFalconFile(const wxString& FileName, ConvertDialog* convertdlg)
{
    ConvertParameters read_params(FileName,                                     // input filename
        SeqData,                                      // sequence data object
        &_outputManager,                               // global network info
        ConvertParameters::READ_MODE_LOAD_MAIN,       // file read mode
        this,                                         // xLights main frame
        convertdlg,
        nullptr,
        &mediaFilename);                             // media filename

    FileConverter::ReadFalconFile(read_params);
}

wxString FromAscii(const char *val) {
    return wxString::FromAscii(val);
}

void xLightsFrame:: ReadXlightsFile(const wxString& FileName, wxString *mediaFilename)
{
    wxFile f;
    char hdr[512], filetype[10];
    int fileversion, numch, numper;

    ConversionInit();
    if (!f.Open(FileName.c_str()))
    {
        PlayerError(wxString("Unable to load sequence:\n") + FileName);
        return;
    }
    f.Read(hdr, 512);
    int scancnt = sscanf(hdr, "%8s %2d %8d %8d", filetype, &fileversion, &numch, &numper);
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
        for (size_t x = 0; x < numch; x++) {
            size_t readcnt = f.Read(buf, numper);
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
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug(string_format(wxString("ReadXlightsFile SeqData.NumFrames()=%ld SeqData.NumChannels()=%ld\n"), SeqData.NumFrames(), SeqData.NumChannels()));
#endif
    }
    f.Close();
}

void xLightsFrame:: WriteFalconPiFile(const wxString& filename)
{
    ConvertParameters write_params(filename,                                     // filename
                                   SeqData,                                      // sequence data object
                                   &_outputManager,                               // global network info
                                   ConvertParameters::READ_MODE_LOAD_MAIN,       // file read mode
                                   this,                                         // xLights main frame
                                   nullptr,
                                   nullptr,
                                   &mediaFilename, // media filename
                                   nullptr,
                                   filename);
    
    FileConverter::WriteFalconPiFile(write_params);
}
