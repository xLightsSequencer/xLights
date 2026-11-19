#pragma once

// Show folders for the join / manifest tests: a show whose sequence has
// audio and media, copies of it standing in for other devices' show
// folders, and edits that make a copy's layout differ.

#include "test_collab_common.h"

#include "collab/CollabManifest.h"
#include "render/SequenceMedia.h"
#include "utils/xlImage.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace CollabTest {

inline std::string SilentWav(int seconds = 1) {
    auto put = [](std::string& s, uint32_t v, int bytes) {
        for (int i = 0; i < bytes; i++) {
            s.push_back(char((v >> (8 * i)) & 0xFF));
        }
    };
    const uint32_t rate = 8000, dataBytes = rate * 2 * seconds; // mono 16-bit
    std::string s = "RIFF";
    put(s, 36 + dataBytes, 4);
    s += "WAVEfmt ";
    put(s, 16, 4);
    put(s, 1, 2);
    put(s, 1, 2);
    put(s, rate, 4);
    put(s, rate * 2, 4);
    put(s, 2, 2);
    put(s, 16, 2);
    s += "data";
    put(s, dataBytes, 4);
    s.append(dataBytes, '\0');
    return s;
}

inline std::string WriteFile(const std::filesystem::path& p, const std::string& body) {
    std::filesystem::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary) << body;
    return p.string();
}

// Copies the named show files (relative paths) from one folder to another.
inline void CopyShowFiles(const std::string& from, const std::filesystem::path& to, const std::vector<std::string>& files) {
    for (const auto& f : files) {
        std::filesystem::create_directories((to / f).parent_path());
        std::filesystem::copy_file(std::filesystem::path(from) / f, to / f, std::filesystem::copy_options::overwrite_existing);
    }
}

// Every file under `dir`: relative path -> size and content hash.
inline std::map<std::string, std::string> FolderState(const std::filesystem::path& dir) {
    std::map<std::string, std::string> out;
    std::error_code ec;
    for (auto it = std::filesystem::recursive_directory_iterator(dir, ec); !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
        if (it->is_regular_file()) {
            uint64_t size = 0;
            uint64_t hash = 0;
            Collab::HashFile(it->path().string(), size, hash);
            out[std::filesystem::relative(it->path(), dir).generic_string()] = std::to_string(size) + ":" + std::to_string(hash);
        }
    }
    return out;
}

// The show's layout with one model changed, one removed and one added.
inline std::string ChangedLayout(std::string layout) {
    auto replace = [&](const std::string& a, const std::string& b) {
        auto at = layout.find(a);
        REQUIRE(at != std::string::npos);
        layout.replace(at, a.size(), b);
    };
    replace("StartChannel=\"200\" NumStrings=\"1\" NodesPerString=\"8\"", "StartChannel=\"200\" NumStrings=\"1\" NodesPerString=\"9\"");
    replace("<model name=\"Pole\" DisplayAs=\"Single Line\" StartChannel=\"400\" NumStrings=\"2\" NodesPerString=\"3\"/>\n",
            "<model name=\"Extra\" DisplayAs=\"Single Line\" StartChannel=\"500\" NumStrings=\"1\" NodesPerString=\"3\"/>\n");
    return layout;
}

// The same layout with layout ids, other attribute order and whitespace.
inline std::string ReformattedLayout(const std::string& layout) {
    std::string out;
    int id = 0;
    for (size_t i = 0; i < layout.size(); ++i) {
        if (layout.compare(i, 7, "<model ") == 0) {
            out += "\n    <model xlId=\"id" + std::to_string(++id) + "\" ";
            i += 6;
        } else {
            out += layout[i];
        }
    }
    return out;
}

// A show whose sequence has audio, a picture in the show folder, one
// outside it, an embedded image and a reference to a file nobody has.
struct MediaShow {
    Show show;
    TempDir outside;
    std::string xsq;
    std::string outsideFile;

    MediaShow() {
        show.dir.Write("song.wav", SilentWav());
        WriteFile(show.dir.path / "Images" / "a.png", "picture-a");
        // Embedded, and also on disk: the embedded copy is what renders.
        WriteFile(show.dir.path / "Images" / "emb.png", "on disk too");
        outsideFile = outside.Write("b.png", "picture-b");
        xsq = show.dir.File("media.xsq");
        HeadlessRenderContext ctx;
        REQUIRE(ctx.LoadShowFolder(show.path));
        SequenceElements& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        SequenceFile sf(xsq, 50);
        sf.SetSequenceType("Media");
        sf.SetMediaFile(show.path, show.dir.File("song.wav"), false);
        SeqMutationGuard::Scope building;
        Element* grid = se.AddElement("Grid", "model", true, false, false, false, false);
        if (grid->GetEffectLayerCount() == 0) {
            grid->AddEffectLayer();
        }
        EffectLayer* l = grid->GetEffectLayer(0);
        l->AddEffect(0, "Pictures", "E_TEXTCTRL_Pictures_Filename=Images/a.png", "", 0, 500, 0, false);
        l->AddEffect(0, "Pictures", "E_TEXTCTRL_Pictures_Filename=" + outsideFile, "", 1000, 1500, 0, false);
        l->AddEffect(0, "Pictures", "E_TEXTCTRL_Pictures_Filename=Images/emb.png", "", 2000, 2500, 0, false);
        l->AddEffect(0, "Pictures", "E_TEXTCTRL_Pictures_Filename=Images/gone.png", "", 3000, 3500, 0, false);
        se.GetSequenceMedia().AddEmbeddedImage("Images/emb.png", xlImage(2, 2));
        Element* line = se.AddElement("Line", "model", true, false, false, false, false);
        if (line->GetEffectLayerCount() == 0) {
            line->AddEffectLayer();
        }
        REQUIRE(sf.Save(se));
    }
};

} // namespace CollabTest
