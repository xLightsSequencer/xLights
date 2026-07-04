#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "render/FSEQFile.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

class SequenceData;
class SequenceElements;
class RenderContext;

// Shared, wx-free fseq I/O used by the desktop, iPad, and headless render hosts
// so the .fseq write path (channel packing, master-view sparse ranges, media /
// source / FPP-command headers) lives in ONE implementation instead of the
// copies that had grown up in iPadRenderContext, HeadlessRenderContext, and
// (partly) FileConverter.
namespace FSEQFileIO {

// A compressed XML blob embedded as a variable header ('XR' rgbeffects, 'XN'
// network/outputs, 'XS' sequence). FSEQFileIO owns the zstd compression: it
// compresses `filePath` from disk when set and present, otherwise compresses
// the in-memory `xml` bytes. The host decides which to supply (e.g. desktop
// uses the on-disk file when there are no unsaved edits, else freshly built
// XML; headless/iPad always use the current on-disk files). Empty blobs are
// skipped.
struct EmbeddedBlob {
    char code[2] = {0, 0};    // e.g. {'X','R'}
    std::string filePath;     // compressed from disk when non-empty and it exists
    std::string xml;          // fallback bytes, compressed when filePath is absent/missing
};

struct WriteOptions {
    int version = 2;                    // fseq major version (1 or 2)
    FSEQFile::CompressionType compression = FSEQFile::CompressionType::zstd;
    int compressionLevel = -99;         // zstd level; -99 = format default
    bool sparse = true;                 // pack only master-view model channels (v2 only)
    bool isEffectSeq = false;           // emit 'eS' marker; channel count not rounded up to 4
    std::string mediaFile;              // 'mf' header (skipped when empty)
    std::string source = "xLights";     // 'sp' header
    std::vector<EmbeddedBlob> embedded; // 'XR'/'XN'/'XS' compressed blobs (in emit order)
    std::vector<FSEQFile::VariableHeader> extraHeaders; // caller-supplied extras
};

// Compute the master-view sparse channel ranges (merged) from the sequence's
// ELEMENT_TYPE_MODEL rows, resolving models via ctx.GetModel and expanding
// ModelGroups. Same algorithm as xLightsFrame::WriteFalconPiFile. Exposed so
// callers (e.g. iPad's TryLoadFseq validity check) share the one definition.
std::vector<std::pair<uint32_t, uint32_t>> ComputeSparseRanges(SequenceElements& elements,
                                                               RenderContext& ctx);

// Write seqData to path as an fseq. When `elements` and `ctx` are non-null,
// FPP-command headers and (when options.sparse, v2 only) the master-view sparse
// ranges are derived from them; pass null for a bare write (e.g. a data-layer
// convert). Media/source/eS/embedded headers come from `options`. Returns false
// on I/O error or when seqData has no channels/frames.
bool Write(const std::string& path,
           SequenceData& seqData,
           SequenceElements* elements,
           RenderContext* ctx,
           const WriteOptions& options = {});

} // namespace FSEQFileIO
