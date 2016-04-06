#include "FileConverter.h"

#include <algorithm>
#include <map>

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"

#ifndef FPP

#include <wx/app.h>
#include <wx/arrstr.h>
#include <wx/file.h>
#include "xLightsMain.h"
#include "ConvertDialog.h"

#define string_format wxString::Format

#endif

static const int MAX_READ_BLOCK_SIZE = 4096 * 1024;

void ConvertParameters::SetStatusText(wxString msg)
{
    if (xLightsFrm != NULL)
    {
        xLightsFrm->SetStatusText(msg);
    }
    if (convertDialog != NULL)
    {
        convertDialog->SetStatusText(msg);
    }
}

void ConvertParameters::ConversionError(wxString msg)
{
    if (convertDialog != NULL)
    {
        convertDialog->ConversionError(msg);
    }
    else if (xLightsFrm != NULL)
    {
        xLightsFrm->ConversionError(msg);
    }
}

void ConvertParameters::PlayerError(wxString msg)
{
    if (convertDialog != NULL)
    {
        convertDialog->PlayerError(msg);
    }
    else if (xLightsFrm != NULL)
    {
        xLightsFrm->PlayerError(msg);
    }
}
ConvertParameters::ConvertParameters( wxString inp_filename_,
                                      SequenceData& seq_data_,
                                      NetInfoClass& NetInfo_,
                                      ReadMode read_mode_,
                                      xLightsFrame* xLightsFrm_,
                                      ConvertDialog* convertDialog_,
                                      wxString* media_filename_,
                                      DataLayer* data_layer_,
                                      wxString out_filename_,
                                      int sequence_interval_,
                                      bool channels_off_at_end_,
                                      bool map_empty_channels_,
                                      bool map_no_network_channels_ )
: inp_filename(inp_filename_),
  out_filename(out_filename_),
  sequence_interval(sequence_interval_),
  seq_data(seq_data_),
  NetInfo(NetInfo_),
  media_filename(media_filename_),
  data_layer(data_layer_),
  channels_off_at_end(channels_off_at_end_),
  map_empty_channels(map_empty_channels_),
  map_no_network_channels(map_no_network_channels_),
  read_mode(read_mode_),
  xLightsFrm(xLightsFrm_),
  convertDialog(convertDialog_)
{
}

FileConverter::FileConverter()
{
    //ctor
}

static double rand01()
{
    return (double)rand()/(double)RAND_MAX;
}

static int rountTo4(int i)  {
    int remainder = i % 4;
    if (remainder == 0) {
        return i;
    }
    return i + 4 - remainder;
}

static wxString FromAscii(const char *val) {
    return wxString::FromAscii(val);
}

static void RemoveAt(wxArrayString &v, int i) {
    v.RemoveAt(i);
}

static wxString Left(const wxString &in, int len) {
    return in.substr(0, len);
}

static wxString Right(const wxString &in, int len) {
    return in.substr(in.size() - len, len);
}

static int getAttributeValueAsInt(SP_XmlStartTagEvent * stagEvent, const char * name)
{
    const char *val = stagEvent -> getAttrValue(name);
    if (!val)
    {
        return 0;
    }
    return atoi(val);
}
static wxString getAttributeValueSafe(SP_XmlStartTagEvent * stagEvent, const char * name)
{
    const char *val = stagEvent -> getAttrValue(name);
    if (!val)
    {
        return "";
    }
    return FromAscii(val);
}

static void ClearLastPeriod( SequenceData& seq_data)
{
    int LastPer = seq_data.NumFrames()-1;
    for (size_t ch=0; ch < seq_data.NumChannels(); ch++)
    {
        seq_data[LastPer][ch] = 0;
    }
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

//returns number of chars at the end that couldn't be decoded
static int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data)
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


void mapLORInfo(const LORInfo &info, std::vector< std::vector<int> > *unitSizes)
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

void FileConverter::ReadLorFile(ConvertParameters& params)
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
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    long TotChannels=params.NetInfo.GetTotChannels();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    params.convertDialog->AppendConvertStatus(string_format("Reading LOR sequence: %s\n", params.inp_filename));
    params.SetStatusText(string_format("Reading LOR sequence: %s\n", params.inp_filename));

    int centisec = -1;
    int nodecnt=0;
    int channelCount = 0;

    SP_XmlPullParser *parser = new SP_XmlPullParser();
    wxFile file(params.inp_filename);
    char *bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);

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
                        params.convertDialog->AppendConvertStatus (string_format(wxString("Channels found so far: %d\n"),channelCount));
                        params.SetStatusText(string_format(wxString("Channels found so far: %d"),channelCount));
                    }

                    deviceType = FromAscii( stagEvent->getAttrValue("deviceType") );
                    network = getAttributeValueAsInt(stagEvent, "network");
                    networkAsString = FromAscii( stagEvent->getAttrValue("network") );
                    unit = getAttributeValueAsInt(stagEvent, "unit");
                    circuit = getAttributeValueAsInt(stagEvent, "circuit");
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");
                    wxString channelName = FromAscii( stagEvent->getAttrValue("name") );
                    rgbChannels[savedIndex] = LORInfo(channelName, deviceType, network, unit, circuit, savedIndex);
                    rgbChannels[savedIndex].empty = !params.map_empty_channels;
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
                        } else if ("" == deviceType && "" == networkAsString && !params.map_no_network_channels) {
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
                    } else if ("" == deviceType && "" == networkAsString && !params.map_no_network_channels) {
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
    params.convertDialog->AppendConvertStatus (string_format(wxString("Track 1 length = %d centiseconds\n"),centisec), false);

    if (centisec > 0)
    {
        int numFrames = centisec * 10 / params.sequence_interval;
        if (numFrames == 0)
        {
            numFrames=1;
        }
        params.seq_data.init(params.NetInfo.GetTotChannels(), numFrames,  params.sequence_interval);
    }
    else
    {
        params.ConversionError(wxString("Unable to determine the length of this LOR sequence (looked for length of track 1)"));
        return;
    }

    for (network = 0; network < lorUnitSizes.size(); network++)
    {
        cnt = 0;
        for (int u = 0; u < lorUnitSizes[network].size(); u++)
        {
            cnt += lorUnitSizes[network][u];
        }
        params.convertDialog->AppendConvertStatus (string_format(wxString("LOR Network %d:  %d channels\n"),network,cnt), false);
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
        params.convertDialog->AppendConvertStatus (string_format(wxString("DMX Network %d:  %d channels\n"),network,cnt), false);
    }
    params.convertDialog->AppendConvertStatus (string_format(wxString("Total channels = %d\n"),channelCount));

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
                        params.convertDialog->AppendConvertStatus (wxString("WARNING: ")+ChannelNames[curchannel] + " is empty\n");
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
                    if( params.media_filename )
                        *params.media_filename = getAttributeValueSafe(stagEvent, "musicFilename");
                }
                if (cnt == 3 && context[1] == wxString("channels") && NodeName == wxString("channel"))
                {
                    channelCount++;
                    if ((channelCount % 1000) == 0)
                    {
                        params.convertDialog->AppendConvertStatus (string_format(wxString("Channels converted so far: %d\n"),channelCount));
                        params.SetStatusText(string_format(wxString("Channels converted so far: %d"),channelCount));
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
                        curchannel = params.NetInfo.CalcAbsChannel(network,chindex);
                    }
                    else if (Left(deviceType, 3) == "LOR")
                    {
                        chindex = 0;
                        for (int z = 0; z < (unit - 1); z++)
                        {
                            chindex += lorUnitSizes[network][z];
                        }
                        chindex += circuit-1;
                        curchannel = params.NetInfo.CalcAbsChannel(network,chindex);
                    } else if ("" == deviceType && "" == networkAsString && !params.map_no_network_channels) {
                        curchannel = -1;
                    } else {
                        chindex++;
                        if (chindex < params.NetInfo.GetTotChannels())
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
                        //params.convertDialog->AppendConvertStatus (string_format(wxString("curchannel %d\n"),curchannel));
                        if (ChannelNames[curchannel].size() != 0)
                        {
                           params.convertDialog->AppendConvertStatus (string_format(wxString("WARNING: ")+ChannelNames[curchannel]+wxString(" and ")
                                                                                       +ChannelName+wxString(" map to the same channel %d\n"), curchannel));
                        }
                        MappedChannelCnt++;
                        ChannelNames[curchannel] = ChannelName;
                        ChannelColor = getAttributeValueAsInt(stagEvent, "color");
                        ChannelColors[curchannel] = ChannelColor;
                    }
                    else
                    {
                        params.convertDialog->AppendConvertStatus (wxString("WARNING: channel '")+ChannelName+wxString("' is unmapped\n"));
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
                    startper = startcsec * 10 / params.sequence_interval;
                    endper = endcsec * 10 / params.sequence_interval;
                    perdiff=endper - startper;  // # of ticks

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
                                    params.seq_data[startper+i][curchannel] = intensity;
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff=endIntensity - startIntensity;
                                for (i=0; i < perdiff; i++)
                                {
                                    intensity=(int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    params.seq_data[startper+i][curchannel] = intensity;
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
                            nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / params.sequence_interval;
                            if (intensity > 0)
                            {
                                for (i=0; i < perdiff; i++)
                                {
                                    params.seq_data[startper + i][curchannel] = intensity * twinklestate;
                                    nexttwinkle--;
                                    if (nexttwinkle <= 0)
                                    {
                                        twinklestate=1-twinklestate;
                                        nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / params.sequence_interval;
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
                                    params.seq_data[startper + i][curchannel] = intensity * twinklestate;
                                    nexttwinkle--;
                                    if (nexttwinkle <= 0)
                                    {
                                        twinklestate=1-twinklestate;
                                        nexttwinkle=static_cast<int>(rand01()*twinkleperiod+100) / params.sequence_interval;
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
                                    params.seq_data[startper+i][curchannel] = intensity * twinklestate;
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
                                    params.seq_data[startper+i][curchannel] = intensity * twinklestate;
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

    if( params.data_layer != nullptr )
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(MappedChannelCnt);
    }

    if( params.channels_off_at_end )
    {
        ClearLastPeriod(params.seq_data);
    }

    params.convertDialog->AppendConvertStatus (string_format(wxString("# of mapped channels with effects=%d\n"),MappedChannelCnt), false);
    params.convertDialog->AppendConvertStatus (string_format(wxString("# of effects=%d\n"),EffectCnt), false);
    if( params.media_filename )
        params.convertDialog->AppendConvertStatus (wxString("Media file=")+*params.media_filename+wxString("\n"), false);
    params.convertDialog->AppendConvertStatus (string_format(wxString("New # of time periods=%ld\n"),params.seq_data.NumFrames()), false);
    params.SetStatusText(wxString("LOR sequence converted successfully"));

    wxYield();
}

void FileConverter::ReadXlightsFile(ConvertParameters& params)
{
    wxFile f;
    char hdr[512],filetype[10];
    int fileversion,numch,numper,scancnt;
    size_t readcnt;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    long TotChannels=params.NetInfo.GetTotChannels();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    wxFile file(params.inp_filename);

    if (!f.Open(params.inp_filename.c_str()))
    {
        params.PlayerError(wxString("Unable to load sequence:\n")+params.inp_filename);
        return;
    }

    f.Read(hdr,512);
    scancnt=sscanf(hdr,"%8s %2d %8d %8d",filetype,&fileversion,&numch,&numper);
    if (scancnt != 4 || strncmp(filetype,"xLights",7) != 0 || numch <= 0 || numper <= 0)
    {
        params.PlayerError(wxString("Invalid file header:\n")+params.inp_filename);
    }
    else
    {
        params.seq_data.init(numch, numper, 50);
        char * buf = new char[numper];
        wxString filename=FromAscii(hdr+32);

        if( params.media_filename ) {
            *params.media_filename = filename;
        }

        if( params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN ) {
            params.xLightsFrm->SetMediaFilename(filename);
        }

        for (int x = 0; x < numch; x++) {
            readcnt = f.Read(buf, numper);
            if (readcnt < numper)
            {
                params.PlayerError(wxString("Unable to read all event data from:\n")+params.inp_filename);
            }
            for (int p = 0; p < numper; p++) {
                params.seq_data[p][x] = buf[p];
            }
        }
        delete [] buf;
#ifndef NDEBUG
        params.convertDialog->AppendConvertStatus (string_format(wxString("ReadXlightsFile SeqData.NumFrames()=%d SeqData.NumChannels()=%d\n"),params.seq_data.NumFrames(),params.seq_data.NumChannels()));
#endif
    }
    f.Close();

    if( params.data_layer != nullptr )
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(params.seq_data.NumChannels());
    }

    if( params.channels_off_at_end )
    {
        ClearLastPeriod(params.seq_data);
    }

    wxYield();

}

void FileConverter::ReadHLSFile(ConvertParameters& params)
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
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    SP_XmlPullParser *parser = new SP_XmlPullParser();
    wxFile file(params.inp_filename);
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
                        *params.media_filename = NodeValue;
                        if (Right(*params.media_filename, 4) == ".PCM")
                        {
                            //nothing can deal with PCM files, we'll assume this came from an mp3
                            *params.media_filename = Left(*params.media_filename, (*params.media_filename).size() - 4);
                            *params.media_filename += ".mp3";
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
        int orig = params.NetInfo.GetNumChannels(tmp / 2);
        if (i < orig) {
            params.convertDialog->AppendConvertStatus (string_format(wxString("Found Universe: %ld   Channels in Seq: %ld   Configured: %d\n"), map[tmp], i, orig), false);
            i = orig;
        } else if (i > orig) {
            params.convertDialog->AppendConvertStatus (string_format(wxString("WARNING Universe: %ld contains more channels than you have configured.\n"), map[tmp]), false);
            params.convertDialog->AppendConvertStatus (string_format(wxString("Found Universe: %ld   Channels in Seq: %ld   Configured: %d\n"), map[tmp], i, orig), false);
        } else {
            params.convertDialog->AppendConvertStatus (string_format(wxString("Found Universe: %ld   Channels in Seq: %ld\n"), map[tmp], i, orig), false);
        }


        map[tmp + 1] = channels;
        channels += i;
    }

    params.convertDialog->AppendConvertStatus (string_format(wxString("TimeCells = %d\n"), timeCells), false);
    params.convertDialog->AppendConvertStatus (string_format(wxString("msPerCell = %d ms\n"), msPerCell), false);
    params.convertDialog->AppendConvertStatus (string_format(wxString("Channels = %d\n"), channels), false);
    if (channels == 0)
    {
        return;
    }
    if (timeCells == 0)
    {
        return;
    }
    params.seq_data.init(channels, timeCells, msPerCell);

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
                        wxString o2 = params.NetInfo.GetChannelName(channels);
                        params.convertDialog->AppendConvertStatus (string_format("Map %s -> %s (%s)\n",
                                                           ChannelNames[channels].c_str(),
                                                           origName.c_str(),
                                                           o2.c_str()), false);
                        for (long newper = 0; newper < params.seq_data.NumFrames(); newper++)
                        {
                            long intensity;
                            intensity = strtoul(Data.substr(newper * 3, 2).c_str(), NULL, 16);
                            params.seq_data[newper][channels] = intensity;
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

    if( params.data_layer != nullptr )
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(params.seq_data.NumChannels());
    }

    if( params.channels_off_at_end )
    {
        ClearLastPeriod(params.seq_data);
    }
    wxYield();
}

// return true on success
#ifndef FPP
static bool LoadVixenProfile(ConvertParameters& params, const wxString& ProfileName, wxArrayInt& VixChannels, wxArrayString& VixChannelNames)
{
    wxString tag,tempstr;
    long OutputChannel;
    wxFileName fn;
    fn.AssignDir(params.xLightsFrm->CurrentDir);
    fn.SetFullName(ProfileName + ".pro");
    if (!fn.FileExists())
    {
        params.ConversionError(wxString("Unable to find Vixen profile: ")+fn.GetFullPath()+wxString("\n\nMake sure a copy is in your xLights directory"));
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
        params.ConversionError(wxString("Unable to load Vixen profile: ")+ProfileName);
    }
    return false;
}
#endif

void FileConverter::ReadVixFile(ConvertParameters& params)
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
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    long TotChannels=params.NetInfo.GetTotChannels();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    params.convertDialog->AppendConvertStatus (wxString("Reading Vixen sequence\n"));

    SP_XmlPullParser *parser = new SP_XmlPullParser();
    parser->setMaxTextSize(MAX_READ_BLOCK_SIZE / 2);
    wxFile file(params.inp_filename);
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
                    *params.media_filename = (getAttributeValueSafe(stagEvent, "filename") );
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
                        LoadVixenProfile(params, NodeValue,VixChannels,VixChannelNames);
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
    params.convertDialog->AppendConvertStatus (string_format(wxString("Max Intensity=%ld\n"),MaxIntensity), false);
    params.convertDialog->AppendConvertStatus (string_format(wxString("# of Channels=%ld\n"),numChannels), false);
    params.convertDialog->AppendConvertStatus (string_format(wxString("Vix Event Period=%ld\n"),VixEventPeriod), false);
    params.convertDialog->AppendConvertStatus (string_format(wxString("Vix data len=%ld\n"),VixDataLen), false);
    if (numChannels == 0)
    {
        return;
    }
    long VixNumPeriods = VixDataLen / VixChannels.size();
    params.convertDialog->AppendConvertStatus (string_format(wxString("Vix # of time periods=%ld\n"),VixNumPeriods), false);
    params.convertDialog->AppendConvertStatus (wxString("Media file=")+*params.media_filename+wxString("\n"), false);
    if (VixNumPeriods == 0) {
        return;
    }
    params.seq_data.init(numChannels, VixNumPeriods, VixEventPeriod);

    // reorder channels according to output number, scale so that max intensity is 255
    int newper,intensity;
    size_t ch;
    for (ch=0; ch < params.seq_data.NumChannels(); ch++)
    {
        OutputChannel = VixChannels[ch] - min;
        if (ch < VixChannelNames.size())
        {
            ChannelNames[OutputChannel] = VixChannelNames[ch];
        }
        for (newper=0; newper < params.seq_data.NumFrames(); newper++)
        {
            intensity=VixSeqData[ch*VixNumPeriods+newper];
            if (MaxIntensity != 255)
            {
                intensity=intensity * 255 / MaxIntensity;
            }
            params.seq_data[newper][OutputChannel] = intensity;
        }
    }

    if( params.data_layer != nullptr )
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(params.seq_data.NumChannels());
    }

    if( params.channels_off_at_end )
    {
        ClearLastPeriod(params.seq_data);
    }

    wxYield();
}

void FileConverter::ReadGlediatorFile(ConvertParameters& params)
{
    wxFile f;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    size_t fileLength;
    int j,period,x_width=32,y_height=32; // for now hard code matrix to be 32x32. after we get this working, we will prompt for this info during convert
    //wxString filename=string_format(wxString("01 - Carol of the Bells.mp3")); // hard code a mp3 file for now
    size_t readcnt;

    long TotChannels=params.NetInfo.GetTotChannels();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    if (!f.Open(params.inp_filename.c_str()))
    {
        params.PlayerError(wxString("Unable to load sequence:\n")+params.inp_filename);
        return;
    }

    fileLength=f.Length();
    int numChannels=(x_width*3*y_height); // 3072 = 32*32*3
    char *frameBuffer=new char[params.seq_data.NumChannels()];

    int numFrames=(int)(fileLength/(x_width*3*y_height));
    //SetMediaFilename(filename);
    params.seq_data.init(numChannels, numFrames, 50);

    wxYield();
    period = 0;
    while((readcnt=f.Read(frameBuffer,params.seq_data.NumChannels())))   // Read one period of channels
    {
        for(j=0; j<readcnt; j++)   // Loop thru all channel.s
        {
            params.seq_data[period][j] = frameBuffer[j];
        }
        period++;
    }

    /*
    for(i=0; i<readcnt-2; i++) {
        params.seq_data[i] = i%256;
    }
    for(i=0; i<readcnt-2; i+=3) { // loop thru channels, jump by 3. so this loop is pixel loop
        period = i/bytes_per_period;
        p=period * (bytes_per_period); // byte offset for start of each period
        ch=p+ (y*x_width*3) + x*3; // shows offset into source buffer
        byte =p+i;
        byte1=p+i+(1)* (params.seq_data.NumFrames());
        byte2=p+i+(2)* (params.seq_data.NumFrames());
        if ( byte2<readcnt) {
            params.seq_data[byte]  = row[i];
            params.seq_data[byte1] = row[i+1];
            params.seq_data[byte2] = row[i+2];
        }
    }
    //   }
    */
    f.Close();
    delete[] frameBuffer;

    if( params.data_layer != nullptr )
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(params.seq_data.NumChannels());
    }

    if( params.channels_off_at_end )
    {
        ClearLastPeriod(params.seq_data);
    }

#ifndef NDEBUG
    params.convertDialog->AppendConvertStatus (string_format(wxString("ReadGlediatorFile SeqData.NumFrames()=%d SeqData.NumChannels()=%d\n"),params.seq_data.NumFrames(),params.seq_data.NumChannels()));
#endif

    wxYield();
}

#ifndef FPP
void FileConverter::ReadConductorFile(ConvertParameters& params)
{
    wxFile f;
    int i,j,ch;
    char row[16384];
    int period=0;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    long TotChannels=params.NetInfo.GetTotChannels();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    if( params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN ) {
        wxWindow* parent = NULL;
        if (params.convertDialog != NULL)
        {
            parent = params.convertDialog;
        }
        else
        {
            parent = params.xLightsFrm;
        }
        wxFileDialog mediaDialog(parent,wxString("Select associated media file, or cancel if this is an animation"));
        if (mediaDialog.ShowModal() == wxID_OK)
        {
            *params.media_filename = mediaDialog.GetPath();
        }
    }
    if (!f.Open(params.inp_filename.c_str()))
    {
        params.PlayerError(wxString("Unable to load sequence:\n")+params.inp_filename);
        return;
    }
    int numPeriods=f.Length()/16384;

    params.seq_data.init(16384, numPeriods, 50);
    while (f.Read(row,16384) == 16384)
    {
        wxYield();
        for (i=0; i < 4096; i++)
        {
            for (j=0; j < 4; j++)
            {
                ch=j * 4096 + i;
                if (ch < params.seq_data.NumChannels())
                {
                    params.seq_data[period][ch] = row[i*4+j];
                }
            }
        }
        period++;
    }
    f.Close();

    if( params.data_layer != nullptr )
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(params.seq_data.NumChannels());
    }

    if( params.channels_off_at_end )
    {
        ClearLastPeriod(params.seq_data);
    }

    wxYield();
}
#endif


void FileConverter::ReadFalconFile(ConvertParameters& params)
{
    wxUint16 fixedHeaderLength = 28;
    wxFile f;
    size_t readcnt;
    int seqStepTime = 0;
	int falconPeriods = 0;
    int periodsRead = 0;
    int i = 0;
    char *tmpBuf = NULL;

    if(params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN )
    {
        params.xLightsFrm->ConversionInit();
    }

    if (!f.Open(params.inp_filename.c_str()))
    {
        params.PlayerError(wxString("Unable to load sequence:\n")+params.inp_filename);
        return;
    }
    unsigned char hdr[1024];
    f.Read(hdr,fixedHeaderLength);

    int dataOffset = hdr[4] + (hdr[5] << 8);
    if (dataOffset < 1024) {
        f.Seek(0);
        f.Read(hdr, dataOffset);
    }
    int numChannels = hdr[10] + (hdr[11] << 8) + (hdr[12] << 16) + (hdr[13] << 24);
    seqStepTime = hdr[18] + (hdr[19] << 8);
    wxString mf = "";
    if (dataOffset > 28 && hdr[30] == 'm' && hdr[31] == 'f') {
        mf = (char *)&hdr[32];
    }

    if( params.media_filename ) {
        *params.media_filename = mf;
    }

    if( params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN ) {
        params.xLightsFrm->SetMediaFilename(mf);
    }

    falconPeriods = 0;
    if (numChannels > 0) falconPeriods = (f.Length() - dataOffset) / numChannels;
    if( params.data_layer != nullptr )
    {
        params.data_layer->SetNumFrames(falconPeriods);
        params.data_layer->SetNumChannels(numChannels);
    }

    if(params.read_mode == ConvertParameters::READ_MODE_HEADER_ONLY )
    {
        f.Close();
        return;
    }

    if(params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN )
    {
        params.seq_data.init(numChannels, falconPeriods, seqStepTime);
    }

    int channel_offset = 0;
    if( params.data_layer )
    {
        channel_offset = params.data_layer->GetChannelOffset();
    }

    f.Seek(dataOffset);
    tmpBuf = new char[numChannels];
    while (periodsRead < falconPeriods)
    {
        readcnt = f.Read(tmpBuf, numChannels);
        if (readcnt < numChannels)
        {
            params.PlayerError(wxString("Unable to read all event data from:\n")+params.inp_filename);
        }

        int new_index;
        for (i = 0; i < numChannels; i++)
        {
            new_index = i + channel_offset;
            if( (new_index < 0) || (new_index >= numChannels) ) continue;
            if(params.read_mode == ConvertParameters::READ_MODE_IGNORE_BLACK )
            {
                if( tmpBuf[i] != 0 )
                {
                    params.seq_data[periodsRead][new_index] = tmpBuf[i];
                }
            }
            else
            {
                params.seq_data[periodsRead][new_index] = tmpBuf[i];
            }
        }

        periodsRead++;
    }
    delete []tmpBuf;

#ifndef NDEBUG
    params.convertDialog->AppendConvertStatus(string_format(wxString("Read ISEQ File SeqData.NumFrames()=%d SeqData.NumChannels()=%d\n"),params.seq_data.NumFrames(),params.seq_data.NumChannels()));
#endif

    f.Close();
}

void FileConverter::WriteFalconPiFile( ConvertParameters& params )
{
	log4cpp::Category& logger = log4cpp::Category::getRoot();
	logger.debug("Start fseq write");
	wxUint8 vMinor = 0;
    wxUint8 vMajor = 1;
    wxUint16 fixedHeaderLength = 28;
    wxUint32 stepSize = rountTo4(params.seq_data.NumChannels());

    wxUint16 stepTime = params.seq_data.FrameTime();
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
    if (!f.Create(params.out_filename,true))
    {
        params.ConversionError(wxString("Unable to create file: ")+params.out_filename);
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
    buf[14] = (wxUint8)(params.seq_data.NumFrames() & 0xFF);
    buf[15] = (wxUint8)((params.seq_data.NumFrames() >> 8) & 0xFF);
    buf[16] = (wxUint8)((params.seq_data.NumFrames() >> 16) & 0xFF);
    buf[17] = (wxUint8)((params.seq_data.NumFrames() >> 24) & 0xFF);
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

    if(params.media_filename) {
        if((*params.media_filename).length() > 0) {
            int len = strlen((*params.media_filename).c_str()) + 5;
            buf[28] = (wxUint8)(len & 0xFF);
            buf[29] = (wxUint8)((len >> 8) & 0xFF);
            buf[30] = 'm';
            buf[31] = 'f';
            strcpy((char *)&buf[32],(*params.media_filename).c_str());
            fixedHeaderLength += len;
            fixedHeaderLength = rountTo4(fixedHeaderLength);
        }
    }
    // Data offset
    buf[4] = (wxUint8)(fixedHeaderLength%256);
    buf[5] = (wxUint8)(fixedHeaderLength/256);
    f.Write(buf,fixedHeaderLength);


    for (long period=0; period < params.seq_data.NumFrames(); period++)
    {
        for(ch=0; ch<stepSize; ch++)
        {
            buf[ch] = params.seq_data[period][ch];
        }
        f.Write(buf,stepSize);
    }
    f.Close();
    free(buf);
	logger.debug("End fseq write");
}

