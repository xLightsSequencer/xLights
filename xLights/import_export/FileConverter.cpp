/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <functional>
#include <map>
#include <fstream>

#include <wx/app.h>
#include <wx/arrstr.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/mstream.h>

#ifndef LINUX
//zstd on Debian 11 doesn't have the thread pool stuff
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>
#undef ZSTD_STATIC_LINKING_ONLY
#else
#include <zstd.h>
#endif


#include "render/FSEQFile.h"
#include "FileConverter.h"
#include <pugixml.hpp>
#include <sstream>
#include "UtilFunctions.h"
#include "ui/shared/utils/wxUtilities.h"
#include "outputs/OutputManager.h"
#include "outputs/Controller.h"
#ifndef FPP
    #include "xLightsMain.h"
    #include "ui/import-export/ConvertDialog.h"
    #include "ui/import-export/ConvertLogDialog.h"
    #include "outputs/Output.h"
    #define string_format wxString::Format
#endif
#include "utils/xLightsVersion.h"
#include "utils/ExternalHooks.h"
#include <log.h>

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
    spdlog::info("Convert Status: " + msg.ToStdString());
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
  data_layer(data_layer_),
  media_filename(media_filename_),
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

static wxString Left(const wxString &in, int len) {
    return in.substr(0, len);
}

static wxString Right(const wxString &in, int len) {
    return in.substr(in.size() - len, len);
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
        name(nm), savedIndex(s), deviceType(dt), unit(u), circuit(c), network(n), empty(true)
    {
    }
    LORInfo(const LORInfo &li) :
        name(li.name), savedIndex(li.savedIndex),
        deviceType(li.deviceType), unit(li.unit),
        circuit(li.circuit), network(li.network), empty(li.empty)
    {

    }
    LORInfo() :
        name(""), savedIndex(0), deviceType(""), unit(0), circuit(0), network(0), empty(true)
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

    if ((size_t)info.network >= unitSizes->size())
    {
        unitSizes->resize(info.network + 1);
    }
    if (unit > (int)(*unitSizes)[info.network].size())
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
    spdlog::debug("ReadLorFile {}.", params.inp_filename.ToStdString());
    spdlog::debug("     Channels Off At End? {}.", toStr(params.channels_off_at_end ));
    spdlog::debug("     Map Empty Channels? {}.", toStr(params.map_empty_channels));
    spdlog::debug("     Map No Network Channels? {}.", toStr(params.map_no_network_channels));

    wxString deviceType, networkAsString;
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

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(params.inp_filename.mb_str());
    if (!result) {
        params.ConversionError(wxString("Unable to load LOR file: ") + params.inp_filename);
        return;
    }

    // Helper lambda to recursively walk the DOM and process elements for pass 1
    int savedIndex = 0;
    std::function<void(pugi::xml_node, const wxArrayString&)> pass1Walk;
    pass1Walk = [&](pugi::xml_node parent, const wxArrayString& parentContext) {
        for (pugi::xml_node node = parent.first_child(); node; node = node.next_sibling()) {
            if (node.type() != pugi::node_element)
                continue;

            wxString NodeName = FromAscii(node.name());
            wxArrayString context = parentContext;
            context.push_back(NodeName);
            cnt = context.size();

            nodecnt++;
            if (nodecnt > 1000)
            {
                nodecnt = 0;
                wxYield();
            }
            if (NodeName == wxString("track"))
            {
                centisec = node.attribute("totalCentiseconds").as_int(0);
            }
            else if (cnt == 3 && context[1] == wxString("channels") && NodeName == wxString("channel"))
            {
                channelCount++;
                if ((channelCount % 1000) == 0)
                {
                    params.AppendConvertStatus(string_format(wxString("Channels found so far: %d"), channelCount));
                    params.SetStatusText(string_format(wxString("Channels found so far: %d"), channelCount));
                }

                deviceType = FromAscii(node.attribute("deviceType").as_string(""));
                network = node.attribute("network").as_int(0);
                networkAsString = FromAscii(node.attribute("network").as_string(""));
                unit = node.attribute("unit").as_int(0);
                circuit = node.attribute("circuit").as_int(0);
                savedIndex = node.attribute("savedIndex").as_int(0);
                wxString channelName = FromAscii(node.attribute("name").as_string(""));
                rgbChannels[savedIndex] = LORInfo(channelName, deviceType, network, unit, circuit, savedIndex);
                rgbChannels[savedIndex].empty = !params.map_empty_channels;

                // Check children for effects
                for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
                    if (child.type() == pugi::node_element && std::string_view(child.name()) == "effect") {
                        rgbChannels[savedIndex].empty = false;
                        break;
                    }
                }

                // Recurse into children (for rgbChannel sub-channels)
                pass1Walk(node, context);

                // End-tag logic: map channel if not empty
                if (!rgbChannels[savedIndex].empty)
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
                continue; // already recursed
            }
            else if (cnt > 3 && context[1] == wxString("channels") && context[2] == wxString("rgbChannel")
                && context[3] == wxString("channels") && NodeName == wxString("channel"))
            {
                savedIndex = node.attribute("savedIndex").as_int(0);
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

            // Recurse into children
            pass1Walk(node, context);
        }
    };

    wxArrayString rootContext;
    rootContext.push_back(FromAscii(doc.document_element().name()));
    pass1Walk(doc.document_element(), rootContext);

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

    for (network = 0; network < (int)lorUnitSizes.size(); network++)
    {
        cnt = 0;
        for (int u = 0; u < (int)lorUnitSizes[network].size(); u++)
        {
            cnt += lorUnitSizes[network][u];
        }
        params.AppendConvertStatus(string_format(wxString("LOR Network %d:  %d channels"), network, cnt), false);
    }
    for (network = 1; network < (int)dmxUnitSizes.size(); network++)
    {
        cnt = 0;
        for (int u = 0; u < (int)dmxUnitSizes[network].size(); u++)
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
    channelCount = 0;
    nodecnt = 0;

    //pass 2, convert the data
    // Helper lambda to process effects on a channel node
    auto processEffects = [&](pugi::xml_node channelNode) {
        for (pugi::xml_node effectNode = channelNode.first_child(); effectNode; effectNode = effectNode.next_sibling()) {
            if (effectNode.type() != pugi::node_element)
                continue;
            if (std::string_view(effectNode.name()) != "effect" || curchannel < 0)
                continue;

            EffectCnt++;
            int startcsec = effectNode.attribute("startCentisecond").as_int(0);
            int endcsec = effectNode.attribute("endCentisecond").as_int(0);
            int intensity = effectNode.attribute("intensity").as_int(0);
            int startIntensity = effectNode.attribute("startIntensity").as_int(0);
            int endIntensity = effectNode.attribute("endIntensity").as_int(0);
            int startper = startcsec * 10 / params.sequence_interval;
            int endper = endcsec * 10 / params.sequence_interval;
            int perdiff = endper - startper;  // # of ticks

            if (perdiff > 0)
            {
                wxString EffectType = FromAscii(effectNode.attribute("type").as_string(""));
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
    };

    // Pass 2: walk the DOM again to convert effects data
    std::function<void(pugi::xml_node, const wxArrayString&)> pass2Walk;
    pass2Walk = [&](pugi::xml_node parent, const wxArrayString& parentContext) {
        for (pugi::xml_node node = parent.first_child(); node; node = node.next_sibling()) {
            if (node.type() != pugi::node_element)
                continue;

            wxString NodeName = FromAscii(node.name());
            wxArrayString context = parentContext;
            context.push_back(NodeName);
            cnt = context.size();

            nodecnt++;
            if (nodecnt > 1000)
            {
                nodecnt = 0;
                wxYield();
            }

            if (NodeName == wxString("sequence"))
            {
                if (params.media_filename)
                    *params.media_filename = FromAscii(node.attribute("musicFilename").as_string(""));
            }
            if (cnt == 3 && context[1] == wxString("channels") && NodeName == wxString("channel"))
            {
                channelCount++;
                if ((channelCount % 1000) == 0)
                {
                    params.AppendConvertStatus(string_format(wxString("Channels converted so far: %d"), channelCount));
                    params.SetStatusText(string_format(wxString("Channels converted so far: %d"), channelCount));
                }

                deviceType = FromAscii(node.attribute("deviceType").as_string(""));
                networkAsString = FromAscii(node.attribute("network").as_string(""));
                network = node.attribute("network").as_int(0);

                unit = node.attribute("unit").as_int(0);
                if (unit < 0)
                {
                    unit += 256;
                }
                if (unit == 0)
                {
                    unit = 1;
                }
                circuit = node.attribute("circuit").as_int(0);
                wxString ChannelName = FromAscii(node.attribute("name").as_string(""));
                savedIndex = node.attribute("savedIndex").as_int(0);

                bool channelEmpty = rgbChannels[savedIndex].empty;
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
                        if ((int)lorUnitSizes.size() > network && (int)lorUnitSizes[network].size() > z)
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
                    if (ChannelNames[curchannel].size() != 0)
                    {
                        params.AppendConvertStatus(string_format(wxString("WARNING: ") + ChannelNames[curchannel] + wxString(" and ")
                            + ChannelName + wxString(" map to the same channel %d"), curchannel));
                    }
                    MappedChannelCnt++;
                    ChannelNames[curchannel] = ChannelName;
                    int ChannelColor = node.attribute("color").as_int(0);
                    ChannelColors[curchannel] = ChannelColor;
                }
                else
                {
                    params.AppendConvertStatus(wxString("WARNING: channel '") + ChannelName + wxString("' is unmapped"));
                }

                // Process effect children — if any effects found, channel is not empty
                for (pugi::xml_node effectNode = node.first_child(); effectNode; effectNode = effectNode.next_sibling()) {
                    if (effectNode.type() == pugi::node_element && std::string_view(effectNode.name()) == "effect") {
                        channelEmpty = false;
                        break;
                    }
                }
                processEffects(node);

                // Recurse into children for sub-channels (rgbChannel, etc.)
                pass2Walk(node, context);

                // End-tag logic for channel
                if (channelEmpty && curchannel >= 0)
                {
                    chindex--;
                    params.AppendConvertStatus(wxString("WARNING: ") + ChannelNames[curchannel] + " is empty");
                    ChannelNames[curchannel].clear();
                    MappedChannelCnt--;
                }
                curchannel = -1;
                continue; // already recursed
            }

            // Recurse into children
            pass2Walk(node, context);
        }
    };

    wxArrayString rootContext2;
    rootContext2.push_back(FromAscii(doc.document_element().name()));
    pass2Walk(doc.document_element(), rootContext2);

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

void FileConverter::ReadHLSFile(ConvertParameters& params)
{
    long timeCells = 0;
    long msPerCell = 50;
    long channels = 0;
    long tmp;
    long universe = 0;
    long channelsInUniverse = 0;
    wxString NodeName, NodeValue, Data, ChannelName;
    wxArrayInt map;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(params.inp_filename.mb_str());
    if (!result) {
        params.ConversionError(wxString("Unable to load HLS file: ") + params.inp_filename);
        return;
    }

    // pass one, get the metadata by walking the DOM
    std::function<void(pugi::xml_node)> pass1Walk;
    pass1Walk = [&](pugi::xml_node parent) {
        for (pugi::xml_node node = parent.first_child(); node; node = node.next_sibling()) {
            if (node.type() != pugi::node_element)
                continue;

            NodeName = FromAscii(node.name());

            // Process text content of leaf elements
            const char* textVal = node.child_value();
            if (textVal && textVal[0] != '\0') {
                NodeValue = FromAscii(textVal);

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

            // Recurse into children first
            pass1Walk(node);

            // End-tag logic: after processing a Universe element, add to map
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
        }
    };
    pass1Walk(doc.document_element());

    channels = 0;

    for (tmp = 0; tmp < (long)map.size(); tmp += 2)
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

    // pass two, convert the channel data
    int nodecnt = 0;
    std::function<void(pugi::xml_node)> pass2Walk;
    pass2Walk = [&](pugi::xml_node parent) {
        for (pugi::xml_node node = parent.first_child(); node; node = node.next_sibling()) {
            if (node.type() != pugi::node_element)
                continue;

            NodeName = FromAscii(node.name());

            // Process text content
            const char* textVal = node.child_value();
            if (textVal && textVal[0] != '\0') {
                NodeValue = FromAscii(textVal);

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
                    for (tmp = 0; tmp < (long)map.size(); tmp += 2)
                    {
                        if (universe == map[tmp])
                        {
                            channels = map[tmp + 1];
                        }
                    }
                }
            }

            // Recurse into children
            pass2Walk(node);

            // End-tag logic
            if (nodecnt > 1000)
            {
                nodecnt = 0;
                wxYield();
            }
            nodecnt++;
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
        }
    };
    pass2Walk(doc.document_element());

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
    if (!FileExists(fn)) {
        fn.AssignDir(wxFileName::FileName(params.inp_filename).GetPath());
        fn.SetFullName(ProfileName + ".pro");
    }

    if (!FileExists(fn)) {
        params.ConversionError(wxString("Unable to find Vixen profile: ") + fn.GetFullPath() + wxString("\n\nMake sure a copy is in your xLights directory"));
        return false;
    }
    pugi::xml_document doc;
    if (doc.load_file(fn.GetFullPath().mb_str()))
    {
        VixChannels.clear();
        pugi::xml_node root = doc.document_element();
        for (pugi::xml_node e = root.first_child(); e; e = e.next_sibling())
        {
            tag = e.name();
            if (tag == wxString("ChannelObjects"))
            {
                for (pugi::xml_node p = e.first_child(); p; p = p.next_sibling())
                {
                    if (std::string_view(p.name()) == "Channel")
                    {
                        if (p.attribute("output"))
                        {
                            tempstr = p.attribute("output").as_string("0");
                            OutputChannel = atol(tempstr.c_str());
                            VixChannels.push_back(OutputChannel);
                        }
                        if (p.attribute("name"))
                        {
                            VixChannelNames.push_back(p.attribute("name").as_string());
                        }
                        else
                        {
                            VixChannelNames.push_back(p.text().get());
                        }
                        if (p.attribute("color")) {
                            int chanColor = p.attribute("color").as_int() & 0xFFFFFF;
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
    std::vector<unsigned char> VixSeqData;
    wxArrayInt VixChannels;
    wxArrayString VixChannelNames;
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

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(params.inp_filename.mb_str());
    if (!result) {
        params.ConversionError(wxString("Unable to load Vixen file: ") + params.inp_filename);
        return;
    }

    pugi::xml_node root = doc.document_element();

    // Walk the top-level children of the root
    for (pugi::xml_node node = root.first_child(); node; node = node.next_sibling()) {
        if (node.type() != pugi::node_element)
            continue;

        wxString NodeName = FromAscii(node.name());

        if (NodeName == wxString("Audio") || NodeName == wxString("Song"))
        {
            *params.media_filename = FromAscii(node.attribute("filename").as_string(""));
        }
        else if (NodeName == wxString("Channels"))
        {
            for (pugi::xml_node chNode = node.first_child(); chNode; chNode = chNode.next_sibling()) {
                if (chNode.type() != pugi::node_element)
                    continue;
                if (std::string_view(chNode.name()) == "Channel")
                {
                    pugi::xml_attribute outputAttr = chNode.attribute("output");
                    if (!outputAttr)
                    {
                        VixChannels.push_back(VixChannels.size());
                    }
                    else
                    {
                        VixChannels.push_back(outputAttr.as_int(0));
                    }
                    // Channel text content is the channel name
                    const char* chText = chNode.child_value();
                    if (chText && chText[0] != '\0') {
                        while (VixChannelNames.size() < VixChannels.size())
                        {
                            VixChannelNames.push_back("");
                        }
                        VixChannelNames[VixChannels.size() - 1] = FromAscii(chText);
                    }
                }
            }
        }
        else if (NodeName == wxString("MaximumLevel"))
        {
            MaxIntensity = atol(node.child_value());
        }
        else if (NodeName == wxString("EventPeriodInMilliseconds"))
        {
            VixEventPeriod = atol(node.child_value());
        }
        else if (NodeName == wxString("EventValues"))
        {
            wxString NodeValue = FromAscii(node.child_value());
            base64_decode(NodeValue, VixSeqData);
        }
        else if (NodeName == wxString("Profile"))
        {
#ifndef FPP
            wxString NodeValue = FromAscii(node.child_value());
            std::vector<xlColor> colors;
            LoadVixenProfile(params, NodeValue,VixChannels,VixChannelNames, colors);
#endif
        }
    }

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
    
    if (params.read_mode == ConvertParameters::READ_MODE_LOAD_MAIN) {
        params.xLightsFrm->ConversionInit();
    }

    FSEQFile *file = FSEQFile::openFSEQFile(params.inp_filename);
    if (!file) {
        spdlog::debug("Unable to load sequence: {}.", (const char*)params.inp_filename.c_str());
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
        
        if ((uint32_t)numChannels < params.xLightsFrm->GetMaxNumChannels()) {
            // if loading the main data AND the number of channels is less than what
            // xLights needs, it's likely due to the fseq being sparse which is fine
            // so set the number of channels to what is needed
            numChannels = params.xLightsFrm->GetMaxNumChannels();
        }
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
                spdlog::error("FSEQ file seems to be corrupt.");
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
                spdlog::error("FSEQ file seems to be corrupt.");
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

static int compressMemoryBuffer(const wxMemoryOutputStream &out, uint8_t *outbuf, int max, void *pool) {
    auto cctx = ZSTD_createCStream();
    ZSTD_initCStream(cctx, 3);
#ifndef LINUX
    ZSTD_CCtx_refThreadPool(cctx, (POOL_ctx_s*)pool);
#endif
    
    ZSTD_outBuffer output;
    output.size = max;
    output.dst = outbuf;
    output.pos = 0;
    
    ZSTD_inBuffer input;
    input.size = out.GetLength();
    uint8_t *src = (uint8_t*)malloc(output.size);
    input.src = src;
    input.pos = 0;
    
    out.CopyTo(src, input.size);

    ZSTD_compressStream2(cctx, &output, &input, ZSTD_e_end);
    free(src);
    ZSTD_freeCStream(cctx);
    return output.pos;
}
static int compressFile(std::vector<uint8_t> &data, const std::string &filepath, void *pool) {
    std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) {
        return 0;
    }
    
    std::streampos fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    data.resize(fileSize);

    auto cctx = ZSTD_createCStream();
    ZSTD_initCStream(cctx, 3);
#ifndef LINUX
    ZSTD_CCtx_refThreadPool(cctx, (POOL_ctx_s*)pool);
#endif
    
    ZSTD_outBuffer output;
    output.size = fileSize;
    output.dst = &data[0];
    output.pos = 0;
    
    ZSTD_inBuffer input;
    constexpr int BUFFER_SIZE = 128 * 1024;
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    input.size = BUFFER_SIZE;
    input.src = &buffer[0];
    input.pos = 0;
    
    while (!ifs.eof()) {
        ifs.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE);
        input.size = ifs.gcount();
        input.pos = 0;
        while (input.pos < input.size) {
            ZSTD_compressStream2(cctx, &output, &input, ZSTD_e_continue);
        }
    }
    ZSTD_compressStream2(cctx, &output, &input, ZSTD_e_end);

    ZSTD_freeCStream(cctx);
    data.resize(output.pos);
    return output.pos;
}
void FileConverter::WriteFalconPiFile(ConvertParameters& params)
{
    spdlog::debug("Start fseq write");

    bool isEffectSeq = params.xLightsFrm->CurrentSeqXmlFile != nullptr &&
                       params.xLightsFrm->CurrentSeqXmlFile->GetSequenceType() == "Effect";

    const wxUint8 fType = params.xLightsFrm->_fseqVersion;
    int vMajor = 2;
    int clevel = 2;
    bool allowSparse = false;
    FSEQFile::CompressionType ctype = FSEQFile::CompressionType::zstd;

    if (isEffectSeq) {
        // Effect sequences always use v2 sparse with zlib compression
        ctype = FSEQFile::CompressionType::zlib;
        clevel = 1;
        allowSparse = true;
    } else {
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
            case 5:
                allowSparse = true;
                break;
            default:
                break;
        }
    }

    FSEQFile *file = FSEQFile::createFSEQFile(params.out_filename, vMajor, ctype, clevel);
    if (!file) {
        params.ConversionError(wxString("Unable to create file: ") + params.out_filename + ". Check directory and file permissions.");
        return;
    }
    size_t stepSize = isEffectSeq ? params.seq_data.NumChannels() : roundTo4(params.seq_data.NumChannels());
    wxUint16 stepTime = params.seq_data.FrameTime();
    if (vMajor == 2) {
        file->enableMinorVersionFeatures(2);
    }
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
    
    if (vMajor >= 2 && allowSparse && !params.ranges.empty()) {
        for (auto &r : params.ranges) {
            V2FSEQFile* v2file = (V2FSEQFile*)file;
            v2file->m_sparseRanges.push_back(r);
            spdlog::info("Sparse range - Start: {}  End: {}   Size: {}\n", r.first + 1, (r.first + r.second), r.second);
        }
    }
    if (isEffectSeq) {
        // Mark file as an effect sequence so FPP upload can distinguish
        // effect sequences from regular sparse files
        FSEQFile::VariableHeader esHeader;
        esHeader.code[0] = 'e';
        esHeader.code[1] = 'S';
        esHeader.data.push_back(1); // version/flag byte
        file->addVariableHeader(esHeader);
    }
    if (vMajor == 2 && params.elements) {
        for (int x = 0; x < params.elements->GetNumberOfTimingElements(); x++) {
            TimingElement *te = params.elements->GetTimingElement(x);
            if (te->GetSubType() == "FPP Commands" || te->GetSubType() == "FPP Effects") {
                std::map<std::string, std::vector<std::pair<uint32_t, uint32_t>>> commands;
                for (int l = 0; l < (int)te->GetEffectLayerCount(); l++) {
                    EffectLayer *layer = te->GetEffectLayer(l);
                    for (auto & eff : layer->GetAllEffects()) {
                        commands[eff->GetEffectName()].push_back(std::make_pair(eff->GetStartTimeMS(), eff->GetEndTimeMS()));
                    }
                }
                int totalLen = 3; // 1 byte ver, 2 byte count
                std::string fppInstances = ""; // null terminated list of hosts these apply to. (not yet used)
                totalLen += fppInstances.size() + 1;
                for (auto &a : commands) {
                    totalLen += a.first.length() + 1 + 4; // null plus count
                    totalLen += a.second.size() * 8;
                }
                FSEQFile::VariableHeader commandHeader;
                commandHeader.extendedData = true;
                commandHeader.code[0] = 'F';
                if (te->GetSubType() == "FPP Effects") {
                    commandHeader.code[1] = 'E';
                } else {
                    commandHeader.code[1] = 'C';
                }
                commandHeader.data.resize(totalLen);
                uint8_t *data = &commandHeader.data[0];
                data[0] = 1; //version flag
                uint32_t *t2 = (uint32_t*)&data[1];
                *t2 = (uint32_t)commands.size();
                strcpy((char *)&data[5], fppInstances.c_str());
                data += 6 + fppInstances.size();
                for (auto &a : commands) {
                    std::string c = a.first;
                    uint32_t count = a.second.size();

                    strcpy((char*)data, c.c_str());
                    data += c.length() + 1;
                    uint32_t *t = (uint32_t*)data;
                    *t = count;
                    data += 4;
                    ++t;
                    for (size_t x = 0; x < count; ++x) {
                        uint32_t sframe = a.second[x].first;
                        uint32_t eframe = a.second[x].second;
                        sframe /= stepTime;
                        eframe /= stepTime;
                        *t = sframe;
                        ++t;
                        *t = eframe;
                        ++t;
                    }
                    data += count * 8;
                }
                file->addVariableHeader(commandHeader);
            }
        }
        if (params.xLightsFrm) {
#ifndef LINUX
            POOL_ctx_s* pool = ZSTD_createThreadPool(std::thread::hardware_concurrency());
#else
            void *pool = nullptr;
#endif
            if (params.xLightsFrm) {
                FSEQFile::VariableHeader header;
                header.code[0] = 'X';
                header.code[1] = 'R';
                header.extendedData = true;
                if (!params.xLightsFrm->UnsavedRgbEffectsChanges) {
                    wxFileName effectsFile;
                    effectsFile.AssignDir(params.xLightsFrm->CurrentDir);
                    effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
                    wxFileName fn(effectsFile.GetFullPath());
                    if (FileExists(fn.GetFullPath())) {
                        compressFile(header.data, fn.GetFullPath().ToStdString(), pool);
                    }
                }
                if (header.data.size() == 0) {
                    auto xml = params.xLightsFrm->BuildEffectsXml();
                    wxMemoryOutputStream out;
                    size_t len = xml.length();
                    out.Write(xml.c_str(), len);
                    header.data.resize(out.GetLength());
                    int sz = compressMemoryBuffer(out, &header.data[0], out.GetLength(), pool);
                    header.data.resize(sz);
                }
                file->addVariableHeader(header);
                // printf("XR:   in: %d   out: %d\n", (int)out.GetLength(), sz);
            }
            if (params._outputManager) {
                FSEQFile::VariableHeader header;
                header.code[0] = 'X';
                header.code[1] = 'N';
                header.extendedData = true;
                if (!params.xLightsFrm->UnsavedNetworkChanges) {
                    wxFileName effectsFile;
                    effectsFile.AssignDir(params.xLightsFrm->CurrentDir);
                    effectsFile.SetFullName(_(XLIGHTS_NETWORK_FILE));
                    wxFileName fn(effectsFile.GetFullPath());
                    if (FileExists(fn.GetFullPath())) {
                        compressFile(header.data, fn.GetFullPath().ToStdString(), pool);
                    }
                }
                if (header.data.size() == 0) {
                    pugi::xml_document xmlDoc;
                    params._outputManager->SaveToXML(xmlDoc);
                    std::ostringstream oss;
                    xmlDoc.save(oss);
                    std::string xmlStr = oss.str();
                    wxMemoryOutputStream out;
                    out.Write(xmlStr.data(), xmlStr.size());
                    header.data.resize(out.GetLength());
                    int sz = compressMemoryBuffer(out, &header.data[0], out.GetLength(), pool);
                    header.data.resize(sz);
                }
                file->addVariableHeader(header);
            }
            if (params.xLightsFrm && params.xLightsFrm->CurrentSeqXmlFile) {
                wxMemoryOutputStream out;
                FSEQFile::VariableHeader header;
                header.code[0] = 'X';
                header.code[1] = 'S';
                header.extendedData = true;

                if (params.xLightsFrm->mSavedChangeCount == params.elements->GetChangeCount()) {
                    if (FileExists(params.xLightsFrm->CurrentSeqXmlFile->GetFullPath())) {
                        compressFile(header.data, params.xLightsFrm->CurrentSeqXmlFile->GetFullPath(), pool);
                    }
                }
                if (header.data.size() == 0) {
                    pugi::xml_document doc;
                    params.xLightsFrm->CurrentSeqXmlFile->BuildDocument(doc, *params.elements);
                    std::ostringstream oss;
                    doc.save(oss, "  ");
                    std::string xmlStr = oss.str();
                    out.Write(xmlStr.data(), xmlStr.size());
                    header.data.resize(out.GetLength());
                    int sz = compressMemoryBuffer(out, &header.data[0], out.GetLength(), pool);
                    header.data.resize(sz);
                }
                file->addVariableHeader(header);
                // printf("XS:   in: %d   out: %d\n", (int)out.GetLength(), sz);
            }
#ifndef LINUX
            ZSTD_freeThreadPool(pool);
#endif
        }
    }

    file->writeHeader();
    size_t size = params.seq_data.NumFrames();
    for (int x = 0; x < (int)size; x++) {
        file->addFrame(x, &params.seq_data[x][0]);
    }
    file->finalize();
    delete file;
    spdlog::debug("End fseq write");
}
