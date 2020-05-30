/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <map>

#include <wx/app.h>
#include <wx/arrstr.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/xml/xml.h>

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"

#include "FSEQFile.h"
#include "FileConverter.h"
#include "UtilFunctions.h"
#include "outputs/OutputManager.h"
#include "outputs/Controller.h"
#ifndef FPP
    #include "xLightsMain.h"
    #include "ConvertDialog.h"
    #include "ConvertLogDialog.h"
    #include "outputs/Output.h"
    #define string_format wxString::Format
#endif
#include "xLightsVersion.h"
#include <log4cpp/Category.hh>

static const int MAX_READ_BLOCK_SIZE = 4096 * 1024;

void ConvertParameters::AppendConvertStatus(const wxString& msg, bool flushbuffer)
{
    if (convertDialog != nullptr)
    {
        convertDialog->AppendConvertStatus(msg + "\n", flushbuffer);
    }
    if (convertLogDialog != nullptr)
    {
        convertLogDialog->AppendConvertStatus(msg + "\n", flushbuffer);
    }
    static log4cpp::Category &logger_conversion = log4cpp::Category::getInstance(std::string("log_conversion"));
    logger_conversion.info("Convert Status: " + msg);
}


void ConvertParameters::SetStatusText(wxString msg)
{
    if (xLightsFrm != nullptr)
    {
        xLightsFrm->SetStatusText(msg);
    }
    if (convertDialog != nullptr)
    {
        convertDialog->SetStatusText(msg);
    }
}

void ConvertParameters::ConversionError(wxString msg)
{
    if (convertDialog != nullptr)
    {
        convertDialog->ConversionError(msg);
    }
    else if (xLightsFrm != nullptr)
    {
        xLightsFrm->ConversionError(msg);
    }
}

void ConvertParameters::PlayerError(wxString msg)
{
    if (convertDialog != nullptr)
    {
        convertDialog->PlayerError(msg);
    }
    else if (xLightsFrm != nullptr)
    {
        xLightsFrm->PlayerError(msg);
    }
}
ConvertParameters::ConvertParameters( wxString inp_filename_,
                                      SequenceData& seq_data_,
                                      OutputManager* outputManager_,
                                      ReadMode read_mode_,
                                      xLightsFrame* xLightsFrm_,
                                      ConvertDialog* convertDialog_,
                                      ConvertLogDialog* convertLogDialog_,
                                      std::string* media_filename_,
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
  _outputManager(outputManager_),
  media_filename(media_filename_),
  data_layer(data_layer_),
  channels_off_at_end(channels_off_at_end_),
  map_empty_channels(map_empty_channels_),
  map_no_network_channels(map_no_network_channels_),
  read_mode(read_mode_),
  xLightsFrm(xLightsFrm_),
  convertDialog(convertDialog_),
  convertLogDialog(convertLogDialog_)
{
}

FileConverter::FileConverter()
{
    //ctor
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("ReadLorFile %s.", (const char *)params.inp_filename.c_str());
    logger_base.debug("     Channels Off At End? %s.", params.channels_off_at_end ? "TRUE" :"FALSE");
    logger_base.debug("     Map Empty Channels? %s.", params.map_empty_channels ? "TRUE" :"FALSE");
    logger_base.debug("     Map No Network Channels? %s.", params.map_no_network_channels ? "TRUE" :"FALSE");

    wxString NodeName, msg, deviceType, networkAsString;
    wxArrayString context;
    int unit, circuit, rampdiff;
    int i, twinklestate;
    int twinkleperiod = 400;
    int curchannel = -1;
    int MappedChannelCnt = 0;
    int MaxIntensity = 100;
    int EffectCnt = 0;
    int network, chindex = -1;
    long cnt = 0;
    std::vector< std::vector<int> > noNetworkUnitSizes;
    std::vector< std::vector<int> > lorUnitSizes;
    std::vector< std::vector<int> > dmxUnitSizes;
    LORInfoMap rgbChannels;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    long totalChannels = params._outputManager->GetTotalChannels();

    for (int x = 0; x < totalChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    params.AppendConvertStatus(string_format("Reading LOR sequence: %s", params.inp_filename));
    params.SetStatusText(string_format("Reading LOR sequence: %s\n", params.inp_filename));

    int centisec = -1;
    int nodecnt = 0;
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
            switch (event->getEventType())
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
                    nodecnt = 0;
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
                        params.AppendConvertStatus(string_format(wxString("Channels found so far: %d"), channelCount));
                        params.SetStatusText(string_format(wxString("Channels found so far: %d"), channelCount));
                    }

                    deviceType = FromAscii(stagEvent->getAttrValue("deviceType"));
                    network = getAttributeValueAsInt(stagEvent, "network");
                    networkAsString = FromAscii(stagEvent->getAttrValue("network"));
                    unit = getAttributeValueAsInt(stagEvent, "unit");
                    circuit = getAttributeValueAsInt(stagEvent, "circuit");
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");
                    wxString channelName = FromAscii(stagEvent->getAttrValue("name"));
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
                        }
                        else if ("" == deviceType && "" == networkAsString && !params.map_no_network_channels) {
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
                    }
                    else if ("" == deviceType && "" == networkAsString && !params.map_no_network_channels) {
                        unitSizes = &noNetworkUnitSizes;
                    }
                    else {
                        unitSizes = &lorUnitSizes;
                    }
                    mapLORInfo(rgbChannels[savedIndex], unitSizes);
                }

                if (cnt > 0)
                {
                    RemoveAt(context, cnt - 1);
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
    params.AppendConvertStatus(string_format(wxString("Track 1 length = %d centiseconds"), centisec), false);

    if (centisec > 0)
    {
        int numFrames = centisec * 10 / params.sequence_interval;
        if (numFrames == 0)
        {
            numFrames = 1;
        }
        params.seq_data.init(params._outputManager->GetTotalChannels(), numFrames, params.sequence_interval);
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
        params.AppendConvertStatus(string_format(wxString("LOR Network %d:  %d channels"), network, cnt), false);
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
        params.AppendConvertStatus(string_format(wxString("DMX Network %d:  %d channels"), network, cnt), false);
    }
    params.AppendConvertStatus(string_format(wxString("Total channels = %d"), channelCount));

    if (totalChannels < channelCount)
    {
        DisplayWarning(wxString::Format("LOR file has %d channels but xLights has only %d channels defined.", channelCount, totalChannels).ToStdString());
    }

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
    bool empty = false;
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
            switch (event->getEventType())
            {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eEndTag:
                if (cnt > 0)
                {
                    RemoveAt(context, cnt - 1);
                }
                cnt = context.size();
                if (cnt == 2)
                {
                    if (empty && curchannel >= 0)
                    {
                        chindex--;
                        params.AppendConvertStatus(wxString("WARNING: ") + ChannelNames[curchannel] + " is empty");
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
                    nodecnt = 0;
                    wxYield();
                }
                //msg=wxString("Element: ") + NodeName + string_format(wxString(" (%ld)\n"),cnt);
                //AppendConvertStatus (msg);
                if (NodeName == wxString("sequence"))
                {
                    if (params.media_filename)
                        *params.media_filename = getAttributeValueSafe(stagEvent, "musicFilename");
                }
                if (cnt == 3 && context[1] == wxString("channels") && NodeName == wxString("channel"))
                {
                    channelCount++;
                    if ((channelCount % 1000) == 0)
                    {
                        params.AppendConvertStatus(string_format(wxString("Channels converted so far: %d"), channelCount));
                        params.SetStatusText(string_format(wxString("Channels converted so far: %d"), channelCount));
                    }

                    deviceType = getAttributeValueSafe(stagEvent, "deviceType");
                    networkAsString = getAttributeValueSafe(stagEvent, "network");
                    network = getAttributeValueAsInt(stagEvent, "network");

                    unit = getAttributeValueAsInt(stagEvent, "unit");
                    if (unit < 0)
                    {
                        unit += 256;
                    }
                    if (unit == 0)
                    {
                        unit = 1;
                    }
                    circuit = getAttributeValueAsInt(stagEvent, "circuit");
                    wxString ChannelName = getAttributeValueSafe(stagEvent, "name");
                    savedIndex = getAttributeValueAsInt(stagEvent, "savedIndex");

                    empty = rgbChannels[savedIndex].empty;
                    if (Left(deviceType, 3) == "DMX")
                    {
                        chindex = circuit - 1;
                        network--;
                        network += lorUnitSizes.size();
                        curchannel = params._outputManager->GetAbsoluteChannel(network, chindex) - 1;
                        if (curchannel < 0) curchannel = -1;
                    }
                    else if (Left(deviceType, 3) == "LOR")
                    {
                        chindex = 0;
                        for (int z = 0; z < (unit - 1); z++)
                        {
                            if (lorUnitSizes.size() > network && lorUnitSizes[network].size() > z)
                            {
                                chindex += lorUnitSizes[network][z];
                            }
                            else
                            {
                                params.AppendConvertStatus("Problem resolving channel. Have you got your setup tab right?");
                            }
                        }
                        chindex += circuit - 1;
                        curchannel = params._outputManager->GetAbsoluteChannel(network, chindex) - 1;
                        if (curchannel < 0) curchannel = -1;
                    }
                    else if ("" == deviceType && "" == networkAsString && !params.map_no_network_channels) {
                        curchannel = -1;
                    }
                    else {
                        chindex++;
                        if (chindex < params._outputManager->GetTotalChannels())
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
                        //params.AppendConvertStatus (string_format(wxString("curchannel %d"),curchannel));
                        if (ChannelNames[curchannel].size() != 0)
                        {
                            params.AppendConvertStatus(string_format(wxString("WARNING: ") + ChannelNames[curchannel] + wxString(" and ")
                                + ChannelName + wxString(" map to the same channel %d"), curchannel));
                        }
                        MappedChannelCnt++;
                        ChannelNames[curchannel] = ChannelName;
                        int ChannelColor = getAttributeValueAsInt(stagEvent, "color");
                        ChannelColors[curchannel] = ChannelColor;
                    }
                    else
                    {
                        params.AppendConvertStatus(wxString("WARNING: channel '") + ChannelName + wxString("' is unmapped"));
                    }
                }
                if (cnt > 1 && context[1] == wxString("channels") && NodeName == wxString("effect") && curchannel >= 0)
                {
                    empty = false;
                    EffectCnt++;
                    int startcsec = getAttributeValueAsInt(stagEvent, "startCentisecond");
                    int endcsec = getAttributeValueAsInt(stagEvent, "endCentisecond");
                    int intensity = getAttributeValueAsInt(stagEvent, "intensity");
                    int startIntensity = getAttributeValueAsInt(stagEvent, "startIntensity");
                    int endIntensity = getAttributeValueAsInt(stagEvent, "endIntensity");
                    int startper = startcsec * 10 / params.sequence_interval;
                    int endper = endcsec * 10 / params.sequence_interval;
                    int perdiff = endper - startper;  // # of ticks

                    if (perdiff > 0)
                    {
                        wxString EffectType = getAttributeValueSafe(stagEvent, "type");
                        if (EffectType != "DMX intensity")
                        {
                            intensity = intensity * 255 / MaxIntensity;
                            startIntensity = startIntensity * 255 / MaxIntensity;
                            endIntensity = endIntensity * 255 / MaxIntensity;
                        }
                        if (EffectType == "intensity" || EffectType == "DMX intensity")
                        {
                            if (intensity > 0)
                            {
                                for (i = 0; i < perdiff; i++)
                                {
                                    params.seq_data[startper + i][curchannel] = intensity;
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff = endIntensity - startIntensity;
                                for (i = 0; i < perdiff; i++)
                                {
                                    intensity = (int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    params.seq_data[startper + i][curchannel] = intensity;
                                }
                            }
                        }
                        else if (EffectType == "twinkle")
                        {
                            if (intensity == 0 && startIntensity == 0 && endIntensity == 0)
                            {
                                intensity = MaxIntensity;
                            }
                            twinklestate = static_cast<int>(rand01()*2.0) & 0x01;
                            int nexttwinkle = static_cast<int>(rand01()*twinkleperiod + 100) / params.sequence_interval;
                            if (intensity > 0)
                            {
                                for (i = 0; i < perdiff; i++)
                                {
                                    params.seq_data[startper + i][curchannel] = intensity * twinklestate;
                                    nexttwinkle--;
                                    if (nexttwinkle <= 0)
                                    {
                                        twinklestate = 1 - twinklestate;
                                        nexttwinkle = static_cast<int>(rand01()*twinkleperiod + 100) / params.sequence_interval;
                                    }
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff = endIntensity - startIntensity;
                                for (i = 0; i < perdiff; i++)
                                {
                                    intensity = (int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    params.seq_data[startper + i][curchannel] = intensity * twinklestate;
                                    nexttwinkle--;
                                    if (nexttwinkle <= 0)
                                    {
                                        twinklestate = 1 - twinklestate;
                                        nexttwinkle = static_cast<int>(rand01()*twinkleperiod + 100) / params.sequence_interval;
                                    }
                                }
                            }
                        }
                        else if (EffectType == "shimmer")
                        {
                            if (intensity == 0 && startIntensity == 0 && endIntensity == 0)
                            {
                                intensity = MaxIntensity;
                            }
                            if (intensity > 0)
                            {
                                for (i = 0; i < perdiff; i++)
                                {
                                    twinklestate = (startper + i) & 0x01;
                                    params.seq_data[startper + i][curchannel] = intensity * twinklestate;
                                }
                            }
                            else if (startIntensity > 0 || endIntensity > 0)
                            {
                                // ramp
                                rampdiff = endIntensity - startIntensity;
                                for (i = 0; i < perdiff; i++)
                                {
                                    twinklestate = (startper + i) & 0x01;
                                    intensity = (int)((double)(i) / perdiff * rampdiff + startIntensity);
                                    params.seq_data[startper + i][curchannel] = intensity * twinklestate;
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
    delete[] bytes;
    delete parser;
    file.Close();

    if (params.data_layer != nullptr)
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(MappedChannelCnt);
    }

    if (params.channels_off_at_end)
    {
        ClearLastPeriod(params.seq_data);
    }

    params.AppendConvertStatus(string_format(wxString("# of mapped channels with effects=%d"), MappedChannelCnt), false);
    params.AppendConvertStatus(string_format(wxString("# of effects=%d"), EffectCnt), false);
    if (params.media_filename)
        params.AppendConvertStatus(wxString("Media file=") + *params.media_filename, false);
    params.AppendConvertStatus(string_format(wxString("New # of time periods=%ld"), params.seq_data.NumFrames()), false);
    params.SetStatusText(wxString("LOR sequence converted successfully"));

    wxYield();
}

void FileConverter::ReadXlightsFile(ConvertParameters& params)
{
    wxFile f;
    char hdr[512], filetype[10];
    int fileversion, numch, numper;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    long TotChannels = params._outputManager->GetTotalChannels();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    wxFile file(params.inp_filename);

    if (!f.Open(params.inp_filename.c_str()))
    {
        params.PlayerError(wxString("Unable to load sequence:\n") + params.inp_filename);
        return;
    }

    f.Read(hdr, 512);
    int scancnt = sscanf(hdr, "%8s %2d %8d %8d", filetype, &fileversion, &numch, &numper);
    if (scancnt != 4 || strncmp(filetype, "xLights", 7) != 0 || numch <= 0 || numper <= 0)
    {
        params.PlayerError(wxString("Invalid file header:\n") + params.inp_filename);
    }
    else
    {
        params.seq_data.init(numch, numper, 50);
        char * buf = new char[numper];
        wxString filename = FromAscii(hdr + 32);

        if (params.media_filename) {
            *params.media_filename = filename;
        }

        if (params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN) {
            params.xLightsFrm->SetMediaFilename(filename);
        }

        for (int x = 0; x < numch; x++) {
            size_t readcnt = f.Read(buf, numper);
            if (readcnt < numper)
            {
                params.PlayerError(wxString("Unable to read all event data from:\n") + params.inp_filename);
            }
            for (int p = 0; p < numper; p++) {
                params.seq_data[p][x] = buf[p];
            }
        }
        delete[] buf;
#ifndef NDEBUG
        params.AppendConvertStatus(string_format(wxString("ReadXlightsFile SeqData.NumFrames()=%d SeqData.NumChannels()=%d"), params.seq_data.NumFrames(), params.seq_data.NumChannels()));
#endif
    }
    f.Close();

    if (params.data_layer != nullptr)
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(params.seq_data.NumChannels());
    }

    if (params.channels_off_at_end)
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
            switch (event->getEventType())
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
                break;
            }
            case SP_XmlPullEvent::eCData:
            {
                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    NodeValue = FromAscii(stagEvent->getText());

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

                    RemoveAt(context, cnt - 1);
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
        int orig = params._outputManager->GetControllerIndex(tmp / 2)->GetChannels();
        //int orig = params._outputManager->GetOutput(tmp / 2)->GetChannels();
        if (i < orig) {
            params.AppendConvertStatus(string_format(wxString("Found Universe: %ld   Channels in Seq: %ld   Configured: %d"), map[tmp], i, orig), false);
            i = orig;
        }
        else if (i > orig) {
            params.AppendConvertStatus(string_format(wxString("WARNING Universe: %ld contains more channels than you have configured."), map[tmp]), false);
            params.AppendConvertStatus(string_format(wxString("Found Universe: %ld   Channels in Seq: %ld   Configured: %d"), map[tmp], i, orig), false);
        }
        else {
            params.AppendConvertStatus(string_format(wxString("Found Universe: %ld   Channels in Seq: %ld"), map[tmp], i, orig), false);
        }


        map[tmp + 1] = channels;
        channels += i;
    }

    params.AppendConvertStatus(string_format(wxString("TimeCells = %d"), timeCells), false);
    params.AppendConvertStatus(string_format(wxString("msPerCell = %d ms"), msPerCell), false);
    params.AppendConvertStatus(string_format(wxString("Channels = %d"), channels), false);
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
            switch (event->getEventType())
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
                break;
            }
            case SP_XmlPullEvent::eCData:
            {
                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                if (cnt > 0)
                {
                    NodeName = context[cnt - 1];
                    NodeValue = FromAscii(stagEvent->getText());

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
                        for (tmp = 0; tmp < map.size(); tmp += 2)
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
                    nodecnt = 0;
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
                        wxString o2 = params._outputManager->GetChannelName(channels);
                        params.AppendConvertStatus(string_format("Map %s -> %s (%s)",
                            ChannelNames[channels].c_str(),
                            origName.c_str(),
                            o2.c_str()), false);
                        for (unsigned long newper = 0; newper < params.seq_data.NumFrames(); newper++)
                        {
                            long intensity;
                            intensity = strtoul(Data.substr(newper * 3, 2).c_str(), NULL, 16);
                            params.seq_data[newper][channels] = intensity;
                        }
                        Data.clear();
                        channels++;
                    }

                    RemoveAt(context, cnt - 1);
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
    delete[] bytes;
    delete parser;
    file.Close();

    if (params.data_layer != nullptr)
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(params.seq_data.NumChannels());
    }

    if (params.channels_off_at_end)
    {
        ClearLastPeriod(params.seq_data);
    }
    wxYield();
}

// return true on success
bool FileConverter::LoadVixenProfile(ConvertParameters& params, const wxString& ProfileName,
    wxArrayInt& VixChannels, wxArrayString& VixChannelNames,
    std::vector<xlColor> &VixChannelColors)
{
    wxString tag, tempstr;
    long OutputChannel;
    wxFileName fn;
    fn.AssignDir(params.xLightsFrm->CurrentDir);
    fn.SetFullName(ProfileName + ".pro");
    if (!fn.FileExists()) {
        fn.AssignDir(wxFileName::FileName(params.inp_filename).GetPath());
        fn.SetFullName(ProfileName + ".pro");
    }

    if (!fn.FileExists())
    {
        params.ConversionError(wxString("Unable to find Vixen profile: ") + fn.GetFullPath() + wxString("\n\nMake sure a copy is in your xLights directory"));
        return false;
    }
    wxXmlDocument doc(fn.GetFullPath());
    if (doc.IsOk())
    {
        VixChannels.clear();
        wxXmlNode* root = doc.GetRoot();
        for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext())
        {
            tag = e->GetName();
            if (tag == wxString("ChannelObjects"))
            {
                for (wxXmlNode* p = e->GetChildren(); p != nullptr; p = p->GetNext())
                {
                    if (p->GetName() == wxString("Channel"))
                    {
                        if (p->HasAttribute("output"))
                        {
                            tempstr = p->GetAttribute("output", "0");
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
                            }
                            else {
                                VixChannelNames.push_back(p->GetContent());
                            }
                        }
                        if (p->HasAttribute("color")) {
                            int chanColor = wxAtoi(p->GetAttribute("color")) & 0xFFFFFF;
                            xlColor c(chanColor, false);
                            VixChannelColors.push_back(c);
                        }
                        else {
                            VixChannelColors.push_back(xlWHITE);
                        }
                    }
                }
            }
        }
        return true;
    }
    else
    {
        params.ConversionError(wxString("Unable to load Vixen profile: ") + ProfileName);
    }
    return false;
}

void FileConverter::ReadVixFile(ConvertParameters& params)
{
    wxString msg;
    std::vector<unsigned char> VixSeqData;
    wxArrayInt VixChannels;
    wxArrayString VixChannelNames;
    long cnt = 0;
    wxArrayString context;
    long VixEventPeriod=-1;
    long MaxIntensity = 255;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    long TotChannels=params._outputManager->GetTotalChannels();
    for (int x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    params.AppendConvertStatus (wxString("Reading Vixen sequence"));

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
                wxString NodeName = FromAscii( stagEvent->getName() );
                context.push_back(NodeName);
                cnt++;
                //msg=wxString("Element: ") + NodeName + string_format(wxString(" (%ld)"),cnt);
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
                    wxString NodeValue = FromAscii( stagEvent->getText() );
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
                        //AppendConvertStatus(string_format(wxString("Chunk Size=%d"), NodeValue.size()));
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
                        std::vector<xlColor> colors;
                        LoadVixenProfile(params, NodeValue,VixChannels,VixChannelNames, colors);
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
    for (size_t x = 0; x < VixChannels.size(); x++)
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
    params.AppendConvertStatus (string_format(wxString("Max Intensity=%ld"),MaxIntensity), false);
    params.AppendConvertStatus (string_format(wxString("# of Channels=%ld"),numChannels), false);
    params.AppendConvertStatus (string_format(wxString("Vix Event Period=%ld"),VixEventPeriod), false);
    params.AppendConvertStatus (string_format(wxString("Vix data len=%ld"),VixDataLen), false);
    if (numChannels == 0)
    {
        return;
    }
    long VixNumPeriods = VixDataLen / VixChannels.size();
    params.AppendConvertStatus (string_format(wxString("Vix # of time periods=%ld"),VixNumPeriods), false);
    params.AppendConvertStatus (wxString("Media file=")+*params.media_filename, false);
    if (VixNumPeriods == 0) {
        return;
    }
    params.seq_data.init(numChannels, VixNumPeriods, VixEventPeriod);

    for (size_t ch=0; ch < params.seq_data.NumChannels(); ch++)
    {
        int OutputChannel = VixChannels[ch] - min;
        if (ch < VixChannelNames.size())
        {
            ChannelNames[OutputChannel] = VixChannelNames[ch];
        }
        for (size_t newper=0; newper < params.seq_data.NumFrames(); newper++)
        {
            int intensity = VixSeqData[ch*VixNumPeriods+newper];
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
    int period,x_width=32,y_height=32; // for now hard code matrix to be 32x32. after we get this working, we will prompt for this info during convert
    //wxString filename=string_format(wxString("01 - Carol of the Bells.mp3")); // hard code a mp3 file for now
    size_t readcnt;

    long TotChannels=params._outputManager->GetTotalChannels();
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
        for(size_t j=0; j<readcnt; j++)   // Loop thru all channel.s
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
    params.AppendConvertStatus (string_format(wxString("ReadGlediatorFile SeqData.NumFrames()=%d SeqData.NumChannels()=%d"),params.seq_data.NumFrames(),params.seq_data.NumChannels()));
#endif

    wxYield();
}

#ifndef FPP
void FileConverter::ReadConductorFile(ConvertParameters& params)
{
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;
    int32_t TotChannels = params._outputManager->GetTotalChannels();
    for (int32_t x = 0; x < TotChannels; x++) {
        ChannelColors.push_back(0);
        ChannelNames.push_back("");
    }
    params.seq_data.init(0, 0, params.sequence_interval);

    if (params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN) {
        wxWindow* parent;
        if (params.convertDialog != nullptr)
        {
            parent = params.convertDialog;
        }
        else
        {
            parent = params.xLightsFrm;
        }
        wxFileDialog mediaDialog(parent, wxString("Select associated media file, or cancel if this is an animation"));
        if (mediaDialog.ShowModal() == wxID_OK)
        {
            *params.media_filename = mediaDialog.GetPath();
        }
    }

    wxFile f;
    if (!f.Open(params.inp_filename.c_str()))
    {
        params.PlayerError(wxString("Unable to load sequence:\n") + params.inp_filename);
        return;
    }

    int numPeriods = f.Length() / 16384;
    int period = 0;
    char row[16384];
    params.seq_data.init(16384, numPeriods, 50);
    while (f.Read(row, 16384) == 16384)
    {
        wxYield();
        for (size_t i = 0; i < 4096; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                uint32_t ch = j * 4096 + i;
                if (ch < params.seq_data.NumChannels())
                {
                    params.seq_data[period][ch] = row[i * 4 + j];
                }
            }
        }
        period++;
    }
    f.Close();

    if (params.data_layer != nullptr)
    {
        params.data_layer->SetNumFrames(params.seq_data.NumFrames());
        params.data_layer->SetNumChannels(params.seq_data.NumChannels());
    }

    if (params.channels_off_at_end)
    {
        ClearLastPeriod(params.seq_data);
    }

    wxYield();
}
#endif


void FileConverter::ReadFalconFile(ConvertParameters& params)
{
    static log4cpp::Category &logger_conversion = log4cpp::Category::getInstance(std::string("log_conversion"));

    if (params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN) {
        params.xLightsFrm->ConversionInit();
    }

    FSEQFile *file = FSEQFile::openFSEQFile(params.inp_filename);
    if (!file) {
        logger_conversion.debug("Unable to load sequence: %s.", (const char *)params.inp_filename.c_str());
        params.PlayerError(wxString("Unable to load sequence:\n") + params.inp_filename);
        return;
    }
    int numChannels = file->getMaxChannel();
    int seqStepTime = file->getStepTime();
    wxString mf = "";
    for (auto &a : file->getVariableHeaders()) {
        if (a.code[0] == 'm' && a.code[1] == 'f') {
            mf = (char *)&a.data[0];
        }
    }
    if (params.media_filename) {
        *params.media_filename = mf;
    }

    if (params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN) {
        params.xLightsFrm->SetMediaFilename(mf);
    }

    int falconPeriods = 0;
    if (numChannels > 0) falconPeriods = file->getNumFrames();
    if (params.data_layer != nullptr) {
        params.data_layer->SetNumFrames(falconPeriods);
        params.data_layer->SetNumChannels(numChannels);
    }

    if (params.read_mode == ConvertParameters::READ_MODE_HEADER_ONLY) {
        delete file;
        return;
    }

    std::vector<std::pair<uint32_t, uint32_t>> rng;
    rng.push_back(std::pair<uint32_t, uint32_t>(0, numChannels));
    file->prepareRead(rng);
    if (params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN ||
        params.read_mode == ConvertParameters::READ_MODE_IMPORT) {
        params.seq_data.init(numChannels, falconPeriods, seqStepTime);
    }

    int channel_offset = 0;
    if (params.data_layer) {
        channel_offset = params.data_layer->GetChannelOffset();
    }

    uint8_t *tmpBuf = new uint8_t[numChannels];
    int periodsRead = 0;
    while (periodsRead < falconPeriods) {
        FSEQFile::FrameData *data = file->getFrame(periodsRead);
        if (data == nullptr) break;
        
        if (channel_offset == 0 && params.read_mode != ConvertParameters::READ_MODE_IGNORE_BLACK) {
            if (!data->readFrame(&params.seq_data[periodsRead][0], params.seq_data.NumChannels()))
            {
                // fseq file corrupt
                logger_conversion.error("FSEQ file seems to be corrupt.");
            }
        } else {
            if (data->readFrame(tmpBuf, numChannels))
            {
                for (int i = 0; i < numChannels; i++) {
                    int new_index = i + channel_offset;
                    if ((new_index < 0) || (new_index >= numChannels)) continue;
                    if (params.read_mode == ConvertParameters::READ_MODE_IGNORE_BLACK) {
                        if (tmpBuf[i] != 0) {
                            params.seq_data[periodsRead][new_index] = tmpBuf[i];
                        }
                    }
                    else {
                        params.seq_data[periodsRead][new_index] = tmpBuf[i];
                    }
                }
            }
            else
            {
                // fseq file corrupt
                logger_conversion.error("FSEQ file seems to be corrupt.");
            }
        }
        delete data;
        periodsRead++;
    }
    delete[]tmpBuf;
#ifndef NDEBUG
    params.AppendConvertStatus(string_format(wxString("Read ISEQ File SeqData.NumFrames()=%d SeqData.NumChannels()=%d"), params.seq_data.NumFrames(), params.seq_data.NumChannels()));
#endif
    delete file;
}

void FileConverter::WriteFalconPiFile(ConvertParameters& params)
{
    static log4cpp::Category &logger_conversion = log4cpp::Category::getInstance(std::string("log_conversion"));
    logger_conversion.debug("Start fseq write");
    
    
    const wxUint8 fType = params.xLightsFrm->_fseqVersion;
    int vMajor = 2;
    int clevel = 2;
    FSEQFile::CompressionType ctype = FSEQFile::CompressionType::zstd;
    switch (fType) {
        case 1:
            vMajor = 1;
            break;
        case 3:
            ctype = FSEQFile::CompressionType::none;
            break;
        case 4:
            ctype = FSEQFile::CompressionType::zlib;
            clevel = 1;
            break;
        default:
            break;
    }
    
    FSEQFile *file = FSEQFile::createFSEQFile(params.out_filename, vMajor, ctype, clevel);
    if (!file) {
        params.ConversionError(wxString("Unable to create file: ") + params.out_filename);
        return;
    }

    size_t stepSize = roundTo4(params.seq_data.NumChannels());
    wxUint16 stepTime = params.seq_data.FrameTime();

    file->setChannelCount(stepSize);
    file->setStepTime(stepTime);
    file->setNumFrames(params.seq_data.NumFrames());
    if (params.media_filename) {
        if ((*params.media_filename).length() > 0) {
            FSEQFile::VariableHeader header;
            header.code[0] = 'm';
            header.code[1] = 'f';
            
            int len = strlen((*params.media_filename).c_str()) + 1;
            header.data.resize(len);
            strcpy((char *)&header.data[0], params.media_filename->c_str());
            file->addVariableHeader(header);
        }
    }
    FSEQFile::VariableHeader header;
    header.code[0] = 's';
    header.code[1] = 'p';
    std::string ver = "xLights " + wxPlatformInfo::Get().GetOperatingSystemFamilyName() + " " + GetDisplayVersionString();
    int len = strlen(ver.c_str()) + 1;
    header.data.resize(len);
    strcpy((char *)&header.data[0], ver.c_str());
    file->addVariableHeader(header);

    file->writeHeader();
    size_t size = params.seq_data.NumFrames();
    for (int x = 0; x < size; x++) {
        file->addFrame(x, &params.seq_data[x][0]);
    }
    file->finalize();
    delete file;
    logger_conversion.debug("End fseq write");
}
