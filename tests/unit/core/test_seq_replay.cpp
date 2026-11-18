#include <catch2/catch_test_macros.hpp>

#include "support/TempDir.h"
#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceBulkEdits.h"
#include "render/SequenceCommands.h"
#include "render/SequenceEditCapture.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceFaces.h"
#include "render/SequenceFile.h"
#include "render/SequenceMedia.h"
#include "render/SequenceMediaCommands.h"
#include "render/SequenceMutationGuard.h"
#include "render/SongStructureManager.h"
#include "render/UndoManager.h"
#include "utils/FileUtils.h"
#include "utils/xlImage.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iterator>
#include <memory>
#include <string>

// A long scripted edit session recorded through CommandLogRecorder, then
// replayed onto a fresh copy of the starting sequence: both must save the
// same .xsq, and undoing every step must save the starting one.

namespace {

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

std::string Read(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// A sequence opened from disk the way the apps open one.
struct Opened {
    HeadlessRenderContext ctx;
    std::unique_ptr<SequenceFile> file;
    Opened(const std::string& showDir, const std::string& path) {
        REQUIRE(ctx.LoadShowFolder(showDir));
        file = std::make_unique<SequenceFile>(path);
        auto doc = file->Open(showDir, false, path);
        REQUIRE(doc);
        REQUIRE(ctx.LoadSequenceElements(*file, *doc));
    }
    SequenceElements& se() { return ctx.GetSequenceElements(); }
    std::string SaveAs(const std::string& path) {
        file->SetFullPath(path);
        REQUIRE(file->Save(se()));
        return Read(path);
    }
};

// Ids aren't saved in the .xsq; a peer opening the same file shares them
// through the session. Give `to` the ids `from` has.
void CopyLayerIds(EffectLayer* from, EffectLayer* to) {
    to->SetUid(from->GetUid());
    REQUIRE(from->GetEffectCount() == to->GetEffectCount());
    for (int i = 0; i < from->GetEffectCount(); ++i) {
        to->GetEffect(i)->SetUid(from->GetEffect(i)->GetUid());
    }
}

void CopyElementIds(Element* from, Element* to) {
    REQUIRE(from->GetFullName() == to->GetFullName());
    to->SetUid(from->GetUid());
    REQUIRE(from->GetEffectLayerCount() == to->GetEffectLayerCount());
    for (size_t i = 0; i < from->GetEffectLayerCount(); ++i) {
        CopyLayerIds(from->GetEffectLayer(i), to->GetEffectLayer(i));
    }
    if (auto* fs = dynamic_cast<StrandElement*>(from)) {
        auto* ts = dynamic_cast<StrandElement*>(to);
        REQUIRE(fs->GetNodeLayerCount() == ts->GetNodeLayerCount());
        for (int n = 0; n < fs->GetNodeLayerCount(); ++n) {
            CopyLayerIds(fs->GetNodeLayer(n), ts->GetNodeLayer(n));
        }
    }
}

void CopyIds(SequenceElements& from, SequenceElements& to) {
    REQUIRE(from.GetElementCount() == to.GetElementCount());
    for (size_t i = 0; i < from.GetElementCount(); ++i) {
        Element* a = from.GetElement(i);
        Element* b = to.GetElement(i);
        CopyElementIds(a, b);
        if (auto* ma = dynamic_cast<ModelElement*>(a)) {
            auto* mb = dynamic_cast<ModelElement*>(b);
            REQUIRE(ma->GetSubModelAndStrandCount() == mb->GetSubModelAndStrandCount());
            for (int x = 0; x < ma->GetSubModelAndStrandCount(); ++x) {
                CopyElementIds(ma->GetSubModel(x), mb->GetSubModel(x));
            }
        }
    }
}

EffectLayer* Layer(SequenceElements& se, const std::string& element, int layer = 0) {
    Element* el = se.GetElement(element);
    REQUIRE(el != nullptr);
    REQUIRE((int)el->GetEffectLayerCount() > layer);
    return el->GetEffectLayer(layer);
}
} // namespace

TEST_CASE("A recorded edit session replays to the same .xsq, and undo-all restores the start", "[seq][replay]") {
    TempDir dir;
    FixFileReset reset;
    dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
    dir.Write("xlights_rgbeffects.xml",
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n"
              "<model name=\"Grid\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\">"
              "<subModel name=\"Top\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>\n"
              "<model name=\"Line\" DisplayAs=\"Single Line\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"4\"/>\n"
              "<model name=\"Arch\" DisplayAs=\"Arches\" StartChannel=\"200\" NumStrings=\"1\" NodesPerString=\"8\"/>\n"
              "</models>\n<modelGroups/>\n</xrgb>\n");
    const std::string show = dir.path.string();
    const std::string start = dir.File("start.xsq");

    // The starting sequence.
    {
        HeadlessRenderContext ctx;
        REQUIRE(ctx.LoadShowFolder(show));
        SequenceElements& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        SequenceFile sf(start, 50);
        SeqMutationGuard::Scope building;
        Element* beats = se.AddElement("Beats", "timing", true, false, true, false, false);
        EffectLayer* bl = beats->AddEffectLayer();
        for (int i = 0; i < 8; ++i) {
            bl->AddEffect(0, "", "", "", i * 500, i * 500 + 500, 0, false);
        }
        Element* grid = se.AddElement("Grid", "model", true, false, false, false, false);
        if (grid->GetEffectLayerCount() == 0) {
            grid->AddEffectLayer();
        }
        grid->GetEffectLayer(0)->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_Start=10", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1", 1000, 1500, 0, false);
        grid->GetEffectLayer(0)->AddEffect(0, "Bars", "", "", 2000, 2500, 0, false);
        grid->GetEffectLayer(0)->AddEffect(0, "Twinkle", "", "", 3000, 3500, 0, false);
        Element* line = se.AddElement("Line", "model", true, false, false, false, false);
        if (line->GetEffectLayerCount() == 0) {
            line->AddEffectLayer();
        }
        line->GetEffectLayer(0)->AddEffect(0, "On", "", "", 0, 200, 0, false);
        REQUIRE(sf.Save(se));
    }

    Opened a(show, start);
    Opened b(show, start);
    CopyIds(a.se(), b.se());
    const std::string baseline = a.SaveAs(dir.File("baseline-a.xsq"));
    REQUIRE(b.SaveAs(dir.File("baseline-b.xsq")) == baseline);

    SequenceElements& se = a.se();
    CommandExecutor& exec = se.GetCommandExecutor();
    UndoManager& undo = se.get_undo_mgr();
    undo.Clear();
    CommandLogRecorder log;
    exec.AddObserver(&log);
    auto step = [&](const std::function<void()>& edit) {
        undo.CreateUndoStep();
        edit();
    };

    // Effects
    Effect* added = nullptr;
    step([&] { added = exec.AddEffect(Layer(se, "Grid"), "On", "E_TEXTCTRL_Eff_On_Start=20", "", 4000, 4500, 0, false); });
    REQUIRE(added != nullptr);
    step([&] { REQUIRE(exec.MoveEffect(added, 4100, 4600)); });
    step([&] {
        REQUIRE(exec.SetEffectSetting(added, "E_TEXTCTRL_Eff_On_End", std::string("80")));
        REQUIRE(exec.SetEffectPalette(added, "C_BUTTON_Palette1=#00FF00,C_CHECKBOX_Palette1=1"));
    });
    step([&] { REQUIRE(exec.DeleteEffect(Layer(se, "Grid")->GetEffect(1))); });
    step([&] { REQUIRE(exec.SetEffectLocked(Layer(se, "Line")->GetEffect(0), true)); });
    // Layers
    EffectLayer* second = nullptr;
    step([&] {
        second = exec.InsertLayer(se.GetElement("Grid"), 1);
        REQUIRE(second != nullptr);
        REQUIRE(exec.SetLayerName(second, "Second"));
        REQUIRE(exec.AddEffect(second, "Bars", "", "", 0, 1000, 0, false) != nullptr);
    });
    step([&] { REQUIRE(exec.InsertLayer(se.GetElement("Grid"), 2) != nullptr); });
    step([&] { REQUIRE(exec.RemoveLayer(se.GetElement("Grid"), 2)); });
    // Rows
    step([&] { REQUIRE(ElementEdits::AddModel(se, -1, "Arch") != nullptr); });
    step([&] { REQUIRE(ElementEdits::MoveElement(se, se.GetElementIndex("Arch"), 1, MASTER_VIEW)); });
    step([&] { REQUIRE(ElementEdits::SetRenderDisabled(se, se.GetElement("Line"), true)); });
    // Timing
    step([&] {
        TimingElement* t = ElementEdits::AddTimingTrack(se, -1, "Words", "");
        REQUIRE(t != nullptr);
        REQUIRE(exec.AddEffect(t->GetEffectLayer(0), "hello", "", "", 0, 400, 0, false) != nullptr);
        REQUIRE(exec.AddEffect(t->GetEffectLayer(0), "world", "", "", 400, 900, 0, false) != nullptr);
    });
    step([&] { REQUIRE(ElementEdits::RenameTimingTrack(se, "Words", "Lyrics")); });
    step([&] { REQUIRE(ElementEdits::SetTimingFixed(se, se.GetTimingElement("Beats"), 50)); });
    // Tags, faces, song structure
    step([&] { REQUIRE(ElementEdits::SetTagPosition(se, 2, 1500)); });
    REQUIRE(ElementEdits::EditSequenceFaces(se, [](SequenceFaces& f) { f.SetFace("Smile", { { "Type", "Matrix" }, { "Mouth-AI", "Images/a.png" } }); }));
    REQUIRE(ElementEdits::EditSongStructure(se, [&](SongStructureManager& s) {
        s.SetActiveView(s.AddView("Parts"));
        s.AddBoundary(1000, 4000);
    }));
    // Media
    REQUIRE(SeqMediaEdits::EmbedImage(se, "Images/a.png", xlImage(4, 4)));
    step([&] { REQUIRE(exec.AddEffect(second, "Pictures", "E_TEXTCTRL_Pictures_Filename=Images/a.png", "", 2000, 3000, 0, false) != nullptr); });
    REQUIRE(SeqMediaEdits::Rename(se, "Images/a.png", "Images/b.png"));
    // Symbols
    std::string sym = SymbolEdits::Create(se, "Sym", Layer(se, "Grid")->GetEffect(0));
    REQUIRE_FALSE(sym.empty());
    REQUIRE(SymbolEdits::Link(se, { Layer(se, "Grid")->GetEffect(1) }, sym) == 1);
    // A bulk tool
    REQUIRE(ShiftEffects(se, 50, false, false, "Shift Effects"));
    // An import, changing the sequence directly inside a capture
    {
        SequenceEditCapture capture(se, "Import Effects");
        Element* imp = se.AddElement("Imported", "timing", true, false, false, false, false);
        EffectLayer* il = imp->AddEffectLayer();
        il->AddEffect(0, "x", "", "", 0, 100, 0, false);
        Layer(se, "Line")->AddEffect(0, "Bars", "", "", 1000, 1200, 0, false);
        se.GetSequenceMedia().AddEmbeddedImage("Images/imported.png", xlImage(3, 3));
        se.GetSequenceFaces().SetFace("ImportedFace", { { "Type", "Rendered" } });
        se.DeleteElement("Lyrics");
    }
    // Row delete
    step([&] { REQUIRE(ElementEdits::DeleteElement(se, "Arch")); });
    exec.RemoveObserver(&log);

    const std::string edited = a.SaveAs(dir.File("edited.xsq"));
    REQUIRE(edited != baseline);
    INFO(log.GetLog().size() << " logged commands");
    REQUIRE(log.GetLog().size() >= 20);

    // Replay onto the fresh copy, every command strictly, through JSON.
    for (const auto& j : log.GetLog()) {
        auto cmd = CommandFromJson(j.dump());
        INFO(j.dump().substr(0, 300));
        REQUIRE(cmd != nullptr);
        CommandResult r;
        REQUIRE(cmd->Apply(b.se(), ApplyMode::Strict, &r) != nullptr);
    }
    const std::string replayed = b.SaveAs(dir.File("replayed.xsq"));
    CHECK(replayed == edited);

    // Undo every step.
    int steps = 0;
    while (undo.CanUndo() && steps < 1000) {
        undo.UndoLastStep();
        ++steps;
    }
    CHECK(steps >= 20);
    const std::string undone = a.SaveAs(dir.File("undone.xsq"));
    CHECK(undone == baseline);
}
