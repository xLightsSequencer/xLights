#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <map>
#include <wx/settings.h>
#include "Color.h"

class xLightsFrame;
class wxXmlDocument;
class wxXmlNode;

class ColorManager
{
    public:
        enum ColorNames
        {
            COLOR_TIMING1,
            COLOR_TIMING2,
            COLOR_TIMING3,
            COLOR_TIMING4,
            COLOR_TIMING5,
            COLOR_TIMING_DEFAULT,
            COLOR_EFFECT_DEFAULT,
            COLOR_EFFECT_SELECTED,
            COLOR_REFERENCE_EFFECT,
            COLOR_ROW_HEADER,
            COLOR_ROW_HEADER_TEXT,
            COLOR_ROW_HEADER_SELECTED,
            COLOR_GRID_DASHES,
            COLOR_GRIDLINES,
            COLOR_LABELS,
            COLOR_LABEL_OUTLINE,
            COLOR_PHRASES,
            COLOR_WORDS,
            COLOR_PHONEMES,
            COLOR_MODEL_DEFAULT,
            COLOR_MODEL_SELECTED,
            COLOR_MODEL_OVERLAP,
            COLOR_LAYOUT_DASHES,
            COLOR_EFFECT_SELECTED_FIXED,
            COLOR_EFFECT_SELECTED_LOCKED,
            COLOR_REFERENCE_EFFECT_LOCKED,
            COLOR_WAVEFORM,
            COLOR_WAVEFORM_BACKGROUND,
            COLOR_WAVEFORM_SELECTED,
            COLOR_WAVEFORM_SELECTEDEFFECT,
            COLOR_DISABLED_EFFECT,
            COLOR_LOCKED_EFFECT,
            COLOR_EFFECT_SELECTED_DISABLED,
            COLOR_REFERENCE_EFFECT_DISABLED,
            COLOR_WAVEFORM_MOUSE_MARKER,
            NUM_COLORS
        };

        enum class ColorCategory {
            COLOR_CAT_TIMINGS,
            COLOR_CAT_EFFECT_GRID,
            COLOR_CAT_LAYOUT_TAB
        };

        class ColorDefinition
        {
        public:
            ColorDefinition(ColorManager::ColorNames i,
                            const std::string &n,
                            const std::string &dn,
                            const xlColor &c,
                            ColorManager::ColorCategory cat) : id(i), name(n), display_name(dn), color(c), category(cat), systemColor(wxSYS_COLOUR_MAX) {}
            ColorDefinition(ColorManager::ColorNames i,
                            const std::string &n,
                            const std::string &dn,
                            const xlColor &c,
                            ColorManager::ColorCategory cat,
                            wxSystemColour sc) : id(i), name(n), display_name(dn), color(c), category(cat), systemColor(sc) {}
            
            ColorNames id;
            std::string name;
            std::string display_name;
            xlColor color;
            ColorCategory category;
            wxSystemColour systemColor;
        };

        ColorManager(xLightsFrame*);
        virtual ~ColorManager();

        static ColorManager* instance();

        void SysColorChanged();
        void RefreshColors();
        void ResetDefaults();
        const xlColor GetTimingColor(int colorIndex);
        void SetNewColor(std::string name, xlColor& color);
        xlColor GetColor(ColorNames name);
        const xlColor* GetColorPtr(ColorNames name);

        void Snapshot();
        void RestoreSnapshot();

        void Save(wxXmlDocument* doc);
        void Load(wxXmlNode* colors_node);

        void SetDirty();

        const ColorDefinition xLights_color[NUM_COLORS] =
        {
            { ColorManager::ColorNames::COLOR_TIMING1, "Timing1", "Timing 1", xlCYAN, ColorManager::ColorCategory::COLOR_CAT_TIMINGS },
            { ColorManager::ColorNames::COLOR_TIMING2, "Timing2", "Timing 2", xlRED, ColorManager::ColorCategory::COLOR_CAT_TIMINGS },
            { ColorManager::ColorNames::COLOR_TIMING3, "Timing3", "Timing 3", xlGREEN, ColorManager::ColorCategory::COLOR_CAT_TIMINGS },
            { ColorManager::ColorNames::COLOR_TIMING4, "Timing4", "Timing 4", xlBLUE, ColorManager::ColorCategory::COLOR_CAT_TIMINGS },
            { ColorManager::ColorNames::COLOR_TIMING5, "Timing5", "Timing 5", xlYELLOW, ColorManager::ColorCategory::COLOR_CAT_TIMINGS },

            { ColorManager::ColorNames::COLOR_TIMING_DEFAULT, "TimingDefault", "Timings", xlWHITE, ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_EFFECT_DEFAULT, "EffectDefault", "Effects", xlColor(192, 192, 192), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_EFFECT_SELECTED, "EffectSelected", "Effect Selected", xlColor(204, 102, 255), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_REFERENCE_EFFECT, "ReferenceEffect", "Reference Effect", xlColor(255, 0, 255), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_ROW_HEADER, "RowHeader", "RowHeader", xlColor(212, 208, 200), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID, wxSYS_COLOUR_BTNFACE },
            { ColorManager::ColorNames::COLOR_ROW_HEADER_TEXT, "RowHeaderText", "RowHeader Text", xlColor(0, 0, 0), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID, wxSYS_COLOUR_BTNTEXT },
            { ColorManager::ColorNames::COLOR_ROW_HEADER_SELECTED, "RowHeaderSelected", "Row Header Selected", xlColor(130, 178, 207), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_GRID_DASHES, "GridDashes", "Dashed Select", xlYELLOW, ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_GRIDLINES, "Gridlines", "Gridlines", xlColor(40, 40, 40), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_LABELS, "Labels", "Labels", xlColor(255, 255, 204), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_LABEL_OUTLINE, "LabelOutline", "Label Outline", xlColor(103, 103, 103), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_PHRASES, "Phrases", "Phrases", xlColor(153, 255, 153), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_WORDS, "Words", "Words", xlColor(255, 218, 145), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_PHONEMES, "Phonemes", "Phonemes", xlColor(255, 181, 218), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },

            { ColorManager::ColorNames::COLOR_MODEL_DEFAULT, "ModelDefault", "Model Default", xlLIGHT_GREY, ColorManager::ColorCategory::COLOR_CAT_LAYOUT_TAB },
            { ColorManager::ColorNames::COLOR_MODEL_SELECTED, "ModelSelected", "Model Selected", xlYELLOW, ColorManager::ColorCategory::COLOR_CAT_LAYOUT_TAB },
            { ColorManager::ColorNames::COLOR_MODEL_OVERLAP, "ModelOverlap", "Model Overlap", xlRED, ColorManager::ColorCategory::COLOR_CAT_LAYOUT_TAB },
            { ColorManager::ColorNames::COLOR_LAYOUT_DASHES, "LayoutDashes", "Dashed Select", xlYELLOW, ColorManager::ColorCategory::COLOR_CAT_LAYOUT_TAB },
            { ColorManager::ColorNames::COLOR_EFFECT_SELECTED_FIXED, "EffectSelectedFixed", "Fixed Timings", xlColor(255, 128, 0), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_EFFECT_SELECTED_LOCKED, "EffectSelectedLocked", "Selected Locked Effects", xlColor(200, 128, 152), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_REFERENCE_EFFECT_LOCKED, "ReferenceEffectLocked", "Locked Reference Effect", xlColor(255, 0, 127), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_WAVEFORM, "Waveform", "Waveform", xlColor(130, 178, 207), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_WAVEFORM_BACKGROUND, "WaveformBackground", "Waveform Background", xlColor(212, 208, 200), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID, wxSYS_COLOUR_BTNFACE },
            { ColorManager::ColorNames::COLOR_WAVEFORM_SELECTED, "WaveformSelected", "Waveform Selected", xlColor(0, 0, 200, 45), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_WAVEFORM_SELECTEDEFFECT, "WaveformSelectedEffect", "Waveform Selected Effect", xlORANGE, ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_DISABLED_EFFECT, "DisabledEffect", "Disabled Effects", xlColor(200, 200, 0), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_LOCKED_EFFECT, "LockedEffect", "Locked Effects", xlColor(200, 0, 0), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_EFFECT_SELECTED_DISABLED, "EffectSelectedDisabled", "Selected Disabled Effects", xlColor(200, 200, 64), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },

            { ColorManager::ColorNames::COLOR_REFERENCE_EFFECT_DISABLED, "ReferenceEffectDisabled", "Disabled Reference Effect", xlColor(255, 255, 127), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID },
            { ColorManager::ColorNames::COLOR_WAVEFORM_MOUSE_MARKER, "WaveformMouseMarker", "Waveform Mouse Marker", xlColor(0, 0, 255), ColorManager::ColorCategory::COLOR_CAT_EFFECT_GRID }
        };

    protected:

    private:
        wxXmlNode* Save() const;

        std::map<std::string, xlColor> colors;
        std::map<std::string, xlColor> colors_backup;
        std::map<std::string, xlColor> colors_default;
        std::map<std::string, xlColor> colors_system;
        xLightsFrame* xlights;
        static ColorManager* pInstance;
};
