/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "TextDrawingContext.h"

#include <mutex>
#include <queue>

// ---------------------------------------------------------------------------
// Generic thread-safe context pool
// ---------------------------------------------------------------------------
template <class CTX>
class ContextPool {
public:
    ContextPool(std::function<CTX*()> alloc) : allocator(alloc) {}
    ~ContextPool() {
        while (!contexts.empty()) {
            CTX* ret = contexts.front();
            delete ret;
            contexts.pop();
        }
    }

    CTX* GetContext() {
        std::unique_lock<std::mutex> locker(lock);
        if (contexts.empty()) {
            locker.unlock();
            return allocator();
        }
        CTX* ret = contexts.front();
        contexts.pop();
        return ret;
    }

    void ReleaseContext(CTX* pctx) {
        std::unique_lock<std::mutex> locker(lock);
        contexts.push(pctx);
    }

private:
    std::mutex lock;
    std::queue<CTX*> contexts;
    std::function<CTX*()> allocator;
};

// ---------------------------------------------------------------------------
// Static state
// ---------------------------------------------------------------------------
static ContextPool<TextDrawingContext>* TEXT_CONTEXT_POOL = nullptr;
static TextDrawingContext::FactoryFn sFactory;
static TextDrawingContext::FontParseFn sTextFontParser;
static TextDrawingContext::FontParseFn sShapeFontParser;

// ---------------------------------------------------------------------------
// Factory registration
// ---------------------------------------------------------------------------
void TextDrawingContext::RegisterFactory(FactoryFn factory,
                                         FontParseFn textFontParser,
                                         FontParseFn shapeFontParser) {
    sFactory = std::move(factory);
    sTextFontParser = std::move(textFontParser);
    sShapeFontParser = std::move(shapeFontParser);
}

// ---------------------------------------------------------------------------
// Context pool management
// ---------------------------------------------------------------------------
void TextDrawingContext::Initialize() {
    if (TEXT_CONTEXT_POOL == nullptr && sFactory) {
        TEXT_CONTEXT_POOL = new ContextPool<TextDrawingContext>([]() {
            return sFactory(10, 10, false);
        });
    }
}

void TextDrawingContext::CleanUp() {
    if (TEXT_CONTEXT_POOL != nullptr) {
        delete TEXT_CONTEXT_POOL;
        TEXT_CONTEXT_POOL = nullptr;
    }
}

TextDrawingContext* TextDrawingContext::GetContext() {
    if (TEXT_CONTEXT_POOL != nullptr) {
        return TEXT_CONTEXT_POOL->GetContext();
    }
    return nullptr;
}

void TextDrawingContext::ReleaseContext(TextDrawingContext* ctx) {
    if (TEXT_CONTEXT_POOL != nullptr) {
        TEXT_CONTEXT_POOL->ReleaseContext(ctx);
    }
}

// ---------------------------------------------------------------------------
// Font parsing dispatch
// ---------------------------------------------------------------------------
TextFontInfo TextDrawingContext::GetTextFont(const std::string& fontString) {
    if (sTextFontParser) {
        return sTextFontParser(fontString);
    }
    return TextFontInfo{};
}

TextFontInfo TextDrawingContext::GetShapeFont(const std::string& fontString) {
    if (sShapeFontParser) {
        return sShapeFontParser(fontString);
    }
    return TextFontInfo{};
}
