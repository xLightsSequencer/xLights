#ifndef GLEDIATOREFFECT_H
#define GLEDIATOREFFECT_H

#include "RenderableEffect.h"
#include <wx/file.h>
#include <wx/textfile.h>

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

// CSV Files have one line per frame.
// On each line is a list of comma separated values
// Each value is between 0 and 255
// Each value is applied to create a shade of white r=g=b which is then applied to a node
// Multiple values are applied to multiple nodes
class CSVReader
{
    std::string _filename;
    wxTextFile _f;
    int _line;

public:
    CSVReader(const std::string& filename);
    virtual ~CSVReader();
    std::string GetFilename() const { return _filename; }
    void GetFrame(size_t frame, char* buffer, size_t size);
    size_t GetFrameCount() const;
};

class GlediatorEffect : public RenderableEffect
{
    public:
        GlediatorEffect(int id);
        virtual ~GlediatorEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void SetSequenceElements(SequenceElements *els) override;
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> GetFileReferences(const SettingsMap &SettingsMap) override;
        virtual bool needToAdjustSettings(const std::string &version) override { return true; }
        virtual bool AppropriateOnNodes() const override { return false; }
        static bool IsGlediatorFile(std::string filename);
    
        // Currently not possible but I think changes could be made to make it support partial
        //virtual bool CanRenderPartialTimeInterval() const override { return true; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        bool IsCSVFile(std::string filename) const;
};

#endif // GLEDIATOREFFECT_H
