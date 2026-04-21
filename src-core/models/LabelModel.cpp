/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LabelModel.h"
#include "ModelScreenLocation.h"
#include "../graphics/IModelPreview.h"
#include "../graphics/xlFontInfo.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "../graphics/xlGraphicsContext.h"

LabelModel::LabelModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Label;
    _fontSize = 14;
    _textColor = xlWHITE;
    _labelText = "Label";
}

LabelModel::~LabelModel()
{
    delete _fontTexture;
}

void LabelModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                               int &BufferWi, int &BufferHi, int stagger) const
{
    BufferHi = 1;
    BufferWi = 1;
}

void LabelModel::InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
    std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi, int stagger, bool deep) const
{
    BufferHi = 1;
    BufferWi = 1;

    NodeBaseClass* node = Nodes[0]->clone();
    node->Coords.resize(1);
    node->Coords[0].bufX = 0;
    node->Coords[0].bufY = 0;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    GetModelScreenLocation().TranslatePoint(x, y, z);
    node->Coords[0].screenX = x;
    node->Coords[0].screenY = y;
    node->Coords[0].screenZ = z;
    newNodes.push_back(NodeBaseClassPtr(node));
}

void LabelModel::InitModel()
{
    SetNodeCount(1, 1, rgbOrder);
    Nodes[0]->ActChan = stringStartChan[0];
    Nodes[0]->StringNum = 0;
    Nodes[0]->Coords[0].screenX = -0.5f;
    Nodes[0]->Coords[0].screenY = -0.5f;
    Nodes[0]->Coords[0].screenZ = -0.5f;
    Nodes[0]->AddBufCoord(0, 0);
    Nodes[0]->Coords[1].screenX = 0.5f;
    Nodes[0]->Coords[1].screenY = 0.5f;
    Nodes[0]->Coords[1].screenZ = 0.5f;

    SetBufferSize(1, 1);
    screenLocation.SetRenderSize(1, 1);
    screenLocation.RenderDp = 10.0f;
}

void LabelModel::DisplayEffectOnWindow(IModelPreview* preview, double pointSize)
{
    bool mustEnd = false;
    xlGraphicsContext *ctx = preview->getCurrentGraphicsContext();
    if (ctx == nullptr) {
        bool success = preview->StartDrawing(pointSize);
        if (success) {
            ctx = preview->getCurrentGraphicsContext();
            mustEnd = true;
        }
    }
    if (ctx) {
        int w = preview->getWidth();
        int h = preview->getHeight();

        if (!_labelText.empty()) {
            int fontSize = std::max(8, std::min(_fontSize, 40));
            const xlFontInfo &font = xlFontInfo::FindFont(fontSize);
            if (_fontTexture == nullptr || _cachedFontSize != fontSize) {
                delete _fontTexture;
                _fontTexture = ctx->createTextureForFont(font);
                _cachedFontSize = fontSize;
            }
            xlTexture *fontTex = _fontTexture;

            float factor = 1.0f;
            float textW = font.widthOf(_labelText, factor);
            float textH = (float)fontSize;

            float x = (w - textW) / 2.0f;
            float yBase = (h + textH) / 2.0f;

            xlVertexTextureAccumulator *va = ctx->createVertexTextureAccumulator();
            font.populate(*va, x, yBase, _labelText, factor);
            va->Flush();

            xlColor tc = _textColor;
            preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext *ctx) {
                ctx->enableBlending();
                ctx->drawTexture(va, fontTex, tc, 0, va->getCount());
                delete va;
            });
        }
    }
    if (mustEnd) {
        preview->EndDrawing();
    }
}

void LabelModel::DisplayModelOnWindow(IModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d,
                                      const xlColor* color, bool allowSelected, bool wiring,
                                      bool highlightFirst, int highlightpixel,
                                      float *boundingBox)
{
    GetModelScreenLocation().PrepareToDraw(is_3d, allowSelected);
    GetModelScreenLocation().UpdateBoundingBox(Nodes);

    if (!_labelText.empty()) {
        int fontSize = std::max(8, std::min(_fontSize, 40));
        const xlFontInfo &font = xlFontInfo::FindFont(fontSize);
        if (_fontTexture == nullptr || _cachedFontSize != fontSize) {
            delete _fontTexture;
            _fontTexture = ctx->createTextureForFont(font);
            _cachedFontSize = fontSize;
        }
        xlTexture *fontTex = _fontTexture;

        // factor = getSize() makes character height = 1.0 model-local unit.
        // The model spans -0.5..+0.5, so the text fills the model height.
        float factor = (float)font.getSize();
        float textW = font.widthOf(_labelText, factor);

        // xlFontInfo::populate uses Y-down convention: yBase is the baseline (bottom
        // of characters), text extends upward.  We flip Y via Scale(1,-1,1) inside
        // the draw step so the layout's Y-up world space is honoured.  With the flip,
        // passing yBase=+0.5 places the baseline at the model bottom (y_world=-0.5)
        // and the glyph tops at the model top (y_world=+0.5).
        float x = -textW / 2.0f;
        float yBase = 0.5f;

        xlVertexTextureAccumulator *va = ctx->createVertexTextureAccumulator();
        font.populate(*va, x, yBase, _labelText, factor);
        va->Flush();

        xlColor tc = _textColor;
        transparentProgram->addStep([=, this](xlGraphicsContext *ctx) {
            ctx->PushMatrix();
            if (!is_3d) {
                ctx->ScaleViewMatrix(1.0f, 1.0f, 0.0f);
            }
            GetModelScreenLocation().ApplyModelViewMatrices(ctx);
            ctx->Scale(1.0f, -1.0f, 1.0f);
            ctx->enableBlending();
            ctx->drawTexture(va, fontTex, tc, 0, va->getCount());
            ctx->PopMatrix();
            delete va;
        });
    }

    if ((Selected() || (Highlighted() && is_3d)) && color != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted(), IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

std::list<std::string> LabelModel::CheckModelSettings()
{
    return std::list<std::string>();
}
