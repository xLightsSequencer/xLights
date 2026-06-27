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

// Wx-free core reader for LOR Pixel Editor `.lpe` exports. Ported from the
// desktop xLightsFrame::ImportLPE parser
// (src-ui-wx/import_export/ImportEffects.cpp) so the iPad import session can
// consume LPE sources through the same available-source / mapping flow the
// LORMusic / LOREdit / Vixen3 readers use.
//
// The document is `<...>` holding `<SequenceProps>` / `<ArchivedProps>`
// elements of `<SeqProp>` / `<ArchiveProp>` props. Each prop has a `name`
// (or a nested `<PropClass Name=...>`) and `<track id=N>` children carrying
// `<effect type="pixelEffect" pixelEffect="...">` rows. A pixelEffect string
// is `|`-separated: blend, palette, sparkle, and per-layer/side effect
// descriptors. The reader rebuilds the desktop's per-side (left/right) and
// per-layer (0/1) effect synthesis, translating LPE effect descriptors into
// xLights effect settings + palettes.

#include <map>
#include <string>
#include <vector>

#include "Color.h"
#include <pugixml.hpp>

class EffectLayer;
class Element;

class LORPixelEditor {
public:
    LORPixelEditor(pugi::xml_document& input_xml, int frequency);
    virtual ~LORPixelEditor() = default;

    // Discovery — the prop names offered as mappable source rows.
    const std::vector<std::string>& GetChannelNames() const { return _channelNames; }
    xlColor GetChannelColor(const std::string&) const { return xlBLACK; }

    // Synthesize effects for a mapped prop onto `model`, fanning across the
    // LPE left/right sides and layers 0/1 (each becomes its own effect layer).
    // Mirrors desktop MapLPEEffects. `startLayer` lets the stacked-mapping path
    // append after a separator layer.
    void MapPropEffects(Element* model, const std::string& mapping, bool eraseExisting, int startLayer = 0) const;

    // Synthesize a single side/layer onto a node/strand `layer` — the node-level
    // mapping branch of desktop ImportLPE (always left side). Mirrors MapLPE.
    void MapPropNodeEffects(EffectLayer* layer, const std::string& mapping, int lpeLayer, bool eraseExisting) const;

private:
    pugi::xml_document& _input_xml;
    int _frequency = 20;

    std::vector<std::string> _channelNames;

    void Index();
    bool HasEffects(const std::string& model, int layer, bool left) const;
    void MapLPE(EffectLayer* layer, int lpeLayer, const std::string& model, bool left, bool eraseExisting) const;
};
