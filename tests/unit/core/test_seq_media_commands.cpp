#include <catch2/catch_test_macros.hpp>

#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceFaces.h"
#include "render/SequenceMedia.h"
#include "render/SequenceMediaCommands.h"
#include "render/SequenceMutationGuard.h"
#include "render/UndoManager.h"
#include "utils/Base64.h"
#include "utils/xlImage.h"

#include "../support/TempDir.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// Commands for sequence media: PutMedia / RemoveMedia / RenameMedia, the
// batches built from them and the edit-site helpers the media UIs call.

namespace {
struct MediaFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    ModelElement* model = nullptr;
    EffectLayer* layer = nullptr;
    MediaFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
        model = dynamic_cast<ModelElement*>(se.AddElement("Tree", "model", true, false, true, false, false));
        layer = model->GetEffectLayerCount() > 0 ? model->GetEffectLayer(0) : model->AddEffectLayer();
    }
    SequenceMedia& Media() { return se.GetSequenceMedia(); }
    UndoManager& Undo() { return se.get_undo_mgr(); }
    Effect* Picture(const std::string& file, int start, int end, bool locked = false) {
        std::string settings = "E_TEXTCTRL_Pictures_Filename=" + file;
        if (locked) settings += ",X_Effect_Locked=True";
        SeqMutationGuard::Scope setup;
        return layer->AddEffect(0, "Pictures", settings, "", start, end, 0, false);
    }
    // Dirty range the model has accumulated since the last call.
    std::pair<int, int> TakeDirty() {
        int changes = 0, s = -1, e = -1;
        model->GetAndResetDirtyRange(changes, s, e);
        return { s, e };
    }
};

struct GuardMode {
    SeqMutationGuard::Mode prev;
    explicit GuardMode(SeqMutationGuard::Mode m) : prev(SeqMutationGuard::GetMode()) {
        SeqMutationGuard::SetMode(m);
        SeqMutationGuard::ResetViolationCount();
    }
    ~GuardMode() { SeqMutationGuard::SetMode(prev); }
};

std::string Png(int w, int h, uint8_t shade) {
    xlImage img(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            img.SetRGB(x, y, shade, shade, shade);
    std::vector<uint8_t> png;
    REQUIRE(img.SaveAsPNG(png));
    return std::string(png.begin(), png.end());
}

MediaEntryState Embedded(const std::string& key, const std::string& bytes, MediaType type = MediaType::Image) {
    auto s = MediaEntryState::EmbeddedBytes(key, type, std::vector<uint8_t>(bytes.begin(), bytes.end()));
    REQUIRE(s.has_value());
    return *s;
}

std::string Bytes(const MediaEntryState& s) {
    REQUIRE(s.data);
    auto v = Base64::Decode(*s.data);
    return std::string(v.begin(), v.end());
}

std::unique_ptr<SequenceCommand> RoundTrip(const SequenceCommand& c) {
    return CommandFromJson(c.ToJson());
}
} // namespace

TEST_CASE("PutMedia adds, replaces and undoes an embedded entry", "[seq][media][commands]") {
    MediaFixture f;
    const std::string a = Png(2, 2, 10), b = Png(3, 3, 200);

    PutMediaCommand add(Embedded("Images/a.png", a));
    auto undoAdd = add.Apply(f.se);
    REQUIRE(undoAdd);
    CHECK(undoAdd->Type() == CommandType::RemoveMedia);
    auto st = f.Media().GetEntryState("Images/a.png");
    REQUIRE(st);
    CHECK(st->embedded);
    CHECK(Bytes(*st) == a);
    auto img = f.Media().GetImage("Images/a.png");
    REQUIRE(img);
    CHECK(img->GetImageWidth() == 2);

    // Replacing the content: the inverse puts the old bytes back.
    PutMediaCommand replace(Embedded("Images/a.png", b));
    auto undoReplace = replace.Apply(f.se);
    REQUIRE(undoReplace);
    CHECK(undoReplace->Type() == CommandType::PutMedia);
    CHECK(Bytes(*f.Media().GetEntryState("Images/a.png")) == b);
    CHECK(f.Media().GetImage("Images/a.png")->GetImageWidth() == 3);

    REQUIRE(undoReplace->Apply(f.se, ApplyMode::Lenient));
    CHECK(Bytes(*f.Media().GetEntryState("Images/a.png")) == a);
    CHECK(f.Media().GetImage("Images/a.png")->GetImageWidth() == 2);

    REQUIRE(undoAdd->Apply(f.se, ApplyMode::Lenient));
    CHECK_FALSE(f.Media().HasMedia("Images/a.png"));
}

TEST_CASE("PutMedia validates strictly and leniently", "[seq][media][commands]") {
    MediaFixture f;
    REQUIRE(PutMediaCommand(Embedded("x.txt", "hello", MediaType::TextFile)).Apply(f.se));

    // Strict refuses to change an entry's type; lenient replaces it.
    PutMediaCommand asImage(Embedded("x.txt", Png(1, 1, 0)));
    CHECK(asImage.Validate(f.se, ApplyMode::Strict).status == CommandStatus::Invalid);
    CHECK(asImage.Validate(f.se, ApplyMode::Lenient).Ok());

    MediaEntryState empty;
    empty.key = "e.png";
    empty.embedded = true;
    CHECK(PutMediaCommand(empty).Validate(f.se, ApplyMode::Strict).status == CommandStatus::Invalid);

    // Videos can never be embedded.
    MediaEntryState video = MediaEntryState::FileReference("v.mp4", MediaType::Video);
    video.embedded = true;
    video.data = std::make_shared<const std::string>("AAAA");
    CHECK(PutMediaCommand(video).Validate(f.se, ApplyMode::Lenient).status == CommandStatus::Invalid);
    CommandResult r;
    CHECK_FALSE(PutMediaCommand(video).Apply(f.se, ApplyMode::Lenient, &r));
    CHECK(r.status == CommandStatus::Invalid);
    CHECK_FALSE(f.Media().HasMedia("v.mp4"));

    MediaEntryState noKey;
    CHECK(PutMediaCommand(noKey).Validate(f.se, ApplyMode::Lenient).status == CommandStatus::Invalid);
}

TEST_CASE("RemoveMedia restores the removed entry on undo", "[seq][media][commands]") {
    MediaFixture f;
    const std::string svg = "<svg xmlns='http://www.w3.org/2000/svg' width='4' height='4'/>";
    REQUIRE(PutMediaCommand(Embedded("Shapes/s.svg", svg, MediaType::SVG)).Apply(f.se));

    CommandResult missing;
    CHECK_FALSE(RemoveMediaCommand("nope.svg").Apply(f.se, ApplyMode::Lenient, &missing));
    CHECK(missing.status == CommandStatus::TargetMissing);

    auto undo = RemoveMediaCommand("Shapes/s.svg").Apply(f.se);
    REQUIRE(undo);
    CHECK_FALSE(f.Media().HasMedia("Shapes/s.svg"));
    REQUIRE(undo->Apply(f.se, ApplyMode::Lenient));
    auto st = f.Media().GetEntryState("Shapes/s.svg");
    REQUIRE(st);
    CHECK(st->type == MediaType::SVG);
    CHECK(st->embedded);
    CHECK(Bytes(*st) == svg);
    CHECK(f.Media().GetSVG("Shapes/s.svg")->GetSVGContent() == svg);
}

TEST_CASE("Media commands survive a JSON round trip", "[seq][media][commands]") {
    MediaFixture f;
    std::vector<xlImage> frames(2, xlImage(2, 2));
    auto series = MediaEntryState::EmbeddedFrames("anim-1.png", frames, 50);
    REQUIRE(series);
    MediaEntryState bin = MediaEntryState::FileReference("g.gled", MediaType::BinaryFile, "glediator");

    for (const MediaEntryState& s : { Embedded("Images/a.png", Png(2, 2, 1)), *series, bin }) {
        PutMediaCommand c(s);
        auto back = RoundTrip(c);
        REQUIRE(back);
        REQUIRE(back->Type() == CommandType::PutMedia);
        CHECK(static_cast<PutMediaCommand&>(*back).GetState() == s);
        CHECK(back->ToJson() == c.ToJson());
    }
    auto rm = RoundTrip(RemoveMediaCommand("k.png"));
    REQUIRE(rm);
    CHECK(static_cast<RemoveMediaCommand&>(*rm).GetKey() == "k.png");
    auto rn = RoundTrip(RenameMediaCommand("a.png", "b.png", false));
    REQUIRE(rn);
    auto& rnc = static_cast<RenameMediaCommand&>(*rn);
    CHECK(rnc.GetFrom() == "a.png");
    CHECK(rnc.GetTo() == "b.png");
    CHECK_FALSE(rnc.GetMoveEntry());

    CHECK(CommandFromJson(R"({"type":"PutMedia","key":"a","mediaType":"Nope","embedded":true})") == nullptr);
    CHECK(CommandFromJson(R"({"type":"PutMedia","key":"","mediaType":"Image","embedded":false})") == nullptr);
    CHECK(CommandFromJson(R"({"type":"PutMedia","key":"a","mediaType":"Image","embedded":true,"frames":[{"time":"x"}]})") == nullptr);
    CHECK(CommandFromJson(R"({"type":"RemoveMedia"})") == nullptr);
    CHECK(CommandFromJson(R"({"type":"RenameMedia","from":"a","to":"b"})") == nullptr);
}

TEST_CASE("A frame series round-trips through PutMedia", "[seq][media][commands]") {
    MediaFixture f;
    std::vector<xlImage> frames;
    frames.emplace_back(3, 2);
    frames.emplace_back(3, 2);
    frames[1].SetRGB(0, 0, 255, 0, 0);
    REQUIRE(SeqMediaEdits::EmbedFrames(f.se, "Scene/anim-1.png", frames, 40));
    auto img = f.Media().GetImage("Scene/anim-1.png");
    REQUIRE(img);
    CHECK(img->GetImageCount() == 2);
    CHECK(img->GetImageWidth() == 3);
    auto st = f.Media().GetEntryState("Scene/anim-1.png");
    REQUIRE(st);
    CHECK(st->frames.size() == 2);
    CHECK(st->frames[0].timeMS == 40);

    // Removing and putting it back keeps the frames.
    auto undo = RemoveMediaCommand("Scene/anim-1.png").Apply(f.se);
    REQUIRE(undo);
    REQUIRE(undo->Apply(f.se, ApplyMode::Lenient));
    CHECK(f.Media().GetEntryState("Scene/anim-1.png")->SameContent(*st));
    CHECK(f.Media().GetImage("Scene/anim-1.png")->GetImageCount() == 2);
}

TEST_CASE("Media edits are one undo step each and redo", "[seq][media][commands][undo]") {
    MediaFixture f;
    REQUIRE(SeqMediaEdits::EmbedImage(f.se, "AIImages/a.png", xlImage(4, 4), "Add Image"));
    CHECK(f.Undo().GetUndoString() == "Undo: Add Image");
    f.Undo().UndoLastStep();
    CHECK_FALSE(f.Media().HasMedia("AIImages/a.png"));
    CHECK(f.Undo().GetRedoString() == "Redo: Add Image");
    f.Undo().RedoLastStep();
    CHECK(f.Media().HasMedia("AIImages/a.png"));
    CHECK(f.Media().GetImage("AIImages/a.png")->GetImageWidth() == 4);
}

TEST_CASE("Renaming media repoints effects and faces in one undoable step", "[seq][media][commands][undo]") {
    MediaFixture f;
    Effect* e1 = f.Picture("img/a.png", 0, 100);
    Effect* e2 = f.Picture("img/a.png", 200, 300, /*locked=*/true);
    Effect* other = f.Picture("img/other.png", 400, 500);
    REQUIRE(SeqMediaEdits::Put(f.se, Embedded("img/a.png", Png(2, 2, 5))));
    f.se.GetSequenceFaces().SetFace("Face1", { { "Type", "Matrix" }, { "Mouth-AI", "img/a.png" } });
    f.Undo().Clear();

    REQUIRE(SeqMediaEdits::Rename(f.se, "img/a.png", "img/b.png"));
    CHECK(f.Undo().GetUndoString() == "Undo: Rename Media");
    CHECK(f.Media().HasMedia("img/b.png"));
    CHECK_FALSE(f.Media().HasMedia("img/a.png"));
    CHECK(e1->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/b.png");
    CHECK(e2->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/b.png"); // locked effects follow too
    CHECK(other->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/other.png");
    CHECK(f.se.GetSequenceFaces().CountImageReferences("img/b.png") == 1);

    f.Undo().UndoLastStep();
    CHECK(f.Media().HasMedia("img/a.png"));
    CHECK_FALSE(f.Media().HasMedia("img/b.png"));
    CHECK(e1->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/a.png");
    CHECK(e2->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/a.png");
    CHECK(f.se.GetSequenceFaces().CountImageReferences("img/a.png") == 1);
    CHECK_FALSE(f.Undo().CanUndo());

    // A taken name is refused unless merging; merging drops the old entry.
    REQUIRE(SeqMediaEdits::Put(f.se, Embedded("img/c.png", Png(1, 1, 9))));
    CHECK_FALSE(SeqMediaEdits::Rename(f.se, "img/a.png", "img/c.png"));
    CHECK(f.Media().HasMedia("img/a.png"));
    REQUIRE(SeqMediaEdits::Rename(f.se, "img/a.png", "img/c.png", /*mergeIfTaken=*/true));
    CHECK_FALSE(f.Media().HasMedia("img/a.png"));
    CHECK(e1->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/c.png");
    CHECK(Bytes(*f.Media().GetEntryState("img/c.png")) == Png(1, 1, 9));
    f.Undo().UndoLastStep();
    CHECK(Bytes(*f.Media().GetEntryState("img/a.png")) == Png(2, 2, 5));
    CHECK(e1->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/a.png");
}

TEST_CASE("BuildRenameMedia batches the rename with every referencing effect", "[seq][media][commands]") {
    MediaFixture f;
    f.Picture("p.png", 0, 100);
    f.Picture("p.png", 100, 200);
    REQUIRE(SeqMediaEdits::Put(f.se, Embedded("p.png", Png(1, 1, 1))));
    auto batch = BuildRenameMedia(f.se, "p.png", "q.png");
    REQUIRE(batch->GetCommands().size() == 3);
    CHECK(batch->GetCommands()[0]->Type() == CommandType::RenameMedia);
    CHECK(batch->GetCommands()[1]->Type() == CommandType::ModifyEffect);
    CHECK(batch->GetCommands()[2]->Type() == CommandType::ModifyEffect);
    CHECK(BuildRenameMedia(f.se, "p.png", "p.png")->IsEmpty());

    // Applying the batch and then its inverse leaves the document as it was.
    std::string before = DescribeSequence(f.se);
    auto inverse = batch->Apply(f.se, ApplyMode::Lenient);
    REQUIRE(inverse);
    CHECK(DescribeSequence(f.se) != before);
    REQUIRE(inverse->Apply(f.se, ApplyMode::Lenient));
    CHECK(DescribeSequence(f.se) == before);
    CHECK(f.Media().HasMedia("p.png"));
}

TEST_CASE("Embedding and extracting are flag changes that undo", "[seq][media][commands][undo]") {
    MediaFixture f;
    TempDir dir;
    const std::string bytes = Png(5, 5, 77);
    std::string file = dir.Write("pic.png", bytes);
    f.Picture(file, 0, 100);
    REQUIRE(SeqMediaEdits::AddFileReference(f.se, file, MediaType::Image));
    f.Undo().Clear();
    REQUIRE(f.Media().GetImage(file)->IsOk());
    CHECK_FALSE(f.Media().GetEntryState(file)->embedded);

    f.TakeDirty();
    REQUIRE(SeqMediaEdits::Embed(f.se, file));
    CHECK(f.Undo().GetUndoString() == "Undo: Embed Media");
    auto st = f.Media().GetEntryState(file);
    CHECK(st->embedded);
    CHECK(Bytes(*st) == bytes);
    // Same bytes: nothing that renders changed.
    CHECK(f.TakeDirty().first == -1);

    f.Undo().UndoLastStep();
    CHECK_FALSE(f.Media().GetEntryState(file)->embedded);
    f.Undo().RedoLastStep();
    CHECK(f.Media().GetEntryState(file)->embedded);

    REQUIRE(SeqMediaEdits::ExtractAll(f.se) == 1);
    CHECK(f.Undo().GetUndoString() == "Undo: Extract All Media");
    CHECK_FALSE(f.Media().GetEntryState(file)->embedded);
    CHECK(f.Media().GetImage(file)->IsOk());
    REQUIRE(SeqMediaEdits::EmbedAll(f.se) == 1);
    CHECK(f.Undo().GetUndoString() == "Undo: Embed All Media");
    CHECK(f.Media().GetEntryState(file)->embedded);
}

TEST_CASE("Extracting to a file re-keys references and undo keeps the file", "[seq][media][commands][undo]") {
    MediaFixture f;
    TempDir dir;
    const std::string bytes = Png(3, 3, 33);
    Effect* e = f.Picture("Images/x.png", 0, 100);
    REQUIRE(SeqMediaEdits::Put(f.se, Embedded("Images/x.png", bytes)));
    f.Undo().Clear();

    std::string dest = dir.File("out/x.png");
    REQUIRE(SeqMediaEdits::Extract(f.se, "Images/x.png", dest, dest));
    CHECK(f.Undo().GetUndoString() == "Undo: Extract Media");
    REQUIRE(std::filesystem::exists(dest));
    CHECK_FALSE(f.Media().HasMedia("Images/x.png"));
    auto st = f.Media().GetEntryState(dest);
    REQUIRE(st);
    CHECK_FALSE(st->embedded);
    CHECK(e->GetSetting("E_TEXTCTRL_Pictures_Filename") == dest);
    CHECK(f.Media().GetImage(dest)->GetImageWidth() == 3);

    f.Undo().UndoLastStep();
    auto back = f.Media().GetEntryState("Images/x.png");
    REQUIRE(back);
    CHECK(back->embedded);
    CHECK(Bytes(*back) == bytes);
    CHECK(e->GetSetting("E_TEXTCTRL_Pictures_Filename") == "Images/x.png");
    CHECK_FALSE(f.Media().HasMedia(dest));
    CHECK(std::filesystem::exists(dest)); // the user's file is never deleted

    // Not embedded: nothing to extract, nothing written.
    CHECK_FALSE(SeqMediaEdits::Extract(f.se, "missing.png", dir.File("m.png")));
    CHECK_FALSE(std::filesystem::exists(dir.File("m.png")));
}

TEST_CASE("Re-embedding from disk replaces the bytes and invalidates users", "[seq][media][commands][undo]") {
    MediaFixture f;
    TempDir dir;
    std::string file = dir.Write("t.txt", "new text");
    f.Picture(file, 1000, 2000);
    REQUIRE(SeqMediaEdits::Put(f.se, Embedded(file, "old text", MediaType::TextFile)));
    CHECK(f.Media().GetTextFile(file)->GetContent() == "old text");
    f.TakeDirty();

    REQUIRE(SeqMediaEdits::ReembedFromDisk(f.se, file));
    CHECK(f.Media().GetTextFile(file)->GetContent() == "new text");
    CHECK(f.Media().GetEntryState(file)->embedded);
    auto dirty = f.TakeDirty();
    CHECK(dirty.first == 1000);
    CHECK(dirty.second == 2000);

    f.Undo().UndoLastStep();
    CHECK(f.Media().GetTextFile(file)->GetContent() == "old text");
    CHECK(f.TakeDirty().first == 1000);
}

TEST_CASE("Applying a media command marks the sequence changed", "[seq][media][commands]") {
    MediaFixture f;
    auto before = f.se.GetChangeCount();
    REQUIRE(SeqMediaEdits::EmbedImage(f.se, "a.png", xlImage(1, 1)));
    CHECK(f.se.GetChangeCount() > before);
}

TEST_CASE("RemoveUnreferencedMedia is one undoable step", "[seq][media][commands][undo]") {
    MediaFixture f;
    f.Picture("used.png", 0, 100);
    for (const char* k : { "used.png", "unused1.png", "unused2.png", "meta.png" }) {
        REQUIRE(SeqMediaEdits::Put(f.se, Embedded(k, Png(1, 1, 3))));
    }
    f.Media().MarkUsedByMetadata("meta.png");
    f.Undo().Clear();

    CHECK(f.se.RemoveUnreferencedMedia() == 2);
    CHECK(f.Undo().GetUndoString() == "Undo: Remove Unused Media");
    CHECK_FALSE(f.Media().HasMedia("unused1.png"));
    CHECK_FALSE(f.Media().HasMedia("unused2.png"));
    CHECK(f.Media().HasMedia("used.png"));
    CHECK(f.Media().HasMedia("meta.png"));
    f.Undo().UndoLastStep();
    CHECK(f.Media().HasMedia("unused1.png"));
    CHECK(f.Media().HasMedia("unused2.png"));
    CHECK_FALSE(f.Undo().CanUndo());

    // Nothing to remove records nothing.
    f.Media().MarkUsedByMetadata("unused1.png");
    f.Media().MarkUsedByMetadata("unused2.png");
    CHECK(f.se.RemoveUnreferencedMedia() == 0);
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("MakeMediaPathRelative is undoable", "[seq][media][commands][undo]") {
    MediaFixture f;
    f.ctx.showDirectory = "/show/dir";
    Effect* e = f.Picture("/show/dir/img/x.png", 0, 100);
    REQUIRE(SeqMediaEdits::Put(f.se, Embedded("/show/dir/img/x.png", Png(1, 1, 1))));
    f.Undo().Clear();
    CHECK(f.se.MakeMediaPathRelative("/show/dir/img/x.png") == "img/x.png");
    CHECK(e->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/x.png");
    f.Undo().UndoLastStep();
    CHECK(f.Media().HasMedia("/show/dir/img/x.png"));
    CHECK(e->GetSetting("E_TEXTCTRL_Pictures_Filename") == "/show/dir/img/x.png");
}

TEST_CASE("Media edit batches reach observers as one labelled batch", "[seq][media][commands]") {
    MediaFixture f;
    CommandLogRecorder log;
    f.se.GetCommandExecutor().AddObserver(&log);
    {
        MediaEditBatch batch(f.se, "Bulk Thing");
        REQUIRE(SeqMediaEdits::EmbedImage(f.se, "a.png", xlImage(1, 1)));
        REQUIRE(SeqMediaEdits::EmbedImage(f.se, "b.png", xlImage(1, 1)));
    }
    f.se.GetCommandExecutor().RemoveObserver(&log);
    REQUIRE(log.GetLog().size() == 1);
    CHECK(log.GetLog()[0]["type"] == "Batch");
    CHECK(log.GetLog()[0]["label"] == "Bulk Thing");
    CHECK(log.GetLog()[0]["commands"].size() == 2);
    CHECK(f.Undo().GetUndoString() == "Undo: Bulk Thing");

    // Replaying the log on a fresh sequence produces the same media.
    MediaFixture g;
    auto replay = DeserializeCommand(log.GetLog()[0]);
    REQUIRE(replay);
    REQUIRE(replay->Apply(g.se));
    CHECK(g.Media().GetEntryState("a.png")->SameContent(*f.Media().GetEntryState("a.png")));
    CHECK(g.Media().GetEntryState("b.png")->SameContent(*f.Media().GetEntryState("b.png")));
}

#ifndef NDEBUG
TEST_CASE("Media edit helpers don't trip the mutation guard", "[seq][media][commands][guard]") {
    MediaFixture f;
    TempDir dir;
    std::string file = dir.Write("p.png", Png(2, 2, 1));
    f.Picture("Images/a.png", 0, 100);
    GuardMode mode(SeqMutationGuard::Mode::Count);

    SeqMediaEdits::EmbedImage(f.se, "Images/a.png", xlImage(2, 2));
    SeqMediaEdits::Rename(f.se, "Images/a.png", "Images/b.png");
    SeqMediaEdits::AddFileReference(f.se, file, MediaType::Image);
    SeqMediaEdits::Embed(f.se, file);
    SeqMediaEdits::EmbedFile(f.se, "Images/c.png", MediaType::Image, file);
    SeqMediaEdits::Extract(f.se, "Images/c.png", dir.File("c.png"), dir.File("c.png"));
    SeqMediaEdits::ExtractAll(f.se);
    SeqMediaEdits::EmbedAll(f.se);
    SeqMediaEdits::Remove(f.se, file);
    f.se.RemoveUnreferencedMedia();
    f.Undo().UndoLastStep();
    f.Undo().RedoLastStep();
    CHECK(SeqMutationGuard::ViolationCount() == 0);

    // Control: the same kind of edit made directly is counted.
    f.Media().AddEmbeddedImage("direct.png", xlImage(1, 1));
    CHECK(SeqMutationGuard::ViolationCount() == 1);
}
#endif

TEST_CASE("A rename that moved a file on disk can skip the undo step", "[seq][media][commands][undo]") {
    MediaFixture f;
    Effect* e = f.Picture("Images/a.png", 0, 100);
    REQUIRE(SeqMediaEdits::Put(f.se, MediaEntryState::FileReference("Images/a.png", MediaType::Image)));
    f.Undo().Clear();
    CommandLogRecorder log;
    f.se.GetCommandExecutor().AddObserver(&log);
    REQUIRE(SeqMediaEdits::Rename(f.se, "Images/a.png", "Images/b.png", false, "Rename Media", /*recordUndo=*/false));
    f.se.GetCommandExecutor().RemoveObserver(&log);
    CHECK(e->GetSetting("E_TEXTCTRL_Pictures_Filename") == "Images/b.png");
    CHECK(f.Media().HasMedia("Images/b.png"));
    CHECK_FALSE(f.Undo().CanUndo());
    CHECK(log.GetLog().size() == 1); // still reported
}

TEST_CASE("The undo step serial tells whether a media edit recorded a step", "[seq][media][commands][undo]") {
    MediaFixture f;
    unsigned int s0 = f.Undo().GetStepSerial();
    REQUIRE(SeqMediaEdits::EmbedImage(f.se, "a.png", xlImage(1, 1)));
    unsigned int s1 = f.Undo().GetStepSerial();
    CHECK(s1 != s0);
    // Nothing to do: no step.
    CHECK_FALSE(SeqMediaEdits::Remove(f.se, "missing.png"));
    CHECK(SeqMediaEdits::RemoveUnreferenced(f.se) == 1);
    unsigned int s2 = f.Undo().GetStepSerial();
    CHECK(s2 != s1);
    CHECK(SeqMediaEdits::RemoveUnreferenced(f.se) == 0);
    CHECK(f.Undo().GetStepSerial() == s2);
    // Applied without undo: no step.
    REQUIRE(SeqMediaEdits::EmbedImage(f.se, "b.png", xlImage(1, 1)));
    unsigned int s3 = f.Undo().GetStepSerial();
    f.Picture("b.png", 0, 10);
    REQUIRE(SeqMediaEdits::Rename(f.se, "b.png", "c.png", false, "Rename Media", false));
    CHECK(f.Undo().GetStepSerial() == s3);
}
