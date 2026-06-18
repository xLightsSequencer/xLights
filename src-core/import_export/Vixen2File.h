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

// Wx-free core reader for Vixen 2.x `.vix` exports. Ported from the desktop
// xLightsFrame::ImportVix parser (src-ui-wx/import_export/ImportEffects.cpp) so
// the iPad import session can consume `.vix` sources through the same
// available-source / mapping flow the HLS / LORMusic readers use.
//
// A `.vix` document holds `EventPeriodInMilliseconds`, `Time` (total ms), a
// channel list (either an inline `<Channels>`/`<Channel>` list with name+color
// attributes, or a `<Profile>` element naming a sibling `<name>.pro` profile
// whose `<ChannelObjects>` carry the channel names + colors), and an
// `<EventValues>` element of base64-encoded per-channel-per-frame intensities
// laid out as `data[frame + numFrames * channel]`. RGB triples are collapsed
// to a single source name by the `Red`/`Green`/`Blue` (or `-R`/`-G`/`-B`)
// suffix convention, matching desktop CheckForVixenRGB.

#include <map>
#include <string>
#include <vector>

#include "Color.h"
#include <pugixml.hpp>

class EffectLayer;

class Vixen2File {
public:
    // `input_xml` is the parsed `.vix`. `profileDir` is searched (alongside the
    // `.vix`'s own directory) for the referenced `<name>.pro` profile.
    Vixen2File(pugi::xml_document& input_xml, const std::string& vixDir, const std::string& profileDir);
    virtual ~Vixen2File() = default;

    bool IsValid() const { return _valid; }
    int GetFrames() const { return _frames; }
    int GetFrameTime() const { return _frameTime; }

    // Discovery — flat channel names. RGB triples are listed once under the
    // collapsed base name (legs are not surfaced individually).
    const std::vector<std::string>& GetChannelNames() const { return _channelNames; }
    xlColor GetChannelColor(const std::string& name) const;

    // Synthesize effects for a mapped channel onto `layer`. Resolves the named
    // channel (single channel or an RGB triple), reads its per-frame
    // intensities from the decoded event stream, and replays the shared
    // ConvertDataRowToEffects On / Color Wash conversion. Returns false if the
    // channel didn't resolve.
    bool MapChannelEffects(EffectLayer* layer, const std::string& name, const xlColor& color, bool eraseExisting) const;

private:
    bool _valid = false;
    int _frames = 0;
    int _frameTime = 50;

    std::vector<std::string> _channelNames;  // discovery names (RGB collapsed)
    std::vector<std::string> _rawChannels;    // raw channel order in the event stream
    std::map<std::string, xlColor> _channelColors;
    std::vector<unsigned char> _eventData;

    bool LoadProfile(const std::string& profileName, const std::string& vixDir, const std::string& profileDir,
                     std::vector<std::string>& names, std::vector<xlColor>& colors) const;
    void RegisterChannel(const std::string& name, const xlColor& color);
    int RawChannelIndex(const std::string& name) const;
};
