#ifndef RENDERABLEEFFECT_H
#define RENDERABLEEFFECT_H

#include <string>

class wxPanel;
class wxWindow;
class wxBitmap;
class ModelClass;
class SequenceElements;
class Effect;
class SettingsMap;
class RenderBuffer;

class RenderableEffect
{
    public:
        RenderableEffect(int id, std::string n);
        virtual ~RenderableEffect();
    
        virtual const std::string &Name() const { return name;};
        virtual const wxBitmap &GetEffectIcon(int size, bool exact = true) const;
        virtual int GetId() const { return id; }

        virtual void SetSequenceElements(SequenceElements *els) {mSequenceElements = els;}

        wxPanel *GetPanel(wxWindow *parent);
        virtual void SetDefaultParameters(ModelClass *cls) {}

        virtual bool CanRenderOnBackgroundThread() { return true; }
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) = 0;
    
    
        //return 0 if this is completely drawin the effect background
        //return 1 to have the grid place a normal icon
        //return 2 to have the grid place a smaller icon
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2);


    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) = 0;
        std::string name;
        int id;
        wxPanel *panel;
        SequenceElements *mSequenceElements;
    private:
};

#endif // RENDERABLEEFFECT_H
