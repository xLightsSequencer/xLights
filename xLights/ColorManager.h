#ifndef COLORMANAGER_H
#define COLORMANAGER_H

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
            NUM_COLORS
        };

         enum ColorCategory
        {
            COLOR_CAT_TIMINGS,
            COLOR_CAT_EFFECT_GRID,
            COLOR_CAT_LAYOUT_TAB
        };

        class ColorDefinition
        {
        public:
            ColorDefinition(enum ColorNames i,
                            const std::string &n,
                            const std::string &dn,
                            const xlColor &c,
                            enum ColorCategory cat) : id(i), name(n), display_name(dn), color(c), category(cat), systemColor(wxSYS_COLOUR_MAX) {}
            ColorDefinition(enum ColorNames i,
                            const std::string &n,
                            const std::string &dn,
                            const xlColor &c,
                            enum ColorCategory cat,
                            enum wxSystemColour sc) : id(i), name(n), display_name(dn), color(c), category(cat), systemColor(sc) {}
            
            enum ColorNames id;
            std::string name;
            std::string display_name;
            xlColor color;
            enum ColorCategory category;
            enum wxSystemColour systemColor;
        };

        ColorManager(xLightsFrame*);
        virtual ~ColorManager();

        static ColorManager* instance();

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
            {COLOR_TIMING1,             "Timing1",           "Timing 1",            xlCYAN,                 COLOR_CAT_TIMINGS},
            {COLOR_TIMING2,             "Timing2",           "Timing 2",            xlRED,                  COLOR_CAT_TIMINGS},
            {COLOR_TIMING3,             "Timing3",           "Timing 3",            xlGREEN,                COLOR_CAT_TIMINGS},
            {COLOR_TIMING4,             "Timing4",           "Timing 4",            xlBLUE,                 COLOR_CAT_TIMINGS},
            {COLOR_TIMING5,             "Timing5",           "Timing 5",            xlYELLOW,               COLOR_CAT_TIMINGS},

            {COLOR_TIMING_DEFAULT,      "TimingDefault",     "Timings",             xlWHITE,                COLOR_CAT_EFFECT_GRID},
            {COLOR_EFFECT_DEFAULT,      "EffectDefault",     "Effects",             xlColor(192,192,192),   COLOR_CAT_EFFECT_GRID},
            {COLOR_EFFECT_SELECTED,     "EffectSelected",    "Effect Selected",     xlColor(204, 102, 255), COLOR_CAT_EFFECT_GRID},
            {COLOR_REFERENCE_EFFECT,    "ReferenceEffect",   "Reference Effect",    xlColor(255,0,255),     COLOR_CAT_EFFECT_GRID},
            {COLOR_ROW_HEADER,          "RowHeader",         "RowHeader",           xlColor(212,208,200),   COLOR_CAT_EFFECT_GRID, wxSYS_COLOUR_BTNFACE},
            {COLOR_ROW_HEADER_TEXT,     "RowHeaderText",     "RowHeader Text",      xlColor(0,0,0),         COLOR_CAT_EFFECT_GRID, wxSYS_COLOUR_BTNTEXT},
            {COLOR_ROW_HEADER_SELECTED, "RowHeaderSelected", "Row Header Selected", xlColor(130,178,207),   COLOR_CAT_EFFECT_GRID},
            {COLOR_GRID_DASHES,         "GridDashes",        "Dashed Select",       xlYELLOW,               COLOR_CAT_EFFECT_GRID},
            {COLOR_GRIDLINES,           "Gridlines",         "Gridlines",           xlColor(40,40,40),      COLOR_CAT_EFFECT_GRID},
            {COLOR_LABELS,              "Labels",            "Labels",              xlColor(255,255,204),   COLOR_CAT_EFFECT_GRID},
            {COLOR_LABEL_OUTLINE,       "LabelOutline",      "Label Outline",       xlColor(103, 103, 103), COLOR_CAT_EFFECT_GRID},
            {COLOR_PHRASES,             "Phrases",           "Phrases",             xlColor(153, 255, 153), COLOR_CAT_EFFECT_GRID},
            {COLOR_WORDS,               "Words",             "Words",               xlColor(255, 218, 145), COLOR_CAT_EFFECT_GRID},
            {COLOR_PHONEMES,            "Phonemes",          "Phonemes",            xlColor(255, 181, 218), COLOR_CAT_EFFECT_GRID},

            {COLOR_MODEL_DEFAULT,       "ModelDefault",      "Model Default",       xlLIGHT_GREY,           COLOR_CAT_LAYOUT_TAB},
            {COLOR_MODEL_SELECTED,      "ModelSelected",     "Model Selected",      xlYELLOW,               COLOR_CAT_LAYOUT_TAB},
            {COLOR_MODEL_OVERLAP,       "ModelOverlap",      "Model Overlap",       xlRED,                  COLOR_CAT_LAYOUT_TAB},
            {COLOR_LAYOUT_DASHES,       "LayoutDashes",      "Dashed Select",       xlYELLOW,               COLOR_CAT_LAYOUT_TAB},
            {COLOR_EFFECT_SELECTED_FIXED,  "EffectSelectedFixed",  "Fixed Timings",          xlColor(255, 128, 0), COLOR_CAT_EFFECT_GRID},
            {COLOR_EFFECT_SELECTED_LOCKED,  "EffectSelectedLocked",  "Locked Effects",          xlColor(200, 0, 0), COLOR_CAT_EFFECT_GRID},
            {COLOR_REFERENCE_EFFECT_LOCKED, "ReferenceEffectLocked", "Locked Reference Effect", xlRED,       COLOR_CAT_EFFECT_GRID},
            {COLOR_WAVEFORM,            "Waveform",           "Waveform",            xlColor(130, 178, 207), COLOR_CAT_EFFECT_GRID},
            {COLOR_WAVEFORM_BACKGROUND, "WaveformBackground", "Waveform Background", xlColor(212, 208, 200), COLOR_CAT_EFFECT_GRID, wxSYS_COLOUR_BTNFACE},
            {COLOR_WAVEFORM_SELECTED,   "WaveformSelected",   "Waveform Selected",   xlColor(0, 0, 200, 45), COLOR_CAT_EFFECT_GRID},
            {COLOR_WAVEFORM_SELECTEDEFFECT,   "WaveformSelectedEffect",   "Waveform Selected Effect",   xlORANGE, COLOR_CAT_EFFECT_GRID}
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

#endif // COLORMANAGER_H
