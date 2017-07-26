#ifndef RENDERABLEEFFECT_H
#define RENDERABLEEFFECT_H

#include <wx/bitmap.h>
#include <string>
#include "../Color.h"
#include "assist/AssistPanel.h"

class wxPanel;
class wxWindow;
class wxBitmap;
class Model;
class SequenceElements;
class Effect;
class SettingsMap;
class RenderBuffer;
class wxSlider;
class wxCheckBox;
class AudioManager;
class wxSpinCtrl;

class RenderableEffect
{
    public:
        RenderableEffect(int id,
                         std::string n,
                         const char **data16,
                         const char **data24,
                         const char **data32,
                         const char **data48,
                         const char **data64);
        virtual ~RenderableEffect();

        virtual const std::string &Name() const { return name;};

        virtual const std::string &ToolTip() const { return tooltip;};

        virtual const wxBitmap &GetEffectIcon(int size, bool exact = false) const;
        virtual int GetId() const { return id; }
        virtual int GetColorSupportedCount() const { return -1; } // -1 is no limit
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) { return false; }
        virtual bool SupportsRadialColorCurves(const SettingsMap &SettingsMap) { return false; }
        virtual std::list<std::string> GetFileReferences(const SettingsMap &SettingsMap) { return std::list<std::string>(); }

        virtual void SetSequenceElements(SequenceElements *els) {mSequenceElements = els;}

        wxPanel *GetPanel(wxWindow *parent);
		virtual void SetDefaultParameters(Model *cls) {}
		virtual void SetPanelStatus(Model *cls) {}
		virtual std::string GetEffectString();

        //Methods for rendering the effect
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) { return true; }
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) = 0;
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect *effect) { }
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) { std::list<std::string> res; return res; };

        virtual bool CanBeRandom() {return true;}

        //return 0 if this is completely drawin the effect background
        //return 1 to have the grid place a normal icon
        //return 2 to have the grid place a smaller icon
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                         DrawGLUtils::xlVertexColorAccumulator &backgrounds, xlColor* colorMask, bool ramps);

        virtual bool needToAdjustSettings(const std::string &version);
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true);

        virtual AssistPanel *GetAssistPanel(wxWindow *parent, xLightsFrame* xl_frame);
        virtual bool HasAssistPanel() { return false; }

    protected:
        static void SetSliderValue(wxSlider *slider, int value);
        static void SetSpinValue(wxSpinCtrl *spin, int value);
        static void SetChoiceValue(wxChoice* choice, std::string value);
        static void SetRadioValue(wxRadioButton* radio);
        static void SetTextValue(wxTextCtrl* choice, std::string value);
        static void SetCheckBoxValue(wxCheckBox *w, bool b);

        double GetValueCurveDouble(const std::string & name, double def, const SettingsMap &SettingsMap, float offset);
        int GetValueCurveInt(const std::string &name, int def, const SettingsMap &SettingsMap, float offset);
        bool IsVersionOlder(const std::string& compare, const std::string& version);
        void AdjustSettingsToBeFitToTime(int effectIdx, SettingsMap &settings, int startMS, int endMS, xlColorVector &colors);
        virtual void RemoveDefaults(const std::string &version, Effect *effect);

        void initBitmaps(const char **data16,
                         const char **data24,
                         const char **data32,
                         const char **data48,
                         const char **data64);

        virtual wxPanel *CreatePanel(wxWindow *parent) = 0;
        std::string name;
        std::string tooltip;
        int id;
        wxPanel *panel;
        SequenceElements *mSequenceElements;

        wxBitmap icon16;
        wxBitmap icon24;
        wxBitmap icon32;
        wxBitmap icon48;
        wxBitmap icon64;

#ifdef __WXOSX__
        wxBitmap icon16e;
        wxBitmap icon24e;
        wxBitmap icon32e;
#endif
    private:
};

#endif // RENDERABLEEFFECT_H
