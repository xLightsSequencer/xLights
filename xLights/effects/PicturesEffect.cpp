/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/gifdecod.h>
#include <wx/image.h>

#include "../../include/pictures-16.xpm"
#include "../../include/pictures-24.xpm"
#include "../../include/pictures-32.xpm"
#include "../../include/pictures-48.xpm"
#include "../../include/pictures-64.xpm"

#include "PicturesEffect.h"
#include "PicturesPanel.h"
#include "../sequencer/Effect.h"
#include "../sequencer/SequenceMedia.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "assist/xlGridCanvasPictures.h"
#include "assist/PicturesAssistPanel.h"
#include "../xLightsXmlFile.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"
#include "../ExternalHooks.h"
#include "../xLightsMain.h" 

#include <log4cpp/Category.hh>

#define wrdebug(...)

static int PicturesEffectId = 0;

PicturesEffect::PicturesEffect(int id) : RenderableEffect(id, "Pictures", pictures_16, pictures_24, pictures_32, pictures_48, pictures_64)
{
    //ctor
    PicturesEffectId = id;
}

PicturesEffect::~PicturesEffect()
{
    //dtor
}

std::list<std::string> PicturesEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    wxLogNull logNo;  // suppress popups from png images. See http://trac.wxwidgets.org/ticket/15331
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    wxString pictureFilename = settings.Get("E_FILEPICKER_Pictures_Filename", "");

    if (pictureFilename == "" || !FileExists(pictureFilename)) {
        res.push_back(wxString::Format("    ERR: Picture effect cant find image file '%s'. Model '%s', Start %s", pictureFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    } else {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, pictureFilename.ToStdString())) {
            res.push_back(wxString::Format("    WARN: Picture effect image file '%s' not under show directory. Model '%s', Start %s", pictureFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        int imageCount = wxImage::GetImageCount(pictureFilename);
        if (imageCount <= 0) {
            res.push_back(wxString::Format("    ERR: Picture effect '%s' contains no images. Image invalid. Model '%s', Start %s", pictureFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        if (!renderCache) {
            wxImage i;
            i.LoadFile(pictureFilename);
            if (i.IsOk()) {
                int ih = i.GetHeight();
                int iw = i.GetWidth();

#define IMAGESIZETHRESHOLD 10
                if (ih > IMAGESIZETHRESHOLD * model->GetDefaultBufferHt() || iw > IMAGESIZETHRESHOLD * model->GetDefaultBufferWi()) {
                    float scale = std::max((float)ih / model->GetDefaultBufferHt(), (float)iw / model->GetDefaultBufferWi());
                    res.push_back(wxString::Format("    WARN: Picture effect image file '%s' is %.1f times the height or width of the model ... xLights is going to need to do lots of work to resize the image. Model '%s', Start %s", pictureFilename, scale, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
                }
            }
        }
    }
    return res;
}

xlEffectPanel *PicturesEffect::CreatePanel(wxWindow *parent) {
    return new PicturesPanel(parent);
}

AssistPanel *PicturesEffect::GetAssistPanel(wxWindow *parent, xLightsFrame* xl_frame) {
    AssistPanel *assist_panel = new AssistPanel(parent);
    xlGridCanvasPictures* grid = new xlGridCanvasPictures(assist_panel->GetCanvasParent(), wxNewId(), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("PicturesGrid"));
    assist_panel->SetGridCanvas(grid);
    PicturesAssistPanel* picture_panel = new PicturesAssistPanel(assist_panel->GetCanvasParent());
    picture_panel->SetxLightsFrame(xl_frame);
    assist_panel->AddPanel(picture_panel);
    picture_panel->SetGridCanvas(grid);
    grid->SetMessageParent(picture_panel);
    return assist_panel;
}

bool PicturesEffect::needToAdjustSettings(const std::string &version)
{
    return true;
}

void PicturesEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap &settings = effect->GetSettings();

    if (settings.Contains("E_CHECKBOX_Pictures_ForceGIFOverlay")) {
        settings.erase("E_CHECKBOX_Pictures_ForceGIFOverlay");
    }

    if (settings.Contains("E_CHECKBOX_Pictures_ScaleToFit")) {
        if (settings.GetBool("E_CHECKBOX_Pictures_ScaleToFit", false)) {
            settings["E_CHOICE_Scaling"] = "Scale To Fit";
        } else {
            settings["E_CHOICE_Scaling"] = "No Scaling";
        }
        settings.erase("E_CHECKBOX_Pictures_ScaleToFit");
    }

    std::string file = settings["E_FILEPICKER_Pictures_Filename"];
    if (!file.empty()) {
        if (!FileExists(file, false)) {
            settings["E_FILEPICKER_Pictures_Filename"] = FixFile("", file);
        }
    }

    if (IsVersionOlder("2016.9", version)) {
        if (settings["E_CHOICE_Pictures_Direction"] == "scaled") {
            settings["E_CHOICE_Pictures_Direction"] = "none";
            settings["E_CHOICE_Scaling"] = "Scale To Fit";
        }
        settings["E_SLIDER_Pictures_StartScale"] = "100";
        settings["E_SLIDER_Pictures_EndScale"] = "100";
    }
}

//CAUTION: these must match EffectDirections exactly:
#define RENDER_PICTURE_LEFT  0
#define RENDER_PICTURE_RIGHT  1
#define RENDER_PICTURE_UP  2
#define RENDER_PICTURE_DOWN  3
#define RENDER_PICTURE_NONE  4
#define RENDER_PICTURE_UPLEFT  5
#define RENDER_PICTURE_DOWNLEFT  6
#define RENDER_PICTURE_UPRIGHT  7
#define RENDER_PICTURE_DOWNRIGHT  8
#define RENDER_PICTURE_PEEKABOO_0  9
#define RENDER_PICTURE_WIGGLE  10
#define RENDER_PICTURE_ZOOMIN  11
#define RENDER_PICTURE_PEEKABOO_90  12
#define RENDER_PICTURE_PEEKABOO_180  13
#define RENDER_PICTURE_PEEKABOO_270  14
// #define RENDER_PICTURE_VIXREMAP  15     /// removed
#define RENDER_PICTURE_FLAGWAVE  16
#define RENDER_PICTURE_UPONCE  17
#define RENDER_PICTURE_DOWNONCE  18
#define RENDER_PICTURE_VECTOR  19
#define RENDER_PICTURE_TILE_LEFT  20
#define RENDER_PICTURE_TILE_RIGHT  21
#define RENDER_PICTURE_TILE_DOWN  22
#define RENDER_PICTURE_TILE_UP 23

static inline int GetPicturesDirection(const std::string &dir) {
    if (dir == "left") {
        return RENDER_PICTURE_LEFT;
    } else if (dir == "right") {
        return RENDER_PICTURE_RIGHT;
    } else if (dir == "up") {
        return RENDER_PICTURE_UP;
    } else if (dir == "down") {
        return RENDER_PICTURE_DOWN;
    } else if (dir == "none") {
        return RENDER_PICTURE_NONE;
    } else if (dir == "up-left") {
        return RENDER_PICTURE_UPLEFT;
    } else if (dir == "down-left") {
        return RENDER_PICTURE_DOWNLEFT;
    } else if (dir == "up-right") {
        return RENDER_PICTURE_UPRIGHT;
    } else if (dir == "down-right") {
        return RENDER_PICTURE_DOWNRIGHT;
    } else if (dir == "peekaboo") {
        return RENDER_PICTURE_PEEKABOO_0;
    } else if (dir == "wiggle") {
        return RENDER_PICTURE_WIGGLE;
    } else if (dir == "zoom in") {
        return RENDER_PICTURE_ZOOMIN;
    } else if (dir == "peekaboo 90") {
        return RENDER_PICTURE_PEEKABOO_90;
    } else if (dir == "peekaboo 180") {
        return RENDER_PICTURE_PEEKABOO_180;
    } else if (dir == "peekaboo 270") {
        return RENDER_PICTURE_PEEKABOO_270;
    } else if (dir == "flag wave") {
        return RENDER_PICTURE_FLAGWAVE;
    } else if (dir == "up once") {
        return RENDER_PICTURE_UPONCE;
    } else if (dir == "down once") {
        return RENDER_PICTURE_DOWNONCE;
    } else if (dir == "vector") {
        return RENDER_PICTURE_VECTOR;
    } else if (dir == "tile-left") {
        return RENDER_PICTURE_TILE_LEFT;
    } else if (dir == "tile-right") {
        return RENDER_PICTURE_TILE_RIGHT;
    } else if (dir == "tile-down") {
        return RENDER_PICTURE_TILE_DOWN;
    } else if (dir == "tile-up") {
        return RENDER_PICTURE_TILE_UP;
    }
    return RENDER_PICTURE_NONE;
}

typedef std::vector< std::pair<wxPoint, xlColor> > PixelVector;

class PicturesRenderCache : public EffectRenderCache {
public:
    PicturesRenderCache() {}
    virtual ~PicturesRenderCache() {}
    
    int imageCount = 0;
    int frame = 0;
    int maxmovieframes = 0;
    std::string PictureName;
    std::shared_ptr<ImageCacheEntry> imageCache;
};

static PicturesRenderCache *GetCache(RenderBuffer &buf) {
    PicturesRenderCache *cache = (PicturesRenderCache*)buf.infoCache[PicturesEffectId];
    if (cache == nullptr) {
        cache = new PicturesRenderCache();
        buf.infoCache[PicturesEffectId] = cache;
    }
    return cache;
}

void PicturesEffect::SetTransparentBlackPixel(RenderBuffer& buffer, int x, int y, xlColor c, bool transparentBlack, int transparentBlackLevel)
{
    if (transparentBlack)
    {
        int level = c.Red() + c.Green() + c.Blue();
        if (level > transparentBlackLevel)
        {
            buffer.SetPixel(x, y, c);
        }
    }
    else
    {
        buffer.SetPixel(x, y, c);
    }
}

void PicturesEffect::SetTransparentBlackPixel(RenderBuffer& buffer, int x, int y, xlColor c, bool wrap, bool transparentBlack, int transparentBlackLevel)
{
    if (transparentBlack)
    {
        int level = c.Red() + c.Green() + c.Blue();
        if (level > transparentBlackLevel)
        {
            buffer.ProcessPixel(x, y, c, wrap);
        }
    }
    else
    {
        buffer.ProcessPixel(x, y, c, wrap);
    }
}

void PicturesEffect::SetDefaultParameters() {
    PicturesPanel *pp = (PicturesPanel*)panel;
    if (pp == nullptr) {
        return;
    }

    pp->BitmapButton_PicturesXC->SetActive(false);
    pp->BitmapButton_PicturesYC->SetActive(false);

    SetSliderValue(pp->Slider_Pictures_Speed, 10);
    SetSliderValue(pp->Slider_Pictures_FR, 10);
    SetSliderValue(pp->Slider1, 0);
    SetSliderValue(pp->Slider_PicturesXC, 0);
    SetSliderValue(pp->Slider_PicturesYC, 0);
    SetSliderValue(pp->Slider_PicturesEndXC, 0);
    SetSliderValue(pp->Slider_PicturesEndYC, 0);
    SetSliderValue(pp->Slider_Pictures_EndScale, 100);
    SetSliderValue(pp->Slider_Pictures_StartScale, 100);

    SetChoiceValue(pp->Choice_Pictures_Direction, "none");
    SetChoiceValue(pp->Choice_Scaling, "No Scaling");

    SetCheckBoxValue(pp->CheckBox_Pictures_PixelOffsets, false);
    SetCheckBoxValue(pp->CheckBox_Pictures_WrapX, false);
    SetCheckBoxValue(pp->CheckBox_Pictures_Shimmer, false);
    SetCheckBoxValue(pp->CheckBox_LoopGIF, false);
    SetCheckBoxValue(pp->CheckBox_SuppressGIFBackground, true);
    SetCheckBoxValue(pp->CheckBox_TransparentBlack, false);

    pp->FilePickerCtrl1->SetFileName(wxFileName());

    pp->ValidateWindow();
}

std::list<std::string> PicturesEffect::GetFileReferences(Model* model, const SettingsMap &SettingsMap) const
{
    std::list<std::string> res;
    if (SettingsMap["E_FILEPICKER_Pictures_Filename"] != "") {
        res.push_back(SettingsMap["E_FILEPICKER_Pictures_Filename"]);
    }
    return res;
}

bool PicturesEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap &SettingsMap)
{
    bool rc = false;
    wxString file = SettingsMap["E_FILEPICKER_Pictures_Filename"];
    if (FileExists(file)) {
        if (!frame->IsInShowFolder(file)) {
            SettingsMap["E_FILEPICKER_Pictures_Filename"] = frame->MoveToShowFolder(file, wxString(wxFileName::GetPathSeparator()) + "Images");
            rc = true;
        }
    }

    return rc;
}

bool PicturesEffect::IsPictureFile(std::string filename)
{
    wxFileName fn(filename);
    auto ext = fn.GetExt().Lower().ToStdString();

    if (ext == "gif" ||
        ext == "jpg" ||
        ext == "jpeg" ||
        ext == "png" ||
        ext == "webp" ||
        ext == "bmp"
        )
    {
        return true;
    }

    return false;
}

void PicturesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    auto dirstr = SettingsMap["CHOICE_Pictures_Direction"];
    Render(buffer,
           dirstr,
           SettingsMap["FILEPICKER_Pictures_Filename"],
           SettingsMap.GetFloat("TEXTCTRL_Pictures_Speed", 1.0),
           SettingsMap.GetFloat("TEXTCTRL_Pictures_FrameRateAdj", 1.0),
           dirstr != "vector" ? GetValueCurveInt("PicturesXC", 0, SettingsMap, oset, PICTURES_XC_MIN, PICTURES_XC_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) : SettingsMap.GetInt("SLIDER_PicturesXC", 0),
           dirstr != "vector" ? GetValueCurveInt("PicturesYC", 0, SettingsMap, oset, PICTURES_YC_MIN, PICTURES_YC_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) : SettingsMap.GetInt("SLIDER_PicturesYC", 0),
           SettingsMap.GetInt("SLIDER_PicturesEndXC", 0),
           SettingsMap.GetInt("SLIDER_PicturesEndYC", 0),
           SettingsMap.GetInt("SLIDER_Pictures_StartScale", 100),
           SettingsMap.GetInt("SLIDER_Pictures_EndScale", 100),
           SettingsMap.Get("CHOICE_Scaling", "No Scaling"),
           SettingsMap.GetBool("CHECKBOX_Pictures_PixelOffsets", false),
           SettingsMap.GetBool("CHECKBOX_Pictures_WrapX", false),
           SettingsMap.GetBool("CHECKBOX_Pictures_Shimmer", false),
           SettingsMap.GetBool("CHECKBOX_LoopGIF", false),
           SettingsMap.GetBool("CHECKBOX_SuppressGIFBackground", true),
           SettingsMap.GetBool("CHECKBOX_Pictures_TransparentBlack", false),
           SettingsMap.GetInt("TEXTCTRL_Pictures_TransparentBlackLevel", 0)
    );
}

void PicturesEffect::Render(RenderBuffer& buffer,
    const std::string& dirstr, const std::string& NewPictureName,
    float movementSpeed, float frameRateAdj,
    int xc_adj, int yc_adj,
    int xce_adj, int yce_adj,
    int start_scale, int end_scale, const std::string& scale_to_fit,
    bool pixelOffsets, bool wrap_x, bool shimmer, bool loopGIF, bool suppressGIFBackground,
    bool transparentBlack, int transparentBlackLevel)
{
    int dir = GetPicturesDirection(dirstr);
    double position = buffer.GetEffectTimeIntervalPosition(movementSpeed);

    int BufferWi = buffer.BufferWi;
    int BufferHt = buffer.BufferHt;
    int curPeriod = buffer.curPeriod;
    int curEffStartPer = buffer.curEffStartPer;
    int scale_image = false;
    bool noImageFile = false;

    PicturesRenderCache* cache = GetCache(buffer);
    bool fitAnimation = true;
    if (NewPictureName.length() == 0) {
        noImageFile = true;
    } else {

        wxFileName fn(NewPictureName);
        wxString suffix = "";
        if (fn.GetName().Length() >= 2) {
            suffix = fn.GetName().Right(2);
        }

        if (suffix == "-1") {// do we have a movie file?
            //  Look at ending of the filename passed in. If we have it ending as *-1.jpg or *-1.png then we will assume
            //  we have a bunch of jpg files made by ffmpeg
            //  movie files can be converted into jpg frames by this command
            //      ffmpeg -i XXXX.mp4 -s 16x50 XXXX-%d.jpg
            //      ffmpeg -i XXXX.avi -s 16x50 XXXX-%d.jpg
            //      ffmpeg -i XXXX.mov -s 16x50 XXXX-%d.jpg
            //      ffmpeg -i XXXX.mts -s 16x50 XXXX-%d.jpg

            if (!buffer.GetSequenceMedia()->HasImage(NewPictureName)) {
                buffer.GetSequenceMedia()->AddAnimatedImage(NewPictureName, buffer.frameTimeInMs);
            }
            fitAnimation = false;
        }

        if (NewPictureName != cache->PictureName || buffer.needToInit) {
            buffer.needToInit = false;
            scale_image = true;

            if (!buffer.GetSequenceMedia()->HasImage(NewPictureName) && !FileExists(NewPictureName)) {
                noImageFile = true;
            } else {
                cache->PictureName = NewPictureName;
                cache->imageCache = buffer.GetSequenceMedia()->GetImage(NewPictureName);
                cache->imageCache->MarkIsUsed();
                cache->imageCount = cache->imageCache->GetImageCount();
            }
        }
        if (!noImageFile && !cache->imageCache->IsOk()) {
            noImageFile = true;
        }
    }
    if (noImageFile) {
        for (int x = 0; x < BufferWi; x++) {
            for (int y = 0; y < BufferHt; y++) {
                buffer.SetPixel(x, y, xlRED);
            }
        }
        return;
    }
    
    if (loopGIF || !fitAnimation) {
        cache->frame = cache->imageCache->GetFrameForTime((buffer.curPeriod - buffer.curEffStartPer) * buffer.frameTimeInMs * frameRateAdj, loopGIF);
    } else {
        cache->frame = cache->imageCount * buffer.GetEffectTimeIntervalPosition(frameRateAdj) * 0.999;
    }
    if (cache->imageCount > 0) {
        scale_image = true;
    }

    if (scale_to_fit == "No Scaling" && (start_scale != end_scale)) {
        scale_image = true;
    }

    wxSize imageSize = cache->imageCache->GetImageSize();
    int imgwidth = imageSize.GetWidth();
    int imght = imageSize.GetHeight();
    int yoffset = (BufferHt + imght) / 2; //centered if sizes don't match
    int xoffset = (imgwidth - BufferWi) / 2; //centered if sizes don't match

    std::shared_ptr<wxImage> image;


    if (scale_to_fit == "Scale To Fit" && (BufferWi != imgwidth || BufferHt != imght)) {
        image = cache->imageCache->GetScaledImage(cache->frame, BufferWi, BufferHt, suppressGIFBackground);
        imgwidth = image->GetWidth();
        imght = image->GetHeight();

        yoffset = (BufferHt + imght) / 2; //centered if sizes don't match
        xoffset = (imgwidth - BufferWi) / 2; //centered if sizes don't match
    } else if (scale_to_fit == "Scale Keep Aspect Ratio" || scale_to_fit == "Scale Keep Aspect Ratio Crop") {
        float xr = (float)BufferWi / (float)imageSize.GetWidth();
        float yr = (float)BufferHt / (float)imageSize.GetHeight();
        float sc = std::min(xr, yr);
        if (scale_to_fit.find("Crop") != std::string::npos) {
            sc = std::max(xr, yr);
        }
        float newWid = sc * (float)imageSize.GetWidth();
        float newHi = sc * (float)imageSize.GetHeight();
        image = cache->imageCache->GetScaledImage(cache->frame, newWid, newHi, suppressGIFBackground);

        imgwidth = image->GetWidth();
        imght = image->GetHeight();
        yoffset = (BufferHt + imght) / 2; //centered if sizes don't match
        xoffset = (imgwidth - BufferWi) / 2; //centered if sizes don't match
    } else {
        if ((start_scale != 100 || end_scale != 100) && scale_image) {
            int delta_scale = end_scale - start_scale;
            int current_scale = start_scale + delta_scale * position;
            imgwidth = (imageSize.GetWidth() * current_scale) / 100;
            imght = (imageSize.GetHeight() * current_scale) / 100;
            imgwidth = std::max(imgwidth, 1);
            imght = std::max(imght, 1);
            image = cache->imageCache->GetScaledImage(cache->frame, imgwidth, imght, suppressGIFBackground);
            yoffset = (BufferHt + imght) / 2; //centered if sizes don't match
            xoffset = (imgwidth - BufferWi) / 2; //centered if sizes don't match
        } else {
            image = cache->imageCache->GetFrame(cache->frame, suppressGIFBackground);
        }
    }

    int waveX = 0;
    int waveW = 0;
    int waveN = 0; //location of first wave, height adjust, width, wave# -DJ

    float xscale = 0;
    float yscale = 0;

    switch (dir) //prep
    {
    case RENDER_PICTURE_ZOOMIN: // src <- dest scale factor -DJ
        xscale = (imgwidth > 1) ? (float)BufferWi / imgwidth : 1;
        yscale = (imght > 1) ? (float)BufferHt / imght : 1;
        xscale *= position;
        yscale *= position;
        break;
    case RENDER_PICTURE_PEEKABOO_0: //up+down 1x -DJ
    case RENDER_PICTURE_PEEKABOO_180: //up+down 1x -DJ
        yoffset = (-BufferHt) * (1.0 - position * 2.0);
        if (yoffset > 10) yoffset = -yoffset + 10; //reverse direction
        else if (yoffset > 0) yoffset = 0; //pause in middle
        break;
    case RENDER_PICTURE_PEEKABOO_90: //peekaboo 90
    case RENDER_PICTURE_PEEKABOO_270: //peekaboo 270
        yoffset = (imght - BufferWi) / 2; //adjust offsets for other axis
        xoffset = (-BufferHt) * (1.0 - position * 2.0); // * speedfactor; //draw_at = (state < BufferHt)? state
        if (xoffset > 10) xoffset = -xoffset + 10; //reverse direction
        else if (xoffset > 0) xoffset = 0; //pause in middle
        break;
    case RENDER_PICTURE_UPONCE:
    case RENDER_PICTURE_DOWNONCE:
        position = buffer.GetEffectTimeIntervalPosition() * movementSpeed;
        if (position > 1.0) {
            position = 1.0;
        }
        break;
    case RENDER_PICTURE_WIGGLE: //wiggle left-right -DJ
        if (position >= 0.5) {
            xoffset += BufferWi * ((1.0 - position) * 2.0 - 0.5);
        }
        else {
            xoffset += BufferWi * (position * 2.0 - 0.5);
        }
        break;
    case RENDER_PICTURE_FLAGWAVE: //flag wave -DJ
        waveW = BufferWi;
        waveX = position * 200;
        waveN = waveX / waveW;
        break;
    default: break;
    }

    int xoffset_adj = xc_adj;
    int yoffset_adj = yc_adj;
    if (dir == RENDER_PICTURE_VECTOR) {
        //adjust the picture offset
        dir = RENDER_PICTURE_NONE;
        xoffset_adj = std::round(position * double(xce_adj - xc_adj)) + xc_adj;
        yoffset_adj = std::round(position * double(yce_adj - yc_adj)) + yc_adj;
    }
    if (!pixelOffsets) {
        xoffset_adj = (xoffset_adj * BufferWi) / 100.0; // xc_adj is from -100 to 100
        yoffset_adj = (yoffset_adj * BufferHt) / 100.0; // yc_adj is from -100 to 100
    }
    // copy image to buffer
    xlColor c;
    const wxImage &img = *image.get();
    if (!img.IsOk()) {
        return;
    }
    
    bool hasAlpha = img.HasAlpha();

    int calc_position_wi = (imgwidth + BufferWi) * position;
    int calc_position_ht = (imght + BufferHt) * position;

    for (int x = 0; x < imgwidth; x++) {
        for (int y = 0; y < imght; y++) {
            if (!hasAlpha || !img.IsTransparent(x, y)) {
                unsigned char alpha = hasAlpha ? img.GetAlpha(x, y) : 255;
                c.Set(img.GetRed(x, y), img.GetGreen(x, y), img.GetBlue(x, y), alpha);
                if (!buffer.allowAlpha && alpha < 64) {
                    //almost transparent, but this mix doesn't support transparent unless it's black;
                    c = xlBLACK;
                }

                switch (dir) {
                case RENDER_PICTURE_LEFT: //0:
                    SetTransparentBlackPixel(buffer, x + xoffset_adj + BufferWi - calc_position_wi, yoffset - y - yoffset_adj - 1, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // left
                case RENDER_PICTURE_RIGHT: //1:
                    SetTransparentBlackPixel(buffer, x + xoffset_adj + calc_position_wi - imgwidth, yoffset - y - yoffset_adj - 1, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // right
                case RENDER_PICTURE_UP: //2:
                case RENDER_PICTURE_UPONCE: //18
                    SetTransparentBlackPixel(buffer, x - xoffset + xoffset_adj, calc_position_ht - y - yoffset_adj, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // up
                case RENDER_PICTURE_DOWN: //3:
                case RENDER_PICTURE_DOWNONCE: //19
                    SetTransparentBlackPixel(buffer, x - xoffset + xoffset_adj, BufferHt + imght - y - yoffset_adj - calc_position_ht, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // down
                case RENDER_PICTURE_UPLEFT: //5:
                    SetTransparentBlackPixel(buffer, x + xoffset_adj + BufferWi - calc_position_wi, calc_position_ht - y - yoffset_adj, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // up-left
                case RENDER_PICTURE_DOWNLEFT: //6:
                    SetTransparentBlackPixel(buffer, x + xoffset_adj + BufferWi - calc_position_wi, BufferHt + imght - y - yoffset_adj - calc_position_ht, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // down-left
                case RENDER_PICTURE_UPRIGHT: //7:
                    SetTransparentBlackPixel(buffer, x + xoffset_adj + calc_position_wi - imgwidth, calc_position_ht - y - yoffset_adj, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // up-right
                case RENDER_PICTURE_DOWNRIGHT: //8:
                    SetTransparentBlackPixel(buffer, x + xoffset_adj + calc_position_wi - imgwidth, BufferHt + imght - y - yoffset_adj - calc_position_ht, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // down-right

                case RENDER_PICTURE_PEEKABOO_0: //10: //up+down 1x (peekaboo) -DJ
                    SetTransparentBlackPixel(buffer, x - xoffset + xoffset_adj, BufferHt + yoffset - y - yoffset_adj - 1, c, wrap_x, transparentBlack, transparentBlackLevel); // - BufferHt, c);
                    break;
                case RENDER_PICTURE_ZOOMIN: //12: //zoom in (explode) -DJ
                    //TODO: use rescale or resize?
                    SetTransparentBlackPixel(buffer, (x + xoffset_adj) * xscale, (BufferHt - 1 - y - yoffset_adj) * yscale, c, wrap_x, transparentBlack, transparentBlackLevel); //CAUTION: y inverted?; TODO: anti-aliasing, averaging, etc.
                    break;
                case RENDER_PICTURE_PEEKABOO_90: //13: //peekaboo 90 -DJ
                    SetTransparentBlackPixel(buffer, BufferWi + xoffset - y + xoffset_adj, x - yoffset - yoffset_adj, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break;
                case RENDER_PICTURE_PEEKABOO_180: //14: //peekaboo 180 -DJ
                    SetTransparentBlackPixel(buffer, x - xoffset + xoffset_adj, y - yoffset - yoffset_adj, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break;
                case RENDER_PICTURE_PEEKABOO_270: //15: //peekabo 270 -DJ
                    SetTransparentBlackPixel(buffer, y - xoffset + xoffset_adj, BufferHt + yoffset + yoffset_adj - x, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break;
                case RENDER_PICTURE_FLAGWAVE: //17: //flag wave in wind -DJ
                {
                    int waveY;
                    if (BufferHt < 20) //small grid => small waves
                    {
                        waveN = (x - waveX) / waveW;
                        waveY = !x ? 0 : (waveN & 1) ? -1 : 0;
                    }
                    else //larger grid => larger waves
                    {
                        waveY = !x ? 0 : (waveN & 1) ? 0 : (waveN & 2) ? -1 : +1;
                        if (waveX < 0) waveY *= -1;
                    }
                    SetTransparentBlackPixel(buffer, x - xoffset + xoffset_adj, yoffset - y - yoffset_adj + waveY - 1, c, wrap_x, transparentBlack, transparentBlackLevel);
                }
                break;
                case RENDER_PICTURE_TILE_LEFT: // 21
                {
                    int xmult = (buffer.BufferWi + 2 * imgwidth) / imgwidth;
                    int ymult = (buffer.BufferHt + 2 * imght) / imght;
                    int startx = xoffset_adj - (int)((float)(curPeriod - curEffStartPer) * movementSpeed) % imgwidth;
                    int starty = yoffset_adj - imght;
                    for (int xx = 0; xx < xmult; ++xx) {
                        for (int yy = 0; yy < ymult; ++yy) {
                            SetTransparentBlackPixel(buffer, xx * imgwidth + x + startx, yy * imght + (imght - y - 1) + starty,
                                c, false, transparentBlack, transparentBlackLevel);
                        }
                    }
                }
                break;
                case RENDER_PICTURE_TILE_RIGHT: // 22
                {
                    int xmult = (buffer.BufferWi + 2 * imgwidth) / imgwidth;
                    int ymult = (buffer.BufferHt + 2 * imght) / imght;
                    int startx = xoffset_adj - imgwidth + (int)((float)(curPeriod - curEffStartPer) * movementSpeed) % imgwidth;
                    int starty = yoffset_adj - imght;
                    for (int xx = 0; xx < xmult; ++xx) {
                        for (int yy = 0; yy < ymult; ++yy) {
                            SetTransparentBlackPixel(buffer, xx * imgwidth + x + startx, yy * imght + (imght - y - 1) + starty,
                                c, false, transparentBlack, transparentBlackLevel);
                        }
                    }
                }
                break;
                case RENDER_PICTURE_TILE_DOWN: // 23
                {
                    int xmult = (buffer.BufferWi + 2 * imgwidth) / imgwidth;
                    int ymult = (buffer.BufferHt + 2 * imght) / imght;
                    int startx = xoffset_adj - imgwidth;
                    int starty = yoffset_adj - (int)((float)(curPeriod - curEffStartPer) * movementSpeed) % imght;
                    for (int xx = 0; xx < xmult; ++xx) {
                        for (int yy = 0; yy < ymult; ++yy) {
                            SetTransparentBlackPixel(buffer, xx * imgwidth + x + startx, yy * imght + (imght - y - 1) + starty,
                                c, false, transparentBlack, transparentBlackLevel);
                        }
                    }
                }
                break;
                case RENDER_PICTURE_TILE_UP: // 24
                {
                    int xmult = (buffer.BufferWi + 2 * imgwidth) / imgwidth;
                    int ymult = (buffer.BufferHt + 2 * imght) / imght;
                    int startx = xoffset_adj - imgwidth;
                    int starty = yoffset_adj - imght + (int)((float)(curPeriod - curEffStartPer) * movementSpeed) % imght;
                    for (int xx = 0; xx < xmult; ++xx) {
                        for (int yy = 0; yy < ymult; ++yy) {
                            SetTransparentBlackPixel(buffer, xx * imgwidth + x + startx, yy * imght + (imght - y - 1) + starty,
                                c, false, transparentBlack, transparentBlackLevel);
                        }
                    }
                }
                break;
                case RENDER_PICTURE_WIGGLE: //11: //back+forth a little (wiggle) -DJ
                    //                    ProcessPixel(x + xoffset+xoffset_adj, yoffset - y - yoffset_adj, c, wrap_x);
                    //                    break;
                default:
                    SetTransparentBlackPixel(buffer, x - xoffset + xoffset_adj, yoffset + yoffset_adj - y - 1, c, wrap_x, transparentBlack, transparentBlackLevel);
                    break; // no movement - centered
                }
            }
        }
    }

    // add shimmer effect which just randomly turns off pixels
    if (shimmer) {
        c = xlBLACK;
        xlColor color;
        for (int x = 0; x < BufferWi; x++) {
            for (int y = 0; y < BufferHt; y++) {
                if (rand01() > 0.5) {
                    buffer.GetPixel(x, y, color);
                    if (color != xlBLACK) {
                        buffer.ProcessPixel(x, y, c, false);
                    }
                }
            }
        }
    }
}
