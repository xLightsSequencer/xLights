/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "render/FSEQFileIO.h"

#include "render/SequenceData.h"
#include "render/SequenceElements.h"
#include "render/Element.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/RenderContext.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "utils/UtilFunctions.h"
#include "utils/ExternalHooks.h"

#include <spdlog/spdlog.h>
#include <zstd.h>

#include <climits>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <utility>

namespace {

// zstd-compress a byte buffer at level 3 (matches FileConverter's XR/XN/XS
// embedding). Single-shot, no worker pool — byte-identical to the desktop
// writer, which never sets ZSTD_c_nbWorkers.
std::vector<uint8_t> zstdCompress(const uint8_t* src, size_t len) {
    std::vector<uint8_t> out;
    if (src == nullptr || len == 0) return out;
    const size_t bound = ZSTD_compressBound(len);
    out.resize(bound);
    ZSTD_CStream* cctx = ZSTD_createCStream();
    ZSTD_initCStream(cctx, 3);
    ZSTD_outBuffer output{ out.data(), bound, 0 };
    ZSTD_inBuffer input{ src, len, 0 };
    ZSTD_compressStream2(cctx, &output, &input, ZSTD_e_end);
    ZSTD_freeCStream(cctx);
    out.resize(output.pos);
    return out;
}

std::vector<uint8_t> zstdCompressFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) return {};
    const std::streampos fileSize = ifs.tellg();
    if (fileSize <= 0) return {};
    ifs.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(static_cast<size_t>(fileSize));
    ifs.read(reinterpret_cast<char*>(buf.data()), fileSize);
    return zstdCompress(buf.data(), buf.size());
}

// Collect each model's [firstChannel, chanCount], expanding ModelGroups to their
// members. Port of xLightsFrame::WriteFalconPiFile's addRanges.
void addModelRanges(Model* m, std::map<uint32_t, uint32_t>& ranges) {
    if (m == nullptr) return;
    if (ModelGroup* grp = dynamic_cast<ModelGroup*>(m)) {
        for (auto* m2 : grp->Models()) {
            addModelRanges(m2, ranges);
        }
    } else {
        uint32_t cur = ranges[m->GetFirstChannel()];
        ranges[m->GetFirstChannel()] = std::max(m->GetChanCount(), cur);
    }
}

// Build an FPP Commands ('FC') / FPP Effects ('FE') variable header from a
// timing track. Port of the desktop / iPad BuildFppCommandHeader.
FSEQFile::VariableHeader buildFppCommandHeader(TimingElement* te, int stepTime) {
    std::map<std::string, std::vector<std::pair<uint32_t, uint32_t>>> commands;
    for (int l = 0; l < (int)te->GetEffectLayerCount(); ++l) {
        EffectLayer* layer = te->GetEffectLayer(l);
        if (!layer) continue;
        for (auto& eff : layer->GetAllEffects()) {
            if (!eff) continue;
            commands[eff->GetEffectName()].push_back(
                std::make_pair(eff->GetStartTimeMS(), eff->GetEndTimeMS()));
        }
    }

    int totalLen = 3; // 1 byte ver, 2 byte count
    const std::string fppInstances; // null-terminated host list, currently empty
    totalLen += fppInstances.size() + 1;
    for (auto& a : commands) {
        totalLen += a.first.length() + 1 + 4;
        totalLen += a.second.size() * 8;
    }

    FSEQFile::VariableHeader header;
    header.extendedData = true;
    header.code[0] = 'F';
    header.code[1] = (te->GetSubType() == "FPP Effects") ? 'E' : 'C';
    header.data.resize(totalLen);

    uint8_t* data = &header.data[0];
    data[0] = 1;
    uint32_t* t2 = reinterpret_cast<uint32_t*>(&data[1]);
    *t2 = static_cast<uint32_t>(commands.size());
    std::memcpy(&data[5], fppInstances.c_str(), fppInstances.size() + 1);
    data += 6 + fppInstances.size();
    for (auto& a : commands) {
        const std::string& c = a.first;
        uint32_t count = static_cast<uint32_t>(a.second.size());
        std::memcpy(data, c.c_str(), c.length() + 1);
        data += c.length() + 1;
        uint32_t* t = reinterpret_cast<uint32_t*>(data);
        *t = count;
        data += 4;
        ++t;
        for (size_t x = 0; x < count; ++x) {
            uint32_t sframe = stepTime ? a.second[x].first / stepTime : 0;
            uint32_t eframe = stepTime ? a.second[x].second / stepTime : 0;
            *t = sframe; ++t;
            *t = eframe; ++t;
        }
        data += count * 8;
    }
    return header;
}

} // namespace

namespace FSEQFileIO {

std::vector<std::pair<uint32_t, uint32_t>> ComputeSparseRanges(SequenceElements& elements,
                                                               RenderContext& ctx) {
    std::map<uint32_t, uint32_t> ranges;
    const int numElements = elements.GetElementCount();
    for (int i = 0; i < numElements; ++i) {
        Element* element = elements.GetElement(i);
        if (element != nullptr && element->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            addModelRanges(ctx.GetModel(element->GetModelName()), ranges);
        }
    }

    std::vector<std::pair<uint32_t, uint32_t>> result;
    const uint32_t gapEliminate = 0;
    std::pair<uint32_t, uint32_t> cur(INT_MAX, INT_MAX);
    for (auto& a : ranges) {
        if (cur.first == INT_MAX) {
            cur.first = a.first;
            cur.second = a.second;
        } else if (a.first <= (cur.first + cur.second + gapEliminate)) {
            uint32_t maxEnd = std::max(cur.first + cur.second - 1, a.first + a.second - 1);
            cur.second = maxEnd - cur.first + 1;
        } else {
            result.push_back(cur);
            cur.first = a.first;
            cur.second = a.second;
        }
    }
    if (cur.first != INT_MAX) {
        result.push_back(cur);
    }
    return result;
}

bool Write(const std::string& path,
           SequenceData& seqData,
           SequenceElements* elements,
           RenderContext* ctx,
           const WriteOptions& options) {
    if (path.empty()) return false;
    if (seqData.NumChannels() == 0 || seqData.NumFrames() == 0) {
        spdlog::warn("FSEQFileIO::Write: sequence data is empty (no render run yet?)");
        return false;
    }

    std::unique_ptr<FSEQFile> file(
        FSEQFile::createFSEQFile(path, options.version, options.compression, options.compressionLevel));
    if (!file) {
        spdlog::error("FSEQFileIO::Write: createFSEQFile failed for {}", path);
        return false;
    }

    if (options.version >= 2) {
        file->enableMinorVersionFeatures(2);
    }
    const long channelCount = options.isEffectSeq ? (long)seqData.NumChannels()
                                                  : roundTo4((long)seqData.NumChannels());
    file->setChannelCount((int)channelCount);
    file->setStepTime(seqData.FrameTime());
    file->setNumFrames(static_cast<uint32_t>(seqData.NumFrames()));

    if (!options.mediaFile.empty()) {
        FSEQFile::VariableHeader mf;
        mf.code[0] = 'm';
        mf.code[1] = 'f';
        mf.data.assign(options.mediaFile.begin(), options.mediaFile.end());
        mf.data.push_back('\0');
        file->addVariableHeader(mf);
    }
    {
        FSEQFile::VariableHeader sp;
        sp.code[0] = 's';
        sp.code[1] = 'p';
        sp.data.assign(options.source.begin(), options.source.end());
        sp.data.push_back('\0');
        file->addVariableHeader(sp);
    }

    if (options.version >= 2 && options.sparse && elements != nullptr && ctx != nullptr) {
        if (auto* v2 = dynamic_cast<V2FSEQFile*>(file.get())) {
            for (auto& r : ComputeSparseRanges(*elements, *ctx)) {
                v2->m_sparseRanges.push_back(r);
            }
        }
    }

    if (options.isEffectSeq) {
        FSEQFile::VariableHeader es;
        es.code[0] = 'e';
        es.code[1] = 'S';
        es.data.push_back(1); // version/flag byte
        file->addVariableHeader(es);
    }

    if (options.version >= 2 && elements != nullptr) {
        for (int x = 0; x < elements->GetNumberOfTimingElements(); ++x) {
            TimingElement* te = elements->GetTimingElement(x);
            if (te == nullptr) continue;
            const std::string& sub = te->GetSubType();
            if (sub == "FPP Commands" || sub == "FPP Effects") {
                file->addVariableHeader(buildFppCommandHeader(te, seqData.FrameTime()));
            }
        }
    }

    for (auto& b : options.embedded) {
        std::vector<uint8_t> comp;
        if (!b.filePath.empty() && FileExists(b.filePath)) {
            comp = zstdCompressFile(b.filePath);
        }
        if (comp.empty() && !b.xml.empty()) {
            comp = zstdCompress(reinterpret_cast<const uint8_t*>(b.xml.data()), b.xml.size());
        }
        if (comp.empty()) continue;
        FSEQFile::VariableHeader h;
        h.extendedData = true;
        h.code[0] = b.code[0];
        h.code[1] = b.code[1];
        h.data = std::move(comp);
        file->addVariableHeader(h);
    }

    for (auto& h : options.extraHeaders) {
        file->addVariableHeader(h);
    }

    file->writeHeader();
    const uint32_t numFrames = static_cast<uint32_t>(seqData.NumFrames());
    for (uint32_t fr = 0; fr < numFrames; ++fr) {
        file->addFrame(fr, &seqData[fr][0]);
    }
    file->finalize();

    spdlog::info("FSEQFileIO::Write: wrote {} frames x {} channels to {}",
                 numFrames, seqData.NumChannels(), path);
    return true;
}

} // namespace FSEQFileIO
