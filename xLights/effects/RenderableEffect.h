#ifndef RENDERABLEEFFECT_H
#define RENDERABLEEFFECT_H

#include <string>

class wxPanel;
class wxWindow;
class wxBitmap;

class RenderableEffect
{
    public:
        RenderableEffect(int id, std::string n);
        virtual ~RenderableEffect();
    
        virtual const std::string &Name() const { return name;};
        virtual const wxBitmap &GetEffectIcon(int size, bool exact = true) const;
        virtual int GetId() const { return id; }

        virtual wxPanel *CreatePanel(wxWindow *parent) = 0;
        //virtual void Render() = 0;

    protected:
        std::string name;
        int id;
    private:
};

#endif // RENDERABLEEFFECT_H
