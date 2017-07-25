#ifndef GLEDIATOREFFECT_H
#define GLEDIATOREFFECT_H

#include "RenderableEffect.h"
#include <wx/file.h>

class GlediatorReader
{
    std::string _filename;
    wxFile _f;
    wxSize _size;
    size_t _frames;

public:
    GlediatorReader(const std::string& filename, const wxSize& size);
    virtual ~GlediatorReader();
    std::string GetFilename() const { return _filename; }
    void GetFrame(size_t frame, char* buffer, size_t size);
    size_t GetFrameCount() const { return _frames; };
    size_t GetBufferSize() const { return _size.x * _size.y * 3; }
};

class GlediatorEffect : public RenderableEffect
{
    public:
        GlediatorEffect(int id);
        virtual ~GlediatorEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void SetSequenceElements(SequenceElements *els) override;
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> GetFileReferences(const SettingsMap &SettingsMap) override;
        virtual bool needToAdjustSettings(const std::string &version) override { return true; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // GLEDIATOREFFECT_H
