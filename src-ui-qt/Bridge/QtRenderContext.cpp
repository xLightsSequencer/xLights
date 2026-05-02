#include "QtRenderContext.h"
#include "QtTextDrawingContext.h"
#include "../../src-core/render/Effect.h"
#include "../../src-core/render/TextDrawingContext.h"

QtRenderContext::QtRenderContext(const std::string& metadataDir)
    : _effectManager(metadataDir)
    , _seqElements(std::make_unique<SequenceElements>(this))
{
    // Background display lists are an OpenGL canvas optimisation — they crash
    // when Effect* is nullptr (as we pass).  Disable them globally for this process.
    Effect::EnableBackgroundDisplayLists(false);

    // Register Qt text rendering so the Text effect can draw glyphs.
    // Called once per process; subsequent calls overwrite the same pointers.
    TextDrawingContext::RegisterFactory(
        QtTextDrawingContext::Create,
        QtTextDrawingContext::ParseTextFont,
        QtTextDrawingContext::ParseShapeFont);
    TextDrawingContext::Initialize();
}
