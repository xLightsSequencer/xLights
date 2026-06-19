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

// Wx-free core reader for Light-O-Rama Music (.lms) and Animation (.las)
// sequences. Ported from the desktop xLightsFrame::ImportLMS parser
// (src-ui-wx/import_export/ImportEffects.cpp) so the iPad import session can
// consume LMS/LAS sources through the same available-source / mapping flow
// the LOREdit (.loredit) and Vixen3 (.tim) readers use.
//
// The two file extensions share the identical XML schema: a `<channels>`
// element holding `<channel>` / `<rgbChannel>` rows, and a `<timingGrids>`
// element holding `<timingGrid>` rows. Effects live as `<effect>` children of
// a channel with startCentisecond/endCentisecond times plus intensity / type
// attributes.

#include <map>
#include <string>
#include <vector>

#include "Color.h"
#include <pugixml.hpp>

class EffectLayer;

class LORMusic {
public:
    LORMusic(pugi::xml_document& input_xml, int frequency);
    virtual ~LORMusic() = default;

    // Discovery — what the import session surfaces as available sources.
    // `GetChannelNames` returns the de-duplicated plain channel / rgbChannel
    // names (CCR pixel channels are collapsed into `GetCCRNames`). The two
    // lists are disjoint and together form the mappable source set.
    const std::vector<std::string>& GetChannelNames() const { return _channelNames; }
    const std::vector<std::string>& GetCCRNames() const { return _ccrNames; }
    xlColor GetChannelColor(const std::string& name) const;

    // Timing tracks (non-"fixed" timingGrids) and their begin/end mark pairs in
    // milliseconds. `offset` shifts every mark (centisecond units on disk).
    std::vector<std::string> GetTimingTracks() const;
    std::vector<std::pair<uint32_t, uint32_t>> GetTimings(const std::string& timingTrackName, int offset = 0) const;

    // Synthesize the effects for a single mapped channel onto `layer`. Mirrors
    // the desktop MapChannelInformation: resolves the named channel (plain or
    // rgb), then replays MapOnEffects / MapRGBEffects. `color` is the channel's
    // base colour, `chanCountPerNode` the destination model's channels-per-node
    // (drives single- vs multi-colour On palettes). Returns false if the name
    // didn't resolve.
    bool MapChannelEffects(EffectLayer* layer, const std::string& name, const xlColor& color, int chanCountPerNode, bool eraseExisting) const;

    // The desktop CCR-prop mapping fans one rgbChannel-prefix name out across a
    // model's node layers, probing the LOR pixel naming variants ("-P01",
    // " p01", "-P1", " p1", " P 01"). Build the per-node channel name for node
    // index `node` (0-based) under prefix `ccrName`, picking the first variant
    // that exists; returns the default ("-P%02d") form if none match.
    std::string ResolveCCRNodeName(const std::string& ccrName, int node) const;
    bool IsCCRName(const std::string& name) const;

private:
    pugi::xml_document& _input_xml;
    int _frequency = 20;

    std::vector<std::string> _channelNames;
    std::vector<std::string> _ccrNames;
    std::map<std::string, xlColor> _channelColors;
    std::map<std::string, pugi::xml_node> _timingTracks;
    std::vector<std::string> _timingTrackNames;

    void Index();
    bool ChannelExists(const std::string& name) const;
};
