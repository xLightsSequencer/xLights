#include "GlediatorEffect.h"
#include "GlediatorPanel.h"

#include "../sequencer/SequenceElements.h"

#include <wx/filename.h>

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../xLightsXmlFile.h"
#include "../SequenceCheck.h"

#include "../../include/glediator-16.xpm"
#include "../../include/glediator-64.xpm"

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

    wxString GledFilename = settings.Get("E_TEXTCTRL_Glediator_Filename", "");

    if (GledFilename == "" || !wxFile::Exists(GledFilename))
    {
        res.push_back(wxString::Format("    ERR: Glediator effect cant find glediator file '%s'. Model '%s', Start %s", GledFilename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
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

    SetTextValue(gp->TextCtrl_Glediator_Filename, "");
}

void GlediatorEffect::adjustSettings(const std::string &version, Effect *effect)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect);
    }

    SettingsMap &settings = effect->GetSettings();

    std::string file = settings["E_TEXTCTRL_Glediator_Filename"];
    if (file != "")
    {
        if (!wxFile::Exists(file))
        {
            settings["E_TEXTCTRL_Glediator_Filename"] = xLightsXmlFile::FixFile("", file);
        }
    }
}

std::list<std::string> GlediatorEffect::GetFileReferences(const SettingsMap &SettingsMap)
{
    std::list<std::string> res;
    res.push_back(SettingsMap["E_TEXTCTRL_Glediator_Filename"]);
    return res;
}

void GlediatorEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    wxString GledFilename = SettingsMap["TEXTCTRL_Glediator_Filename"];
    wxFile f;
    //int maxframes=wxAtoi( MaxFrames ); // get max frames the user has passed in
    wxFileOffset fileLength;
    xlColor color;
    wxImage::HSVValue hsv;
    int x,y,i,j,period; // for now hard code matrix to be 32x32. after we get this working, we will prompt for this info during convert
    wxString filename=wxString::Format(_("01 - Carol of the Bells.mp3")); // hard code a mp3 file for now
    size_t readcnt;
    off_t offset;
    int SeqNumPeriods,SeqDataLen,SeqNumChannels;
    wxString suffix,extension,BasePicture,sPicture,NewPictureName,buff;


    if (!wxFileExists(GledFilename)) // if it doesnt exist, just return
    {
        return;
    }

    if (!f.Open(GledFilename.c_str())) // open the *.gled file
    {
        //   PlayerError(_("Unable to load sequence:\n")+FileName);
        return;
    }

    fileLength=f.Length();
    SeqNumChannels=(buffer.BufferWi*3*buffer.BufferHt); // 3072 = 32*32*3
    char *frameBuffer=new char[SeqNumChannels];
    SeqNumPeriods=(int)(fileLength/SeqNumChannels);
    SeqDataLen=fileLength;

    period = buffer.curPeriod - buffer.curEffStartPer;
    offset = period*SeqNumChannels;
    f.Seek(offset, wxFromStart);
    readcnt=f.Read(frameBuffer,SeqNumChannels); // Read one period of channels
    i=0;
    for(j=0; j<readcnt; j+=3)
    {
        // Loop thru all channels
        color.Set(frameBuffer[j],frameBuffer[j+1],frameBuffer[j+2]);
        x=(j%(buffer.BufferWi*3))/3;
        y=(buffer.BufferHt-1) - (j/(buffer.BufferWi*3));
        if(x<buffer.BufferWi && y<buffer.BufferHt && y>= 0)
        {
            buffer.SetPixel(x,y,color);
        }

    }
    delete[] frameBuffer;
}

