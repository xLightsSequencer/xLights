#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Wx-free core reader for HLS `.hlsIdata` exports. Ported from the desktop
// xLightsFrame::ImportHLS parser (src-ui-wx/import_export/ImportEffects.cpp) so
// the iPad import session can consume HLS sources through the same
// available-source / mapping flow the LORMusic / LOREdit / Vixen3 readers use.
//
// The document is `<HLS_OutputSequence>` holding `NumberOfTimeCells`,
// `MilliSecPerTimeUnit`, and a `<TotalUniverses>` element of `<Universe>` ->
// `<Channels>` -> `<ChannelData>` rows. Each `<ChannelData>` carries a
// `<ChanInfo>` naming the channel (suffix ", Normal" for single-colour or
// ", RGB-R" / ", RGB-G" / ", RGB-B" for an RGB triple) plus
// `<IlluminationData>` blocks of per-frame hex intensity values. The reader
// collapses RGB triples into one source name and synthesizes On / Color Wash
// effects from the decoded per-frame colour stream (the iPad analogue of the
// desktop DoConvertDataRowToEffects path).

#include <map>
#include <string>
#include <vector>

#include "Color.h"
#include <pugixml.hpp>

class EffectLayer;

class HLSFile {
public:
    HLSFile(pugi::xml_document& input_xml);
    virtual ~HLSFile() = default;

    int GetFrames() const { return _frames; }
    int GetFrameTime() const { return _frameTime; }

    // Discovery — flat channel names (RGB triples collapsed to the base name,
    // each colour leg also listed) and the CCR strand-prefix names.
    const std::vector<std::string>& GetChannelNames() const { return _channelNames; }
    const std::vector<std::string>& GetCCRNames() const { return _ccrNames; }
    xlColor GetChannelColor(const std::string& name) const;

    // Synthesize the effects for a single mapped channel onto `layer`. Resolves
    // the named channel (single ", Normal" or an "RGB-R/G/B" triple), decodes
    // the per-frame intensities, and replays the desktop's
    // DoConvertDataRowToEffects On / Color Wash conversion. Returns false if the
    // channel didn't resolve.
    bool MapChannelEffects(EffectLayer* layer, const std::string& name, const xlColor& color, bool eraseExisting) const;

    // Build the per-node channel name for node `node` (1-based) under CCR strand
    // prefix `ccrName`, probing the HLS naming variants ("P%03d", "P%04d",
    // "P%02d", "_%04d", "_%03d"); mirrors desktop FindHLSStrandName.
    std::string FindStrandName(const std::string& ccrName, int node) const;
    bool IsCCRName(const std::string& name) const;

private:
    pugi::xml_document& _input_xml;
    int _frames = 0;
    int _frameTime = 0;
    pugi::xml_node _totalUniverses;

    std::vector<std::string> _channelNames;
    std::vector<std::string> _ccrNames;
    std::map<std::string, xlColor> _channelColors;

    void Index();
    bool ChannelExists(const std::string& name) const;
};
