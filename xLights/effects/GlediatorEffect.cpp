#include "GlediatorEffect.h"
#include "GlediatorPanel.h"

#include "../sequencer/SequenceElements.h"

#include <wx/filename.h>
#include <wx/filepicker.h>

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../xLightsXmlFile.h"
#include "../SequenceCheck.h"

#include "../../include/glediator-16.xpm"
#include "../../include/glediator-64.xpm"
#include <log4cpp/Category.hh>
#include "../UtilFunctions.h"

GlediatorReader::GlediatorReader(const std::string& filename, const wxSize& size)
{
    _filename = filename;
    _size = size;

    _f.Open(_filename);

    if (_f.IsOpened())
    {
        _frames = (int)(_f.Length() / GetBufferSize());

        if (_frames * GetBufferSize() != _f.Length())
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("Opening glediator file %s size (%d,%d) looks suspicious as it does not match file size %ld.", (const char *)_filename.c_str(), _size.x, _size.y, (long)_f.Length());
        }
    }
}

GlediatorReader::~GlediatorReader()
{
    if (_f.IsOpened())
    {
        _f.Close();
    }
}

void GlediatorReader::GetFrame(size_t frame, char* buffer, size_t size)
{
    if (size != GetBufferSize() || frame >= GetFrameCount())
    {
        // invalid sized buffer ... so fill it with red
        // or illegal frame
        for (size_t i = 0; i < size; i++)
        {
            if (i%3 == 0)
            {
                *(buffer + i) = (char)0xFF;
            }
            else
            {
                *(buffer + i) = (char)0x00;
            }
        }
    }
    else
    {
        size_t offset = frame * GetBufferSize();
        _f.Seek(offset, wxFromStart);
        size_t readcnt = _f.Read(buffer, size); // Read one period of channels

        wxASSERT(readcnt == size);
    }
}

GlediatorEffect::GlediatorEffect(int id) : RenderableEffect(id, "Glediator", glediator_16, glediator_64, glediator_64, glediator_64, glediator_64)
{
    //ctor
}

GlediatorEffect::~GlediatorEffect()
{
    //dtor
}

std::list<std::string> GlediatorEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    wxString GledFilename = settings.Get("E_FILEPICKERCTRL_Glediator_Filename", "");

    if (GledFilename == "" || !wxFile::Exists(GledFilename))
    {
        res.push_back(wxString::Format("    ERR: Glediator effect cant find glediator file '%s'. Model '%s', Start %s", GledFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else if (!IsFileInShowDir(xLightsFrame::CurrentDir, GledFilename.ToStdString()))
    {
        res.push_back(wxString::Format("    WARN: Glediator effect glediator file '%s' not under show directory. Model '%s', Start %s", GledFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

void GlediatorEffect::SetSequenceElements(SequenceElements *els) {
    mSequenceElements = els;
    if (panel == nullptr) {
        return;
    }
    wxFileName fn(els->GetFileName());

    ((GlediatorPanel*)panel)->defaultDir = fn.GetPath();
}

wxPanel *GlediatorEffect::CreatePanel(wxWindow *parent) {
    return new GlediatorPanel(parent);
}

void GlediatorEffect::SetDefaultParameters(Model *cls) {
    GlediatorPanel *gp = (GlediatorPanel*)panel;
    if (gp == nullptr) {
        return;
    }

    gp->FilePickerCtrl_Glediator_Filename->SetFileName(wxFileName());
    SetChoiceValue(gp->Choice_Glediator_DurationTreatment, "Normal");
}

void GlediatorEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap &settings = effect->GetSettings();

    std::string file = settings.Get("E_TEXTCTRL_Glediator_Filename", "");

    if (file != "")
    {
        settings.erase("E_TEXTCTRL_Glediator_Filename");
        settings["E_FILEPICKERCTRL_Glediator_Filename"] = file;
    }

    if (file != "")
    {
        if (!wxFile::Exists(file))
        {
            settings["E_FILEPICKERCTRL_Glediator_Filename"] = FixFile("", file);
        }
    }
}

std::list<std::string> GlediatorEffect::GetFileReferences(const SettingsMap &SettingsMap)
{
    std::list<std::string> res;
    res.push_back(SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"]);
    return res;
}

class GlediatorRenderCache : public EffectRenderCache {
public:
    GlediatorRenderCache()
    {
        _glediatorReader = nullptr;
        _loops = 0;
        _frameMS = 50;
    };
    virtual ~GlediatorRenderCache() {
        if (_glediatorReader != nullptr)
        {
            delete _glediatorReader;
            _glediatorReader = nullptr;
        }
    };

    GlediatorReader* _glediatorReader;
    int _loops;
    int _frameMS;
};

void GlediatorEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string glediatorFilename = SettingsMap["FILEPICKERCTRL_Glediator_Filename"];
    std::string durationTreatment = SettingsMap["CHOICE_Glediator_DurationTreatment"];

    bool rendered = false;

    GlediatorRenderCache *cache = (GlediatorRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new GlediatorRenderCache();
        buffer.infoCache[id] = cache;
    }

    int &_loops = cache->_loops;
    GlediatorReader* &_glediatorReader = cache->_glediatorReader;
    int& _frameMS = cache->_frameMS;

    if (buffer.needToInit)
    {
        buffer.needToInit = false;

        _loops = 0;
        _frameMS = buffer.frameTimeInMs;
        if (_glediatorReader != nullptr)
        {
            delete _glediatorReader;
            _glediatorReader = nullptr;
        }

        if (wxFileExists(glediatorFilename))
        {
            // have to open the file
            _glediatorReader = new GlediatorReader(glediatorFilename, wxSize(buffer.BufferWi, buffer.BufferHt));

            if (_glediatorReader == nullptr)
            {
                logger_base.warn("GlediatorEffect: Failed to load glediator file %s.", (const char *)glediatorFilename.c_str());
            }
            else
            {
                if (durationTreatment == "Slow/Accelerate")
                {
                    size_t effectFrames = buffer.curEffEndPer - buffer.curEffStartPer + 1;
                    size_t glediatorFrames = _glediatorReader->GetFrameCount();
                    float speedFactor = (float)glediatorFrames / (float)effectFrames;
                    _frameMS = (int)((float)buffer.frameTimeInMs * speedFactor);
                }
                logger_base.debug("Glediator effect length: %d, glediator length: %d, duration treatment: %s.",
                    (buffer.curEffEndPer - buffer.curEffStartPer + 1) * _frameMS, _glediatorReader->GetFrameCount() * _frameMS,
                    (const char *)durationTreatment.c_str());
            }
        }
        else
        {
            logger_base.warn("GlediatorEffect: Glediator file '%s' not found.", (const char *)glediatorFilename.c_str());
        }
    }

    if (_glediatorReader != nullptr)
    {
        size_t frame = ((buffer.curPeriod - buffer.curEffStartPer) - _loops * (_glediatorReader->GetFrameCount())) * _frameMS / buffer.frameTimeInMs;

        // if we have reached the end and we are to loop
        if (frame >= _glediatorReader->GetFrameCount() && durationTreatment == "Loop")
        {
            // jump back to start and try to read frame again
            _loops++;
            frame = ((buffer.curPeriod - buffer.curEffStartPer) - _loops * (_glediatorReader->GetFrameCount())) * _frameMS / buffer.frameTimeInMs;
            logger_base.debug("Glediator effect loop #%d at frame %d.", _loops, buffer.curPeriod - buffer.curEffStartPer);
        }

        if (frame >= _glediatorReader->GetFrameCount())
        {
            for (int y = 0; y < buffer.BufferHt; y++)
            {
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y, xlBLUE);
                }
            }
            rendered = true;
        }
        else
        {
            size_t bufsize = _glediatorReader->GetBufferSize();
            char *frameBuffer = new char[bufsize];

            if (frameBuffer != nullptr)
            {
                _glediatorReader->GetFrame(frame, frameBuffer, bufsize);
                xlColor color;

                for (size_t j = 0; j < bufsize; j += 3)
                {
                    // Loop thru all channels
                    color.Set(frameBuffer[j], frameBuffer[j + 1], frameBuffer[j + 2]);
                    int x = (j % (buffer.BufferWi * 3)) / 3;
                    int y = (buffer.BufferHt - 1) - (j / (buffer.BufferWi * 3));
                    if (x < buffer.BufferWi && y < buffer.BufferHt && y >= 0)
                    {
                        buffer.SetPixel(x, y, color);
                    }
                }

                delete[] frameBuffer;
                rendered = true;
            }
        }
    }

    // display a red background to show we have a problem
    if (!rendered)
    {
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            for (int x = 0; x < buffer.BufferWi; x++)
            {
                buffer.SetPixel(x, y, xlRED);
            }
        }
    }
}

