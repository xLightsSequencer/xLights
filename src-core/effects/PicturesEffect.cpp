/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <filesystem>
#include <spdlog/fmt/fmt.h>

#include "../../include/pictures-16.xpm"
#include "../../include/pictures-24.xpm"
#include "../../include/pictures-32.xpm"
#include "../../include/pictures-48.xpm"
#include "../../include/pictures-64.xpm"

#include "PicturesEffect.h"
#include "../render/Effect.h"
#include "../render/SequenceMedia.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../render/SequenceFile.h"
#include "../models/Model.h"
#include "UtilFunctions.h"
#include "utils/FileUtils.h"
#include "utils/ExternalHooks.h"
#include "../render/RenderContext.h"
#include "../render/SequenceElements.h"
#include "../utils/xlPoint.h"

#include <log.h>

#define wrdebug(...)

static int PicturesEffectId = 0;

std::string PicturesEffect::sDirectionDefault = "none";
double PicturesEffect::sSpeedDefault = 1.0;
double PicturesEffect::sFrameRateAdjDefault = 1.0;
bool PicturesEffect::sPixelOffsetsDefault = false;
std::string PicturesEffect::sScalingDefault = "No Scaling";
bool PicturesEffect::sShimmerDefault = false;
bool PicturesEffect::sLoopGIFDefault = false;
bool PicturesEffect::sSuppressGIFBackgroundDefault = true;
int PicturesEffect::sXCDefault = 0;
int PicturesEffect::sXCMin = -100;
int PicturesEffect::sXCMax = 100;
bool PicturesEffect::sWrapXDefault = false;
int PicturesEffect::sYCDefault = 0;
int PicturesEffect::sYCMin = -100;
int PicturesEffect::sYCMax = 100;
int PicturesEffect::sEndXCDefault = 0;
int PicturesEffect::sEndYCDefault = 0;
int PicturesEffect::sStartScaleDefault = 100;
int PicturesEffect::sEndScaleDefault = 100;

PicturesEffect::PicturesEffect(int id) : RenderableEffect(id, "Pictures", pictures_16, pictures_24, pictures_32, pictures_48, pictures_64)
{
    //ctor
    PicturesEffectId = id;
}

PicturesEffect::~PicturesEffect()
{
    //dtor
}

void PicturesEffect::OnMetadataLoaded()
{
    sDirectionDefault = GetStringDefault("Pictures_Direction", sDirectionDefault);
    sSpeedDefault = GetDoubleDefault("Pictures_Speed", sSpeedDefault);
    sFrameRateAdjDefault = GetDoubleDefault("Pictures_FrameRateAdj", sFrameRateAdjDefault);
    sPixelOffsetsDefault = GetBoolDefault("Pictures_PixelOffsets", sPixelOffsetsDefault);
    sScalingDefault = GetStringDefault("Scaling", sScalingDefault);
    sShimmerDefault = GetBoolDefault("Pictures_Shimmer", sShimmerDefault);
    sLoopGIFDefault = GetBoolDefault("LoopGIF", sLoopGIFDefault);
    sSuppressGIFBackgroundDefault = GetBoolDefault("SuppressGIFBackground", sSuppressGIFBackgroundDefault);
    sXCDefault = GetIntDefault("PicturesXC", sXCDefault);
    sXCMin = (int)GetMinFromMetadata("PicturesXC", sXCMin);
    sXCMax = (int)GetMaxFromMetadata("PicturesXC", sXCMax);
    sWrapXDefault = GetBoolDefault("Pictures_WrapX", sWrapXDefault);
    sYCDefault = GetIntDefault("PicturesYC", sYCDefault);
    sYCMin = (int)GetMinFromMetadata("PicturesYC", sYCMin);
    sYCMax = (int)GetMaxFromMetadata("PicturesYC", sYCMax);
    sEndXCDefault = GetIntDefault("PicturesEndXC", sEndXCDefault);
    sEndYCDefault = GetIntDefault("PicturesEndYC", sEndYCDefault);
    sStartScaleDefault = GetIntDefault("Pictures_StartScale", sStartScaleDefault);
    sEndScaleDefault = GetIntDefault("Pictures_EndScale", sEndScaleDefault);
}

std::list<std::string> PicturesEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    std::string pictureFilename = settings.Get("E_TEXTCTRL_Pictures_Filename", "");
    auto &mm = eff->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();

    // Don't gate on `HasImage` — that only reports cache hits. The
    // image cache is populated lazily by `GetImage`, which (a) does
    // FixFile resolution against the show + media folders, (b) loads
    // on demand. Calling `GetImage` directly is the same code path
    // the renderer uses, so a "missing" result here matches what the
    // renderer would observe at frame time.
    auto img = pictureFilename.empty() ? nullptr : mm.GetImage(pictureFilename);
    if (pictureFilename == "" || !img) {
        res.push_back(fmt::format("    ERR: Picture effect cant find image file '{}'. Model '{}', Start {}", pictureFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    } else {
        if (!img->IsOk()) {
            res.push_back(fmt::format("    ERR: Picture effect cant load image '{}'. Model '{}', Start {}", pictureFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        } else {
            if (!img->IsEmbedded()) {
                if (!FileUtils::IsFileInShowDir(std::string(), pictureFilename)) {
                    res.push_back(fmt::format("    WARN: Picture effect image file '{}' not under show directory. Model '{}', Start {}", pictureFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
                }
            }
            int imageCount = img->GetImageCount();
            if (imageCount <= 0) {
                res.push_back(fmt::format("    ERR: Picture effect '{}' contains no images. Image invalid. Model '{}', Start {}", pictureFilename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
            }

            if (!renderCache) {
                int ih = img->GetImageHeight();
                int iw = img->GetImageWidth();

#define IMAGESIZETHRESHOLD 10
                if (ih > IMAGESIZETHRESHOLD * model->GetDefaultBufferHt() || iw > IMAGESIZETHRESHOLD * model->GetDefaultBufferWi()) {
                    float scale = std::max((float)ih / model->GetDefaultBufferHt(), (float)iw / model->GetDefaultBufferWi());
                    res.push_back(fmt::format("    WARN: Picture effect image file '{}' is {:.1f} times the height or width of the model ... xLights is going to need to do lots of work to resize the image. Model '{}', Start {}", pictureFilename, scale, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
                }
            }
        }
    }
    return res;
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

    std::string file = settings["E_TEXTCTRL_Pictures_Filename"];
    auto &media = effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
    if (!file.empty() && !media.HasImage(file)) {
        if (!std::filesystem::path(file).is_absolute()) {
            std::string fixed = FileUtils::FixFile("", file);
            if (fixed != file) {
                std::string rel = FileUtils::MakeRelativeFile(fixed);
                std::string newPath = rel.empty() ? fixed : rel;
                auto normalize = [](std::string s) {
                    std::replace(s.begin(), s.end(), '\\', '/');
                    return s;
                };
                if (normalize(newPath) != normalize(file)) {
                    settings["E_TEXTCTRL_Pictures_Filename"] = newPath;
                    media.RecordRelocation(file, newPath);
                }
            }
        } else if (!FileExists(file, false)) {
            std::string fixed = FileUtils::FixFile("", file);
            // If the resolved path is inside a show/media directory, store as
            // relative so the sequence is portable across machines.
            std::string rel = FileUtils::MakeRelativeFile(fixed);
            std::string newPath = rel.empty() ? fixed : rel;
            settings["E_TEXTCTRL_Pictures_Filename"] = newPath;
            if (newPath != file)
                media.RecordRelocation(file, newPath);
        } else {
            // File exists at its absolute path — still prefer relative storage
            std::string rel = FileUtils::MakeRelativeFile(file);
            if (!rel.empty()) {
                settings["E_TEXTCTRL_Pictures_Filename"] = rel;
            }
        }
        std::string NewPictureName = settings["E_TEXTCTRL_Pictures_Filename"];
        std::string suffix = "";
        std::string fnName = std::filesystem::path(NewPictureName).stem().string();
        if (fnName.length() >= 2) {
            suffix = fnName.substr(fnName.length() - 2);
        }
        if (suffix == "-1") {// do we have a movie file?
            //  Look at ending of the filename passed in. If we have it ending as *-1.jpg or *-1.png then we will assume
            //  we have a bunch of jpg files made by ffmpeg
            //  movie files can be converted into jpg frames by this command
            //      ffmpeg -i XXXX.mp4 -s 16x50 XXXX-%d.jpg
            //      ffmpeg -i XXXX.avi -s 16x50 XXXX-%d.jpg
            //      ffmpeg -i XXXX.mov -s 16x50 XXXX-%d.jpg
            //      ffmpeg -i XXXX.mts -s 16x50 XXXX-%d.jpg

            if (!media.HasImage(NewPictureName)) {
                media.AddAnimatedImage(NewPictureName, effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetFrameMS());
            }
        }
        auto a = effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia().GetImage(settings["E_TEXTCTRL_Pictures_Filename"]);
        if (!a->IsOk()) {
            spdlog::warn("Could not load image file: {}", settings["E_TEXTCTRL_Pictures_Filename"]);
        }
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

typedef std::vector< std::pair<xlPoint, xlColor> > PixelVector;

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
    if (transparentBlack) {
        int level = c.Red() + c.Green() + c.Blue();
        if (level > transparentBlackLevel) {
            buffer.ProcessPixel(x, y, c, wrap);
        }
    } else {
        buffer.ProcessPixel(x, y, c, wrap);
    }
}

std::list<std::string> PicturesEffect::GetFileReferences(Model* model, const SettingsMap &SettingsMap) const
{
    std::list<std::string> res;
    std::string file = SettingsMap["E_TEXTCTRL_Pictures_Filename"];
    if (!file.empty()) {
        res.push_back(FileUtils::FixFile("", file));
    }
    return res;
}

bool PicturesEffect::CleanupFileLocations(RenderContext* ctx, SettingsMap &SettingsMap)
{
    bool rc = false;
    std::string file = SettingsMap["E_TEXTCTRL_Pictures_Filename"];
    if (FileExists(file)) {
        if (!ctx->IsInShowFolder(file)) {
            SettingsMap["E_TEXTCTRL_Pictures_Filename"] = ctx->MoveToShowFolder(file, std::string(1, std::filesystem::path::preferred_separator) + "Images");
            rc = true;
        }
    }

    return rc;
}

bool PicturesEffect::IsPictureFile(std::string filename)
{
    auto ext = std::filesystem::path(filename).extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

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
    const auto& dirstr = SettingsMap.Get("CHOICE_Pictures_Direction", sDirectionDefault);
    Render(buffer,
           dirstr,
           SettingsMap["TEXTCTRL_Pictures_Filename"],
           SettingsMap.GetFloat("TEXTCTRL_Pictures_Speed", sSpeedDefault),
           SettingsMap.GetFloat("TEXTCTRL_Pictures_FrameRateAdj", sFrameRateAdjDefault),
           dirstr != "vector" ? GetValueCurveInt("PicturesXC", sXCDefault, SettingsMap, oset, sXCMin, sXCMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) : SettingsMap.GetInt("SLIDER_PicturesXC", sXCDefault),
           dirstr != "vector" ? GetValueCurveInt("PicturesYC", sYCDefault, SettingsMap, oset, sYCMin, sYCMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) : SettingsMap.GetInt("SLIDER_PicturesYC", sYCDefault),
           SettingsMap.GetInt("SLIDER_PicturesEndXC", sEndXCDefault),
           SettingsMap.GetInt("SLIDER_PicturesEndYC", sEndYCDefault),
           SettingsMap.GetInt("SLIDER_Pictures_StartScale", sStartScaleDefault),
           SettingsMap.GetInt("SLIDER_Pictures_EndScale", sEndScaleDefault),
           SettingsMap.Get("CHOICE_Scaling", sScalingDefault),
           SettingsMap.GetBool("CHECKBOX_Pictures_PixelOffsets", sPixelOffsetsDefault),
           SettingsMap.GetBool("CHECKBOX_Pictures_WrapX", sWrapXDefault),
           SettingsMap.GetBool("CHECKBOX_Pictures_Shimmer", sShimmerDefault),
           SettingsMap.GetBool("CHECKBOX_LoopGIF", sLoopGIFDefault),
           SettingsMap.GetBool("CHECKBOX_SuppressGIFBackground", sSuppressGIFBackgroundDefault),
           SettingsMap.GetBool("CHECKBOX_Pictures_TransparentBlack", false),
           SettingsMap.GetInt("TEXTCTRL_Pictures_TransparentBlack", 0)
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

        std::string suffix = "";
        std::string fnName = std::filesystem::path(NewPictureName).stem().string();
        if (fnName.length() >= 2) {
            suffix = fnName.substr(fnName.length() - 2);
        }

        if (suffix == "-1") {// do we have a movie file?
            fitAnimation = false;
        }

        if (NewPictureName != cache->PictureName || buffer.needToInit) {
            buffer.needToInit = false;
            scale_image = true;

            // `FileExists(NewPictureName)` is tested on the raw stored
            // path — desktop-saved sequences store absolute paths that
            // don't resolve on iPad (or any machine other than the one
            // they were saved on). Resolve through FixFile first so the
            // existence check uses the actual target path the loader
            // will use; otherwise this guard short-circuits to red and
            // `GetImage` never gets a chance to run its own FixFile.
            std::string resolvedName = FileUtils::FixFile("", NewPictureName);
            if (!buffer.GetSequenceMedia()->HasImage(NewPictureName) &&
                !FileExists(resolvedName, false)) {
                noImageFile = true;
                spdlog::warn("No image for: {}", resolvedName);
            } else {
                cache->PictureName = NewPictureName;
                cache->imageCache = buffer.GetSequenceMedia()->GetImage(NewPictureName);
                if (!cache->imageCache) {
                    noImageFile = true;
                } else {
                    cache->imageCache->MarkIsUsed();
                    cache->imageCount = cache->imageCache->GetImageCount();
                }
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

    int imageWidth = cache->imageCache->GetImageWidth();
    int imageHeight = cache->imageCache->GetImageHeight();
    int imgwidth = imageWidth;
    int imght = imageHeight;
    int yoffset = (BufferHt + imght) / 2; //centered if sizes don't match
    int xoffset = (imgwidth - BufferWi) / 2; //centered if sizes don't match

    std::shared_ptr<xlImage> image;


    if (scale_to_fit == "Scale To Fit" && (BufferWi != imgwidth || BufferHt != imght)) {
        image = cache->imageCache->GetScaledImage(cache->frame, BufferWi, BufferHt, suppressGIFBackground);
        imgwidth = image->GetWidth();
        imght = image->GetHeight();

        yoffset = (BufferHt + imght) / 2; //centered if sizes don't match
        xoffset = (imgwidth - BufferWi) / 2; //centered if sizes don't match
    } else if (scale_to_fit == "Scale Keep Aspect Ratio" || scale_to_fit == "Scale Keep Aspect Ratio Crop") {
        float xr = (float)BufferWi / (float)imageWidth;
        float yr = (float)BufferHt / (float)imageHeight;
        float sc = std::min(xr, yr);
        if (scale_to_fit.find("Crop") != std::string::npos) {
            sc = std::max(xr, yr);
        }
        float newWid = sc * (float)imageWidth;
        float newHi = sc * (float)imageHeight;
        image = cache->imageCache->GetScaledImage(cache->frame, newWid, newHi, suppressGIFBackground);

        imgwidth = image->GetWidth();
        imght = image->GetHeight();
        yoffset = (BufferHt + imght) / 2; //centered if sizes don't match
        xoffset = (imgwidth - BufferWi) / 2; //centered if sizes don't match
    } else {
        if ((start_scale != 100 || end_scale != 100) && scale_image) {
            int delta_scale = end_scale - start_scale;
            int current_scale = start_scale + delta_scale * position;
            imgwidth = (imageWidth * current_scale) / 100;
            imght = (imageHeight * current_scale) / 100;
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
    const xlImage &img = *image.get();
    if (!img.IsOk()) {
        return;
    }
    
    bool hasAlpha = img.HasAlpha();

    int calc_position_wi = (imgwidth + BufferWi) * position;
    int calc_position_ht = (imght + BufferHt) * position;

    for (int x = 0; x < imgwidth; x++) {
        for (int y = 0; y < imght; y++) {
            if (!hasAlpha || !img.IsTransparent(x, y, 1)) {
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
