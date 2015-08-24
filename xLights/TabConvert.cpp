/***************************************************************
 * Name:      xLightsMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/


// xml
#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"
#include "../include/spxml-0.5/spxmlparser.cpp"
#include "../include/spxml-0.5/spxmlevent.cpp"
#include "../include/spxml-0.5/spxmlcodec.cpp"
#include "../include/spxml-0.5/spxmlreader.cpp"
#include "../include/spxml-0.5/spxmlutils.cpp"
#include "../include/spxml-0.5/spxmlstag.cpp"

/*
 * ********************************************
 * ********************************************
 * Process Convert Panel Events
 * ********************************************
 * ********************************************
 */

#ifndef FPP

#include "xLightsMain.h"
#include "SeqSettingsDialog.h"
#include "FileConverter.h"

#define FRAMECLASS xLightsFrame::

void FRAMECLASS AppendConvertLog(const wxString& msg) {
    TextCtrlLog->AppendText(msg);
}
static wxString msgBuffer;
void FRAMECLASS AppendConvertStatus(const wxString &msg, bool flushBuffer) {
    if (flushBuffer && !msgBuffer.IsEmpty()) {
        msgBuffer.append(msg);
        TextCtrlConversionStatus->AppendText(msgBuffer);
        msgBuffer.Clear();
    } else if (flushBuffer) {
        TextCtrlConversionStatus->AppendText(msg);
    } else {
        msgBuffer.append(msg);
        if (msgBuffer.size() > 10000) {
            TextCtrlConversionStatus->AppendText(msgBuffer);
            msgBuffer.Clear();
        }
    }
}
void FRAMECLASS ConversionError(const wxString& msg)
{
    wxMessageBox(msg, wxString("Error"), wxOK | wxICON_EXCLAMATION);
}
bool FRAMECLASS mapEmptyChannels() {
    return CheckBoxMapEmptyChannels->IsChecked();
}
bool FRAMECLASS isSetOffAtEnd() {
    return CheckBoxOffAtEnd->IsChecked();
}
void FRAMECLASS SetStatusText(const wxString &msg) {
    StatusBar1->SetStatusText(msg);
}
#define string_format wxString::Format


wxString FromAscii(const char *val) {
    return wxString::FromAscii(val);
}
void RemoveAt(wxArrayString &v, int i) {
    v.RemoveAt(i);
}

#endif

#ifndef MAX_READ_BLOCK_SIZE
#define MAX_READ_BLOCK_SIZE 4096 * 1024
#endif

wxString Left(const wxString &in, int len) {
    return in.substr(0, len);
}
wxString Right(const wxString &in, int len) {
    return in.substr(in.size() - len, len);
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

                for(i = 0; (i <4) ; i++)
                {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
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

        while((i++ < 3))
        {
            ret += '=';
        }

    }
    return ret;
}

//returns number of chars at the end that couldn't be decoded
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data)
{
    size_t in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i ==4)
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
    AppendConvertLog ("Setting media file to: "+filename+"\n");
#endif
    mediaFilename=filename;
    if (mediaFilename.size() == 0)
    {
        mMediaLengthMS = 0;
        return;
    }
#ifndef FPP
    wxPathFormat PathFmt = mediaFilename.Contains(wxString("\\")) ? wxPATH_DOS : wxPATH_NATIVE;
    wxFileName fn1(mediaFilename, PathFmt);
    if (!fn1.FileExists())
    {
        wxFileName fn2(CurrentDir,fn1.GetFullName());
        mediaFilename=fn2.GetFullPath();
    }
#endif
}

#ifndef FPP
bool FRAMECLASS WriteVixenFile(const wxString& filename)
{
    wxString ChannelName,TestName;
    int32_t ChannelColor;
    long TotalTime=SeqData.TotalTime();
    wxXmlNode *node,*chparent,*textnode;
    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "Program" );
    doc.SetRoot( root );

    // add nodes to root in reverse order



    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, "EventValues" );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, base64_encode() );


    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, "Audio" );
    node->AddAttribute( "filename", mediaFilename);
    node->AddAttribute( "duration", string_format("%ld",TotalTime));
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, "Music" );

    chparent = new wxXmlNode( root, wxXML_ELEMENT_NODE, "Channels" );


    for (int ch=0; ch < SeqData.NumChannels(); ch++ )
    {
        SetStatusText(string_format("Status: Channel %ld ",ch));

        node = new wxXmlNode( wxXML_ELEMENT_NODE, "Channel" );
        node->AddAttribute( "output", string_format("%d",ch));
        node->AddAttribute( "id", "0");
        node->AddAttribute( "enabled", "True");
        chparent->AddChild( node );
        if (ch < CheckListBoxTestChannels->GetCount())
        {
            TestName=CheckListBoxTestChannels->GetString(ch);
        }
        else
        {
            TestName=string_format("Ch: %d",ch);
        }
        if (ch < ChannelNames.size() && ChannelNames[ch].size() != 0)
        {
            ChannelName = ChannelNames[ch];
        }
        else
        {
            ChannelName = TestName;
        }
        // LOR is BGR with high bits=0
        // Vix is RGB with high bits=1
        if (ch < ChannelColors.size() && ChannelColors[ch] > 0)
        {
            ChannelColor = 0xff000000 | (ChannelColors[ch] >> 16 & 0x0000ff) | (ChannelColors[ch] & 0x00ff00) | (ChannelColors[ch] << 16 & 0xff0000);
        }
        else if (TestName.Last() == 'R')
        {
            ChannelColor = 0xffff0000;
        }
        else if (TestName.Last() == 'G')
        {
            ChannelColor = 0xff00ff00;
        }
        else if (TestName.Last() == 'B')
        {
            ChannelColor = 0xff0000ff;
        }
        else
        {
            // default to white
            ChannelColor = 0xffffffff;
        }
        node->AddAttribute( "color", string_format("%d",ChannelColor));
        textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, ChannelName );
    }

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, "AudioDevice" );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, "-1" );

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, "MaximumLevel" );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, "255" );

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, "MinimumLevel" );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, "0" );

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, "EventPeriodInMilliseconds" );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, string_format("%ld", SeqData.FrameTime()) );

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, "Time" );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, string_format("%ld",TotalTime) );

    return doc.Save( filename );
}
#endif

void FRAMECLASS WriteVirFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf)
{
    wxString buff;

    int ch,p;
    wxFile f;
    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
        return;
    }

    for (ch=0; ch < numChans; ch++ )
    {
        SetStatusText(wxString("Status: " )+string_format(" Channel %ld ",ch));

        buff="";
        for (p=0; p < numPeriods; p++)
        {
            buff += string_format("%d ",(*dataBuf)[p][ch]);
        }
        buff += string_format("\n");
        f.Write(buff);
    }
    f.Close();
}

void FRAMECLASS WriteVirFile(const wxString& filename)
{
    WriteVirFile(filename, SeqData.NumChannels(), SeqData.NumFrames(), &SeqData);
}

void FRAMECLASS WriteHLSFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf)
{
    wxString ChannelName,TestName,buff;
    int ch,p;
    unsigned long rgb;
    int seqidx=0;

    wxFile f;
    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
        return;
    }

    for (ch=0; ch+2 < numChans; ch+=3 )   // since we want to combine 3 channels into one 24 bit rgb value, we jump by 3
    {
        SetStatusText(wxString("Status: " )+string_format(" Channel %ld ",ch));

        buff="";

        for (p=0; p < numPeriods; p++, seqidx++)
        {
            rgb = ((*dataBuf)[p][ch]& 0xff) << 16 |
                  ((*dataBuf)[p][ch+1]& 0xff) << 8 |
                  ((*dataBuf)[p][ch+2]& 0xff); // we want a 24bit value for HLS
            if(p<numPeriods-1)
            {
                buff += string_format("%d ",rgb);
            }
            else
            {
                buff += string_format("%d",rgb);
            }
        }
        buff += string_format("\n");
        f.Write(buff);
    }
    f.Close();

}

void FRAMECLASS WriteHLSFile(const wxString& filename)
{
    WriteHLSFile(filename, SeqData.NumChannels(), SeqData.NumFrames(), &SeqData);
}


void FRAMECLASS ReadFalconFile(const wxString& FileName)
{
    ConvertParameters read_params(FileName,                                     // input filename
                                  SeqData,                                      // sequence data object
                                  GetNetInfo(),                                 // global network info
                                  ConvertParameters::READ_MODE_LOAD_MAIN,       // file read mode
                                  this,                                         // xLights main frame
                                  &mediaFilename );                             // media filename

    FileConverter::ReadFalconFile(read_params);
}

int rountTo4(int i)  {
    int remainder = i % 4;
    if (remainder == 0) {
        return i;
    }
    return i + 4 - remainder;
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
    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
        return;
    }

    wxUint8* buf;
    buf = (wxUint8 *)calloc(sizeof(wxUint8),stepSize < 1024 ? 1024 : stepSize);

    // Header Information
    // Format Identifier
    buf[0] = 'P';
    buf[1] = 'S';
    buf[2] = 'E';
    buf[3] = 'Q';

    buf[6] = vMinor;
    buf[7] = vMajor;
    // Fixed header length
    buf[8] = (wxUint8)(fixedHeaderLength%256);
    buf[9] = (wxUint8)(fixedHeaderLength/256);
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
        strcpy((char *)&buf[32],mediaFilename.c_str());
        fixedHeaderLength += len;
        fixedHeaderLength = rountTo4(fixedHeaderLength);
    }
    // Data offset
    buf[4] = (wxUint8)(fixedHeaderLength%256);
    buf[5] = (wxUint8)(fixedHeaderLength/256);
    f.Write(buf,fixedHeaderLength);


    for (long period=0; period < SeqData.NumFrames(); period++)
    {
        for(ch=0; ch<stepSize; ch++)
        {
            buf[ch] = SeqData[period][ch];
        }
        f.Write(buf,stepSize);
    }
    f.Close();
    free(buf);
}

void FRAMECLASS WriteFalconPiModelFile(const wxString& filename, long numChans, long numPeriods,
        SeqDataType *dataBuf, int startAddr, int modelSize)
{
    wxUint16 fixedHeaderLength = 20;
    wxUint32 stepSize = rountTo4(numChans);

    modelSize = stepSize; // also rounded to 4-byte multiple

    wxFile f;

    size_t ch;
    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
        return;
    }

    wxUint8* buf;
    buf = (wxUint8 *)calloc(sizeof(wxUint8),stepSize);

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
    f.Write(buf,fixedHeaderLength);

    for (long period=0; period < numPeriods; period++)
    {
        //if (period % 500 == 499) AppendConvertStatus (string_format("Writing time period %ld\n",period+1));
        wxYield();
        for(ch=0; ch<stepSize; ch++)
        {
            buf[ch] = (*dataBuf)[period][ch];
        }
        f.Write(buf,stepSize);
    }
    f.Close();
}
void FRAMECLASS WriteXLightsFile(const wxString& filename)
{
    wxFile f;
    char hdr[512];
    memset(hdr,0,512);
    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
        return;
    }
    if (mediaFilename.size() > 470)
    {
        ConversionError(wxString("Media file name is too long"));
        return;
    }
    if (SeqData.FrameTime() != 50) {
        ConversionError(wxString("xseq file must be 50ms timing"));
        return;
    }
    int xseq_format_version = 1;

    sprintf(hdr,"xLights %2d %8d %8d",xseq_format_version,SeqData.NumChannels(),SeqData.NumFrames());
    strncpy(&hdr[32],mediaFilename.c_str(),470);
    f.Write(hdr,512);
    for (int x = 0; x < SeqData.NumChannels(); x++) {
        for (int p = 0; p < SeqData.NumFrames(); p++) {
            f.Write(&SeqData[p][x], 1);
        }
    }
    f.Close();

}

#ifndef FPP
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

    wxString ChannelName,TestName,xmlString,guiString;
    int ch,p,channels_exported=0;
    int pos,bst,old_bst,ben;
    unsigned long rgb;
    float seconds;
    wxFile f;
    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
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


    old_bst=999;   // pick a value to gaurantee we will use a eff=3 line on the next pass
    for (ch=0; ch+(cpn-1) < numChans; ch+=cpn )   // since we want to combine 3 channels into one 24 bit rgb value, we jump by 3
    {
        old_bst=999;   // pick a value to gaurantee we will use a eff=3 line on the next pass

        if(ch%9==0)
        {
            SetStatusText(wxString("Status: " )+string_format(" Channel %ld. ",ch));
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

        if(ch==156)
        {
            rgb=0;
        }

        guiString= string_format("{DA98BD5D-9C00-40fe-A11C-AD3242573443}");
        f.Write("\t\t<Intervals>\n");
        //  for (p=0,csec=0; p < numPeriods; p++, csec+=interval, seqidx++)

        channels_exported+=cpn;

        for (p=0; p < numPeriods; p++)
        {
            seconds = (p*dataBuf->FrameTime())/1000.0;
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
            if(cpn==1)  // cpn (Channels per Node. if non rgb, we only use one byte
                rgb = ((*dataBuf)[p][ch]& 0xff) << 16 ;
            else
                rgb = ((*dataBuf)[p][ch]& 0xff) << 16 |
                      ((*dataBuf)[p][ch+1]& 0xff) << 8 |
                      ((*dataBuf)[p][ch+2]& 0xff); // we want a 24bit value for HLS

            //  if(rgb>0 or rgb<0)
            {
                bst=rgb;
                ben=rgb;
                // 4410 = 1/20th of a second. 88200/20
                if(rgb==0)
                {
                    if(cpn==1)
                        f.Write(string_format("\t\t\t<TimeInterval eff=\"4\" dat=\"\" gui=\"\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\"/>\n",pos));
                    else
                        f.Write(string_format("\t\t\t<TimeInterval eff=\"4\" dat=\"\" gui=\"\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\"  bst=\"-1\" ben=\"-1\"/>\n",pos));

                }
                else if(bst==old_bst)
                {
                    f.Write(string_format("\t\t\t<TimeInterval eff=\"7\" dat=\"\" gui=\"\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"2\"  />\n",pos));
                }
                else
                {
                    if(cpn==1)
                        f.Write(string_format("\t\t\t<TimeInterval eff=\"3\" dat=\"%s\" gui=\"%s\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\" />\n",xmlString,guiString,pos));
                    else
                        f.Write(string_format("\t\t\t<TimeInterval eff=\"3\" dat=\"%s\" gui=\"%s\"  in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\" bst=\"%ld\" ben=\"%ld\" />\n",xmlString,guiString,pos,bst,ben));

                }
                old_bst=bst;
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
    SetStatusText(wxString("Status: Export Complete. " )+string_format(" Channels exported=%4d ",channels_exported));

}

void FRAMECLASS WriteLSPFile(const wxString& filename )
{
    WriteLSPFile(filename, SeqData.NumChannels(), SeqData.NumFrames(), &SeqData, 0);
}



void FRAMECLASS WriteLorFile(const wxString& filename)
{
    wxString ChannelName,TestName;
    int32_t ChannelColor;
    int ch,p,csec,StartCSec, ii;
    int intensity,LastIntensity;
    wxFile f;
    int* savedIndexes;
    int savedIndexCount = 0;
    savedIndexes = (int *)calloc(SeqData.NumChannels(), sizeof(int));

    int index = 0;
    int rgbChanIndexes[3] = {0,0,0};
    int curRgbChanCount = 0;

    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
        return;
    }
    int interval=SeqData.FrameTime() / 10;  // in centiseconds
    long centiseconds=SeqData.NumFrames() * interval;

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    f.Write("<sequence saveFileVersion=\"3\"");
    if (mediaFilename.size() != 0)
    {
        f.Write(" musicFilename=\""+mediaFilename+"\"");
    }
    f.Write(">\n");
    f.Write("\t<channels>\n");
    for (ch=0; ch < SeqData.NumChannels(); ch++ )
    {
        SetStatusText(wxString("Status: " )+string_format(" Channel %ld ",ch));

        if (ch < CheckListBoxTestChannels->GetCount())
        {
            TestName=CheckListBoxTestChannels->GetString(ch);
        }
        else
        {
            TestName=string_format("Ch: %d",ch);
        }
        if (ch < ChannelNames.size() && ChannelNames[ch].size() != 0)
        {
            ChannelName = ChannelNames[ch];
        }
        else
        {
            ChannelName = TestName;
        }
        // LOR is BGR with high bits=0
        // Vix is RGB with high bits=1
        if (ch < ChannelColors.size() && ChannelColors[ch] > 0)
        {
            ChannelColor = ChannelColors[ch];
        }
        else if (TestName.Last() == 'R')
        {
            ChannelColor = 0x000000ff;
        }
        else if (TestName.Last() == 'G')
        {
            ChannelColor = 0x0000ff00;
        }
        else if (TestName.Last() == 'B')
        {
            ChannelColor = 0x00ff0000;
        }
        else
        {
            // default to white
            ChannelColor = 0x00ffffff;
        }
        f.Write("\t\t<channel name=\""+ChannelName+string_format("\" color=\"%d\" centiseconds=\"%ld\" savedIndex=\"%d\">\n",ChannelColor,centiseconds,index));
        // write intensity values for this channel
        LastIntensity=0;
        for (p=0,csec=0; p < SeqData.NumFrames(); p++, csec+=interval)
        {
            intensity=SeqData[p][ch] * 100 / 255;
            if (intensity != LastIntensity)
            {
                if (LastIntensity != 0)
                {
                    f.Write(string_format("\t\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n",StartCSec,csec,LastIntensity));
                }
                StartCSec=csec;
            }
            LastIntensity=intensity;
        }

        if (LastIntensity != 0)
        {
            f.Write(string_format("\t\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n",StartCSec,csec,LastIntensity));
        }
        f.Write("\t\t</channel>\n");
        if ( ch < CheckListBoxTestChannels->GetCount() &&
                (TestName.Last() == 'R' || TestName.Last() == 'G' || TestName.Last() == 'B'))
        {
            rgbChanIndexes[curRgbChanCount++]= index;
            if (curRgbChanCount == 3)
            {
                index++;
                f.Write("\t\t<rgbChannel name=\""+ Left(ChannelName, ChannelName.size()-1)+
                        string_format("(RGB)\" totalCentiseconds=\"%d\" savedIndex=\"%d\">\n", centiseconds, index));
                savedIndexes[savedIndexCount++] = index;
                f.Write("\t\t\t<channels>\n");
                for ( ii =0; ii <3 ; ii++)
                {
                    f.Write(string_format("\t\t\t\t<channel savedIndex=\"%d\"/>\n",rgbChanIndexes[ii]));
                }
                f.Write("\t\t\t</channels>\n");
                f.Write("\t\t</rgbChannel>\n");
                curRgbChanCount = 0;
            }
        }
        else
        {
            savedIndexes[savedIndexCount++] = index;
        }
        index++;
    }
    f.Write("\t</channels>\n");
    f.Write("\t<tracks>\n");
    f.Write(string_format("\t\t<track totalCentiseconds=\"%ld\">\n",centiseconds));
    f.Write("\t\t\t<channels>\n");
    for (ii=0; ii < savedIndexCount; ii++ )
    {
        f.Write(string_format("\t\t\t\t<channel savedIndex=\"%d\"/>\n",savedIndexes[ii]));
    }
    f.Write("\t\t\t</channels>\n");
    f.Write("\t\t\t<timings>\n");
    for (p=0,csec=0; p < SeqData.NumFrames(); p++, csec+=interval)
    {
        f.Write(string_format("\t\t\t\t<timing centisecond=\"%d\"/>\n",csec));
    }
    f.Write("\t\t\t</timings>\n");
    f.Write("\t\t</track>\n");
    f.Write("\t</tracks>\n");
    f.Write("</sequence>\n");
    f.Close();
    free(savedIndexes);

}
void FRAMECLASS WriteLcbFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf)
{
    wxString ChannelName,TestName;
    int ch,p,csec,StartCSec;
    int intensity,LastIntensity;
    wxFile f;
    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
        return;
    }


    wxString m_Path, m_Name, m_Ext;
    wxFileName::SplitPath(filename, &m_Path, &m_Name, &m_Ext);
    //  printf("'%s' is split as '%s', '%s', '%s'\n", m_FileName, m_Path,
    //  m_Name, m_Ext);

    int interval=SeqData.FrameTime() / 10;  // in centiseconds
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    f.Write("<channelsClipboard version=\"1\" name=\"" + m_Name + "\">\n");

    //  <channels>
    //  <channel>
    //  <effect type="intensity" startCentisecond="0" endCentisecond="10" intensity="83" />
    f.Write("<cellDemarcations>\n");
    for (p=0,csec=0; p < numPeriods; p++, csec+=interval)
    {
        f.Write(string_format("\t<cellDemarcation centisecond=\"%d\"/>\n",csec));
    }
    f.Write("</cellDemarcations>\n");
    //
    // LOR is BGR with high bits=0
    // Vix is RGB with high bits=1
    f.Write("<channels>\n");
    for (ch=0; ch < numChans; ch++ )
    {
        SetStatusText(wxString("Status: " )+string_format(" Channel %ld ",ch));

        f.Write("\t<channel>\n");
        LastIntensity=0;
        for (p=0,csec=0; p < numPeriods; p++, csec+=interval)
        {
            intensity=(*dataBuf)[p][ch] * 100 / 255;
            if (intensity != LastIntensity)
            {
                if (LastIntensity != 0)
                {
                    f.Write(string_format("\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n",
                                             StartCSec,csec,LastIntensity));
                }
                StartCSec=csec;
            }
            LastIntensity=intensity;
        }
        if (LastIntensity != 0)
        {
            f.Write(string_format("\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n",
                                     StartCSec,csec,LastIntensity));
        }
        f.Write("\t</channel>\n");
    }
    f.Write("</channels>\n");
    f.Write("</channelsClipboard>\n");
    f.Close();

}
void FRAMECLASS WriteLcbFile(const wxString& filename)
{
    WriteLcbFile(filename, SeqData.NumChannels(), SeqData.NumFrames(), &SeqData);
}

void FRAMECLASS WriteConductorFile(const wxString& filename)
{
    wxFile f;
    wxUint8 buf[16384];
    size_t ch,i,j;
    if (!f.Create(filename,true))
    {
        ConversionError(wxString("Unable to create file: ")+filename);
        return;
    }
    for (long period=0; period < SeqData.NumFrames(); period++)
    {
        //if (period % 500 == 499) AppendConvertStatus (string_format("Writing time period %ld\n",period+1));
        wxYield();
        for (i=0; i < 4096; i++)
        {
            for (j=0; j < 4; j++)
            {
                ch=j * 4096 + i;
                buf[i*4+j] = SeqData[period][ch];
            }
        }
        f.Write(buf,16384);
    }

    // pad the end of the file with 512 bytes of 0's
    memset(buf,0,512);
    f.Write(buf,512);
    f.Close();
}
#endif

// return true on success
#ifndef FPP
bool FRAMECLASS LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels, wxArrayString& VixChannelNames)
{
    wxString tag,tempstr;
    long OutputChannel;
    wxFileName fn;
    fn.AssignDir(CurrentDir);
    fn.SetFullName(ProfileName + ".pro");
    if (!fn.FileExists())
    {
        ConversionError(wxString("Unable to find Vixen profile: ")+fn.GetFullPath()+wxString("\n\nMake sure a copy is in your xLights directory"));
        return false;
    }
    wxXmlDocument doc( fn.GetFullPath() );
    if (doc.IsOk())
    {
        VixChannels.clear();
        wxXmlNode* root=doc.GetRoot();
        for( wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            tag = e->GetName();
            if (tag == wxString("ChannelObjects"))
            {
                for( wxXmlNode* p=e->GetChildren(); p!=NULL; p=p->GetNext() )
                {
                    if (p->GetName() == wxString("Channel"))
                    {
                        if (p->HasAttribute("output"))
                        {
                            tempstr=p->GetAttribute("output", "0");
                            OutputChannel = atol(tempstr.c_str());
                            VixChannels.push_back(OutputChannel);
                        }
                        if (p->HasAttribute("name"))
                        {
                            VixChannelNames.push_back(p->GetAttribute("name"));
                        }
                        else
                        {
                            if (p->GetChildren() != NULL) {
                                VixChannelNames.push_back(p->GetChildren()->GetContent());
                            } else {
                                VixChannelNames.push_back(p->GetContent());
                            }
                        }
                    }
                }
            }
        }
        return true;
    }
    else
    {
        ConversionError(wxString("Unable to load Vixen profile: ")+ProfileName);
    }
    return false;
}

void FRAMECLASS ReadConductorFile(const wxString& FileName)
{
    wxFile f;
    int i,j,ch;
    char row[16384];
    int period=0;
    ConversionInit();
    wxFileDialog mediaDialog(this,wxString("Select associated media file, or cancel if this is an animation"));
    if (mediaDialog.ShowModal() == wxID_OK)
    {
        SetMediaFilename(mediaDialog.GetPath());
    }
    if (!f.Open(FileName.c_str()))
    {
        PlayerError(wxString("Unable to load sequence:\n")+FileName);
        return;
    }
    int numPeriods=f.Length()/16384;

    SeqData.init(NetInfo.GetTotChannels(),numPeriods,50);
    while (f.Read(row,16384) == 16384)
    {
        wxYield();
        for (i=0; i < 4096; i++)
        {
            for (j=0; j < 4; j++)
            {
                ch=j * 4096 + i;
                if (ch < SeqData.NumChannels())
                {
                    SeqData[period][ch] = row[i*4+j];
                }
            }
        }
        period++;
    }
    f.Close();
}
#endif

void FRAMECLASS ReadXlightsFile(const wxString& FileName, wxString *mediaFilename)
{
    wxFile f;
    char hdr[512],filetype[10];
    int fileversion,numch,numper,scancnt;
    size_t readcnt;

    ConversionInit();
    if (!f.Open(FileName.c_str()))
    {
        PlayerError(wxString("Unable to load sequence:\n")+FileName);
        return;
    }
    f.Read(hdr,512);
    scancnt=sscanf(hdr,"%8s %2d %8d %8d",filetype,&fileversion,&numch,&numper);
    if (scancnt != 4 || strncmp(filetype,"xLights",7) != 0 || numch <= 0 || numper <= 0)
    {
        PlayerError(wxString("Invalid file header:\n")+FileName);
    }
    else
    {
        SeqData.init(numch, numper, 50);
        char * buf = new char[numper];
        wxString filename=FromAscii(hdr+32);
        if (mediaFilename) {
            *mediaFilename = filename;
        } else {
            SetMediaFilename(filename);
        }
        for (int x = 0; x < numch; x++) {
            readcnt = f.Read(buf, numper);
            if (readcnt < numper)
            {
                PlayerError(wxString("Unable to read all event data from:\n")+FileName);
            }
            for (int p = 0; p < numper; p++) {
                SeqData[p][x] = buf[p];
            }
        }
        delete [] buf;
#ifndef NDEBUG
        AppendConvertLog (string_format(wxString("ReadXlightsFile SeqData.NumFrames()=%ld SeqData.NumChannels()=%ld\n"),SeqData.NumFrames(),SeqData.NumChannels()));
#endif
    }
    f.Close();
}

void FRAMECLASS ReadGlediatorFile(const wxString& FileName)
{
    wxFile f;

    size_t fileLength;
    int j,period,x_width=32,y_height=32; // for now hard code matrix to be 32x32. after we get this working, we will prompt for this info during convert
    wxString filename=string_format(wxString("01 - Carol of the Bells.mp3")); // hard code a mp3 file for now
    size_t readcnt;

    ConversionInit();
    if (!f.Open(FileName.c_str()))
    {
        PlayerError(wxString("Unable to load sequence:\n")+FileName);
        return;
    }

    fileLength=f.Length();
    int numChannels=(x_width*3*y_height); // 3072 = 32*32*3
    char *frameBuffer=new char[SeqData.NumChannels()];

    int numFrames=(int)(fileLength/(x_width*3*y_height));
    SetMediaFilename(filename);
    SeqData.init(numChannels, numFrames, 50);

    wxYield();
    period = 0;
    while((readcnt=f.Read(frameBuffer,SeqData.NumChannels())))   // Read one period of channels
    {
        for(j=0; j<readcnt; j++)   // Loop thru all channel.s
        {
            SeqData[period][j] = frameBuffer[j];
        }
        period++;
    }

    /*
    for(i=0; i<readcnt-2; i++) {
        SeqData[i] = i%256;
    }
    for(i=0; i<readcnt-2; i+=3) { // loop thru channels, jump by 3. so this loop is pixel loop
        period = i/bytes_per_period;
        p=period * (bytes_per_period); // byte offset for start of each period
        ch=p+ (y*x_width*3) + x*3; // shows offset into source buffer
        byte =p+i;
        byte1=p+i+(1)* (SeqData.NumFrames());
        byte2=p+i+(2)* (SeqData.NumFrames());
        if ( byte2<readcnt) {
            SeqData[byte]  = row[i];
            SeqData[byte1] = row[i+1];
            SeqData[byte2] = row[i+2];
        }
    }
    //   }
    */
    f.Close();
    delete[] frameBuffer;

#ifndef NDEBUG
    AppendConvertLog (string_format(wxString("ReadGlediatorFile SeqData.NumFrames()=%ld SeqData.NumChannels()=%ld\n"),SeqData.NumFrames(),SeqData.NumChannels()));
#endif
}


int getAttributeValueAsInt(SP_XmlStartTagEvent * stagEvent, const char * name)
{
    const char *val = stagEvent -> getAttrValue(name);
    if (!val)
    {
        return 0;
    }
    return atoi(val);
}
wxString getAttributeValueSafe(SP_XmlStartTagEvent * stagEvent, const char * name)
{
    const char *val = stagEvent -> getAttrValue(name);
    if (!val)
    {
        return "";
    }
    return FromAscii(val);
}


void FRAMECLASS ReadVixFile(const wxString& filename)
{
    wxString NodeName,NodeValue,msg;
    std::vector<unsigned char> VixSeqData;
    wxArrayInt VixChannels;
    wxArrayString VixChannelNames;
    long cnt = 0;
    wxArrayString context;
    long VixEventPeriod=-1;
    long MaxIntensity = 255;
    int OutputChannel;

    ConversionInit();
    AppendConvertStatus (wxString("Reading Vixen sequence\n"));

    SP_XmlPullParser *parser = new SP_XmlPullParser();
    parser->setMaxTextSize(MAX_READ_BLOCK_SIZE / 2);
    wxFile file(filename);
    char *bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);
    wxString carryOver;

    //pass 1, read the length, determine number of networks, units/network, channels per unit
    SP_XmlPullEvent * event = parser->getNext();
    int done = 0;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch(event -> getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii( stagEvent->getName() );
                context.push_back(NodeName);
                cnt++;
                //msg=wxString("Element: ") + NodeName + string_format(wxString(" (%ld)\n"),cnt);
                //AppendConvertStatus (msg);
                if (cnt == 2 && (NodeName == wxString("Audio") || NodeName == wxString("Song")))
                {
                    SetMediaFilename(getAttributeValueSafe(stagEvent, "filename") );
                }
                if (cnt > 1 && context[1] == wxString("Channels") && NodeName == wxString("Channel"))
                {
                    const char *val = stagEvent -> getAttrValue("output");
                    if (!val)
                    {
                        VixChannels.push_back(VixChannels.size());
                    }
                    else
                    {
                        VixChannels.push_back(atoi(val));
                    }
                }
            }
            break;
            case SP_XmlPullEvent::eCData:
            {
                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt == 2)
                {
                    NodeValue = FromAscii( stagEvent->getText() );
                    if (context[1] == wxString("MaximumLevel"))
                    {
                        MaxIntensity = atol(NodeValue.c_str());
                    }
                    else if (context[1] == wxString("EventPeriodInMilliseconds"))
                    {
                        VixEventPeriod = atol(NodeValue.c_str());
                    }
                    else if (context[1] == wxString("EventValues"))
                    {
                        //AppendConvertStatus(string_format(wxString("Chunk Size=%d\n"), NodeValue.size()));
                        if (carryOver.size() > 0) {
                            NodeValue.insert(0, carryOver);
                        }
                        int i = base64_decode(NodeValue, VixSeqData);
                        if (i != 0) {
                            int start = NodeValue.size() - i - 1;
                            carryOver = NodeValue.substr(start, start + i);
                        } else {
                            carryOver.clear();
                        }
                    }
                    else if (context[1] == wxString("Profile"))
                    {
#ifndef FPP
                        LoadVixenProfile(NodeValue,VixChannels,VixChannelNames);
#endif
                    }
                    else if (context[1] == wxString("Channels") && context[2] == wxString("Channel"))
                    {
                        while (VixChannelNames.size() < VixChannels.size())
                        {
                            VixChannelNames.push_back("");
                        }
                        VixChannelNames[VixChannels.size() - 1] = NodeValue;
                    }
                }
                break;
            }
            case SP_XmlPullEvent::eEndTag:
            {
                if (cnt > 0)
                {
                    RemoveAt(context, cnt-1);
                }
                cnt = context.size();
                break;
            }
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete [] bytes;
    delete parser;
    file.Close();

    int min = 999999;
    int max = 0;
    for (int x = 0; x < VixChannels.size(); x++)
    {
        int i = VixChannels[x];
        if (i > max)
        {
            max = i;
        }
        if (i < min)
        {
            min = i;
        }
    }

    long VixDataLen = VixSeqData.size();
    int numChannels = (max - min) + 1;
    if (numChannels < 0)
    {
        numChannels = 0;
    }
    AppendConvertStatus (string_format(wxString("Max Intensity=%ld\n"),MaxIntensity), false);
    AppendConvertStatus (string_format(wxString("# of Channels=%ld\n"),numChannels), false);
    AppendConvertStatus (string_format(wxString("Vix Event Period=%ld\n"),VixEventPeriod), false);
    AppendConvertStatus (string_format(wxString("Vix data len=%ld\n"),VixDataLen), false);
    if (numChannels == 0)
    {
        return;
    }
    long VixNumPeriods = VixDataLen / VixChannels.size();
    AppendConvertStatus (string_format(wxString("Vix # of time periods=%ld\n"),VixNumPeriods), false);
    AppendConvertStatus (wxString("Media file=")+mediaFilename+wxString("\n"), false);
    if (VixNumPeriods == 0) {
        return;
    }
    SeqData.init(numChannels, VixNumPeriods, VixEventPeriod);

    // reorder channels according to output number, scale so that max intensity is 255
    int newper,intensity;
    size_t ch;
    for (ch=0; ch < SeqData.NumChannels(); ch++)
    {
        OutputChannel = VixChannels[ch] - min;
        if (ch < VixChannelNames.size())
        {
            ChannelNames[OutputChannel] = VixChannelNames[ch];
        }
        for (newper=0; newper < SeqData.NumFrames(); newper++)
        {
            intensity=VixSeqData[ch*VixNumPeriods+newper];
            if (MaxIntensity != 255)
            {
                intensity=intensity * 255 / MaxIntensity;
            }
            SeqData[newper][OutputChannel] = intensity;
        }
    }
}

void FRAMECLASS ReadHLSFile(const wxString& filename)
{
    long timeCells = 0;
    long msPerCell = 50;
    long channels = 0;
    long cnt = 0;
    long tmp;
    long universe = 0;
    long channelsInUniverse = 0;
    wxString NodeName, NodeValue, Data, ChannelName;
    wxArrayString context;
    wxArrayInt map;


    SP_XmlPullParser *parser = new SP_XmlPullParser();
    wxFile file(filename);
    char *bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);

    // pass one, get the metadata
    SP_XmlPullEvent * event = parser->getNext();
    int done = 0;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch(event -> getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii( stagEvent->getName() );
                context.push_back(NodeName);
                cnt++;
                break;
            }
            case SP_XmlPullEvent::eCData:
            {
                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    NodeValue = FromAscii( stagEvent -> getText());

                    if (NodeName == wxString("MilliSecPerTimeUnit"))
                    {
                        msPerCell = atol(NodeValue.c_str());
                    }
                    if (NodeName == wxString("NumberOfTimeCells"))
                    {
                        timeCells = atol(NodeValue.c_str());
                    }
                    if (NodeName == wxString("AudioSourcePcmFile"))
                    {
                        mediaFilename = NodeValue;
                        if (Right(mediaFilename, 4) == ".PCM")
                        {
                            //nothing can deal with PCM files, we'll assume this came from an mp3
                            mediaFilename = Left(mediaFilename, mediaFilename.size() - 4);
                            mediaFilename += ".mp3";
                        }
                    }
                    if (NodeName == wxString("ChannelsInUniverse"))
                    {
                        channelsInUniverse = atol(NodeValue.c_str());
                        channels += channelsInUniverse;
                    }
                    if (NodeName == wxString("UniverseNumber"))
                    {
                        tmp = atol(NodeValue.c_str());
                        universe = tmp;
                    }
                }
                break;
            }
            case SP_XmlPullEvent::eEndTag:
            {
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    if (NodeName == wxString("Universe"))
                    {
                        map.push_back(universe);
                        map.push_back(channelsInUniverse);
                        for (tmp = map.size() - 2; tmp > 0; tmp -= 2)
                        {
                            if (map[tmp] < map[tmp - 2])
                            {
                                long t1 = map[tmp];
                                long t2 = map[tmp + 1];
                                map[tmp] = map[tmp - 2];
                                map[tmp + 1] = map[tmp - 1];
                                map[tmp - 2] = t1;
                                map[tmp - 1] = t2;
                            }
                        }
                    }

                    RemoveAt(context, cnt-1);
                }
                cnt = context.size();
                break;
            }
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete parser;

    file.Seek(0);



    channels = 0;

    for (tmp = 0; tmp < map.size(); tmp += 2)
    {
        int i = map[tmp + 1];
        int orig = NetInfo.GetNumChannels(tmp / 2);
        if (i < orig) {
            AppendConvertStatus (string_format(wxString("Found Universe: %ld   Channels in Seq: %ld   Configured: %d\n"), map[tmp], i, orig), false);
            i = orig;
        } else if (i > orig) {
            AppendConvertStatus (string_format(wxString("WARNING Universe: %ld contains more channels than you have configured.\n"), map[tmp]), false);
            AppendConvertStatus (string_format(wxString("Found Universe: %ld   Channels in Seq: %ld   Configured: %d\n"), map[tmp], i, orig), false);
        } else {
            AppendConvertStatus (string_format(wxString("Found Universe: %ld   Channels in Seq: %ld\n"), map[tmp], i, orig), false);
        }


        map[tmp + 1] = channels;
        channels += i;
    }

    AppendConvertStatus (string_format(wxString("TimeCells = %d\n"), timeCells), false);
    AppendConvertStatus (string_format(wxString("msPerCell = %d ms\n"), msPerCell), false);
    AppendConvertStatus (string_format(wxString("Channels = %d\n"), channels), false);
    if (channels == 0)
    {
        return;
    }
    if (timeCells == 0)
    {
        return;
    }
    SeqData.init(channels, timeCells, msPerCell);

    ChannelNames.resize(channels);
    ChannelColors.resize(channels);

    channels = 0;

    wxYield();

    parser = new SP_XmlPullParser();
    read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);


    event = parser->getNext();
    done = 0;
    int nodecnt = 0;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch(event -> getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii( stagEvent->getName() );
                context.push_back(NodeName);
                cnt++;
                break;
            }
            case SP_XmlPullEvent::eCData:
            {
                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    NodeValue = FromAscii( stagEvent -> getText());

                    if (NodeName == wxString("ChanInfo"))
                    {
                        //channel name and type
                        ChannelName = NodeValue;
                    }
                    if (NodeName == wxString("Block"))
                    {
                        int idx = NodeValue.find("-");
                        Data.append(NodeValue.substr(idx + 1));
                    }
                    if (NodeName == wxString("UniverseNumber"))
                    {
                        tmp = atol(NodeValue.c_str());
                        universe = tmp;
                        for (tmp = 0; tmp < map.size() ; tmp += 2)
                        {
                            if (universe == map[tmp])
                            {
                                channels = map[tmp + 1];
                            }
                        }
                    }
                }
                break;
            }
            case SP_XmlPullEvent::eEndTag:
            {
                if (nodecnt > 1000)
                {
                    nodecnt=0;
                    wxYield();
                }
                nodecnt++;
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    if (NodeName == wxString("ChannelData"))
                    {

                        //finished reading this channel, map the data
                        int idx = ChannelName.find(", ");
                        wxString type = ChannelName.substr(idx + 2);
                        wxString origName = ChannelNames[channels];
                        if (type == wxString("RGB-R"))
                        {
                            ChannelNames[channels] = Left(ChannelName, idx) + wxString("-R");
                            ChannelColors[channels] = 0x000000FF;
                        }
                        else if (type == wxString("RGB-G"))
                        {
                            ChannelNames[channels] = Left(ChannelName, idx) + wxString("-G");
                            ChannelColors[channels] = 0x0000FF00;
                        }
                        else if (type == wxString("RGB-B"))
                        {
                            ChannelNames[channels] = Left(ChannelName, idx) + wxString("-B");
                            ChannelColors[channels] = 0x00FF0000;
                        }
                        else
                        {
                            ChannelNames[channels] = Left(ChannelName, idx);
                            ChannelColors[channels] = 0x00FFFFFF;
                        }
                        wxString o2 = NetInfo.GetChannelName(channels);
                        AppendConvertStatus (string_format("Map %s -> %s (%s)\n",
                                                           ChannelNames[channels].c_str(),
                                                           origName.c_str(),
                                                           o2.c_str()), false);
                        for (long newper = 0; newper < SeqData.NumFrames(); newper++)
                        {
                            long intensity;
                            intensity = strtoul(Data.substr(newper * 3, 2).c_str(), NULL, 16);
                            SeqData[newper][channels] = intensity;
                        }
                        Data.clear();
                        channels++;
                    }

                    RemoveAt(context, cnt-1);
                }
                cnt = context.size();
                break;
            }
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete [] bytes;
    delete parser;
    file.Close();
}

class LORInfo
{
public:
    wxString name;
    int savedIndex;
    wxString deviceType;
    int unit;
    int circuit;
    int network;
    bool empty;

    LORInfo(const wxString & nm, const wxString &dt, int n, int u, int c, int s) :
        name(nm), deviceType(dt), network(n), unit(u), circuit(c), savedIndex(s), empty(true)
    {
    }
    LORInfo(const LORInfo &li) :
        name(li.name),
        deviceType(li.deviceType), network(li.network), unit(li.unit),
        circuit(li.circuit), savedIndex(li.savedIndex), empty(li.empty)
    {

    }
    LORInfo() :
        name(""), deviceType(""), network(0), unit(0), circuit(0), savedIndex(0), empty(true)
    {

    }
};

typedef std::map<int, LORInfo> LORInfoMap;


static void mapLORInfo(const LORInfo &info, std::vector< std::vector<int> > *unitSizes)
{
    int unit = info.unit;
    if (unit < 0)
    {
        unit+=256;
    }
    if (unit == 0)
    {
        unit = 1;
    }

    if (info.network >= unitSizes->size())
    {
        unitSizes->resize(info.network + 1);
    }
    if (unit > (*unitSizes)[info.network].size())
    {
        (*unitSizes)[info.network].resize(unit);
    }
    if (info.circuit == 0)
    {
        (*unitSizes)[info.network][unit - 1]++;
    }
    else if (info.circuit > (*unitSizes)[info.network][unit - 1])
    {
        (*unitSizes)[info.network][unit - 1] = info.circuit;
    }
}

void FRAMECLASS ReadLorFile(const wxString& filename, int LORImportInterval)
{
    wxString NodeName,msg,EffectType,ChannelName,deviceType,networkAsString;
    wxArrayString context;
    int unit,circuit,startcsec,endcsec,intensity,startIntensity,endIntensity,rampdiff,ChannelColor;
    int i,startper,endper,perdiff,twinklestate,nexttwinkle;
    int twinkleperiod = 400;
    int curchannel = -1;
    int MappedChannelCnt = 0;
    int MaxIntensity = 100;
    int EffectCnt = 0;
    int network,chindex = -1;
    long cnt = 0;
    std::vector< std::vector<int> > noNetworkUnitSizes;
    std::vector< std::vector<int> > lorUnitSizes;
    std::vector< std::vector<int> > dmxUnitSizes;
    LORInfoMap rgbChannels;

    LorTimingList.clear();

    ConversionInit();
    AppendConvertStatus (wxString("Reading LOR sequence\n"));
    SetStatusText(wxString("Reading LOR sequence"));


    int centisec = -1;
    int nodecnt=0;
    int channelCount = 0;

    SP_XmlPullParser *parser = new SP_XmlPullParser();
    wxFile file(filename);
    char *bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);
    bool mapEmpty = mapEmptyChannels();

    //pass 1, read the length, determine number of networks, units/network, channels per unit
    SP_XmlPullEvent * event = parser->getNext();
    int done = 0;
    int savedIndex = 0;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch(event -> getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii(stagEvent->getName());
                context.push_back(NodeName);
                cnt++;

                nodecnt++;
                if (nodecnt > 1000)
                {
                    nodecnt=0;
                    wxYield();
                }
                if (NodeName == wxString("track"))
                {
                    centisec = getAttributeValueAsInt(stagEvent, "totalCentiseconds");
                }
                else if (cnt == 3 && context[1] == wxString("channels") && NodeName == wxString("channel"))
                {
                    channelCount++;
                    if ((channelCount % 1000) == 0)
                    {
                        AppendConvertStatus (string_format(wxString("Channels found so far: %d\n"),channelCount));
                        SetStatusText(string_format(wxString("Channels found so far: %d"),channelCount));
                    }

                    deviceType = FromAscii( stagEvent->getAttrValue("deviceType") );
                    network = getAttributeValueAsInt(stagEvent, "network");
                    networkAsString = FromAscii( stagEvent->getAttrValue("network") );
                    unit = getAttributeValueAsInt(stagEvent, "unit");
                    circuit = getAttributeValueAsInt(stagEvent, "circuit");
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");
                    wxString channelName = FromAscii( stagEvent->getAttrValue("name") );
                    rgbChannels[savedIndex] = LORInfo(channelName, deviceType, network, unit, circuit, savedIndex);
                    rgbChannels[savedIndex].empty = !mapEmpty;
                }
                else if (cnt > 1 && context[1] == wxString("channels") && NodeName == wxString("effect"))
                {
                    rgbChannels[savedIndex].empty = false;
                }
                else if (cnt > 3 && context[1] == wxString("channels") && context[2] == wxString("rgbChannel")
                         && context[3] == wxString("channels") && NodeName == wxString("channel"))
                {
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");
                    if (rgbChannels[savedIndex].empty == true)
                    {
                        std::vector< std::vector<int> > *unitSizes;
                        if (Left(rgbChannels[savedIndex].deviceType, 3) == "DMX")
                        {
                            unitSizes = &dmxUnitSizes;
                        } else if ("" == deviceType && "" == networkAsString && !MapLORChannelsWithNoNetwork->IsChecked()) {
                            unitSizes = &noNetworkUnitSizes;
                        }
                        else
                        {
                            unitSizes = &lorUnitSizes;
                        }
                        mapLORInfo(rgbChannels[savedIndex], unitSizes);
                    }
                    rgbChannels[savedIndex].empty = false;
                }
            }
            break;
            case SP_XmlPullEvent::eEndTag:
                if (cnt == 3 && context[1] == wxString("channels") && context[2] == wxString("channel") && !rgbChannels[savedIndex].empty)
                {
                    std::vector< std::vector<int> > *unitSizes;
                    if (Left(rgbChannels[savedIndex].deviceType, 3) == "DMX") {
                        unitSizes = &dmxUnitSizes;
                    } else if ("" == deviceType && "" == networkAsString && !MapLORChannelsWithNoNetwork->IsChecked()) {
                        unitSizes = &noNetworkUnitSizes;
                    } else {
                        unitSizes = &lorUnitSizes;
                    }
                    mapLORInfo(rgbChannels[savedIndex], unitSizes);
                }

                if (cnt > 0)
                {
                    RemoveAt(context, cnt-1);
                }
                cnt = context.size();
                break;
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete parser;
    AppendConvertStatus (string_format(wxString("Track 1 length = %d centiseconds\n"),centisec), false);

    if (centisec > 0)
    {
        int numFrames = centisec * 10 / LORImportInterval;
        if (numFrames == 0)
        {
            numFrames=1;
        }
        SeqData.init(NetInfo.GetTotChannels(), numFrames, LORImportInterval);
    }
    else
    {
        ConversionError(wxString("Unable to determine the length of this LOR sequence (looked for length of track 1)"));
        return;
    }

    for (network = 0; network < lorUnitSizes.size(); network++)
    {
        cnt = 0;
        for (int u = 0; u < lorUnitSizes[network].size(); u++)
        {
            cnt += lorUnitSizes[network][u];
        }
        AppendConvertStatus (string_format(wxString("LOR Network %d:  %d channels\n"),network,cnt), false);
    }
    for (network = 1; network < dmxUnitSizes.size(); network++)
    {
        cnt = 0;
        for (int u = 0; u < dmxUnitSizes[network].size(); u++)
        {
            if (cnt < dmxUnitSizes[network][u]) {
                cnt = dmxUnitSizes[network][u];
            }
        }
        AppendConvertStatus (string_format(wxString("DMX Network %d:  %d channels\n"),network,cnt), false);
    }
    AppendConvertStatus (string_format(wxString("Total channels = %d\n"),channelCount));

    cnt = 0;
    context.clear();
    channelCount = 0;


    parser = new SP_XmlPullParser();
    file.Seek(0);
    read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);

    //pass 2, convert the data
    event = parser->getNext();
    done = 0;
    bool empty;
    while (!done)
    {
        if (!event)
        {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0)
            {
                done = true;
            }
            else
            {
                parser->append(bytes, read);
            }
        }
        else
        {
            switch(event -> getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eEndTag:
                if (cnt > 0)
                {
                    RemoveAt(context, cnt-1);
                }
                cnt = context.size();
                if (cnt == 2)
                {
                    if (empty && curchannel != -1)
                    {
                        chindex--;
                        AppendConvertStatus (wxString("WARNING: ")+ChannelNames[curchannel] + " is empty\n");
                        ChannelNames[curchannel].clear();
                        MappedChannelCnt--;
                    }
                    curchannel = -1;
                }
                break;
            case SP_XmlPullEvent::eStartTag:
            {
                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                NodeName = FromAscii(stagEvent->getName());
                context.push_back(NodeName);
                cnt++;

                nodecnt++;
                if (nodecnt > 1000)
                {
                    nodecnt=0;
                    wxYield();
                }
                //msg=wxString("Element: ") + NodeName + string_format(wxString(" (%ld)\n"),cnt);
                //AppendConvertStatus (msg);
                if (NodeName == wxString("sequence"))
                {
                    SetMediaFilename( getAttributeValueSafe(stagEvent, "musicFilename") );
                }
                if (cnt == 3 && context[1] == wxString("channels") && NodeName == wxString("channel"))
                {
                    channelCount++;
                    if ((channelCount % 1000) == 0)
                    {
                        AppendConvertStatus (string_format(wxString("Channels converted so far: %d\n"),channelCount));
                        SetStatusText(string_format(wxString("Channels converted so far: %d"),channelCount));
                    }

                    deviceType = getAttributeValueSafe(stagEvent, "deviceType");
                    networkAsString = getAttributeValueSafe(stagEvent, "network");
                    network = getAttributeValueAsInt(stagEvent, "network");

                    unit = getAttributeValueAsInt(stagEvent, "unit");
                    if (unit < 0)
                    {
                        unit+=256;
                    }
                    if (unit == 0)
                    {
                        unit = 1;
                    }
                    circuit = getAttributeValueAsInt(stagEvent, "circuit");
                    ChannelName = getAttributeValueSafe(stagEvent, "name");
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");

                    empty = rgbChannels[savedIndex].empty;
                    if (Left(deviceType, 3) == "DMX")
                    {
                        chindex=circuit-1;
                        network--;
                        network += lorUnitSizes.size();
                        curchannel = NetInfo.CalcAbsChannel(network,chindex);
                    }
                    else if (Left(deviceType, 3) == "LOR")
                    {
                        chindex = 0;
                        for (int z = 0; z < (unit - 1); z++)
                        {
                            chindex += lorUnitSizes[network][z];
                        }
                        chindex += circuit-1;
                        curchannel = NetInfo.CalcAbsChannel(network,chindex);
                    } else if ("" == deviceType && "" == networkAsString && !MapLORChannelsWithNoNetwork->IsChecked()) {
                        curchannel = -1;
                    } else {
                        chindex++;
                        if (chindex < NetInfo.GetTotChannels())
                        {
                            curchannel = chindex;
                        }
                        else
                        {
                            curchannel = -1;
                        }
                    }
                    if (curchannel >= 0)
                    {
                        //AppendConvertStatus (string_format(wxString("curchannel %d\n"),curchannel));
                        if (ChannelNames[curchannel].size() != 0)
                        {
                            AppendConvertStatus (string_format(wxString("WARNING: ")+ChannelNames[curchannel]+wxString(" and ")
                                                                 +ChannelName+wxString(" map to the same channel %d\n"), curchannel));
                        }
                        MappedChannelCnt++;
                        ChannelNames[curchannel] = ChannelName;
                        ChannelColor = getAttributeValueAsInt(stagEvent, "color");
                        ChannelColors[curchannel] = ChannelColor;
                    }
                    else
                    {
                        AppendConvertStatus (wxString("WARNING: channel '")+ChannelName+wxString("' is unmapped\n"));
                    }
                }
                if (cnt > 1 && context[1] == wxString("channels") && NodeName == wxString("effect") && curchannel >= 0)
                {
                    empty = false;
                    EffectCnt++;
                    startcsec = getAttributeValueAsInt(stagEvent, "startCentisecond");
                    endcsec = getAttributeValueAsInt(stagEvent, "endCentisecond");
                    intensity = getAttributeValueAsInt(stagEvent, "intensity");
                    startIntensity = getAttributeValueAsInt(stagEvent, "startIntensity");
                    endIntensity = getAttributeValueAsInt(stagEvent, "endIntensity");
                    startper = startcsec * 10 / LORImportInterval;
                    endper = endcsec * 10 / LORImportInterval;
                    perdiff=endper - startper;  // # of ticks
                    LorTimingList.insert(startper);

                    if (perdiff > 0)
                    {
                        intensity=intensity * 255 / MaxIntensity;
                        startIntensity=startIntensity * 255 / MaxIntensity;
                        endIntensity=endIntensity * 255 / MaxIntensity;
                        EffectType = getAttributeValueSafe(stagEvent, "type");
                        if (EffectType == "intensity")
                        {
                            if (intensity > 0)
                            {
                                for (i=0; i < perdiff; i++)
                                {
                                    SeqData[startper+i][curchannel] = intensity;
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff=endIntensity - startIntensity;
                                for (i=0; i < perdiff; i++)
                                {
                                    intensity=(int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    SeqData[startper+i][curchannel] = intensity;
                                }
                            }
                        }
                        else if (EffectType == "twinkle")
                        {
                            if (intensity == 0 && startIntensity == 0 && endIntensity == 0)
                            {
                                intensity=MaxIntensity;
                            }
                            twinklestate=static_cast<int>(rand01()*2.0) & 0x01;
                            nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / LORImportInterval;
                            if (intensity > 0)
                            {
                                for (i=0; i < perdiff; i++)
                                {
                                    SeqData[startper + i][curchannel] = intensity * twinklestate;
                                    nexttwinkle--;
                                    if (nexttwinkle <= 0)
                                    {
                                        twinklestate=1-twinklestate;
                                        nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / LORImportInterval;
                                    }
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff=endIntensity - startIntensity;
                                for (i=0; i < perdiff; i++)
                                {
                                    intensity=(int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    SeqData[startper + i][curchannel] = intensity * twinklestate;
                                    nexttwinkle--;
                                    if (nexttwinkle <= 0)
                                    {
                                        twinklestate=1-twinklestate;
                                        nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / LORImportInterval;
                                    }
                                }
                            }
                        }
                        else if (EffectType == "shimmer")
                        {
                            if (intensity == 0 && startIntensity == 0 && endIntensity == 0)
                            {
                                intensity=MaxIntensity;
                            }
                            if (intensity > 0)
                            {
                                for (i=0; i < perdiff; i++)
                                {
                                    twinklestate=(startper + i) & 0x01;
                                    SeqData[startper+i][curchannel] = intensity * twinklestate;
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff=endIntensity - startIntensity;
                                for (i=0; i < perdiff; i++)
                                {
                                    twinklestate=(startper + i) & 0x01;
                                    intensity=(int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    SeqData[startper+i][curchannel] = intensity * twinklestate;
                                }
                            }
                        }
                    }
                }
            }
            default:
                break;
            }
            delete event;
        }
        if (!done)
        {
            event = parser->getNext();
        }
    }
    delete [] bytes;
    delete parser;
    file.Close();


    AppendConvertStatus (string_format(wxString("# of mapped channels with effects=%d\n"),MappedChannelCnt), false);
    AppendConvertStatus (string_format(wxString("# of effects=%d\n"),EffectCnt), false);
    AppendConvertStatus (wxString("Media file=")+mediaFilename+wxString("\n"), false);
    AppendConvertStatus (string_format(wxString("New # of time periods=%ld\n"),SeqData.NumFrames()), false);
    SetStatusText(wxString("LOR sequence loaded successfully"));
}

void FRAMECLASS ClearLastPeriod()
{
    int LastPer = SeqData.NumFrames()-1;
    for (size_t ch=0; ch < SeqData.NumChannels(); ch++)
    {
        SeqData[LastPer][ch] = 0;
    }
}

void FRAMECLASS DoConversion(const wxString& Filename, const wxString& OutputFormat)
{
    wxString fullpath;
    wxString Out3=Left(OutputFormat, 3);

    // read sequence file
    AppendConvertStatus (wxString("\nReading: ") + Filename + "\n");
    wxYield();
    wxFileName oName(Filename);
    wxString ext = oName.GetExt();
    if (ext == wxString("vix"))
    {
        if (Out3 == "Vix")
        {
            ConversionError(wxString("Cannot convert from Vixen to Vixen!"));
            return;
        }
        ReadVixFile(Filename);
    }
    else if (ext == wxString(XLIGHTS_SEQUENCE_EXT))
    {
        if (Out3 == "xLi")
        {
            ConversionError(wxString("Cannot convert from xLights to xLights!"));
            return;
        }
        ReadXlightsFile(Filename);
    }
    else if (ext == wxString("fseq"))
    {
        if (Out3 == "Fal")
        {
            ConversionError(wxString("Cannot convert from Falcon Player file to Falcon Player file!"));
            return;
        }
        ReadFalconFile(Filename);
    }
#ifndef FPP
    else if (ext == wxString("seq"))
    {
        if (Out3 == "Lyn")
        {
            ConversionError(wxString("Cannot convert from Conductor file to Conductor file!"));
            return;
        }
        ReadConductorFile(Filename);
    }
#endif
    else if (ext == wxString("gled"))
    {
        if (Out3 == "Gle")
        {
            ConversionError(wxString("Cannot convert from Glediator file to Glediator file!"));
            return;
        }
        ReadGlediatorFile(Filename);
    }
    else if (ext == wxString("hlsIdata"))
    {
        ReadHLSFile(Filename);
    }
    else if (ext == wxString("lms") || ext == wxString("las"))
    {
        if (Out3 == "LOR")
        {
            ConversionError(wxString("Cannot convert from LOR to LOR!"));
            return;
        }
        int i = LORImportTimeResolution->GetSelection();
        switch (i) {
            case 0:
                i = 25;
                break;
            case 2:
                i = 100;
                break;
            default:
                i = 50;
                break;
        }
        ReadLorFile(Filename, i);
    }
    else
    {
        ConversionError(wxString("Unknown sequence file extension"));
        return;
    }

    // check for errors
    if (SeqData.NumChannels() == 0)
    {
        AppendConvertStatus (wxString("ERROR: no channels defined\n"));
        return;
    }

    if (isSetOffAtEnd())
    {
        ClearLastPeriod();
    }
    wxYield();

    // write converted file to xLights directory
    oName.SetPath( CurrentDir );

    if (Out3 == "xLi")
    {
        oName.SetExt(wxString(XLIGHTS_SEQUENCE_EXT));
        fullpath=oName.GetFullPath();
        AppendConvertStatus (wxString("Writing xLights sequence\n"));
        WriteXLightsFile(fullpath);
        AppendConvertStatus (wxString("Finished writing new file: ")+fullpath+wxString("\n"));
    }
    else if (Out3 == "Fal")
    {
        oName.SetExt(wxString("fseq"));
        fullpath=oName.GetFullPath();
        AppendConvertStatus (wxString("Writing Falcon Player sequence\n"));
        WriteFalconPiFile(fullpath);
        AppendConvertStatus (wxString("Finished writing new file: ")+fullpath+wxString("\n"));
    }
#ifndef FPP
    else if (Out3 == "Lyn")
    {
        oName.SetExt(wxString("seq"));
        fullpath=oName.GetFullPath();
        AppendConvertStatus (wxString("Writing Lynx Conductor sequence\n"));
        WriteConductorFile(fullpath);
        AppendConvertStatus (wxString("Finished writing new file: ")+fullpath+wxString("\n"));
    }
    else if (Out3 == "Vix")
    {
        oName.SetExt(wxString("vix"));
        fullpath=oName.GetFullPath();
        AppendConvertStatus (wxString("Writing Vixen sequence\n"));
        if (WriteVixenFile(fullpath))
        {
            AppendConvertStatus (wxString("Finished writing new file: ")+fullpath+wxString("\n"));
        }
        else
        {
            ConversionError(wxString("Unable to save: ")+fullpath+wxString("\n"));
        }
    }
    else if (Out3 == "Vir")
    {
        oName.SetExt(wxString("vir"));
        fullpath=oName.GetFullPath();
        AppendConvertStatus (wxString("Writing Vixen routine\n"));
        WriteVirFile(fullpath);
    }
    else if (Out3 == "HLS")
    {
        oName.SetExt(wxString("hlsnc"));
        fullpath=oName.GetFullPath();
        AppendConvertStatus (wxString("Writing HLS routine\n"));
        WriteHLSFile(fullpath);
    }
    else if (Out3 == "LOR")
    {
        if (mediaFilename.size() == 0)
        {
            oName.SetExt(wxString("las"));
        }
        else
        {
            oName.SetExt(wxString("lms"));
        }
        fullpath=oName.GetFullPath();
        AppendConvertStatus (wxString("Writing LOR sequence\n"));
        WriteLorFile(fullpath);
        AppendConvertStatus (wxString("Finished writing LOR file: ")+fullpath+wxString("\n"));
    }
    else if (Out3 == "Lcb")
    {

        oName.SetExt(wxString("lcb"));

        fullpath=oName.GetFullPath();
        AppendConvertStatus (wxString("Writing LOR clipboard sequence\n"));
        WriteLcbFile(fullpath);
        AppendConvertStatus (wxString("Finished writing LOR lcb file: ")+fullpath+wxString("\n"));
    }
#endif
    else
    {
        AppendConvertStatus (wxString("Nothing to write - invalid output format\n"));
    }
    AppendConvertStatus("", true);
}

#ifndef FPP
void FRAMECLASS OnButtonChooseFileClick(wxCommandEvent& event)
{
    wxArrayString ShortNames;
    wxString AllNames;
    if (FileDialogConvert->ShowModal() == wxID_OK)
    {
        FileDialogConvert->GetPaths(FileNames);
        FileDialogConvert->GetFilenames(ShortNames);
        for (size_t i=0; i < ShortNames.GetCount(); i++)
        {
            AllNames.Append(ShortNames[i]);
            AllNames.Append("\n");
        }
        TextCtrlFilename->ChangeValue( AllNames );
        wxString ConvertDir = FileDialogConvert->GetDirectory();
        wxConfigBase* config = wxConfigBase::Get();
        config->Write(wxString("ConvertDir"), ConvertDir);
        //delete config;
    }
}

void FRAMECLASS OnButtonStartConversionClick(wxCommandEvent& event)
{
    ButtonStartConversion->Enable(false);
    wxString OutputFormat = ChoiceOutputFormat->GetStringSelection();
    TextCtrlConversionStatus->Clear();

    // check inputs
    if (FileNames.size() == 0)
    {
        wxMessageBox(wxString("Please select one or more sequence files"), wxString("Error"));
    }
    else if (OutputFormat.IsEmpty())
    {
        wxMessageBox(wxString("Please select an output format"), wxString("Error"));
    }
    else
    {
        for (size_t i=0; i < FileNames.GetCount(); i++)
        {
            DoConversion(FileNames[i], OutputFormat);
        }
        AppendConvertStatus (wxString("Finished converting all files\n"));
    }

    ButtonStartConversion->Enable(true);
}
#endif //FPP

