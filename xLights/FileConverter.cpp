#include "FileConverter.h"

#include <wx/app.h>
#include <wx/arrstr.h>
#include <wx/file.h>
#include <algorithm>
#include <map>

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"

#include "xLightsMain.h"

static const int MAX_READ_BLOCK_SIZE = 4096 * 1024;

ConvertParameters::ConvertParameters( wxString inp_filename_,
                                      SequenceData& seq_data_,
                                      NetInfoClass& NetInfo_,
                                      ReadMode read_mode_,
                                      xLightsFrame* xLightsParent_,
                                      wxString* media_filename_,
                                      DataLayer* data_layer_,
                                      wxCheckListBox* CheckListBoxTestChannels_,
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
  CheckListBoxTestChannels(CheckListBoxTestChannels_),
  data_layer(data_layer_),
  channels_off_at_end(channels_off_at_end_),
  map_empty_channels(map_empty_channels_),
  map_no_network_channels(map_no_network_channels_),
  read_mode(read_mode_),
  xLightsParent(xLightsParent_)
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


void mapLORInfo(const LORInfo &info, std::vector<std::vector<int>> *unitSizes)
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
    std::vector<std::vector<int>> noNetworkUnitSizes;
    std::vector<std::vector<int>> lorUnitSizes;
    std::vector<std::vector<int>> dmxUnitSizes;
    LORInfoMap rgbChannels;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    long TotChannels=params.NetInfo.GetTotChannels();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    params.xLightsParent->AppendConvertStatus(wxString::Format("Reading LOR sequence: %s\n", params.inp_filename));
    params.xLightsParent->SetStatusText(wxString::Format("Reading LOR sequence: %s\n", params.inp_filename));

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
                        params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("Channels found so far: %d\n"),channelCount));
                        params.xLightsParent->SetStatusText(wxString::Format(wxString("Channels found so far: %d"),channelCount));
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
                        std::vector<std::vector<int>> *unitSizes;
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
                    std::vector<std::vector<int>> *unitSizes;
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
    params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("Track 1 length = %d centiseconds\n"),centisec), false);

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
        params.xLightsParent->ConversionError(wxString("Unable to determine the length of this LOR sequence (looked for length of track 1)"));
        return;
    }

    for (network = 0; network < lorUnitSizes.size(); network++)
    {
        cnt = 0;
        for (int u = 0; u < lorUnitSizes[network].size(); u++)
        {
            cnt += lorUnitSizes[network][u];
        }
        params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("LOR Network %d:  %d channels\n"),network,cnt), false);
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
        params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("DMX Network %d:  %d channels\n"),network,cnt), false);
    }
    params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("Total channels = %d\n"),channelCount));

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
                        params.xLightsParent->AppendConvertStatus (wxString("WARNING: ")+ChannelNames[curchannel] + " is empty\n");
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
                //msg=wxString("Element: ") + NodeName + wxString::Format(wxString(" (%ld)\n"),cnt);
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
                        params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("Channels converted so far: %d\n"),channelCount));
                        params.xLightsParent->SetStatusText(wxString::Format(wxString("Channels converted so far: %d"),channelCount));
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
                        //params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("curchannel %d\n"),curchannel));
                        if (ChannelNames[curchannel].size() != 0)
                        {
                           params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("WARNING: ")+ChannelNames[curchannel]+wxString(" and ")
                                                                                       +ChannelName+wxString(" map to the same channel %d\n"), curchannel));
                        }
                        MappedChannelCnt++;
                        ChannelNames[curchannel] = ChannelName;
                        ChannelColor = getAttributeValueAsInt(stagEvent, "color");
                        ChannelColors[curchannel] = ChannelColor;
                    }
                    else
                    {
                        params.xLightsParent->AppendConvertStatus (wxString("WARNING: channel '")+ChannelName+wxString("' is unmapped\n"));
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

    params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("# of mapped channels with effects=%d\n"),MappedChannelCnt), false);
    params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("# of effects=%d\n"),EffectCnt), false);
    if( params.media_filename )
        params.xLightsParent->AppendConvertStatus (wxString("Media file=")+*params.media_filename+wxString("\n"), false);
    params.xLightsParent->AppendConvertStatus (wxString::Format(wxString("New # of time periods=%ld\n"),params.seq_data.NumFrames()), false);
    params.xLightsParent->SetStatusText(wxString("LOR sequence converted successfully"));
}

void FileConverter::WriteFalconPiFile( ConvertParameters& params )
{
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
        params.xLightsParent->ConversionError(wxString("Unable to create file: ")+params.out_filename);
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
}

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
        params.xLightsParent->ConversionInit();
    }

    if (!f.Open(params.inp_filename.c_str()))
    {
        params.xLightsParent->PlayerError(wxString("Unable to load sequence:\n")+params.inp_filename);
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
        params.xLightsParent->SetMediaFilename(mf);
    }

    falconPeriods = (f.Length() - dataOffset) / numChannels;
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
            params.xLightsParent->PlayerError(wxString("Unable to read all event data from:\n")+params.inp_filename);
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
    params.xLightsParent->AppendConvertLog(wxString::Format(wxString("Read ISEQ File SeqData.NumFrames()=%ld SeqData.NumChannels()=%ld\n"),params.seq_data.NumFrames(),params.seq_data.NumChannels()));
#endif

    f.Close();
}

