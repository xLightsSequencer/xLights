#include <catch2/catch_test_macros.hpp>

#include <list>
#include <set>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "import_export/EffectMapper.h"
#include "render/Effect.h"
#include "render/SeqMediaMigration.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFaces.h"
#include "render/SequenceMedia.h"
#include "render/SequencePackage.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/ZipUtils.h"
#include "models/ViewObjectManager.h"
#include "utils/UtilClasses.h"
#include "utils/xlImage.h"

#include <algorithm>
#include <filesystem>
#include <list>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct FixFileGuard {
    explicit FixFileGuard(const std::string& dir) { FileUtils::SetFixFileShowDir(dir); }
    ~FixFileGuard() { FileUtils::SetFixFileShowDir(""); }
};

std::string PngBytes(int seed) {
    xlImage img(2, 2);
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x)
            img.SetRGBA(x, y, static_cast<uint8_t>(seed + x), static_cast<uint8_t>(y), 9, 255);
    std::vector<uint8_t> out;
    img.SaveAsPNG(out);
    return std::string(out.begin(), out.end());
}

bool HasSuffix(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

constexpr const char* kXsq = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence FixedPointTiming=\"1\"><head><version>2024.01</version><sequenceTiming>50 ms</sequenceTiming>"
                             "<sequenceType>Animation</sequenceType><sequenceDuration>10.000</sequenceDuration></head></xsequence>";

constexpr const char* kRgb =
    "<xrgb><models>"
    "<model name=\"Src\">"
    "<faceInfo Name=\"Node\" Type=\"NodeRange\" Mouth-AI=\"1-4\"/>"
    "<faceInfo Name=\"Happy\" Type=\"Matrix\" Mouth-AI-EyesOpen=\"faces/m.png\" Eyes-Open=\"faces/e.png\" Mouth-AI-Color=\"#ff0000\" CustomColors=\"0\"/>"
    "<faceInfo Name=\"Lost\" Type=\"Matrix\" Mouth-E-EyesOpen=\"faces/gone.png\"/>"
    "</model>"
    "<model name=\"Plain\"/>"
    "</models></xrgb>";

// A source show folder holding a plain .xsq + rgbeffects + media, and an empty target show.
struct ImportFixture {
    TempDir src;
    TempDir show;
    HeadlessRenderContext srcCtx;
    HeadlessRenderContext tgtCtx;
    EffectLayer* srcLayer = nullptr;
    EffectLayer* plainLayer = nullptr;
    EffectLayer* tgtLayer = nullptr;
    std::unique_ptr<SequencePackage> pkg;
    FixFileGuard guard;

    Model* tgtModel = nullptr;

    ImportFixture(bool withRgb = true, bool withModels = false) : guard(show.path.string()) {
        if (withModels) {
            pugi::xml_document md;
            REQUIRE(md.load_string("<model name=\"Tgt\" DisplayAs=\"Horiz Matrix\" NumStrings=\"1\" NodesPerString=\"16\" StrandsPerString=\"4\" StringType=\"RGB Nodes\" StartChannel=\"1\"/>"));
            tgtModel = tgtCtx.AllModels.createAndAddModel(md.first_child(), 100, 100);
            REQUIRE(tgtModel != nullptr);
        }
        srcCtx.GetSequenceElements().SetViewsManager(&srcCtx._sequenceViewManager);
        tgtCtx.GetSequenceElements().SetViewsManager(&tgtCtx._sequenceViewManager);
        srcLayer = srcCtx.GetSequenceElements().AddElement("Src", "model", true, false, true, false, false)->AddEffectLayer();
        plainLayer = srcCtx.GetSequenceElements().AddElement("Plain", "model", true, false, true, false, false)->AddEffectLayer();
        tgtLayer = tgtCtx.GetSequenceElements().AddElement("Tgt", "model", true, false, true, false, false)->AddEffectLayer();

        if (withRgb) src.Write("xlights_rgbeffects.xml", kRgb);
        auto xsq = src.Write("seq.xsq", kXsq);
        fs::create_directories(src.path / "media");
        fs::create_directories(src.path / "faces");
        src.Write("media/pic.png", PngBytes(10));
        src.Write("media/clip.mp4", "VIDEO");
        src.Write("media/wave.fs", "/* shader */ void main() {}");
        src.Write("media/anim.gled", "GLED");
        src.Write("media/shape.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"2\" height=\"2\"/>");
        src.Write("faces/m.png", PngBytes(20));
        src.Write("faces/e.png", PngBytes(30));
        pkg = std::make_unique<SequencePackage>(xsq, show.path.string(), "new_sequence", withModels ? &tgtCtx.AllModels : nullptr);
        pkg->FindRGBEffectsFile();
    }

    Effect* SrcEffect(const std::string& name, const std::string& settings, EffectLayer* layer = nullptr) {
        auto* l = layer ? layer : srcLayer;
        return l->AddEffect(0, name, settings, "", l->GetEffectCount() * 100, l->GetEffectCount() * 100 + 100, 0, false);
    }
    std::string Import(Effect* e) { return pkg->FixAndImportMedia(e, tgtLayer); }
    SettingsMap Parse(const std::string& s) {
        SettingsMap m;
        m.Parse(nullptr, s, "");
        return m;
    }
    fs::path Imported(const std::string& sub, const std::string& file) { return show.path / "ImportedMedia" / "seq" / sub / file; }
    std::list<std::string> Missing() { return pkg->GetMissingMedia(); }
};

} // namespace

TEST_CASE("Plain xsq import finds rgbeffects and default folders", "[seq2][package]") {
    ImportFixture f;
    CHECK(f.pkg->HasRGBEffects());
    CHECK(f.pkg->HasMedia());
    CHECK_FALSE(f.pkg->HasMissingMedia());
    CHECK_FALSE(f.pkg->ModelsChanged());
    CHECK(f.pkg->GetTempShowFolder() == f.src.path.string());
    CHECK(f.pkg->GetRgbEffectsFile().child("xrgb").child("models"));
    CHECK(f.pkg->GetImportOptions()->GetDir(MediaTargetDir::GLEDIATORS_DIR) == f.Imported("Glediators", "").parent_path().string());
}

TEST_CASE("FixAndImportMedia copies an external picture and rewrites the setting", "[seq2][package]") {
    ImportFixture f;
    auto* e = f.SrcEffect("Pictures", "E_TEXTCTRL_Pictures_Filename=/old/show/media/pic.png,E_CHECKBOX_X=1");
    auto out = f.Parse(f.Import(e));
    auto target = f.Imported("Images", "pic.png");
    CHECK(fs::exists(target));
    CHECK(HasSuffix(out.Get("E_TEXTCTRL_Pictures_Filename", ""), "ImportedMedia/seq/Images/pic.png"));
    CHECK(out.Get("E_CHECKBOX_X", "") == "1");
    CHECK(f.tgtCtx.GetSequenceElements().GetSequenceMedia().HasImage(out.Get("E_TEXTCTRL_Pictures_Filename", "")));
    CHECK_FALSE(f.pkg->HasMissingMedia());
}

TEST_CASE("FixAndImportMedia handles a Windows style path and the old pictures key", "[seq2][package]") {
    ImportFixture f;
    // the effect itself remaps the legacy key on creation
    auto* e = f.SrcEffect("Pictures", "E_FILEPICKER_Pictures_Filename=C:\\Show\\media\\pic.png");
    auto out = f.Parse(f.Import(e));
    CHECK_FALSE(out.Contains("E_FILEPICKER_Pictures_Filename"));
    CHECK(HasSuffix(out.Get("E_TEXTCTRL_Pictures_Filename", ""), "Images/pic.png"));
    CHECK(fs::exists(f.Imported("Images", "pic.png")));
}

TEST_CASE("FixAndImportMedia carries an embedded picture over without copying a file", "[seq2][package]") {
    ImportFixture f;
    xlImage img(3, 1);
    img.SetRGBA(0, 0, 1, 2, 3, 255);
    f.srcCtx.GetSequenceElements().GetSequenceMedia().AddEmbeddedImage("pics/emb.png", img);
    auto* e = f.SrcEffect("Pictures", "E_TEXTCTRL_Pictures_Filename=pics/emb.png");
    auto out = f.Parse(f.Import(e));
    CHECK(out.Get("E_TEXTCTRL_Pictures_Filename", "") == "pics/emb.png");
    auto& tm = f.tgtCtx.GetSequenceElements().GetSequenceMedia();
    REQUIRE(tm.HasImage("pics/emb.png"));
    CHECK(tm.GetImage("pics/emb.png")->IsEmbedded());
    CHECK(tm.GetImage("pics/emb.png")->GetImageWidth() == 3);
    CHECK_FALSE(fs::exists(f.show.path / "ImportedMedia"));
    CHECK_FALSE(f.pkg->HasMissingMedia());
}

TEST_CASE("FixAndImportMedia copies a picture the source media cache knows as external", "[seq2][package]") {
    ImportFixture f;
    auto loc = (f.src.path / "media" / "pic.png").string();
    f.srcCtx.GetSequenceElements().GetSequenceMedia().GetImage(loc);
    auto* e = f.SrcEffect("Pictures", "E_TEXTCTRL_Pictures_Filename=" + loc);
    auto out = f.Parse(f.Import(e));
    CHECK(HasSuffix(out.Get("E_TEXTCTRL_Pictures_Filename", ""), "Images/pic.png"));
}

TEST_CASE("FixAndImportMedia routes each media type to its folder", "[seq2][package]") {
    ImportFixture f;
    auto video = f.Parse(f.Import(f.SrcEffect("Video", "E_FILEPICKERCTRL_Video_Filename=/x/clip.mp4")));
    CHECK(HasSuffix(video.Get("E_FILEPICKERCTRL_Video_Filename", ""), "Videos/clip.mp4"));
    CHECK(fs::exists(f.Imported("Videos", "clip.mp4")));

    auto shader = f.Parse(f.Import(f.SrcEffect("Shader", "E_0FILEPICKERCTRL_IFS=/x/wave.fs")));
    CHECK(HasSuffix(shader.Get("E_0FILEPICKERCTRL_IFS", ""), "Shaders/wave.fs"));
    CHECK(fs::exists(f.Imported("Shaders", "wave.fs")));

    auto gled = f.Parse(f.Import(f.SrcEffect("Glediator", "E_FILEPICKERCTRL_Glediator_Filename=anim.gled")));
    CHECK(HasSuffix(gled.Get("E_FILEPICKERCTRL_Glediator_Filename", ""), "Glediators/anim.gled"));
    CHECK(fs::exists(f.Imported("Glediators", "anim.gled")));

    auto shape = f.Parse(f.Import(f.SrcEffect("Shape", "E_FILEPICKERCTRL_SVG=/x/shape.svg")));
    CHECK(HasSuffix(shape.Get("E_FILEPICKERCTRL_SVG", ""), "Images/shape.svg"));

    auto ripple = f.Parse(f.Import(f.SrcEffect("Ripple", "E_FILEPICKERCTRL_Ripple_SVG=/x/shape.svg")));
    CHECK(HasSuffix(ripple.Get("E_FILEPICKERCTRL_Ripple_SVG", ""), "Images/shape.svg"));
    CHECK_FALSE(f.pkg->HasMissingMedia());
}

TEST_CASE("FixAndImportMedia leaves effects without media alone", "[seq2][package]") {
    ImportFixture f;
    CHECK(f.Parse(f.Import(f.SrcEffect("Shape", "E_CHOICE_Shape=Circle"))).Get("E_CHOICE_Shape", "") == "Circle");
    CHECK(f.Parse(f.Import(f.SrcEffect("Ripple", "E_CHOICE_Ripple_Object_To_Draw=Circle"))).Get("E_FILEPICKERCTRL_Ripple_SVG", "").empty());
    CHECK(f.Parse(f.Import(f.SrcEffect("Bars", "E_SLIDER_Bars_BarCount=3"))).Get("E_SLIDER_Bars_BarCount", "") == "3");
    // an empty Video filename is not reported as missing
    f.Import(f.SrcEffect("Video", "E_FILEPICKERCTRL_Video_Filename="));
    CHECK_FALSE(f.pkg->HasMissingMedia());
    CHECK_FALSE(fs::exists(f.show.path / "ImportedMedia"));
}

TEST_CASE("FixAndImportMedia records missing media once per name", "[seq2][package]") {
    ImportFixture f;
    auto out = f.Parse(f.Import(f.SrcEffect("Video", "E_FILEPICKERCTRL_Video_Filename=/x/absent.mp4")));
    CHECK(out.Get("E_FILEPICKERCTRL_Video_Filename", "") == "/x/absent.mp4");
    f.Import(f.SrcEffect("Video", "E_FILEPICKERCTRL_Video_Filename=/y/absent.mp4"));
    f.Import(f.SrcEffect("Pictures", "E_TEXTCTRL_Pictures_Filename=nothere.png"));
    CHECK(f.pkg->HasMissingMedia());
    CHECK(f.Missing() == std::list<std::string>{ "absent.mp4", "nothere.png" });
}

TEST_CASE("FixAndImportMedia leaves an embedded shader reference untouched", "[seq2][package]") {
    ImportFixture f;
    auto& sm = f.srcCtx.GetSequenceElements().GetSequenceMedia();
    auto path = (f.src.path / "media" / "wave.fs").string();
    auto sh = sm.GetShader(path);
    REQUIRE(sh != nullptr);
    sm.EmbedMedia(path);
    REQUIRE(sm.GetMediaEmbedState(path).first);
    REQUIRE_FALSE(sm.FindEmbeddedKey(path).empty());
    auto out = f.Parse(f.Import(f.SrcEffect("Shader", "E_0FILEPICKERCTRL_IFS=" + path)));
    CHECK(out.Get("E_0FILEPICKERCTRL_IFS", "") == path);
    CHECK_FALSE(fs::exists(f.Imported("Shaders", "wave.fs")));
    CHECK_FALSE(f.pkg->HasMissingMedia());
}

TEST_CASE("SourceModelHasFace looks up faceInfo in the source rgbeffects", "[seq2][package]") {
    ImportFixture f;
    CHECK(f.pkg->SourceModelHasFace("Src", ""));
    CHECK(f.pkg->SourceModelHasFace("Src", "Happy"));
    CHECK(f.pkg->SourceModelHasFace("Src", "Node"));
    CHECK_FALSE(f.pkg->SourceModelHasFace("Src", "Sad"));
    CHECK_FALSE(f.pkg->SourceModelHasFace("Plain", ""));
    CHECK_FALSE(f.pkg->SourceModelHasFace("Nobody", ""));

    ImportFixture noRgb(false);
    CHECK_FALSE(noRgb.pkg->SourceModelHasFace("Src", ""));
}

TEST_CASE("ImportModelFaceToSequence embeds a matrix face's images into the target sequence", "[seq2][package]") {
    ImportFixture f;
    auto* e = f.SrcEffect("Faces", "E_CHOICE_Faces_FaceDefinition=Happy");
    REQUIRE(f.pkg->ImportModelFaceToSequence(e, f.tgtLayer, "Happy"));
    auto& faces = f.tgtCtx.GetSequenceElements().GetSequenceFaces();
    const auto* def = faces.GetFace("Happy");
    REQUIRE(def != nullptr);
    CHECK(def->at("Type") == "Matrix");
    CHECK(def->at("Mouth-AI-EyesOpen") == "faces/m.png");
    CHECK(def->at("Mouth-AI-Color") == "#ff0000");
    CHECK(def->count("Name") == 0);
    auto& tm = f.tgtCtx.GetSequenceElements().GetSequenceMedia();
    REQUIRE(tm.HasImage("faces/m.png"));
    CHECK(tm.GetImage("faces/m.png")->IsEmbedded());
    CHECK(tm.IsUsedByMetadata("faces/m.png"));
    CHECK(tm.IsUsedByMetadata("faces/e.png"));
    CHECK_FALSE(tm.HasImage("#ff0000"));
    CHECK_FALSE(f.pkg->HasMissingMedia());

    // a second import of the same face is a no-op that still reports success
    CHECK(f.pkg->ImportModelFaceToSequence(e, f.tgtLayer, "Happy"));
}

TEST_CASE("ImportModelFaceToSequence picks the first matrix face for Default and skips node faces", "[seq2][package]") {
    ImportFixture f;
    auto* e = f.SrcEffect("Faces", "");
    REQUIRE(f.pkg->ImportModelFaceToSequence(e, f.tgtLayer, "Default"));
    CHECK(f.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFace("Happy") != nullptr);
    CHECK(f.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFace("Node") == nullptr);
    CHECK_FALSE(f.pkg->ImportModelFaceToSequence(e, f.tgtLayer, "Node"));
    CHECK_FALSE(f.pkg->ImportModelFaceToSequence(e, f.tgtLayer, "Unknown"));
    auto* p = f.SrcEffect("Faces", "", f.plainLayer);
    CHECK_FALSE(f.pkg->ImportModelFaceToSequence(p, f.tgtLayer, ""));
}

TEST_CASE("ImportModelFaceToSequence records face images it cannot find", "[seq2][package]") {
    ImportFixture f;
    auto* e = f.SrcEffect("Faces", "");
    REQUIRE(f.pkg->ImportModelFaceToSequence(e, f.tgtLayer, "Lost"));
    CHECK(f.Missing() == std::list<std::string>{ "gone.png" });
    const auto* def = f.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFace("Lost");
    REQUIRE(def != nullptr);
    CHECK(def->at("Mouth-E-EyesOpen") == "faces/gone.png");
}

TEST_CASE("ImportModelFaceToSequence needs the source rgbeffects", "[seq2][package]") {
    ImportFixture f(false);
    auto* e = f.SrcEffect("Faces", "");
    CHECK_FALSE(f.pkg->ImportModelFaceToSequence(e, f.tgtLayer, "Happy"));
}

TEST_CASE("ImportSequenceFaceInfo copies a sequence level face definition", "[seq2][package]") {
    ImportFixture f;
    auto& srcSE = f.srcCtx.GetSequenceElements();
    xlImage img(2, 1);
    srcSE.GetSequenceMedia().AddEmbeddedImage("emb/mouth.png", img);
    srcSE.GetSequenceFaces().SetFace("Seq", { { "Type", "Matrix" }, { "Mouth-AI-EyesOpen", "emb/mouth.png" }, { "Eyes-Open", "faces/e.png" }, { "Eyes-Closed", "faces/none.png" }, { "Mouth-O-Color", "#00ff00" } });
    auto* e = f.SrcEffect("Faces", "", f.plainLayer);

    REQUIRE(f.pkg->ImportSequenceFaceInfo(e, f.tgtLayer, "Seq"));
    auto& tgtSE = f.tgtCtx.GetSequenceElements();
    const auto* def = tgtSE.GetSequenceFaces().GetFace("Seq");
    REQUIRE(def != nullptr);
    CHECK(def->at("Mouth-AI-EyesOpen") == "emb/mouth.png");
    CHECK(tgtSE.GetSequenceMedia().HasImage("emb/mouth.png"));
    CHECK(tgtSE.GetSequenceMedia().IsUsedByMetadata("emb/mouth.png"));
    CHECK(fs::exists(f.show.path / "DownloadedFaces" / "e.png"));
    CHECK(f.Missing() == std::list<std::string>{ "none.png" });

    // already imported
    CHECK(f.pkg->ImportSequenceFaceInfo(e, f.tgtLayer, "Seq"));
    // not a sequence face
    CHECK_FALSE(f.pkg->ImportSequenceFaceInfo(e, f.tgtLayer, "Other"));
}

TEST_CASE("ImportSequenceFaceInfo defers to model faces at the source", "[seq2][package]") {
    ImportFixture f;
    auto& srcSE = f.srcCtx.GetSequenceElements();
    srcSE.GetSequenceFaces().SetFace("Happy", { { "Type", "Matrix" } });
    srcSE.GetSequenceFaces().SetFace("Alpha", { { "Type", "Matrix" } });

    // Src has model faces: Default resolves to the model face, a named model face shadows the sequence one
    auto* onSrc = f.SrcEffect("Faces", "");
    CHECK_FALSE(f.pkg->ImportSequenceFaceInfo(onSrc, f.tgtLayer, "Default"));
    CHECK_FALSE(f.pkg->ImportSequenceFaceInfo(onSrc, f.tgtLayer, "Happy"));
    CHECK(f.pkg->ImportSequenceFaceInfo(onSrc, f.tgtLayer, "Alpha"));

    // Plain has no model faces: Default is the first sequence face
    auto* onPlain = f.SrcEffect("Faces", "", f.plainLayer);
    CHECK(f.pkg->ImportSequenceFaceInfo(onPlain, f.tgtLayer, ""));
    CHECK(f.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFace("Alpha") != nullptr);
}

TEST_CASE("ImportSequenceFaceInfo with no sequence faces does nothing", "[seq2][package]") {
    ImportFixture f;
    auto* e = f.SrcEffect("Faces", "", f.plainLayer);
    CHECK_FALSE(f.pkg->ImportSequenceFaceInfo(e, f.tgtLayer, "Default"));
    CHECK(f.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFaces().empty());
}

TEST_CASE("FixAndImportMedia of a Faces effect imports the sequence face", "[seq2][package]") {
    ImportFixture f;
    f.srcCtx.GetSequenceElements().GetSequenceFaces().SetFace("Seq", { { "Type", "Matrix" }, { "Eyes-Open", "faces/e.png" } });
    auto* e = f.SrcEffect("Faces", "E_CHOICE_Faces_FaceDefinition=Seq", f.plainLayer);
    auto out = f.Parse(f.Import(e));
    CHECK(out.Get("E_CHOICE_Faces_FaceDefinition", "") == "Seq");
    CHECK(f.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFace("Seq") != nullptr);

    // a model face with no model manager is skipped without error
    auto* m = f.SrcEffect("Faces", "E_CHOICE_Faces_FaceDefinition=Happy");
    f.Import(m);
    f.pkg->SetImportFacesToSequence(true);
    CHECK(f.pkg->IsImportFacesToSequence());
    f.Import(m);
    CHECK_FALSE(f.pkg->ModelsChanged());
}

TEST_CASE("ImportFaceInfo adds a matrix face to the target model and copies its images", "[seq2][package]") {
    ImportFixture f(true, true);
    auto* e = f.SrcEffect("Faces", "E_CHOICE_Faces_FaceDefinition=Happy");
    f.pkg->ImportFaceInfo(e, f.tgtLayer, "Happy");
    CHECK(f.pkg->ModelsChanged());
    const auto& fi = f.tgtModel->GetFaceInfo();
    REQUIRE(fi.count("Happy") == 1);
    const auto& def = fi.at("Happy");
    CHECK(def.at("Type") == "Matrix");
    CHECK(def.at("CustomColors") == "0");
    CHECK(HasSuffix(def.at("Mouth-AI-EyesOpen"), "DownloadedFaces/m.png"));
    CHECK(fs::exists(f.show.path / "DownloadedFaces" / "m.png"));
    CHECK(fs::exists(f.show.path / "DownloadedFaces" / "e.png"));

    // a face the model already has is left alone
    f.pkg->ImportFaceInfo(e, f.tgtLayer, "Happy");
    CHECK(f.tgtModel->GetFaceInfo().size() == fi.size());

    // model faces on the target win over sequence and model-to-sequence imports
    f.srcCtx.GetSequenceElements().GetSequenceFaces().SetFace("Happy", { { "Type", "Matrix" } });
    auto* onPlain = f.SrcEffect("Faces", "", f.plainLayer);
    CHECK(f.pkg->ImportSequenceFaceInfo(onPlain, f.tgtLayer, "Happy"));
    CHECK(f.pkg->ImportModelFaceToSequence(e, f.tgtLayer, "Happy"));
    CHECK(f.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFace("Happy") == nullptr);
}

TEST_CASE("ImportFaceInfo keeps face colour attributes", "[seq2]") {
    ImportFixture f(true, true);
    auto* e = f.SrcEffect("Faces", "E_CHOICE_Faces_FaceDefinition=Happy");
    f.pkg->ImportFaceInfo(e, f.tgtLayer, "Happy");
    const auto& fi = f.tgtModel->GetFaceInfo();
    REQUIRE(fi.count("Happy") == 1);
    CHECK(fi.at("Happy").count("Mouth-AI-Color") == 1);
    CHECK_FALSE(f.pkg->HasMissingMedia());
}

TEST_CASE("ImportFaceInfo for Default takes the first matrix face and records missing images", "[seq2][package]") {
    ImportFixture f(true, true);
    auto* e = f.SrcEffect("Faces", "");
    f.pkg->ImportFaceInfo(e, f.tgtLayer, "Lost");
    CHECK(f.tgtModel->GetFaceInfo().count("Lost") == 1);
    CHECK(f.Missing() == std::list<std::string>{ "gone.png" });

    ImportFixture g(true, true);
    auto* d = g.SrcEffect("Faces", "");
    g.pkg->ImportFaceInfo(d, g.tgtLayer, "Default");
    CHECK(g.tgtModel->GetFaceInfo().count("Happy") == 1);
    CHECK(g.tgtModel->GetFaceInfo().count("Node") == 0);

    // a node face is never imported, and an unknown source model is ignored
    ImportFixture h(true, true);
    h.pkg->ImportFaceInfo(h.SrcEffect("Faces", ""), h.tgtLayer, "Node");
    h.pkg->ImportFaceInfo(h.SrcEffect("Faces", "", h.plainLayer), h.tgtLayer, "Happy");
    CHECK_FALSE(h.pkg->ModelsChanged());
}

TEST_CASE("FixAndImportMedia of a model Faces effect imports into the target layout", "[seq2][package]") {
    ImportFixture f(true, true);
    auto* e = f.SrcEffect("Faces", "E_CHOICE_Faces_FaceDefinition=Happy");
    f.Import(e);
    CHECK(f.tgtModel->GetFaceInfo().count("Happy") == 1);
    CHECK(f.pkg->ModelsChanged());
}

TEST_CASE("ConvertIncompatibleVideos reports failed transcodes and converts gifs", "[seq2][package]") {
    TempDir d;
    HeadlessRenderContext ctx;
    auto& se = ctx.GetSequenceElements();
    se.SetViewsManager(&ctx._sequenceViewManager);
    auto* m = dynamic_cast<ModelElement*>(se.AddElement("M", "model", true, false, true, false, false));
    auto* strandLayer = m->GetStrand(0, true)->GetEffectLayer(0);
    auto* subLayer = m->GetSubModel("S", true)->GetEffectLayer(0);
    auto junk = d.Write("junk.avi", "not a video");
    auto mov = d.Write("old.mov", "not a video either");
    auto gif = d.File("anim.gif");
    strandLayer->AddEffect(0, "Video", "E_FILEPICKERCTRL_Video_Filename=" + gif, "", 0, 100, 0, false);
    subLayer->AddEffect(0, "Video", "E_FILEPICKERCTRL_Video_Filename=" + gif, "", 0, 100, 0, false);
    subLayer->AddEffect(0, "Video", "E_FILEPICKERCTRL_Video_Filename=" + junk, "", 200, 300, 0, false);

    std::vector<MediaCompatibilityIssue> issues{
        { junk, "Unsupported video codec", true },
        { mov, "Unsupported video codec", true },
        { gif, "Animated GIF", true },
        { d.File("a.mp3"), "Unsupported audio codec", false },
    };
    std::vector<std::string> seen;
    auto r = seqmedia::ConvertIncompatibleVideos(se, issues, [&](size_t i, size_t n, const std::string& src) {
        seen.push_back(src);
        CHECK(n == 2);
        return true;
    });
    CHECK(r.gifEffectsConverted == 2);
    CHECK(strandLayer->GetEffect(0)->GetEffectName() == "Pictures");
    CHECK(r.attempted == 2);
    CHECK(seen == std::vector<std::string>{ junk, mov });
    CHECK(r.converted == 0);
    CHECK(r.effectsUpdated == 0);
    CHECK_FALSE(r.cancelled);
    REQUIRE(r.failures.size() == 2);
    CHECK(r.failures[0].rfind("junk.avi: ", 0) == 0);
    CHECK(r.failures[1].rfind("old.mov: ", 0) == 0);
    // no partial outputs are left behind, and the source .mov is never overwritten
    CHECK_FALSE(fs::exists(d.path / "junk.mov"));
    CHECK_FALSE(fs::exists(d.path / "old_converted.mov"));
    CHECK(fs::exists(mov));
    CHECK(subLayer->GetEffect(1)->GetSetting("E_FILEPICKERCTRL_Video_Filename") == junk);
}

TEST_CASE("ConvertIncompatibleVideos stops when the caller cancels", "[seq2][package]") {
    TempDir d;
    HeadlessRenderContext ctx;
    auto& se = ctx.GetSequenceElements();
    se.SetViewsManager(&ctx._sequenceViewManager);
    auto junk = d.Write("junk.avi", "x");
    std::vector<MediaCompatibilityIssue> issues{ { junk, "Unsupported video codec", true }, { d.Write("b.avi", "x"), "Unsupported video codec", true } };
    int calls = 0;
    auto r = seqmedia::ConvertIncompatibleVideos(se, issues, [&](size_t, size_t, const std::string&) {
        ++calls;
        return false;
    }, [](int, int) { return true; });
    CHECK(calls == 1);
    CHECK(r.cancelled);
    CHECK(r.attempted == 2);
    CHECK(r.failures.empty());
}

TEST_CASE("Pack bundles the images of the face definitions the sequence uses", "[seq2][package]") {
    TempDir show;
    TempDir ext;
    FixFileGuard guard(show.path.string());
    fs::create_directories(ext.path / "Woody");
    fs::create_directories(ext.path / "Seq");
    auto m = ext.Write("Woody/m.png", PngBytes(1));
    auto e = ext.Write("Woody/e.png", PngBytes(2));
    auto u = ext.Write("Woody/u.png", PngBytes(3));
    auto sface = ext.Write("Seq/s.png", PngBytes(4));
    show.Write("xlights_rgbeffects.xml", "<xrgb><models/></xrgb>");

    HeadlessRenderContext ctx;
    ctx.showDirectory = show.path.string();
    auto& se = ctx.GetSequenceElements();
    se.SetViewsManager(&ctx._sequenceViewManager);
    pugi::xml_document md;
    REQUIRE(md.load_string(("<model name=\"Face\" DisplayAs=\"Horiz Matrix\" NumStrings=\"1\" NodesPerString=\"16\" StrandsPerString=\"4\" StringType=\"RGB Nodes\" StartChannel=\"1\">"
                            "<faceInfo Name=\"Happy\" Type=\"Matrix\" CustomColors=\"0\" ImagePlacement=\"Centered\" Mouth-AI-EyesOpen=\"" + m + "\" Eyes-Open=\"" + e + "\"/>"
                            "<faceInfo Name=\"Unused\" Type=\"Matrix\" Mouth-AI-EyesOpen=\"" + u + "\"/>"
                            "<faceInfo Name=\"Broken\" Type=\"Matrix\" Mouth-AI-EyesOpen=\"" + (ext.path / "gone" / "x.png").string() + "\"/>"
                            "<faceInfo Name=\"Nodes\" Type=\"NodeRange\" Mouth-AI=\"1-3\"/>"
                            "</model>").c_str()));
    REQUIRE(ctx.AllModels.createAndAddModel(md.first_child(), 100, 100) != nullptr);
    auto* layer = se.AddElement("Face", "model", true, false, true, false, false)->AddEffectLayer();
    layer->AddEffect(0, "Faces", "E_CHOICE_Faces_FaceDefinition=Happy", "", 0, 100, 0, false);
    layer->AddEffect(0, "Faces", "E_CHOICE_Faces_FaceDefinition=Broken", "", 100, 200, 0, false);
    layer->AddEffect(0, "Faces", "E_CHOICE_Faces_FaceDefinition=Nodes", "", 200, 300, 0, false);
    layer->AddEffect(0, "Text", "E_FONTPICKER_Text_Font=0;-16;0;0;0;400;0;0;0;0;0;0;0;0;No Such Font Face", "", 300, 400, 0, false);
    layer->AddEffect(0, "Text", "E_FONTPICKER_Text_Font='Missing Quoted Face' 12", "", 400, 500, 0, false);
    layer->AddEffect(0, "Text", "E_FONTPICKER_Text_Font=NoSuchBareFace 10", "", 500, 600, 0, false);
    se.GetSequenceFaces().SetFace("SeqFace", { { "Type", "Matrix" }, { "Eyes-Open", sface } });
    se.GetSequenceMedia().AddEmbeddedImage("emb/face.png", xlImage(2, 2));
    se.GetSequenceFaces().SetFace("EmbFace", { { "Type", "Matrix" }, { "Eyes-Open", "emb/face.png" } });

    auto xsq = show.Write("seq.xsq", kXsq);
    auto out = show.path / "out.xsqz";
    std::vector<std::string> warnings;
    REQUIRE(SequencePackage::Pack(out, show.path.string(), xsq, "", {}, {}, se.GetSequenceMedia(), ctx.AllModels, ctx.AllObjects, se, {}, &warnings));
    auto list = ZipUtils::ListEntries(out.string());
    std::set<std::string> entries(list.begin(), list.end());
    CHECK(entries.count("Faces/Woody/m.png") == 1);
    CHECK(entries.count("Faces/Woody/e.png") == 1);
    CHECK(entries.count("Faces/Woody/u.png") == 0);
    CHECK(entries.count("Faces/Seq/s.png") == 1);
    CHECK(*ZipUtils::ReadEntry(out.string(), "Faces/Woody/m.png") == PngBytes(1));
    bool warnedBroken = false;
    for (const auto& w : warnings) {
        if (w.find("x.png") != std::string::npos && w.find("Face") != std::string::npos) warnedBroken = true;
    }
    CHECK(warnedBroken);
    // fonts that cannot be found are not bundled
    for (const auto& n : entries) CHECK(n.rfind("Fonts/", 0) != 0);
    // the rgbeffects file is always packaged
    auto rgb = ZipUtils::ReadEntry(out.string(), "xlights_rgbeffects.xml");
    REQUIRE(rgb.has_value());
}

TEST_CASE("Model Faces onto a base show folder model go into the sequence", "[seq2][package]") {
    // The base copy replaces a FromBase model on the next load, so a face
    // added to it would be lost - the definition must land in the sequence.
    ImportFixture f(true, true);
    f.tgtModel->SetFromBase(true);
    REQUIRE_FALSE(f.pkg->IsImportFacesToSequence());
    CHECK(f.pkg->ImportsFacesToSequence(f.tgtLayer));

    f.SrcEffect("Faces", "E_CHOICE_Faces_FaceDefinition=Happy");
    std::vector<EffectLayer*> mapped;
    MapXLightsEffects(f.tgtLayer, f.srcLayer, mapped, false, *f.pkg, false, { { "Src", "Tgt" } }, false, {});
    REQUIRE(f.tgtLayer->GetEffectCount() == 1);
    CHECK(f.tgtModel->GetFaceInfo().count("Happy") == 0);
    CHECK_FALSE(f.pkg->ModelsChanged());
    const auto* def = f.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFace("Happy");
    REQUIRE(def != nullptr);
    CHECK(def->at("Type") == "Matrix");
    CHECK(f.tgtCtx.GetSequenceElements().GetSequenceMedia().HasImage(def->at("Mouth-AI-EyesOpen")));

    // calling the layout path directly is redirected the same way
    ImportFixture g(true, true);
    g.tgtModel->SetFromBase(true);
    g.pkg->ImportFaceInfo(g.SrcEffect("Faces", ""), g.tgtLayer, "Happy");
    CHECK(g.tgtModel->GetFaceInfo().count("Happy") == 0);
    CHECK(g.tgtCtx.GetSequenceElements().GetSequenceFaces().GetFace("Happy") != nullptr);

    // a local model still takes the face when the option is off
    ImportFixture h(true, true);
    CHECK_FALSE(h.pkg->ImportsFacesToSequence(h.tgtLayer));
}
