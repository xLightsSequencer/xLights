#include "FacesEffect.h"
#include "FacesPanel.h"
#include "../models/Model.h"
#include "../sequencer/SequenceElements.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../SequenceCheck.h"
#include "../xLightsMain.h" 
#include "PicturesEffect.h"

#include <list>

#include "../../include/corofaces.xpm"

#include <wx/tokenzr.h>

FacesEffect::FacesEffect(int id) : RenderableEffect(id, "Faces", corofaces, corofaces, corofaces, corofaces, corofaces)
{
    //ctor
}

FacesEffect::~FacesEffect()
{
    //dtor
}

std::list<std::string> FacesEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    wxString definition = settings.Get("E_CHOICE_Faces_FaceDefinition", "");
    if (definition == "Default" && !model->faceInfo.empty() && model->faceInfo.begin()->first != "") {
        definition = model->faceInfo.begin()->first;
    }
    bool found = true;
    std::map<std::string, std::map<std::string, std::string> >::iterator it = model->faceInfo.find(definition.ToStdString());
    if (it == model->faceInfo.end()) {
        //not found
        found = false;
    }
    if (!found) {
        if ("Coro" == definition && model->faceInfo.find("SingleNode") != model->faceInfo.end()) {
            definition = "SingleNode";
            found = true;
        }
        else if ("SingleNode" == definition && model->faceInfo.find("Coro") != model->faceInfo.end()) {
            definition = "Coro";
            found = true;
        }
    }

    wxString modelType = found ? wxString(model->faceInfo[definition.ToStdString()]["Type"].c_str()) : definition;
    if (modelType == "") {
        modelType = definition;
    }

    if (modelType != "Matrix" && modelType != "Rendered")
    {
        // -Buffer not rotated
        wxString bufferTransform = settings.Get("B_CHOICE_BufferTransform", "None");

        if (bufferTransform != "None")
        {
            res.push_back(wxString::Format("    WARN: Face effect with transformed buffer '%s' may not render correctly. Model '%s', Start %s", model->GetName(), bufferTransform, FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        if (settings.GetInt("B_SLIDER_Rotation", 0) != 0 || 
            settings.GetInt("B_SLIDER_Rotations", 0) != 0 ||
            settings.GetInt("B_SLIDER_Zoom",1) != 1 || 
            settings.Get("B_VALUECURVE_Rotation", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_Rotations", "").find("Active=TRUE") != std::string::npos ||
            settings.Get("B_VALUECURVE_Zoom", "").find("Active=TRUE") != std::string::npos
            )
        {
            res.push_back(wxString::Format("    WARN: Face effect with rotozoom active '%s' may not render correctly. Model '%s', Start %s", model->GetName(), bufferTransform, FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    if (modelType == "Matrix")
    {
        auto images = model->faceInfo[definition.ToStdString()];
        for (auto it2 = images.begin(); it2 != images.end(); ++it2)
        {
            if ((*it2).first.find("Mouth") == 0)
            {
                std::string picture = (*it2).second;

                if (picture != "")
                {
                    if (!wxFileExists(picture))
                    {
                        res.push_back(wxString::Format("    ERR: Face effect image file not found '%s'. Model '%s', Start %s", picture, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
                    }
                }
            }
        }
    }

    wxString timing = settings.Get("E_CHOICE_Faces_TimingTrack", "");
    wxString phoneme = settings.Get("E_CHOICE_Faces_Phoneme", "");

    // - Face chosen or specific phoneme
    if (phoneme == "" && timing == "")
    {
        res.push_back(wxString::Format("    ERR: Face effect with no timing selected. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

void FacesEffect::SetPanelStatus(Model *cls) {
    FacesPanel *fp = (FacesPanel*)panel;
    if (fp == nullptr) {
        return;
    }
    fp->Choice_Faces_TimingTrack->Clear();
    fp->Face_FaceDefinitonChoice->Clear();
    if (mSequenceElements == nullptr) {
        return;
    }
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
        if (mSequenceElements->GetElement(i)->GetEffectLayerCount() == 3
            && mSequenceElements->GetElement(i)->GetType() == ELEMENT_TYPE_TIMING) {
            fp->Choice_Faces_TimingTrack->Append(mSequenceElements->GetElement(i)->GetName());
        }
    }
    bool addRender = true;
    if (cls != nullptr) {
        for (std::map<std::string, std::map<std::string, std::string> >::iterator it = cls->faceInfo.begin(); it != cls->faceInfo.end(); it++) {
            if (it->first != "")
            {
                fp->Face_FaceDefinitonChoice->Append(it->first);
                if (it->second["Type"] == "Coro" || it->second["Type"] == "SingleNode" || it->second["Type"] == "NodeRange") {
                    addRender = false;
                }
            }
        }
    }
    if (fp->Face_FaceDefinitonChoice->GetCount() == 0) {
        fp->Face_FaceDefinitonChoice->Append("Default");
        addRender = false;
    }
    if (addRender) {
        fp->Face_FaceDefinitonChoice->Append("Rendered");
    }
    fp->Face_FaceDefinitonChoice->SetSelection(0);
}

wxPanel *FacesEffect::CreatePanel(wxWindow *parent) {
    return new FacesPanel(parent);
}

void FacesEffect::SetDefaultParameters(Model *cls) {
    FacesPanel *fp = (FacesPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    SetRadioValue(fp->RadioButton1);
    SetChoiceValue(fp->Choice_Faces_Phoneme, "AI");
    SetChoiceValue(fp->Choice_Faces_Eyes, "Auto");

    if (fp->Face_FaceDefinitonChoice->GetCount() > 0)
    {
        fp->Face_FaceDefinitonChoice->SetSelection(0);
    }

    SetCheckBoxValue(fp->CheckBox_Faces_Outline, false);
}

void FacesEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_Faces_TimingTrack", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_Faces_TimingTrack"] = wxString(newname);
    }
}

void FacesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    if (SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default") == "Rendered"
        && SettingsMap.Get("CHECKBOX_Faces_Outline", "") == "") {
        //3.x style Faces effect
        RenderFaces(buffer, SettingsMap["CHOICE_Faces_Phoneme"], "Auto", true);
    } else if (SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default") == XLIGHTS_PGOFACES_FILE) {
        RenderCoroFacesFromPGO(buffer,
                               SettingsMap["CHOICE_Faces_Phoneme"],
                               SettingsMap.Get("CHOICE_Faces_Eyes", "Auto"),
                               SettingsMap.GetBool("CHECKBOX_Faces_Outline"));
    } else {
        RenderFaces(buffer,
                    effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
                    SettingsMap.Get("CHOICE_Faces_FaceDefinition", "Default"),
                    SettingsMap["CHOICE_Faces_Phoneme"],
                    SettingsMap["CHOICE_Faces_TimingTrack"],
                    SettingsMap["CHOICE_Faces_Eyes"],
                    SettingsMap.GetBool("CHECKBOX_Faces_Outline"));
    }
}


void FacesEffect::RenderFaces(RenderBuffer &buffer, const std::string &Phoneme, const std::string &eyes, bool outline)
{
    static const std::map<wxString, int> phonemeMap = {
        {"AI", 0},
        {"E", 1},
        {"FV", 2},
        {"L", 3},
        {"MBP", 4},
        {"O", 5},
        {"U", 6},
        {"WQ", 7},
        {"etc", 8},
        {"rest", 9},
    };
    wxImage::HSVValue hsv;
    std::map<wxString, int>::const_iterator it = phonemeMap.find(Phoneme);
    int PhonemeInt = 0;
    if (it != phonemeMap.end()) {
        PhonemeInt = it->second;
    }

    //    std::vector<int> chmap;
    //    std::vector<std::vector<int>> chmap; //array of arrays
    //    chmap.resize(BufferHt * BufferWi,0);
    //    Model mc;
    //    mc.GetChannelCoords(chmap, true); //method is on Model object



    //    wxString html = "<html><body><table border=0>";
    int Ht, Wt;
    Ht = buffer.BufferHt;
    Wt = buffer.BufferWi;
    //    int mode; // 1=auto, 2=coroface, 3=picture,4=movie;

    drawoutline(buffer, PhonemeInt, outline, eyes, buffer.BufferHt, buffer.BufferWi);
    mouth(buffer, PhonemeInt, Ht,  Wt); // draw a mouth syllable



    //size_t NodeCount=GetNodeCount();

    //    above is from Model::ChannelLayoutHtml()
#if 0 //sample code for Sean
    std::vector<std::vector<int>> face_channels;
    wxString model_name = "(change this)";
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if (model_name.CmpNoCase((*it)->name)) continue; //don't check this model
        wxSize wh = (*it)->GetChannelCoords(face_channels, true);
        //        debug(1, "model '%s' is %d x %d, channel[0,0] = %d, ...", (const char*)(*it)->name.c_str(), wh.x, wh.y, face_channels[0][0]);
        break;
    }
#endif // 1
#if 0 //DEBUG
    //get list of models:
    wxString buf;
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
        buf += ", " + (*it)->name; //ModelPtr*
    debug(1, "faces: models = %s", (CmpNoCaseconst char*)buf + 2);

    //get info about one of the models:
    buf = xLightsFrame::PreviewModels[0]->name;
    debug(1, "first model is %s", (const char*)buf);
    buf = xLightsFrame::PreviewModels[0]->ChannelLayoutHtml();
    if (buf.size() > 500) buf.resize(500);
    debug(1, "first 500 char of layout html = %s", (const char*)buf);
#endif
}

//TODO: add params for eyes, outline
void FacesEffect::mouth(RenderBuffer &buffer, int Phoneme,int BufferHt, int BufferWi)
{
    /*
     FacesPhoneme.Add("AI");     0
     FacesPhoneme.Add("E");      1
     FacesPhoneme.Add("FV");     2
     FacesPhoneme.Add("L");      3
     FacesPhoneme.Add("MBP");    4
     FacesPhoneme.Add("O");      5
     FacesPhoneme.Add("U");      6
     FacesPhoneme.Add("WQ");     7
     FacesPhoneme.Add("etc");    8
     FacesPhoneme.Add("rest");   9

     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     ...............|...............
     -1-------------+-------------1-         5% and 95% in x, 52% in y
     .1.............|.............1.         55% in y
     .11111111111111111111111111111.         58% in y
     .2..........55.|.55..........2.
     .2........55...|...55........2.
     .2......55.....|.....55......2.
     .2....55.......|.......55....2.
     .2..55.........|.........55..2.
     .255...........|...........552.
     .2.............|.............2.      y 81%
     .2.............|.............2.
     .255...........|...........552.
     .2..55.........|.........55..2.
     .2....55.......|.......55....2.
     .2......55.....|.....55......2.
     .2........55...|...55........2.
     ............55.|.55............
     */

    int xc,yc;
    double radius,offset=0.0;
    int Ht = BufferHt-1;
    int Wt = BufferWi-1;
    int x1=(int)(offset + Wt*0.25);
    int x2=(int)(offset + Wt*0.75);
    int x3=(int)(offset + Wt*0.30);
    int x4=(int)(offset + Wt*0.70);

    int y1=(int)(offset + Ht*0.48);
    int y2=(int)(offset + Ht*0.40);
    int y3=(int)(offset + Ht*0.25);
    int y4=(int)(offset + Ht*0.20);
    int y5=(int)(offset + Ht*0.30);

    // eyes
    switch (Phoneme)
    {
        case 0:         // AI
            drawline1(buffer,  Phoneme, x1, x2, y1,y2);
            drawline1(buffer,  Phoneme, x1, x2, y1,y4);
            break;
        case 3:
        case 1:       // E, L
            drawline1(buffer,  Phoneme, x1, x2, y1,y2);
            drawline1(buffer,  Phoneme, x1, x2, y1,y3);
            break;
        case 2:       // FV
            drawline1(buffer,  Phoneme, x1, x2, y1,y2);
            drawline1(buffer,  Phoneme, x1, x2, y1,y2-1);
            break;
        case 4:
        case 9:     //  MBP,rest

            drawline1(buffer, Phoneme, x1, x2, y1,y2);
            break;
        case 5:
        case 6:       // O,U,WQ
        case 7:
            xc = (int)(0.5 + Wt*0.50);
            yc = (int) (y2-y5)/2 + y5;
            radius = Wt*0.15;  // O
            if(Phoneme==6) radius = Wt*0.10;  // U
            if(Phoneme==7) radius = Wt*0.05;  // WQ
            facesCircle(buffer, Phoneme,xc, yc, radius,0,360);
            break;
        case 8:       // WQ, etc
            drawline3(buffer, Phoneme, x3, x4, y5, y2);
            break;

    }
}

void FacesEffect::drawline1(RenderBuffer &buffer, int Phoneme, int x1,int x2,int y1,int y2)
{
    HSVValue hsv;
    int ColorIdx,x=0,y=0;
    size_t colorcnt=buffer.GetColorCount();


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
    hsv.hue = (float)Phoneme/10.0;
    //int ch=FindChannelAtXY( x,  y,'A');
    hsv.value=1.0;


    for(x=x1+1; x<x2; x++)
    {
        buffer.SetPixel(x,y2,hsv); // Turn pixel on
    }
    for(y=y2+1; y<=y1; y++)
    {
        buffer.SetPixel(x1,y,hsv); // Left side of mouyh
        buffer.SetPixel(x2,y,hsv); // rightside
    }
}

void FacesEffect::drawline3(RenderBuffer &buffer, int Phoneme, int x1,int x2,int y6,int y7)
{
    HSVValue hsv;

    int ColorIdx,x,y;
    size_t colorcnt=buffer.GetColorCount();


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    buffer.palette.GetHSV(ColorIdx, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;

    for(y=y6+1; y<y7; y++)
    {
        buffer.SetPixel(x1,y,hsv); // Left side of mouyh
        buffer.SetPixel(x2,y,hsv); // rightside
    }
    for(x=x1+1; x<x2; x++)
    {
        buffer.SetPixel(x,y6,hsv); // Bottom
        buffer.SetPixel(x,y7,hsv); // Bottom
    }
    /*
     dy = (y7-y6)/2;
     xc = x1+dy;
     yc = y7+dy;
     radius = (double) dy;
     facesCircle(Phoneme,xc, yc, radius,90,270);
     xc = x1-dy;
     facesCircle(Phoneme,xc, yc, radius,270,630);
     */

}
/*
 faces draw circle
 */
void FacesEffect::facesCircle(RenderBuffer &buffer, int Phoneme, int xc,int yc,double radius,int start_degrees, int end_degrees)
{
    int x,y,degrees;
    HSVValue hsv;
    buffer.palette.GetHSV(0, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;
    double t,PI=3.1415926;
    for(degrees=start_degrees; degrees<end_degrees; degrees+=1)
    {
        t = degrees * (PI/180);
        x = (int)xc+radius*cos(t);
        y = (int)yc+radius*sin(t);
        buffer.SetPixel(x,y,hsv); // Bottom
    }
}

void FacesEffect::drawoutline(RenderBuffer &buffer, int Phoneme, bool outline, const std::string &eyes, int BufferHt,int BufferWi)
{
    HSVValue hsv;
    double radius;
    int ColorIdx,x,y,xc,yc;
    int Ht, Wt;
    size_t colorcnt=buffer.GetColorCount();
    Ht = BufferHt-1;
    Wt = BufferWi-1;


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    buffer.palette.GetHSV(ColorIdx, hsv);
    hsv.hue = 0.0;
    hsv.saturation = 1.0;
    hsv.value=1.0;

    //  DRAW EYES
    int start_degrees=0;
    int end_degrees=360;
    if (eyes == "Auto") {
        if(Phoneme==5 || Phoneme==6 || Phoneme==7)
        {
            start_degrees=180;
            end_degrees=360;
        }
    } else if (eyes == "Closed") {
        start_degrees=180;
        end_degrees=360;
    }
    if (eyes != "(off)") {
        xc = (int)(0.5 + Wt*0.33); // left eye
        yc = (int)(0.5 + Ht*0.75);
        radius = Wt*0.08;
        facesCircle(buffer,Phoneme,xc, yc, radius,start_degrees,end_degrees);
        xc = (int)(0.5 + Wt*0.66); // right eye
        yc = (int)(0.5 + Ht*0.75);
        radius = Wt*0.08;
        facesCircle(buffer,Phoneme,xc, yc, radius,start_degrees,end_degrees);
    }


    /*
     ...********...
     ..*
     .*
     *
     *
     *
     */
    if (outline) {
        for(y=3; y<BufferHt-3; y++)
        {
            buffer.SetPixel(0,y,hsv); // Left side of mouyh
            buffer.SetPixel(BufferWi-1,y,hsv); // rightside
        }
        for(x=3; x<BufferWi-3; x++)
        {
            buffer.SetPixel(x,0,hsv); // Bottom
            buffer.SetPixel(x,BufferHt-1,hsv); // Bottom
        }
        buffer.SetPixel(2,1,hsv); // Bottom left
        buffer.SetPixel(1,2,hsv); //

        buffer.SetPixel(BufferWi-3,1,hsv); // Bottom Right
        buffer.SetPixel(BufferWi-2,2,hsv); //

        buffer.SetPixel(BufferWi-3,BufferHt-2,hsv); // Bottom Right
        buffer.SetPixel(BufferWi-2,BufferHt-3,hsv); //

        buffer.SetPixel(2,BufferHt-2,hsv); // Bottom Right
        buffer.SetPixel(1,BufferHt-3,hsv); //
    }
}






/*----------------------CoroFaces--------------------------*/


//TODO: move this to a shared location:
static wxString NoInactive(wxString name)
{
    const wxString InactiveIndicator = "?";
    return name.StartsWith(InactiveIndicator)? name.substr(InactiveIndicator.size()): name;
}

//cached model info:
static std::unordered_map<std::string, std::unordered_map<std::string, /*wxPoint*/ std::string> > model_xy;
//static std::unordered_map<std::string, wxXmlNode*> model_xy; //since (X,Y) info is already in settings file, just re-use it

static bool parse_model(const wxString& want_model)
{
    if (model_xy.find((const char*)want_model.c_str()) != model_xy.end()) return true; //already have info
    //    std::unordered_map<std::string, wxPoint>& xy_info = model_xy[model];

#if 1
    wxFileName pgoFile;
    wxXmlDocument pgoXml;
    pgoFile.AssignDir(xLightsFrame::CurrentDir);
    pgoFile.SetFullName(_(XLIGHTS_PGOFACES_FILE));
    if (!pgoFile.FileExists()) return false;
    if (!pgoXml.Load(pgoFile.GetFullPath())) return false;
    wxXmlNode* root = pgoXml.GetRoot();
    if (!root || (root->GetName() != "papagayo")) return false;
    for (int compat = 0; compat < 2; ++compat)
    {
        wxXmlNode* Presets = xLightsFrame::FindNode(pgoXml.GetRoot(), compat? wxT("corofaces"): wxT("presets"), wxT("Name"), wxEmptyString, false); //kludge: backwards compatible with current settings
        if (!Presets) continue; //should be there if seq was generated in this folder
        //    wxString buf;
        //group name is not available, so use first occurrence of model in *any* group:
        //NOTE: assumes phoneme/face mapping is consistent for any given model across groups, which should be the case since the lights don't move
        for (wxXmlNode* group = Presets->GetChildren(); group != nullptr; group = group->GetNext())
        {
            wxString grpname = group->GetAttribute(wxT("name"));
            //        if (group->GetName() != "coro") continue;
            //        if (grpname.IsEmpty()) continue;
            //        wxXmlNode* voice = FindNode(group, "voice", wxT("voiceNumber"), wxString::Format(wxT("%i"), i + 1), true);
            for (wxXmlNode* voice = group->GetChildren(); voice != nullptr; voice = voice->GetNext())
            {
                wxString voice_name = NoInactive(voice->GetAttribute(wxT("name")));
                if (voice_name != want_model) continue;
                //            model_xy[(const char*)want_model.c_str()] = voice;
                //XmlNode getting trashed later, so save it here
                std::unordered_map<std::string, std::string>& map = model_xy[(const char*)want_model.c_str()];
                map.clear();
                for (wxXmlAttribute* attrp = voice->GetAttributes(); attrp; attrp = attrp->GetNext())
                {
                    wxString value = attrp->GetValue();
                    if (!value.empty()) map[(const char*)attrp->GetName().c_str()] = (const char*)value.c_str();
                }
                return true;
            }
        }
    }
#endif
    return false;
}



//NOTE: params are re-purposed as follows for Coro face mode:
// x_y = list of active elements for this frame
// Outline_x_y = list of persistent/sticky elements (stays on after frame ends)
// Eyes_x_y = list of random elements (intended for eye blinks, etc)

void FacesEffect::RenderCoroFacesFromPGO(RenderBuffer& buffer, const std::string& Phoneme, const std::string& eyes, bool face_outline)
{
    //    const wxString& parsed_xy = IsParsed(x_y)? x_y: wxEmptyString;
    //NOTE:
    //PixelBufferClass contains 2 RgbEffects members, which this method is a member of
    //xLightsFrame contains a PixelBufferClass member named buffer, which is derived from Model and gives the name of the model currently being used
    //therefore we can access the model info by going to parent object's buffer member
    //    wxString model_name = "???";
    if (!buffer.curPeriod) model_xy.clear(); //flush cache once at start of each effect

    static std::unordered_map<std::string, std::string> auto_phonemes {{"a-AI", "AI"}, {"a-E", "E"}, {"a-FV", "FV"}, {"a-L", "L"},
        {"a-MBP", "MBP"}, {"a-O", "O"}, {"a-U", "U"}, {"a-WQ", "WQ"}, {"a-etc", "etc"}, {"a-rest", "rest"}};
    //    static const char* auto_eyes[] = {"Open", "Closed", "Left", "Right", "Up", "Down"};

    if (auto_phonemes.find((const char*)Phoneme.c_str()) != auto_phonemes.end())
    {
        RenderFaces(buffer, auto_phonemes[(const char*)Phoneme.c_str()], eyes, face_outline);
        return;
    }

    HSVValue hsv;

    xlColor color;
    buffer.palette.GetColor(0, color); //use first color; user must make sure it matches model node type
    color = xlWHITE; //kludge: must use WHITE to get single-color nodes to show correctly
    buffer.Color2HSV(color, hsv);

    std::vector<wxPoint> first_xy;
    Model* model_info = buffer.frame->AllModels[buffer.cur_model];
    if (!model_info || !parse_model(buffer.cur_model))
    {
        return;
    }
    std::unordered_map<std::string, std::string>& map = model_xy[(const char*)buffer.cur_model.c_str()];
    if (Phoneme == "(test)")
    {
        /*static*/ std::string info = eyes;
        Model::ParseFaceElement(info, first_xy);
    }
    if (!Phoneme.empty())
    {
        std::string info = map[(const char*)Phoneme.c_str()];
        Model::ParseFaceElement(info, first_xy);
    }
    if (!eyes.empty())
    {
        std::string eyesLower(eyes);
        std::transform(eyesLower.begin(), eyesLower.end(), eyesLower.begin(), ::tolower);

        std::string info = map[wxString::Format(wxT("Eyes_%s"), eyesLower.c_str()).ToStdString()];
        Model::ParseFaceElement(info, first_xy);
    }
    if (face_outline)
    {
        std::string info = map["Outline"];
        Model::ParseFaceElement(info, first_xy);
    }
    for (auto it = first_xy.begin(); it != first_xy.end(); ++it)
    {
        --(*it).x; // "A" = 1 = first col
        buffer.SetPixel((*it).x, buffer.BufferHt - (*it).y, hsv); //only need to turn on first pixel for each face part
    }
}


class FacesRenderCache : public EffectRenderCache {
public:
    FacesRenderCache() : blinkEndTime(0), nextBlinkTime(0) {
    };
    virtual ~FacesRenderCache() {
    };
    void Clear() {
        nodeNameCache.clear();
    }
    
    int blinkEndTime;
    int nextBlinkTime;
    std::map<std::string, int> nodeNameCache;
};


void FacesEffect::RenderFaces(RenderBuffer &buffer,
                             SequenceElements *elements, const std::string &faceDefinition,
                             const std::string& Phoneme, const std::string &trackName,
                             const std::string& eyesIn, bool face_outline)
{
    FacesRenderCache *cache = (FacesRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new FacesRenderCache();
        buffer.infoCache[id] = cache;
    }

    if (buffer.needToInit) {
        buffer.needToInit = false;
        elements->AddRenderDependency(trackName, buffer.cur_model);
        cache->Clear();
    }
    std::string eyes = eyesIn;

    Element *track = elements->GetElement(trackName);
    std::recursive_mutex tmpLock;
    std::recursive_mutex *lock = &tmpLock;
    if (track != nullptr) {
        lock = &track->GetChangeLock();
    }
    std::unique_lock<std::recursive_mutex> locker(*lock);

    if (buffer.cur_model == "") {
        return;
    }
    Model* model_info = buffer.frame->AllModels[buffer.cur_model];
    if (model_info == nullptr) {
        return;
    }
    if (cache->nodeNameCache.empty()) {
        for (int x = 0; x < model_info->GetNodeCount(); x++) {
            cache->nodeNameCache[model_info->GetNodeName(x, true)] = x;
        }
    }

    std::string definition = faceDefinition;
    if (definition == "Default" && !model_info->faceInfo.empty() && model_info->faceInfo.begin()->first != "") {
        definition = model_info->faceInfo.begin()->first;
    }
    bool found = true;
    std::map<std::string, std::map<std::string, std::string> >::iterator it = model_info->faceInfo.find(definition);
    if (it == model_info->faceInfo.end()) {
        //not found
        found = false;
    }
    if (!found) {
        if ("Coro" == definition && model_info->faceInfo.find("SingleNode") != model_info->faceInfo.end()) {
            definition = "SingleNode";
            found = true;
        } else if ("SingleNode" == definition && model_info->faceInfo.find("Coro") != model_info->faceInfo.end()) {
            definition = "Coro";
            found = true;
        }
    }
    std::string modelType = found ? model_info->faceInfo[definition]["Type"] : definition;
    if (modelType == "") {
        modelType = definition;
    }

    int type = 3;

    if ("Coro" == modelType || "SingleNode" == modelType) {
        type = 0;
    } else if ("NodeRange" == modelType) {
        type = 1;
    } else if ("Rendered" == definition || "Default" == definition) {
        type = 2;
    }

    if (buffer.curEffStartPer == buffer.curPeriod)
    {
        if (modelType != "Matrix" && modelType != "Rendered" && modelType != "Default")
        {
            if (buffer.isTransformed)
            {
                static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                logger_base.warn("Faces effect starting at %dms until %dms on model %s has a transformed buffer. This may not work as expected.", buffer.curEffStartPer * buffer.frameTimeInMs, buffer.curEffEndPer * buffer.frameTimeInMs, (const char *)buffer.cur_model.c_str());
            }
        }
    }

    std::string phoneme = Phoneme;
    if (phoneme == "") {
        //GET Phoneme from timing track
        if (track == nullptr || track->GetEffectLayerCount() < 3) {
            phoneme = "rest";
            if ("Auto" == eyes) {
                if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                    //roughly every 5 seconds we'll blink
                    cache->nextBlinkTime += (4500 + (rand() % 1000));
                    cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; //100ms blink
                    eyes = "Closed";
                } else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                    eyes = "Closed";
                } else {
                    eyes = "Open";
                }
            }
        } else {
            int startms = -1;
            int endms = -1;

            EffectLayer *layer = track->GetEffectLayer(2);
            std::unique_lock<std::recursive_mutex> locker(layer->GetLock());
            int time = buffer.curPeriod * buffer.frameTimeInMs + 1;
            Effect *ef = layer->GetEffectByTime(time);
            if (ef == nullptr) {
                phoneme = "rest";
            } else {
                startms = ef->GetStartTimeMS();
                endms = ef->GetEndTimeMS();
                phoneme = ef->GetEffectName();
            }
            if ("Auto" == eyes && phoneme == "rest" && type != 2) {
                if (startms == -1) {
                    //need to figure out the time
                    for (int x = 0; x < layer->GetEffectCount() && startms == -1; x++) {
                        ef = layer->GetEffect(x);
                        if (ef->GetStartTimeMS() > buffer.curPeriod * buffer.frameTimeInMs) {
                            endms = ef->GetStartTimeMS();
                            if (x > 0) {
                                startms = layer->GetEffect(x - 1)->GetEndTimeMS();
                            } else {
                                startms = 0;
                            }
                        }
                    }
                }

                if ((buffer.curPeriod * buffer.frameTimeInMs) >= cache->nextBlinkTime) {
                    if ((startms + 150) >= (buffer.curPeriod * buffer.frameTimeInMs)) {
                        //don't want to blink RIGHT at the start of the rest, delay a little bie
                        int tmp =  (buffer.curPeriod * buffer.frameTimeInMs) + 150 + rand() % 400;

                        //also don't want it right at the end
                        if ((tmp + 130) > endms) {
                            cache->nextBlinkTime = (startms + endms) / 2;
                        } else {
                            cache->nextBlinkTime = tmp;
                        }
                    } else {
                        //roughly every 5 seconds we'll blink
                        cache->nextBlinkTime += (4500 + (rand() % 1000));
                        cache->blinkEndTime = buffer.curPeriod * buffer.frameTimeInMs + 101; //100ms blink
                        eyes = "Closed";
                    }
                } else if ((buffer.curPeriod * buffer.frameTimeInMs) < cache->blinkEndTime) {
                    eyes = "Closed";
                } else {
                    eyes = "Open";
                }
            }
        }
    }

    int colorOffset = 0;
    xlColor color;
    buffer.palette.GetColor(0, color); //use first color for mouth; user must make sure it matches model node type

    bool customColor = found ? model_info->faceInfo[definition]["CustomColors"] == "1" : false;

    std::vector<std::string> todo;
    std::vector<xlColor> colors;
    if (phoneme != "(off)") {
        todo.push_back("Mouth-" + phoneme);
        colorOffset = 1;
        if (customColor) {
            std::string cname = model_info->faceInfo[definition][ "Mouth-" + phoneme + "-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
            } else {
                colors.push_back(xlColor(cname));
            }
        } else {
            colors.push_back(color);
        }
    }
    if (buffer.palette.Size() > colorOffset) {
        buffer.palette.GetColor(colorOffset, color); //use second color for eyes; user must make sure it matches model node type
    }
    if (eyes == "Open" || eyes == "Auto") {
        todo.push_back("Eyes-Open");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition][ "Eyes-Open-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
            } else {
                colors.push_back(xlColor(cname));
            }
        } else {
            colors.push_back(color);
        }
    }
    if (eyes == "Closed") {
        todo.push_back("Eyes-Closed");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition][ "Eyes-Closed-Color"];
            if (cname == "") {
                colors.push_back(xlWHITE);
            } else {
                colors.push_back(xlColor(cname));
            }
        } else {
            colors.push_back(color);
        }
    }
    if (eyes == "(off)") {
        //no eyes
    }
    if (buffer.palette.Size() > (1 + colorOffset)) {
        buffer.palette.GetColor((1 + colorOffset), color); //use third color for outline; user must make sure it matches model node type
    }
    if (face_outline) {
        todo.insert(todo.begin(), "FaceOutline");
        if (customColor) {
            std::string cname = model_info->faceInfo[definition][ "FaceOutline-Color"];
            if (cname == "") {
                colors.insert(colors.begin(), xlWHITE);
            } else {
                colors.insert(colors.begin(), xlColor(cname));
            }
        } else {
            colors.insert(colors.begin(), color);
        }
    }


    if (type == 2) {
        RenderFaces(buffer, phoneme, eyes, face_outline);
        return;
    }
    if (type == 3) {
        //picture
        std::string e = eyes;
        if (eyes == "Auto") {
            e = "Open";
        }
        if (eyes == "(off)") {
            e = "Closed";
        }
        std::string key = "Mouth-" + phoneme + "-Eyes";
        std::string picture = model_info->faceInfo[definition][key + e];
        if (picture == "" && e == "Closed") {
            picture = model_info->faceInfo[definition][key + "Open"];
        }
        std::string i = "none"; /*RENDER_PICTURE_NONE*/
        if (model_info->faceInfo[definition]["ImagePlacement"] == "Centered") {
            i = "none"; /*RENDER_PICTURE_NONE */
        }        PicturesEffect::Render(buffer, i, picture, 0, 0, 0, 0, 0, 0, 100, 100, "Scale To Fit", false, false, false, false);  // set for scale to fit
    }
    for (size_t t = 0; t < todo.size(); t++) {
        std::string channels = model_info->faceInfo[definition][todo[t]];
        wxStringTokenizer wtkz(channels, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();

            if (type == 0) {
                auto it2 = cache->nodeNameCache.find(valstr.ToStdString());
                if (it2 != cache->nodeNameCache.end()) {
                    int n = it2->second;
                    for (auto a = buffer.Nodes[n]->Coords.begin() ; a != buffer.Nodes[n]->Coords.end(); a++) {
                        buffer.SetPixel(a->bufX, a->bufY, colors[t]);
                    }
                }
            } else if (type == 1) {
                int start, end;
                if (valstr.Contains("-")) {
                    int idx = valstr.Index('-');
                    start = wxAtoi(valstr.Left(idx));
                    end = wxAtoi(valstr.Right(valstr.size() - idx - 1));
                } else {
                    start = end = wxAtoi(valstr);
                }
                if (start > end) {
                    start = end;
                }
                start--;
                end--;
                for (int n = start; n <= end; n++) {
                    if (n < buffer.Nodes.size()) {
                        for (auto a = buffer.Nodes[n]->Coords.begin() ; a != buffer.Nodes[n]->Coords.end(); a++) {
                            buffer.SetPixel(a->bufX, a->bufY, colors[t]);
                        }
                    }
                }
            }
        }
    }
}



