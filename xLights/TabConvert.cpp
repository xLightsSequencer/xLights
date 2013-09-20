/*
 * ********************************************
 * ********************************************
 * Process Convert Panel Events
 * ********************************************
 * ********************************************
 */


void xLightsFrame::ConversionError(const wxString& msg)
{
    wxMessageBox(msg, _("Error"), wxOK | wxICON_EXCLAMATION);
}

/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

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

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

// encodes contents of SeqData
wxString xLightsFrame::base64_encode()
{
    wxString ret;
    int i = 0;
    int j = 0;

    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for(long SeqDataIdx = 0; SeqDataIdx < SeqDataLen; SeqDataIdx++)
    {
        char_array_3[i++] = SeqData[SeqDataIdx];
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }
    return ret;
}


std::string xLightsFrame::base64_decode(const wxString& encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i ==4)
        {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }
    return ret;
}


void xLightsFrame::OnButtonChooseFileClick(wxCommandEvent& event)
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
            AllNames.Append(wxT("\n"));
        }
        TextCtrlFilename->ChangeValue( AllNames );
        wxString ConvertDir = FileDialogConvert->GetDirectory();
        wxConfig* config = new wxConfig(_(XLIGHTS_CONFIG_ID));
        config->Write(_("ConvertDir"), ConvertDir);
        delete config;
    }
}

bool xLightsFrame::WriteVixenFile(const wxString& filename)
{
    wxString ChannelName,TestName;
    int32_t ChannelColor;
    long TotalTime=SeqNumPeriods * Timer1.GetInterval();
    wxXmlNode *node,*chparent,*textnode;
    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("Program") );
    doc.SetRoot( root );

    // add nodes to root in reverse order



    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, wxT("EventValues") );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, base64_encode() );


    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, wxT("Audio") );
    node->AddAttribute( wxT("filename"), mediaFilename);
    node->AddAttribute( wxT("duration"), wxString::Format(wxT("%ld"),TotalTime));
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, wxT("Music") );

    chparent = new wxXmlNode( root, wxXML_ELEMENT_NODE, wxT("Channels") );


    for (int ch=0; ch < SeqNumChannels; ch++ )
    {

        node = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("Channel") );
        node->AddAttribute( wxT("output"), wxString::Format(wxT("%d"),ch));
        node->AddAttribute( wxT("id"), wxT("0"));
        node->AddAttribute( wxT("enabled"), wxT("True"));
        chparent->AddChild( node );
        if (ch < CheckListBoxTestChannels->GetCount())
        {
            TestName=CheckListBoxTestChannels->GetString(ch);
        }
        else
        {
            TestName=wxString::Format(wxT("Ch: %d"),ch);
        }
        if (ch < ChannelNames.size() && !ChannelNames[ch].IsEmpty())
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
        node->AddAttribute( wxT("color"), wxString::Format(wxT("%d"),ChannelColor));
        textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, ChannelName );
    }

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, wxT("AudioDevice") );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, wxT("-1") );

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, wxT("MaximumLevel") );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, wxT("255") );

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, wxT("MinimumLevel") );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, wxT("0") );

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, wxT("EventPeriodInMilliseconds") );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, wxT("50") );

    node = new wxXmlNode( root, wxXML_ELEMENT_NODE, wxT("Time") );
    textnode = new wxXmlNode( node, wxXML_TEXT_NODE, wxEmptyString, wxString::Format(wxT("%ld"),TotalTime) );


#if defined(__WXMSW__)
    Sleep(1000);
#else
    sleep(1);
#endif


    return doc.Save( filename );
}


void xLightsFrame::WriteVirFile(const wxString& filename)
{
    wxString buff;

    int ch,p;
    int seqidx=0;
    wxFile f;
    if (!f.Create(filename,true))
    {
        ConversionError(_("Unable to create file: ")+filename);
        return;
    }

    for (ch=0; ch < SeqNumChannels; ch++ )
    {

        buff=wxT("");
        for (p=0; p < SeqNumPeriods; p++, seqidx++)
        {
            buff += wxString::Format(wxT("%d "),SeqData[seqidx]);
        }
        buff += wxString::Format(wxT("\n"));
        f.Write(buff);
    }
    f.Close();

}

void xLightsFrame::WriteHLSFile(const wxString& filename)
{
    wxString ChannelName,TestName,buff;
    int ch,p;
    unsigned long rgb;
    int seqidx=0;

    wxFile f;
    if (!f.Create(filename,true))
    {
        ConversionError(_("Unable to create file: ")+filename);
        return;
    }

    for (ch=0; ch+2 < SeqNumChannels; ch+=3 ) // since we want to combine 3 channels into one 24 bit rgb value, we jump by 3
    {

        buff=wxT("");

        for (p=0; p < SeqNumPeriods; p++, seqidx++)
        {
            //    rgb = (SeqData[seqidx]& 0xff) << 16 | (SeqData[seqidx+SeqNumPeriods]& 0xff) << 8 | (SeqData[seqidx+(2*SeqNumPeriods)]& 0xff); // we want a 24bit value for HLS
            //  buff += wxString::Format(wxT("%d (%d:%d %d %d)"),rgb,seqidx,SeqData[seqidx],SeqData[seqidx+SeqNumPeriods],SeqData[seqidx+2*SeqNumPeriods]);
            rgb = (SeqData[(ch*SeqNumPeriods)+p]& 0xff) << 16 | (SeqData[((ch+1)*SeqNumPeriods)+p]& 0xff) << 8 | (SeqData[((ch+2)*SeqNumPeriods)+p]& 0xff); // we want a 24bit value for HLS
            if(p<SeqNumPeriods-1)
                buff += wxString::Format(wxT("%d "),rgb);
            else
                buff += wxString::Format(wxT("%d"),rgb);
        }
        buff += wxString::Format(wxT("\n"));
        f.Write(buff);
    }
    f.Close();

}


void xLightsFrame::WriteFalconPiFile(const wxString& filename)
{
    wxUint8 vMinor = 0;
    wxUint8 vMajor = 1;
    wxUint32 dataOffset = 28;
    wxUint16 fixedHeaderLength = 28;
    wxUint32 stepSize = SeqNumChannels + (SeqNumChannels%4);
    // Fixed 50 Milliseconds
    wxUint16 stepTime = 50;
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
    wxUint8 buf[stepSize];

    size_t ch;
    if (!f.Create(filename,true))
    {
        ConversionError(_("Unable to create file: ")+filename);
        return;
    }

    // Header Information
    // Format Identifier
    buf[0] = 'P';
    buf[1] = 'S';
    buf[2] = 'E';
    buf[3] = 'Q';
    // Data offset
    buf[4] = (wxUint8)(dataOffset%256);
    buf[5] = (wxUint8)(dataOffset/256);
    // Data offset
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
    buf[14] = (wxUint8)(SeqNumPeriods & 0xFF);
    buf[15] = (wxUint8)((SeqNumPeriods >> 8) & 0xFF);
    buf[16] = (wxUint8)((SeqNumPeriods >> 16) & 0xFF);
    buf[17] = (wxUint8)((SeqNumPeriods >> 24) & 0xFF);
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
    buf[26] =0;
    buf[27] = 0;
    f.Write(buf,fixedHeaderLength);

    for (long period=0; period < SeqNumPeriods; period++)
    {
        //if (period % 500 == 499) TextCtrlConversionStatus->AppendText(wxString::Format(wxT("Writing time period %ld\n"),period+1));
        wxYield();
        for(ch=0;ch<stepSize;ch++)
        {
          buf[ch] = ch < SeqNumChannels ? SeqData[(ch *SeqNumPeriods) + period] : 0;
        }
        f.Write(buf,stepSize);
    }
    f.Close();
}

void xLightsFrame::WriteXLightsFile(const wxString& filename)
{
    wxFile f;
    char hdr[512];
    memset(hdr,0,512);
    if (!f.Create(filename,true))
    {
        ConversionError(_("Unable to create file: ")+filename);
        return;
    }
    if (mediaFilename.size() > 470)
    {
        ConversionError(_("Media file name is too long"));
        return;
    }
    int xseq_format_version = 1;

    sprintf(hdr,"xLights %2d %8ld %8ld",xseq_format_version,SeqNumChannels,SeqNumPeriods);
    strncpy(&hdr[32],mediaFilename.c_str(),470);
    f.Write(hdr,512);
    f.Write((const char *)&SeqData.front(),SeqDataLen);
    f.Close();

}

void xLightsFrame::WriteLSPFile(const wxString& filename)
{
    wxString ChannelName,TestName;
    int ch,p,csec;
    int seqidx=0;
    int pos,bst,ben,byte;
    unsigned long rgb;
    float seconds;
    wxFile f;
    if (!f.Create(filename,true))
    {
        ConversionError(_("Unable to create file: ")+filename);
        return;
    }
    int interval=Timer1.GetInterval() / 10;  // in centiseconds



    f.Write(wxT("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"));

    f.Write(wxT("<ArrayOfPattern xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n"));
    f.Write(wxT("\t<Pattern>\n"));
    f.Write(wxT("\t<GroupName>Nutcracker-1</GroupName>\n"));
    f.Write(wxT("\t<Name>" + filename + "</Name>\n"));
    f.Write(wxT("\t<Image>\n"));
    f.Write(wxT("\t\t<Width>999</Width>\n"));
    f.Write(wxT("\t\t<Height>200</Height>\n"));
    f.Write(wxT("\t</Image>\n"));
    f.Write(wxT("\t<Tracks>\n"));



    for (ch=0; ch < SeqNumChannels; ch++ )
    {

        f.Write(wxT("\t<Track>\n"));
        f.Write(wxT("\t\t<TrackGuid>60cc0c76-f458-4e67-abb4-5d56a9c1d97c</TrackGuid>\n"));
        f.Write(wxT("\t\t<IsHidden>false</IsHidden>\n"));
        f.Write(wxT("\t\t<IsPrimaryTrack>false</IsPrimaryTrack>\n"));
        f.Write(wxT("\t\t<TrackColorName>Gainsboro</TrackColorName>\n"));
        f.Write(wxT("\t\t<TrackColorARGB>-2302756</TrackColorARGB>\n"));
        f.Write(wxT("\t\t<TrackID>0</TrackID>\n"));
        f.Write(wxT("\t\t<TrackType>0</TrackType>\n"));
        //    f.Write(wxT("\t\t<WiiMapping inv=\"0\" ibn=\"\" inbn=\"\" ani=\"0\" ain=\"\" hty=\"-1\" fed=\"0"\ wind=\"-1\" wibt=\"0\" cint=\"False\" ceff=\"False\" hefsd=\"True\" lef=\"3\" lefl=\"1\" intb=\"0\" efd=\"0\" />\n"));
        f.Write(wxT("\t\t<Name />\n"));
        f.Write(wxT("\t\t<Intervals>\n"));
        for (p=0,csec=0; p < SeqNumPeriods; p++, csec+=interval, seqidx++)
        {
            seconds = (p*50)/1000.0;
            pos = seconds * 88200;
            byte = SeqData[seqidx];
            rgb = (SeqData[(ch*SeqNumPeriods)+p]& 0xff) << 16 | (SeqData[((ch+1)*SeqNumPeriods)+p]& 0xff) << 8 | (SeqData[((ch+2)*SeqNumPeriods)+p]& 0xff); // we want a 24bit value for HLS

            if(rgb>0)
            {
                bst=rgb;
                ben=rgb;
                // 4410 = 1/20th of a second. 88200/20
                f.Write(wxString::Format(wxT("\t\t\t<TimeInterval eff=\"3\" dat=\"&lt;?xml version=&quot;1.0&quot; encoding=&quot;utf-16&quot;?&gt;&#xD;&#xA;&lt;ec&gt;&#xD;&#xA;  &lt;in&gt;100&lt;/in&gt;&#xD;&#xA;  &lt;out&gt;100&lt;/out&gt;&#xD;&#xA;&lt;/ec&gt;\" gui=\"{DA98BD5D-9C00-40fe-A11C-AD3242573443}\" in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\" bst=\"%ld\" ben=\"%ld\" />\n"),pos,bst,ben));
                bst=ben=0;
                f.Write(wxString::Format(wxT("\t\t\t<TimeInterval eff=\"3\" dat=\"&lt;?xml version=&quot;1.0&quot; encoding=&quot;utf-16&quot;?&gt;&#xD;&#xA;&lt;ec&gt;&#xD;&#xA;  &lt;in&gt;100&lt;/in&gt;&#xD;&#xA;  &lt;out&gt;100&lt;/out&gt;&#xD;&#xA;&lt;/ec&gt;\" gui=\"{DA98BD5D-9C00-40fe-A11C-AD3242573443}\" in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\" bst=\"%ld\" ben=\"%ld\" />\n"),pos+4410,bst,ben));
            }
            else
            {


                //  f.Write(wxString::Format(wxT("\t\t\t <TimeInterval eff=\"7\" dat=\"\" gui=\"\" in=\"100\" out=\"100\" pos=\"%d\" sin=\"-1\" att=\"0\" />\n"),pos));
            }
        }
        f.Write(wxT("</Intervals>\n"));
        f.Write(wxT("</Track>\n"));
    }


    f.Write(wxT("\t\t</Tracks>\n"));
    f.Write(wxT("\t</Pattern>\n"));
    f.Write(wxT("</ArrayOfPattern>\n"));
    f.Close();

}


void xLightsFrame::WriteLorFile(const wxString& filename)
{
    wxString ChannelName,TestName;
    int32_t ChannelColor;
    int ch,p,csec,StartCSec, ii;
    int seqidx=0;
    int intensity,LastIntensity;
    wxFile f;
    int* savedIndexes;
    int savedIndexCount = 0;
    savedIndexes = (int *)calloc(SeqNumChannels, sizeof(int));

    int index = 0;
    int rgbChanIndexes[3] = {0,0,0};
    int curRgbChanCount = 0;

    if (!f.Create(filename,true))
    {
        ConversionError(_("Unable to create file: ")+filename);
        return;
    }
    int interval=Timer1.GetInterval() / 10;  // in centiseconds
    long centiseconds=SeqNumPeriods * interval;

    f.Write(wxT("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"));
    f.Write(wxT("<sequence saveFileVersion=\"3\""));
    if (!mediaFilename.IsEmpty())
    {
        f.Write(wxT(" musicFilename=\"")+mediaFilename+wxT("\""));
    }
    f.Write(wxT(">\n"));
    f.Write(wxT("\t<channels>\n"));
    for (ch=0; ch < SeqNumChannels; ch++ )
    {

        if (ch < CheckListBoxTestChannels->GetCount())
        {
            TestName=CheckListBoxTestChannels->GetString(ch);
        }
        else
        {
            TestName=wxString::Format(wxT("Ch: %d"),ch);
        }
        if (ch < ChannelNames.size() && !ChannelNames[ch].IsEmpty())
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
        f.Write(wxT("\t\t<channel name=\"")+ChannelName+wxString::Format(wxT("\" color=\"%d\" centiseconds=\"%ld\" savedIndex=\"%d\">\n"),ChannelColor,centiseconds,index));
        // write intensity values for this channel
        LastIntensity=0;
        for (p=0,csec=0; p < SeqNumPeriods; p++, csec+=interval, seqidx++)
        {
            intensity=SeqData[seqidx] * 100 / 255;
            if (intensity != LastIntensity)
            {
                if (LastIntensity != 0)
                {
                    f.Write(wxString::Format(wxT("\t\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n"),StartCSec,csec,LastIntensity));
                }
                StartCSec=csec;
            }
            LastIntensity=intensity;
        }
        if (LastIntensity != 0)
        {
            f.Write(wxString::Format(wxT("\t\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n"),StartCSec,csec,LastIntensity));
        }
        f.Write(wxT("\t\t</channel>\n"));
        if ( ch < CheckListBoxTestChannels->GetCount() &&
             (TestName.Last() == 'R' || TestName.Last() == 'G' || TestName.Last() == 'B'))
        {
            rgbChanIndexes[curRgbChanCount++]= index;
            if (curRgbChanCount == 3)
            {   index++;
                f.Write(wxT("\t\t<rgbChannel name=\"")+ChannelName.Left(ChannelName.size()-1)+
                        wxString::Format(wxT("(RGB)\" totalCentiseconds=\"%d\" savedIndex=\"%d\">\n"), centiseconds, index));
                savedIndexes[savedIndexCount++] = index;
                f.Write(wxT("\t\t\t<channels>\n"));
                for ( ii =0; ii <3 ; ii++)
                {
                    f.Write(wxString::Format(wxT("\t\t\t\t<channel savedIndex=\"%d\"/>\n"),rgbChanIndexes[ii]));
                }
                f.Write(wxT("\t\t\t</channels>\n"));
                f.Write(wxT("\t\t</rgbChannel>\n"));
                curRgbChanCount = 0;
            }
        }
        else
        {
            savedIndexes[savedIndexCount++] = index;
        }
        index++;
    }
    f.Write(wxT("\t</channels>\n"));
    f.Write(wxT("\t<tracks>\n"));
    f.Write(wxString::Format(wxT("\t\t<track totalCentiseconds=\"%ld\">\n"),centiseconds));
    f.Write(wxT("\t\t\t<channels>\n"));
    for (ii=0; ii < savedIndexCount; ii++ )
    {
        f.Write(wxString::Format(wxT("\t\t\t\t<channel savedIndex=\"%d\"/>\n"),savedIndexes[ii]));
    }
    f.Write(wxT("\t\t\t</channels>\n"));
    f.Write(wxT("\t\t\t<timings>\n"));
    for (p=0,csec=0; p < SeqNumPeriods; p++, csec+=interval)
    {
        f.Write(wxString::Format(wxT("\t\t\t\t<timing centisecond=\"%d\"/>\n"),csec));
    }
    f.Write(wxT("\t\t\t</timings>\n"));
    f.Write(wxT("\t\t</track>\n"));
    f.Write(wxT("\t</tracks>\n"));
    f.Write(wxT("</sequence>\n"));
    f.Close();
    free(savedIndexes);

}

void xLightsFrame::WriteLcbFile(const wxString& filename)
{
    wxString ChannelName,TestName;
    int ch,p,csec,StartCSec;
    int seqidx=0;
    int intensity,LastIntensity;
    wxFile f;
    if (!f.Create(filename,true))
    {
        ConversionError(_("Unable to create file: ")+filename);
        return;
    }


    wxString m_Path, m_Name, m_Ext;
    wxFileName::SplitPath(filename, &m_Path, &m_Name, &m_Ext);
    //  printf("'%s' is split as '%s', '%s', '%s'\n", m_FileName, m_Path,
    //  m_Name, m_Ext);

    int interval=Timer1.GetInterval() / 10;  // in centiseconds
    f.Write(wxT("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"));
    f.Write(wxT("<channelsClipboard version=\"1\" name=\"" + m_Name + "\">\n"));

//  <channels>
//  <channel>
//  <effect type="intensity" startCentisecond="0" endCentisecond="10" intensity="83" />
    f.Write(wxT("<cellDemarcations>\n"));
    for (p=0,csec=0; p < SeqNumPeriods; p++, csec+=interval)
    {
        f.Write(wxString::Format(wxT("\t<cellDemarcation centisecond=\"%d\"/>\n"),csec));
    }
    f.Write(wxT("</cellDemarcations>\n"));
    //
    // LOR is BGR with high bits=0
    // Vix is RGB with high bits=1
    f.Write(wxT("<channels>\n"));
    for (ch=0; ch < SeqNumChannels; ch++ )
    {
        f.Write(wxT("\t<channel>\n"));
        LastIntensity=0;
        for (p=0,csec=0; p < SeqNumPeriods; p++, csec+=interval, seqidx++)
        {
            intensity=SeqData[seqidx] * 100 / 255;
            if (intensity != LastIntensity)
            {
                if (LastIntensity != 0)
                {
                    f.Write(wxString::Format(wxT("\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n"),
                                             StartCSec,csec,LastIntensity));
                }
                StartCSec=csec;
            }
            LastIntensity=intensity;
        }
        if (LastIntensity != 0)
        {
            f.Write(wxString::Format(wxT("\t\t<effect type=\"intensity\" startCentisecond=\"%d\" endCentisecond=\"%d\" intensity=\"%d\"/>\n"),
                                     StartCSec,csec,LastIntensity));
        }
        f.Write(wxT("\t</channel>\n"));
    }
    f.Write(wxT("</channels>\n"));
    f.Write(wxT("</channelsClipboard>\n"));
    f.Close();

}
void xLightsFrame::WriteConductorFile(const wxString& filename)
{
    wxFile f;
    wxUint8 buf[16384];
    size_t ch,i,j;
    if (!f.Create(filename,true))
    {
        ConversionError(_("Unable to create file: ")+filename);
        return;
    }
    for (long period=0; period < SeqNumPeriods; period++)
    {
        //if (period % 500 == 499) TextCtrlConversionStatus->AppendText(wxString::Format(wxT("Writing time period %ld\n"),period+1));
        wxYield();
        for (i=0; i < 4096; i++)
        {
            for (j=0; j < 4; j++)
            {
                ch=j * 4096 + i;
                buf[i*4+j] = ch < SeqNumChannels ? SeqData[ch * SeqNumPeriods + period] : 0;
            }
        }
        f.Write(buf,16384);
    }

    // pad the end of the file with 512 bytes of 0's
    memset(buf,0,512);
    f.Write(buf,512);
    f.Close();
}

// return true on success
bool xLightsFrame::LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels)
{
    wxString tag,tempstr;
    long OutputChannel;
    wxFileName fn;
    fn.AssignDir(CurrentDir);
    fn.SetFullName(ProfileName + wxT(".pro"));
    if (!fn.FileExists())
    {
        ConversionError(_("Unable to find Vixen profile: ")+ProfileName+_("\n\nMake sure a copy is in your xLights directory"));
        return false;
    }
    wxXmlDocument doc( fn.GetFullPath() );
    if (doc.IsOk())
    {
        VixChannels.Clear();
        wxXmlNode* root=doc.GetRoot();
        for( wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
        {
            tag = e->GetName();
            if (tag == _("ChannelObjects"))
            {
                for( wxXmlNode* p=e->GetChildren(); p!=NULL; p=p->GetNext() )
                {
                    if (p->GetName() == _("Channel"))
                    {
                        if (p->HasAttribute(wxT("output")))
                        {
                            tempstr=p->GetAttribute(wxT("output"), wxT("0"));
                            tempstr.ToLong(&OutputChannel);
                            VixChannels.Add(OutputChannel);
                        }
                    }
                }
            }
        }
        return true;
    }
    else
    {
        ConversionError(_("Unable to load Vixen profile: ")+ProfileName);
    }
    return false;
}

void xLightsFrame::SetMediaFilename(const wxString& filename)
{
#ifndef NDEBUG
    TextCtrlLog->AppendText(wxT("Setting media file to: ")+filename+wxT("\n"));
#endif
    mediaFilename=filename;
    if (mediaFilename.IsEmpty()) return;
    wxPathFormat PathFmt = mediaFilename.Contains(_("\\")) ? wxPATH_DOS : wxPATH_NATIVE;
    wxFileName fn1(mediaFilename, PathFmt);
    if (!fn1.FileExists())
    {
        wxFileName fn2(CurrentDir,fn1.GetFullName());
        mediaFilename=fn2.GetFullPath();
    }
}

void xLightsFrame::ReadConductorFile(const wxString& FileName)
{
    wxFile f;
    int i,j,ch;
    char row[16384];
    int period=0;
    ConversionInit();
    wxFileDialog mediaDialog(this,_("Select associated media file, or cancel if this is an animation"));
    if (mediaDialog.ShowModal() == wxID_OK)
    {
        SetMediaFilename(mediaDialog.GetPath());
    }
    if (!f.Open(FileName.c_str()))
    {
        PlayerError(_("Unable to load sequence:\n")+FileName);
        return;
    }
    SeqNumPeriods=f.Length()/16384;
    SeqDataLen=SeqNumPeriods * SeqNumChannels;
    SeqData.resize(SeqDataLen);
    while (f.Read(row,16384) == 16384)
    {
        wxYield();
        for (i=0; i < 4096; i++)
        {
            for (j=0; j < 4; j++)
            {
                ch=j * 4096 + i;
                if (ch < SeqNumChannels)
                {
                    SeqData[ch * SeqNumPeriods + period] = row[i*4+j];
                }
            }
        }
        period++;
    }
    f.Close();
}

void xLightsFrame::ReadXlightsFile(const wxString& FileName)
{
    wxFile f;
    char hdr[512],filetype[10];
    int fileversion,numch,numper,scancnt;
    size_t readcnt;

    ConversionInit();
    if (!f.Open(FileName.c_str()))
    {
        PlayerError(_("Unable to load sequence:\n")+FileName);
        return;
    }
    f.Read(hdr,512);
    scancnt=sscanf(hdr,"%8s %2d %8d %8d",filetype,&fileversion,&numch,&numper);
    if (scancnt != 4 || strncmp(filetype,"xLights",7) != 0 || numch <= 0 || numper <= 0)
    {
        PlayerError(_("Invalid file header:\n")+FileName);
    }
    else
    {
        SeqNumPeriods=numper;
        SeqNumChannels=numch;
        SeqDataLen=SeqNumPeriods * SeqNumChannels;
        wxString filename=wxString::FromAscii(hdr+32);
        SetMediaFilename(filename);
        SeqData.resize(SeqDataLen);
        readcnt = f.Read((char *)&SeqData.front(),SeqDataLen);
        if (readcnt < SeqDataLen)
        {
            PlayerError(_("Unable to read all event data from:\n")+FileName);
        }
#ifndef NDEBUG
        TextCtrlLog->AppendText(wxString::Format(_("ReadXlightsFile SeqNumPeriods=%ld SeqNumChannels=%ld\n"),SeqNumPeriods,SeqNumChannels));
#endif
    }
    f.Close();
}

void xLightsFrame::ConversionInit()
{
    long TotChannels=NetInfo.GetTotChannels();
    mediaFilename.clear();
    ChannelNames.Clear();
    ChannelNames.Add(wxEmptyString, TotChannels);
    ChannelColors.Clear();
    ChannelColors.Add(0, TotChannels);
    SeqData.clear();
    SeqNumChannels=TotChannels;
    SeqNumPeriods=0;
}

void xLightsFrame::ReadVixFile(const char* filename)
{
    wxString NodeName,NodeValue,msg;
    std::string VixSeqData;
    wxArrayInt VixChannels;
    long cnt = 0;
    wxArrayString context;
    long VixEventPeriod=-1;
    long MaxIntensity = 255;
    int OutputChannel;

    ConversionInit();
    TextCtrlConversionStatus->AppendText(_("Reading Vixen sequence\n"));
    IrrXMLReader* xml = createIrrXMLReader(filename);

    // parse the file until end reached
    while(xml && xml->read())
    {
        switch(xml->getNodeType())
        {
        case EXN_TEXT:
            // in this xml file, the only text which occurs is the messageText
            //messageText = xml->getNodeData();
            if (cnt == 2)
            {
                NodeValue = wxString::FromAscii( xml->getNodeData() );
                if (context[1] == _("MaximumLevel")) NodeValue.ToLong(&MaxIntensity);
                if (context[1] == _("EventPeriodInMilliseconds")) NodeValue.ToLong(&VixEventPeriod);
                if (context[1] == _("EventValues")) VixSeqData=base64_decode(NodeValue);
                if (context[1] == _("Profile")) LoadVixenProfile(NodeValue,VixChannels);
            }
            break;
        case EXN_ELEMENT:
            NodeName = wxString::FromAscii( xml->getNodeName() );
            context.Add(NodeName);
            cnt++;
            //msg=_("Element: ") + NodeName + wxString::Format(_(" (%ld)\n"),cnt);
            //TextCtrlConversionStatus->AppendText(msg);
            if (cnt == 2 && (NodeName == _("Audio") || NodeName == _("Song")))
            {
                mediaFilename = wxString::FromAscii( xml->getAttributeValueSafe("filename") );
            }
            if (cnt > 1 && context[1] == _("Channels") && NodeName == _("Channel"))
            {
                OutputChannel = xml->getAttributeValueAsInt("output");
                VixChannels.Add(OutputChannel);
            }
            if (xml->isEmptyElement())
            {
                context.RemoveAt(cnt-1);
                cnt--;
            }
            break;
        case EXN_ELEMENT_END:
            NodeName = wxString::FromAscii( xml->getNodeName() );
            if (cnt > 0) context.RemoveAt(cnt-1);
            cnt = context.GetCount();
            break;
        default:
            break;
        }
    }
    delete xml;
    long VixDataLen = VixSeqData.size();
    SeqNumChannels = VixChannels.GetCount();
    TextCtrlConversionStatus->AppendText(wxString::Format(_("Max Intensity=%ld\n"),MaxIntensity));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("# of Channels=%ld\n"),SeqNumChannels));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("Vix Event Period=%ld\n"),VixEventPeriod));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("Vix data len=%ld\n"),VixDataLen));
    if (SeqNumChannels == 0) return;
    long VixNumPeriods = VixDataLen / SeqNumChannels;
    TextCtrlConversionStatus->AppendText(wxString::Format(_("Vix # of time periods=%ld\n"),VixNumPeriods));
    TextCtrlConversionStatus->AppendText(_("Media file=")+mediaFilename+_("\n"));
    SeqNumPeriods = VixNumPeriods * VixEventPeriod / XTIMER_INTERVAL;
    SeqDataLen = SeqNumPeriods * SeqNumChannels;
    TextCtrlConversionStatus->AppendText(wxString::Format(_("New # of time periods=%ld\n"),SeqNumPeriods));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("New data len=%ld\n"),SeqDataLen));
    if (SeqDataLen == 0) return;
    SeqData.resize(SeqDataLen);

    // convert to 50ms timing, reorder channels according to output number, scale so that max intensity is 255
    int newper,vixper,intensity;
    size_t ch;
    for (ch=0; ch < SeqNumChannels; ch++)
    {
        OutputChannel = VixChannels[ch];
        for (newper=0; newper < SeqNumPeriods; newper++)
        {
            vixper=newper * VixNumPeriods / SeqNumPeriods;
            intensity=VixSeqData[ch*VixNumPeriods+vixper];
            if (MaxIntensity != 255) intensity=intensity * 255 / MaxIntensity;
            SeqData[OutputChannel*SeqNumPeriods+newper] = intensity;
        }
    }
}

void xLightsFrame::ReadHLSFile(const wxString& filename)
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
    IrrXMLReader* xml = createIrrXMLReader(filename);
    wxArrayInt map;
    

    // pass one, get the metadata
    while(xml && xml->read())
    {
        switch(xml->getNodeType())
        {
            case EXN_TEXT:
                NodeValue = wxString::FromAscii( xml->getNodeData() );
                if (context[cnt - 1] == _("MilliSecPerTimeUnit")) NodeValue.ToLong(&msPerCell);
                if (context[cnt - 1] == _("NumberOfTimeCells")) NodeValue.ToLong(&timeCells);
                if (context[cnt - 1] == _("AudioSourcePcmFile")) mediaFilename = NodeValue;
                if (context[cnt - 1] == _("ChannelsInUniverse")) {
                    NodeValue.ToLong(&channelsInUniverse);
                    channels += channelsInUniverse;
                }
                if (context[cnt - 1] == _("UniverseNumber")) {
                    NodeValue.ToLong(&tmp);
                    universe = tmp;
                }
                break;
            case EXN_ELEMENT:
                NodeName = wxString::FromAscii( xml->getNodeName() );
                context.Add(NodeName);
                cnt++;

                if (xml->isEmptyElement())
                {
                    context.RemoveAt(cnt-1);
                    cnt--;
                }

                break;
            case EXN_ELEMENT_END:
                NodeName = wxString::FromAscii( xml->getNodeName() );
                if (NodeName == _("Universe")) {
                    map.Add(universe);
                    map.Add(channelsInUniverse);
                    for (tmp = map.size() - 2; tmp > 0; tmp -= 2) {
                        if (map[tmp] < map[tmp - 2]) {
                            long t1 = map[tmp];
                            long t2 = map[tmp + 1];
                            map[tmp] = map[tmp - 2];
                            map[tmp + 1] = map[tmp - 1];
                            map[tmp - 2] = t1;
                            map[tmp - 1] = t2;
                        }
                    }
                }
                NodeName = wxString::FromAscii( xml->getNodeName() );
                if (cnt > 0) context.RemoveAt(cnt-1);
                cnt = context.GetCount();
                break;
            default:
                break;
        }
    }
    delete xml;
    
    
    TextCtrlConversionStatus->AppendText(wxString::Format(_("TimeCells = %d\n"), timeCells));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("msPerCell = %d ms\n"), msPerCell));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("Channels = %d\n"), channels));
    SeqNumChannels = channels;
    if (SeqNumChannels == 0) return;
    SeqNumPeriods = timeCells * msPerCell / XTIMER_INTERVAL;
    SeqDataLen = SeqNumPeriods * SeqNumChannels;
    TextCtrlConversionStatus->AppendText(wxString::Format(_("New # of time periods=%ld\n"),SeqNumPeriods));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("New data len=%ld\n"),SeqDataLen));
    if (SeqDataLen == 0) return;
    SeqData.resize(SeqDataLen);

    xml = createIrrXMLReader(filename);
    ChannelNames.resize(channels);
    ChannelColors.resize(channels);
    channels = 0;

    for (tmp = 0; tmp < map.size(); tmp += 2) {
        int i = map[tmp + 1];
        map[tmp + 1] = channels;
        channels += i;
    }
    channels = 0;

    
    //pass2 read the sequence data
    while(xml && xml->read())
    {
        switch(xml->getNodeType())
        {
            case EXN_TEXT:
                if (context[cnt - 1] == _("ChanInfo")) {
                    //channel name and type
                    ChannelName = wxString::FromAscii( xml->getNodeData() );
                }
                if (context[cnt - 1] == _("Block")) {
                    NodeValue = wxString::FromAscii( xml->getNodeData() );

                    int idx = NodeValue.Find("-");
                    Data.Append(NodeValue.SubString(idx + 1, NodeValue.size()));
                }
                if (context[cnt - 1] == _("UniverseNumber")) {
                    NodeValue = wxString::FromAscii( xml->getNodeData() );
                    NodeValue.ToLong(&tmp);
                    universe = tmp;
                    for (tmp = 0; tmp < map.size() ; tmp += 2) {
                        if (universe == map[tmp]) {
                            channels = map[tmp + 1];
                        }
                    }
                }
                break;
            case EXN_ELEMENT:
                NodeName = wxString::FromAscii( xml->getNodeName() );
                context.Add(NodeName);
                cnt++;

                if (xml->isEmptyElement())
                {
                    context.RemoveAt(cnt-1);
                    cnt--;
                }

                break;
            case EXN_ELEMENT_END:
                NodeName = wxString::FromAscii( xml->getNodeName() );
                if (NodeName == _("ChannelData")) {
                    //finished reading this channel, map the data
                    int idx = ChannelName.find(", ");
                    wxString type = ChannelName.SubString(idx + 2, ChannelName.size());
                    wxString origName = ChannelNames[channels];
                    if (type == _("RGB-R")) {
                        ChannelNames[channels] = ChannelName.Left(idx) + _("-R");
                        ChannelColors[channels] = 0x000000FF;
                    } else if (type == _("RGB-G")) {
                        ChannelNames[channels] = ChannelName.Left(idx) + _("-G");
                        ChannelColors[channels] = 0x0000FF00;
                    } else if (type == _("RGB-B")) {
                        ChannelNames[channels] = ChannelName.Left(idx) + _("-B");
                        ChannelColors[channels] = 0x00FF0000;
                    } else {
                        ChannelNames[channels] = ChannelName.Left(idx);
                        ChannelColors[channels] = 0x00FFFFFF;
                    }
                    wxString o2 = NetInfo.GetChannelName(channels);
                    TextCtrlConversionStatus->AppendText(wxString::Format(_("Map %s -> %s (%s)\n"),
                                                                        ChannelNames[channels],origName,o2));
                    for (long newper = 0; newper < SeqNumPeriods; newper++) {
                        int hlsper = newper * timeCells / SeqNumPeriods;
                        long intensity;
                        Data.SubString(hlsper * 3, hlsper * 3 + 1).ToLong(&intensity, 16);
                        SeqData[channels * SeqNumPeriods + newper] = intensity;
                    }
                    Data.Clear();
                    channels++;
                }
                if (cnt > 0) context.RemoveAt(cnt-1);
                cnt = context.GetCount();
                break;
            default:
                break;
        }
    }
    delete xml;
}

// returns length of first track in centiseconds
int xLightsFrame::GetLorTrack1Length(const char* filename)
{
    int centisec = -1;
    int nodecnt=0;
    wxString NodeName;
    IrrXMLReader* xml = createIrrXMLReader(filename);

    // parse the file until end reached
    while(xml && xml->read() && centisec < 0)
    {
        switch(xml->getNodeType())
        {
        case EXN_TEXT:
            break;
        case EXN_ELEMENT:
            NodeName = wxString::FromAscii( xml->getNodeName() );
            nodecnt++;
            if (nodecnt > 1000)
            {
                nodecnt=0;
                wxYield();
            }
            if (NodeName == _("track"))
            {
                centisec = xml->getAttributeValueAsInt("totalCentiseconds");
            }
            break;
        default:
            break;
        }
    }
    delete xml;
    TextCtrlConversionStatus->AppendText(wxString::Format(_("Track 1 length = %d centiseconds\n"),centisec));
    return centisec;
}

void xLightsFrame::ReadLorFile(const char* filename)
{
    wxString NodeName,msg,EffectType,ChannelName,deviceType;
    wxArrayString context;
    int unit,circuit,startcsec,endcsec,intensity,startIntensity,endIntensity,rampdiff,ChannelColor;
    int i,startper,endper,perdiff,twinklestate,nexttwinkle;
    int twinkleperiod = 400;
    int curchannel = -1;
    int MappedChannelCnt = 0;
    int MaxIntensity = 100;
    int EffectCnt = 0;
    size_t network,chindex;
    long cnt = 0;

    ConversionInit();
    TextCtrlConversionStatus->AppendText(_("Reading LOR sequence\n"));
    int centisec = GetLorTrack1Length(filename);
    if (centisec > 0)
    {
        SeqNumPeriods = centisec * 10 / XTIMER_INTERVAL;
        if (SeqNumPeriods == 0) SeqNumPeriods=1;
        SeqDataLen = SeqNumPeriods * SeqNumChannels;
        SeqData.resize(SeqDataLen,0);
    }
    else
    {
        ConversionError(_("Unable to determine the length of this LOR sequence (looked for length of track 1)"));
        return;
    }
    IrrXMLReader* xml = createIrrXMLReader(filename);

    // parse the file until end reached
    while(xml && xml->read())
    {
        switch(xml->getNodeType())
        {
        case EXN_TEXT:
            break;
        case EXN_ELEMENT:
            NodeName = wxString::FromAscii( xml->getNodeName() );
            context.Add(NodeName);
            cnt++;
            //msg=_("Element: ") + NodeName + wxString::Format(_(" (%ld)\n"),cnt);
            //TextCtrlConversionStatus->AppendText(msg);
            if (NodeName == _("sequence"))
            {
                mediaFilename = wxString::FromAscii( xml->getAttributeValueSafe("musicFilename") );
            }
            if (cnt > 1 && context[1] == _("channels") && NodeName == _("channel") && !xml->isEmptyElement())
            {
                wxYield();
                deviceType = wxString::FromAscii( xml->getAttributeValueSafe("deviceType") );
                network = xml->getAttributeValueAsInt("network");
                unit = xml->getAttributeValueAsInt("unit");
                if (unit < 0) unit+=256;
                circuit = xml->getAttributeValueAsInt("circuit");
                if (deviceType.Left(3) == wxT("DMX"))
                {
                    chindex=circuit-1;
                    network--;
                }
                else
                {
                    chindex=(unit-1)*16+circuit-1;
                }
                ChannelName = wxString::FromAscii( xml->getAttributeValueSafe("name") );
                curchannel = NetInfo.CalcAbsChannel(network,chindex);
                if (curchannel >= 0)
                {
                    //TextCtrlConversionStatus->AppendText(wxString::Format(_("curchannel %d\n"),curchannel));
                    if (!ChannelNames[curchannel].IsEmpty())
                    {
                        TextCtrlConversionStatus->AppendText(_("WARNING: ")+ChannelNames[curchannel]+_(" and ")+ChannelName+_(" map to the same channel\n"));
                    }
                    MappedChannelCnt++;
                    ChannelNames[curchannel] = ChannelName;
                    ChannelColor = xml->getAttributeValueAsInt("color");
                    ChannelColors[curchannel] = ChannelColor;
                }
                else
                {
                    TextCtrlConversionStatus->AppendText(_("WARNING: channel '")+ChannelName+_("' is unmapped\n"));
                }
            }
            if (cnt > 1 && context[1] == _("channels") && NodeName == _("effect") && curchannel >= 0)
            {
                EffectCnt++;
                startcsec = xml->getAttributeValueAsInt("startCentisecond");
                endcsec = xml->getAttributeValueAsInt("endCentisecond");
                intensity = xml->getAttributeValueAsInt("intensity");
                startIntensity = xml->getAttributeValueAsInt("startIntensity");
                endIntensity = xml->getAttributeValueAsInt("endIntensity");
                startper = startcsec * 10 / XTIMER_INTERVAL;
                endper = endcsec * 10 / XTIMER_INTERVAL;
                perdiff=endper - startper;  // # of 50ms ticks
                if (perdiff > 0)
                {
                    intensity=intensity * 255 / MaxIntensity;
                    startIntensity=startIntensity * 255 / MaxIntensity;
                    endIntensity=endIntensity * 255 / MaxIntensity;
                    EffectType = wxString::FromAscii( xml->getAttributeValueSafe("type") );
                    if (EffectType == wxT("intensity"))
                    {
                        if (intensity > 0)
                        {
                            for (i=0; i < perdiff; i++)
                            {
                                SeqData[curchannel*SeqNumPeriods+startper+i] = intensity;
                            }
                        }
                        else if (startIntensity > 0 || endIntensity > 0)
                        {
                            // ramp
                            rampdiff=endIntensity - startIntensity;
                            for (i=0; i < perdiff; i++)
                            {
                                intensity=(int)((double)(i) / perdiff * rampdiff + startIntensity);
                                SeqData[curchannel*SeqNumPeriods+startper+i] = intensity;
                            }
                        }
                    }
                    else if (EffectType == wxT("twinkle"))
                    {
                        if (intensity == 0 && startIntensity == 0 && endIntensity == 0) intensity=MaxIntensity;
                        twinklestate=static_cast<int>(rand01()*2.0) & 0x01;
                        nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / XTIMER_INTERVAL;
                        if (intensity > 0)
                        {
                            for (i=0; i < perdiff; i++)
                            {
                                SeqData[curchannel*SeqNumPeriods+startper+i] = intensity * twinklestate;
                                nexttwinkle--;
                                if (nexttwinkle <= 0)
                                {
                                    twinklestate=1-twinklestate;
                                    nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / XTIMER_INTERVAL;
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
                                SeqData[curchannel*SeqNumPeriods+startper+i] = intensity * twinklestate;
                                nexttwinkle--;
                                if (nexttwinkle <= 0)
                                {
                                    twinklestate=1-twinklestate;
                                    nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / XTIMER_INTERVAL;
                                }
                            }
                        }
                    }
                    else if (EffectType == wxT("shimmer"))
                    {
                        if (intensity == 0 && startIntensity == 0 && endIntensity == 0) intensity=MaxIntensity;
                        if (intensity > 0)
                        {
                            for (i=0; i < perdiff; i++)
                            {
                                twinklestate=(startper + i) & 0x01;
                                SeqData[curchannel*SeqNumPeriods+startper+i] = intensity * twinklestate;
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
                                SeqData[curchannel*SeqNumPeriods+startper+i] = intensity * twinklestate;
                            }
                        }
                    }
                }
            }
            if (xml->isEmptyElement())
            {
                context.RemoveAt(cnt-1);
                cnt--;
            }
            break;
        case EXN_ELEMENT_END:
            NodeName = wxString::FromAscii( xml->getNodeName() );
            if (NodeName == _("channel")) curchannel = -1;
            if (cnt > 0) context.RemoveAt(cnt-1);
            cnt = context.GetCount();
            break;
        default:
            break;
        }
    }
    delete xml;
    TextCtrlConversionStatus->AppendText(wxString::Format(_("# of mapped channels with effects=%d\n"),MappedChannelCnt));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("# of effects=%d\n"),EffectCnt));
    TextCtrlConversionStatus->AppendText(_("Media file=")+mediaFilename+_("\n"));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("New # of time periods=%ld\n"),SeqNumPeriods));
    TextCtrlConversionStatus->AppendText(wxString::Format(_("New data len=%ld\n"),SeqDataLen));
}

void xLightsFrame::ClearLastPeriod()
{
    long LastPer = SeqNumPeriods-1;
    for (size_t ch=0; ch < SeqNumChannels; ch++)
    {
        SeqData[ch*SeqNumPeriods+LastPer] = 0;
    }
}

void xLightsFrame::DoConversion(const wxString& Filename, const wxString& OutputFormat)
{
    wxString fullpath;
    wxString Out3=OutputFormat.Left(3);

    // read sequence file
    TextCtrlConversionStatus->AppendText(_("\nReading: ") + Filename + wxT("\n"));
    wxYield();
    wxFileName oName(Filename);
    wxString ext = oName.GetExt();
    if (ext == _("vix"))
    {
        if (Out3 == wxT("Vix"))
        {
            ConversionError(_("Cannot convert from Vixen to Vixen!"));
            return;
        }
        ReadVixFile(Filename.char_str());
    }
    else if (ext == _(XLIGHTS_SEQUENCE_EXT))
    {
        if (Out3 == wxT("xLi"))
        {
            ConversionError(_("Cannot convert from xLights to xLights!"));
            return;
        }
        ReadXlightsFile(Filename);
    }
    else if (ext == _("seq"))
    {
        if (Out3 == wxT("Lyn"))
        {
            ConversionError(_("Cannot convert from Conductor file to Conductor file!"));
            return;
        }
        ReadConductorFile(Filename);
    }
    else if (ext == _("hlsIdata"))
    {
        ReadHLSFile(Filename);
    }
    else if (ext == _("lms") || ext == _("las"))
    {
        if (Out3 == wxT("LOR"))
        {
            ConversionError(_("Cannot convert from LOR to LOR!"));
            return;
        }
        ReadLorFile(Filename.char_str());
    }
    else
    {
        ConversionError(_("Unknown sequence file extension"));
        return;
    }

    // check for errors
    if (SeqNumChannels == 0)
    {
        TextCtrlConversionStatus->AppendText(_("ERROR: no channels defined\n"));
        return;
    }
    if (SeqDataLen == 0)
    {
        TextCtrlConversionStatus->AppendText(_("ERROR: sequence length is 0\n"));
        return;
    }

    if (CheckBoxOffAtEnd->IsChecked())
    {
        ClearLastPeriod();
    }
    wxYield();

    // write converted file to xLights directory
    oName.SetPath( CurrentDir );

    if (Out3 == wxT("xLi"))
    {
        oName.SetExt(_(XLIGHTS_SEQUENCE_EXT));
        fullpath=oName.GetFullPath();
        TextCtrlConversionStatus->AppendText(_("Writing xLights sequence\n"));
        WriteXLightsFile(fullpath);
        TextCtrlConversionStatus->AppendText(_("Finished writing new file: ")+fullpath+_("\n"));
    }
    else if (Out3 == wxT("Fal"))
    {
        oName.SetExt(_("fseq"));
        fullpath=oName.GetFullPath();
        TextCtrlConversionStatus->AppendText(_("Writing Falcon Pi Player sequence\n"));
        WriteFalconPiFile(fullpath);
        TextCtrlConversionStatus->AppendText(_("Finished writing new file: ")+fullpath+_("\n"));
    }
    else if (Out3 == wxT("Lyn"))
    {
        oName.SetExt(_("seq"));
        fullpath=oName.GetFullPath();
        TextCtrlConversionStatus->AppendText(_("Writing Lynx Conductor sequence\n"));
        WriteConductorFile(fullpath);
        TextCtrlConversionStatus->AppendText(_("Finished writing new file: ")+fullpath+_("\n"));
    }
    else if (Out3 == wxT("Vix"))
    {
        oName.SetExt(_("vix"));
        fullpath=oName.GetFullPath();
        TextCtrlConversionStatus->AppendText(_("Writing Vixen sequence\n"));
        if (WriteVixenFile(fullpath))
        {
            TextCtrlConversionStatus->AppendText(_("Finished writing new file: ")+fullpath+_("\n"));
        }
        else
        {
            ConversionError(_("Unable to save: ")+fullpath+_("\n"));
        }
    }
    else if (Out3 == wxT("Vir"))
    {
        oName.SetExt(_("vir"));
        fullpath=oName.GetFullPath();
        TextCtrlConversionStatus->AppendText(_("Writing Vixen routine\n"));
        WriteVirFile(fullpath);
    }
    else if (Out3 == wxT("HLS"))
    {
        oName.SetExt(_("hlsnc"));
        fullpath=oName.GetFullPath();
        TextCtrlConversionStatus->AppendText(_("Writing HLS routine\n"));
        WriteHLSFile(fullpath);
    }
    else if (Out3 == wxT("LOR"))
    {
        if (mediaFilename.IsEmpty())
        {
            oName.SetExt(_("las"));
        }
        else
        {
            oName.SetExt(_("lms"));
        }
        fullpath=oName.GetFullPath();
        TextCtrlConversionStatus->AppendText(_("Writing LOR sequence\n"));
        WriteLorFile(fullpath);
        TextCtrlConversionStatus->AppendText(_("Finished writing LOR file: ")+fullpath+_("\n"));
    }
    else if (Out3 == wxT("Lcb"))
    {

        oName.SetExt(_("lcb"));

        fullpath=oName.GetFullPath();
        TextCtrlConversionStatus->AppendText(_("Writing LOR clipboard sequence\n"));
        WriteLcbFile(fullpath);
        TextCtrlConversionStatus->AppendText(_("Finished writing LOR lcb file: ")+fullpath+_("\n"));
    }
    else
    {
        TextCtrlConversionStatus->AppendText(_("Nothing to write - invalid output format\n"));
    }
}


void xLightsFrame::OnButtonStartConversionClick(wxCommandEvent& event)
{
    ButtonStartConversion->Enable(false);
    wxString OutputFormat = ChoiceOutputFormat->GetStringSelection();
    TextCtrlConversionStatus->Clear();

    // check user inputs
    if (FileNames.IsEmpty())
    {
        wxMessageBox(_("Please select one or more sequence files"), _("Error"));
    }
    else if (OutputFormat.IsEmpty())
    {
        wxMessageBox(_("Please select an output format"), _("Error"));
    }
    else
    {
        for (size_t i=0; i < FileNames.GetCount(); i++)
        {
            DoConversion(FileNames[i], OutputFormat);
        }
        TextCtrlConversionStatus->AppendText(_("Finished converting all files\n"));
    }

    ButtonStartConversion->Enable(true);
}

