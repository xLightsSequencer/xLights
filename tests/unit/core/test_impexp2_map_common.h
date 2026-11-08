#pragma once

#include "import_export/BasicImportMappingNode.h"
#include "import_export/ImportMappingNode.h"
#include "models/Model.h"
#include "render/HeadlessRenderContext.h"
#include "support/TempDir.h"
#include "test_render_common.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace impexp2 {

// A show folder with arbitrary models/groups loaded into a headless context (no sequence open).
struct LayoutFixture {
    TempDir dir;
    renderhelp::FixFileReset reset;
    std::unique_ptr<HeadlessRenderContext> ctx;
    bool loaded = false;

    explicit LayoutFixture(const std::string& models, const std::string& groups = "") {
        renderhelp::EnsureLoggers();
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
        dir.Write("xlights_rgbeffects.xml",
                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n" + models + "\n</models>\n<modelGroups>\n" + groups + "\n</modelGroups>\n</xrgb>\n");
        ctx = std::make_unique<HeadlessRenderContext>();
        loaded = ctx->LoadShowFolder(dir.path.string());
    }
    Model* M(const std::string& n) { return ctx->GetModel(n); }
};

inline std::unique_ptr<BasicImportMappingNode> Node(const std::string& model, const std::string& strand = "", const std::string& node = "",
                                                    bool group = false, std::list<std::string> aliases = {}) {
    return std::make_unique<BasicImportMappingNode>(model, strand, node, group, std::move(aliases), "", !strand.empty());
}

inline AvailableSource Src(const std::string& display, const std::string& type = "Model", bool selected = false) {
    AvailableSource s;
    s.displayName = display;
    std::string c;
    // canonical form: trimmed + lowered (what the callers precompute)
    size_t b = display.find_first_not_of(" \t\r\n");
    size_t e = display.find_last_not_of(" \t\r\n");
    if (b != std::string::npos) c = display.substr(b, e - b + 1);
    for (auto& ch : c) {
        if (ch >= 'A' && ch <= 'Z') ch = (char)(ch - 'A' + 'a');
    }
    s.canonicalName = c;
    s.modelType = type;
    s.selected = selected;
    return s;
}

} // namespace impexp2
