/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
#include "../xLightsMain.h" 
#include "../UtilFunctions.h"
#include "../ExternalHooks.h"

#include "../../include/glediator-16.xpm"
#include "../../include/glediator-64.xpm"
#include <log4cpp/Category.hh>
#include "../UtilFunctions.h"

GlediatorReader::GlediatorReader(const std::string& filename, const wxSize& size)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _filename = filename;
    _size = size;

    _f.Open(_filename);

    if (_f.IsOpened())
    {
        _frames = (int)(_f.Length() / GetBufferSize());

        if (_frames * GetBufferSize() != _f.Length())
        {
            logger_base.warn("Opening glediator file %s size (%d,%d) looks suspicious as it does not match file size %ld.", (const char *)_filename.c_str(), _size.x, _size.y, (long)_f.Length());
        }
    }
    else
    {
        logger_base.warn("Failed to open file %s", (const char *)_filename.c_str());
    }

}

CSVReader::CSVReader(const std::string& filename)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _line = -1;
    _filename = filename;

    _f.Open(_filename);

    if (_f.IsOpened())
    {

    }
    else
    {
        logger_base.warn("Failed to open file %s", (const char *)_filename.c_str());
    }
}

GlediatorReader::~GlediatorReader()
{
    if (_f.IsOpened())
    {
        _f.Close();
    }
}

CSVReader::~CSVReader()
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

void CSVReader::GetFrame(size_t frame, char* buffer, size_t size)
{
    wxString line = _f.GetLine(frame);

    wxArrayString data = wxSplit(line, ',');

    for (int i = 0; i < std::min(data.size(), size); i++)
    {
        *(buffer + i) = wxAtoi(data[i]);
    }
}

size_t CSVReader::GetFrameCount() const
{
    return _f.GetLineCount();
}

GlediatorEffect::GlediatorEffect(int id) : RenderableEffect(id, "Glediator", glediator_16, glediator_64, glediator_64, glediator_64, glediator_64)
{
    //ctor
}

GlediatorEffect::~GlediatorEffect()
{
    //dtor
}

std::list<std::string> GlediatorEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    wxString GledFilename = settings.Get("E_FILEPICKERCTRL_Glediator_Filename", "");

    if (GledFilename == "" || !FileExists(GledFilename))
    {
        res.push_back(wxString::Format("    ERR: Glediator effect cant find file '%s'. Model '%s', Start %s", GledFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else if (!IsFileInShowDir(xLightsFrame::CurrentDir, GledFilename.ToStdString()))
    {
        res.push_back(wxString::Format("    WARN: Glediator effect file '%s' not under show directory. Model '%s', Start %s", GledFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
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

bool GlediatorEffect::IsGlediatorFile(std::string filename)
{
    wxFileName fn(filename);
    auto ext = fn.GetExt().Lower().ToStdString();

    if (ext == "gled" ||
        ext == "csv" ||
        ext == "out"
        )
    {
        return true;
    }

    return false;
}

bool GlediatorEffect::IsCSVFile(std::string filename) const
{
    wxFileName fn(filename);
    auto ext = fn.GetExt().Lower().ToStdString();

    if (ext == "csv")
    {
        return true;
    }

    return false;
}

xlEffectPanel *GlediatorEffect::CreatePanel(wxWindow *parent) {
    return new GlediatorPanel(parent);
}

void GlediatorEffect::SetDefaultParameters() {
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
        if (!FileExists(file))
        {
            settings["E_FILEPICKERCTRL_Glediator_Filename"] = FixFile("", file);
        }
    }
}

std::list<std::string> GlediatorEffect::GetFileReferences(Model* model, const SettingsMap &SettingsMap) const 
{
    std::list<std::string> res;
    if (SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"] != "") {
        res.push_back(SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"]);
    }
    return res;
}

bool GlediatorEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap &SettingsMap)
{
    bool rc = false;
    wxString file = SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"];
    if (FileExists(file))
    {
        if (!frame->IsInShowFolder(file))
        {
            SettingsMap["E_FILEPICKERCTRL_Glediator_Filename"] = frame->MoveToShowFolder(file, wxString(wxFileName::GetPathSeparator()) + "Glediator");
            rc = true;
        }
    }

    return rc;
}

class GlediatorRenderCache : public EffectRenderCache {
public:
    GlediatorRenderCache()
    {
        _glediatorReader = nullptr;
        _csvReader = nullptr;
        _loops = 0;
        _frameMS = 50.0;
    };
    virtual ~GlediatorRenderCache() {
        if (_glediatorReader != nullptr)
        {
            delete _glediatorReader;
            _glediatorReader = nullptr;
        }
        if (_csvReader != nullptr)
        {
            delete _csvReader;
            _csvReader = nullptr;
        }
    };

    GlediatorReader* _glediatorReader;
    CSVReader* _csvReader;
    int _loops;
    float _frameMS;
};

void GlediatorEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string glediatorFilename = SettingsMap["FILEPICKERCTRL_Glediator_Filename"];
    std::string durationTreatment = SettingsMap["CHOICE_Glediator_DurationTreatment"];

    GlediatorRenderCache *cache = (GlediatorRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new GlediatorRenderCache();
        buffer.infoCache[id] = cache;
    }

    int &_loops = cache->_loops;
    GlediatorReader* &_glediatorReader = cache->_glediatorReader;
    CSVReader* &_csvReader = cache->_csvReader;
    float& _frameMS = cache->_frameMS;

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
        if (_csvReader != nullptr)
        {
            delete _csvReader;
            _csvReader = nullptr;
        }

        if (FileExists(glediatorFilename))
        {
            if (IsCSVFile(glediatorFilename))
            {
                _csvReader = new CSVReader(glediatorFilename);

                if (_csvReader == nullptr)
                {
                    logger_base.warn("GlediatorEffect: Failed to load csv file %s.", (const char *)glediatorFilename.c_str());
                }
                else
                {
                    size_t glediatorFrames = _csvReader->GetFrameCount();
                    if (durationTreatment == "Slow/Accelerate")
                    {
                        size_t effectFrames = buffer.curEffEndPer - buffer.curEffStartPer + 1;
                        float speedFactor = (float)glediatorFrames / (float)effectFrames;
                        _frameMS = ((float)buffer.frameTimeInMs * speedFactor);
                    }
                    logger_base.debug("Glediator effect length: %d, glediator length: %d, duration treatment: %s.",
                        (int)(((float)(buffer.curEffEndPer - buffer.curEffStartPer + 1)) * _frameMS), 
                        (int)((float)glediatorFrames * _frameMS),
                        (const char *)durationTreatment.c_str());
                }
            }
            else
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
                        _frameMS = ((float)buffer.frameTimeInMs * speedFactor);
                    }
                    logger_base.debug("Glediator effect length: %d, glediator length: %d, duration treatment: %s.",
                        (int)(((float)(buffer.curEffEndPer - buffer.curEffStartPer + 1)) * _frameMS), 
                        (int)((float)_glediatorReader->GetFrameCount() * _frameMS),
                        (const char *)durationTreatment.c_str());
                }
            }
        }
        else
        {
            logger_base.warn("GlediatorEffect: Glediator file '%s' not found.", (const char *)glediatorFilename.c_str());
        }
    }

    if (_csvReader != nullptr)
    {
        size_t frameCount = _csvReader->GetFrameCount();
        size_t frame = (float)((buffer.curPeriod - buffer.curEffStartPer) - _loops * frameCount) * _frameMS / (float)(buffer.frameTimeInMs);

        // if we have reached the end and we are to loop
        if (frame >= frameCount && durationTreatment == "Loop")
        {
            // jump back to start and try to read frame again
            _loops++;
            frame = (float)((buffer.curPeriod - buffer.curEffStartPer) - _loops * frameCount) * _frameMS / (float)buffer.frameTimeInMs;
            logger_base.debug("Glediator effect loop #%d at frame %d.", _loops, buffer.curPeriod - buffer.curEffStartPer);
        }

        if (frame >= frameCount)
        {
            // do nothing
        }
        else
        {
            size_t bufsize = buffer.BufferWi * buffer.BufferHt;
            char *frameBuffer = new char[bufsize];
            memset(frameBuffer, 0x00, bufsize);

            if (frameBuffer != nullptr)
            {
                _csvReader->GetFrame(frame, frameBuffer, bufsize);
                xlColor color;

                for (size_t j = 0; j < bufsize; j++)
                {
                    // Loop thru all channels
                    color.Set(frameBuffer[j], frameBuffer[j], frameBuffer[j]);
                    int x = j % buffer.BufferWi;
                    int y = (buffer.BufferHt - 1) - (j / buffer.BufferWi);
                    if (x < buffer.BufferWi && y < buffer.BufferHt && y >= 0)
                    {
                        buffer.SetPixel(x, y, color);
                    }
                }

                delete[] frameBuffer;
            }
        }
    }
    else if (_glediatorReader != nullptr)
    {
        bool rendered = false;

        size_t frame = (float)((buffer.curPeriod - buffer.curEffStartPer) - _loops * (_glediatorReader->GetFrameCount())) * _frameMS / (float)buffer.frameTimeInMs;

        // if we have reached the end and we are to loop
        if (frame >= _glediatorReader->GetFrameCount() && durationTreatment == "Loop")
        {
            // jump back to start and try to read frame again
            _loops++;
            frame = (float)((buffer.curPeriod - buffer.curEffStartPer) - _loops * (_glediatorReader->GetFrameCount())) * _frameMS / (float)buffer.frameTimeInMs;
            logger_base.debug("Glediator effect loop #%d at frame %d.", _loops, buffer.curPeriod - buffer.curEffStartPer);
        }

        if (frame >= _glediatorReader->GetFrameCount())
        {
            for (int y = 0; y < buffer.BufferHt; y++)
            {
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y, xlBLACK);
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
}

