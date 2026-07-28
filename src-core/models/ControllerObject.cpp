/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <glm/gtc/matrix_transform.hpp>

#include <log.h>

#include "ControllerObject.h"
#include "../graphics/IModelPreview.h"
#include "../graphics/xlGraphicsContext.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "../graphics/xlFontInfo.h"
#include "../graphics/xlMesh.h"
#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"

namespace {
// Nominal enclosure proportions. Everything scales from here via the screen
// location, so these only set the default aspect a new object appears with.
constexpr float BOX_WIDTH = 100.0f;
constexpr float BOX_HEIGHT = 60.0f;
constexpr float BOX_DEPTH = 25.0f;

// The shipped enclosure's raised lid stands proud of the nominal front face, so
// anything painted on the front has to clear the LID, not the face - at the
// nominal face it is simply buried inside the mesh. Keep in step with
// resources/meshobjects/controller/controller.obj (its front-most vertex).
constexpr float FRONT_SURFACE_Z = BOX_DEPTH * 0.5f + 1.2f;
constexpr float FRONT_DECAL_OFFSET = 0.8f;  // clear of the lid, no z-fighting

// Label geometry, all relative to the face so it scales with the box.
constexpr float LABEL_HEIGHT_FRACTION = 0.16f;    // glyph height vs box height
constexpr float LABEL_MAX_WIDTH_FRACTION = 0.86f; // shrink to fit inside this

// Per-face shading so the box reads as a solid in 3D rather than a flat
// silhouette. Values are multipliers on the body colour.
xlColor shade(const xlColor& c, float f) {
    auto ch = [f](uint8_t v) {
        float r = v * f;
        if (r < 0.0f) r = 0.0f;
        if (r > 255.0f) r = 255.0f;
        return (uint8_t)(r + 0.5f);
    };
    return xlColor(ch(c.red), ch(c.green), ch(c.blue));
}
} // namespace

ControllerObject::ControllerObject(const ViewObjectManager& manager)
 : ObjectWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Controller;
    // The enclosure is a real solid, so depth is meaningful. Without this the
    // Scale tool emits no Z cube at all (BoxedScreenLocation::GetHandles gates
    // the Z axis on it, while Translate always emits all three) and the
    // scale session refuses Z, so depth would be editable only numerically.
    screenLocation.SetSupportsZScaling(true);
    // A controller sits somewhere physical whether or not the user works in 3D,
    // so unlike terrain / gridlines / meshes this one draws in a 2D preview too.
    SetIs3dOnly(false);
}

ControllerObject::~ControllerObject()
{
    for (auto& it : _fontTextures) {
        delete it.second;
    }
}

std::string ControllerObject::ObjectNameFor(const std::string& controllerName)
{
    return "Controller: " + controllerName;
}

std::string ControllerObject::VisibilityToString(Visibility v)
{
    switch (v) {
    case Visibility::ControllerTab: return "ControllerTab";
    case Visibility::LayoutPanel:   return "LayoutPanel";
    case Visibility::Always:        return "Always";
    case Visibility::Off:           return "Off";
    }
    return "Off";
}

ControllerObject::Visibility ControllerObject::VisibilityFromString(const std::string& s)
{
    if (s == "ControllerTab") return Visibility::ControllerTab;
    if (s == "LayoutPanel")   return Visibility::LayoutPanel;
    if (s == "Always")        return Visibility::Always;
    return Visibility::Off;
}

bool ControllerObject::ShouldDraw(Visibility v, ControllerObjectContext ctx)
{
    switch (v) {
    case Visibility::Off:
        return false;
    case Visibility::ControllerTab:
        return ctx == ControllerObjectContext::LayoutEditorControllerTab;
    case Visibility::LayoutPanel:
        return ctx == ControllerObjectContext::LayoutEditor ||
               ctx == ControllerObjectContext::LayoutEditorControllerTab;
    case Visibility::Always:
        return true;
    }
    return false;
}

void ControllerObject::SetControllerName(const std::string& name)
{
    if (_controllerName != name) {
        _controllerName = name;
        IncrementChangeCount();
    }
}

void ControllerObject::InitModel()
{
    screenLocation.SetRenderSize(BOX_WIDTH, BOX_HEIGHT, BOX_DEPTH);
}

bool ControllerObject::Draw(IModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* solid, xlGraphicsProgram* transparent, bool allowSelected)
{
    if (!IsActive()) { return true; }

    const bool is3d = preview->Is3D();
    GetObjectScreenLocation().PrepareToDraw(is3d, allowSelected);

    if (!_meshLoadAttempted) {
        _meshLoadAttempted = true;
        const std::string objFile = FileUtils::GetResourcesDir() + "/meshobjects/controller/controller.obj";
        if (FileExists(objFile)) {
            _mesh = ctx->loadMeshFromObjFile(objFile);
            if (_mesh) {
                _mesh->SetName("ControllerEnclosure");
            }
        }
        if (!_mesh) {
            spdlog::debug("Controller enclosure mesh not available ('{}'); drawing the fallback box.", objFile);
        }
    }

    if (!is3d) {
        // 2D is a flat plan view and its projection is glm::ortho(0,w,0,h) -
        // a clip range of z in [-1,1] with the view matrix leaving z alone. The
        // box's own depth (and any worldPos_z picked up while dragging in 3D)
        // is far outside that, so a solid box is clipped away entirely. Draw
        // the footprint flat on the canvas plane instead.
        DrawFlatBody(solid);
        GetObjectScreenLocation().UpdateBoundingBox(BOX_WIDTH, BOX_HEIGHT, BOX_DEPTH);
        DrawStatusLed(solid, true);
        DrawLabel(preview, ctx, solid, true);
        if ((Selected() || Highlighted()) && allowSelected) {
            GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
        return true;
    }

    if (_mesh) {
        const glm::vec3 scale = GetObjectScreenLocation().GetScaleMatrix();
        const glm::vec3 rot = GetObjectScreenLocation().GetRotation();
        const float hcx = GetObjectScreenLocation().GetHcenterPos();
        const float vcy = GetObjectScreenLocation().GetVcenterPos();
        const float dcz = GetObjectScreenLocation().GetDcenterPos();
        xlMesh* mesh = _mesh.get();
        solid->addStep([=](xlGraphicsContext* ctx) {
            ctx->PushMatrix()
                ->Translate(hcx, vcy, dcz)
                ->Rotate(rot.z, 0, 0, 1)
                ->Rotate(rot.y, 0, 1, 0)
                ->Rotate(rot.x, 1, 0, 0)
                ->Scale(scale.x, scale.y, scale.z)
                ->drawMeshSolids(mesh, 100, true)
                ->PopMatrix();
        });
    } else {
        DrawFallbackBox(solid);
    }

    GetObjectScreenLocation().UpdateBoundingBox(BOX_WIDTH, BOX_HEIGHT, BOX_DEPTH);

    DrawStatusLed(solid, false);
    DrawLabel(preview, ctx, solid, false);

    if ((Selected() || Highlighted()) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return true;
}

// 2D body: the enclosure footprint as a flat filled rectangle with a lighter
// edge, on the canvas plane. See the note in Draw for why nothing with real
// depth survives the 2D projection.
void ControllerObject::DrawFlatBody(xlGraphicsProgram* solid)
{
    const float hw = BOX_WIDTH * 0.5f;
    const float hh = BOX_HEIGHT * 0.5f;

    float px[4] = { -hw,  hw,  hw, -hw };
    float py[4] = { -hh, -hh,  hh,  hh };
    float pz[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    for (int i = 0; i < 4; ++i) {
        GetObjectScreenLocation().TranslatePoint(px[i], py[i], pz[i]);
        pz[i] = 0.0f;  // flatten: worldPos_z would otherwise clip the whole quad
    }

    auto vac = solid->getAccumulator();
    const int startVert = vac->getCount();
    vac->PreAlloc(6);
    const xlColor body = shade(_statusColor, 0.9f);
    vac->AddVertex(px[0], py[0], pz[0], body);
    vac->AddVertex(px[1], py[1], pz[1], body);
    vac->AddVertex(px[2], py[2], pz[2], body);
    vac->AddVertex(px[2], py[2], pz[2], body);
    vac->AddVertex(px[3], py[3], pz[3], body);
    vac->AddVertex(px[0], py[0], pz[0], body);
    const int endVert = vac->getCount();
    solid->addStep([vac, startVert, endVert](xlGraphicsContext* ctx) {
        ctx->drawTriangles(vac, startVert, endVert - startVert);
    });

    // Outline so the box reads against a dark background photo.
    auto lac = solid->getAccumulator();
    const int lineStart = lac->getCount();
    lac->PreAlloc(8);
    const xlColor edge = shade(_statusColor, 1.6f);
    for (int i = 0; i < 4; ++i) {
        const int j = (i + 1) % 4;
        lac->AddVertex(px[i], py[i], pz[i], edge);
        lac->AddVertex(px[j], py[j], pz[j], edge);
    }
    const int lineEnd = lac->getCount();
    solid->addStep([lac, lineStart, lineEnd](xlGraphicsContext* ctx) {
        ctx->drawLines(lac, lineStart, lineEnd - lineStart);
    });
}

// Used when the shipped enclosure mesh is missing or fails to load. Keeps the
// feature usable without the asset - which is what let phases 1-5 be built and
// tested before it existed.
void ControllerObject::DrawFallbackBox(xlGraphicsProgram* solid)
{
    const float hw = BOX_WIDTH * 0.5f;
    const float hh = BOX_HEIGHT * 0.5f;
    const float hd = BOX_DEPTH * 0.5f;

    // Object-space corners, indexed so each face can name its four in winding
    // order. -Z is towards the viewer's front face.
    float cx[8] = { -hw,  hw,  hw, -hw, -hw,  hw,  hw, -hw };
    float cy[8] = { -hh, -hh,  hh,  hh, -hh, -hh,  hh,  hh };
    float cz[8] = {  hd,  hd,  hd,  hd, -hd, -hd, -hd, -hd };
    for (int i = 0; i < 8; ++i) {
        GetObjectScreenLocation().TranslatePoint(cx[i], cy[i], cz[i]);
    }

    struct Face { int a, b, c, d; float shading; };
    static const Face faces[6] = {
        { 0, 1, 2, 3, 1.00f }, // front
        { 5, 4, 7, 6, 0.55f }, // back
        { 4, 0, 3, 7, 0.75f }, // left
        { 1, 5, 6, 2, 0.75f }, // right
        { 3, 2, 6, 7, 0.90f }, // top
        { 4, 5, 1, 0, 0.45f }, // bottom
    };

    auto vac = solid->getAccumulator();
    const int startVert = vac->getCount();
    vac->PreAlloc(36);
    for (const auto& f : faces) {
        const xlColor fc = shade(_statusColor, f.shading);
        vac->AddVertex(cx[f.a], cy[f.a], cz[f.a], fc);
        vac->AddVertex(cx[f.b], cy[f.b], cz[f.b], fc);
        vac->AddVertex(cx[f.c], cy[f.c], cz[f.c], fc);
        vac->AddVertex(cx[f.c], cy[f.c], cz[f.c], fc);
        vac->AddVertex(cx[f.d], cy[f.d], cz[f.d], fc);
        vac->AddVertex(cx[f.a], cy[f.a], cz[f.a], fc);
    }
    const int endVert = vac->getCount();
    solid->addStep([vac, startVert, endVert](xlGraphicsContext* ctx) {
        ctx->drawTriangles(vac, startVert, endVert - startVert);
    });
}

// Ping state as a small lamp on the front face. Drawn for both the mesh and the
// fallback box: the mesh takes its colours from its .mtl, so this is the only
// way status survives the artwork path.
void ControllerObject::DrawStatusLed(xlGraphicsProgram* solid, bool flat)
{
    const float cx0 = BOX_WIDTH * 0.5f - 12.0f;
    const float cy0 = BOX_HEIGHT * 0.5f - 8.0f;
    const float r = 3.5f;
    const float z = flat ? 0.0f : (FRONT_SURFACE_Z + FRONT_DECAL_OFFSET);

    float px[4] = { cx0 - r, cx0 + r, cx0 + r, cx0 - r };
    float py[4] = { cy0 - r, cy0 - r, cy0 + r, cy0 + r };
    float pz[4] = { z, z, z, z };
    for (int i = 0; i < 4; ++i) {
        GetObjectScreenLocation().TranslatePoint(px[i], py[i], pz[i]);
        if (flat) pz[i] = 0.0f;
    }

    auto vac = solid->getAccumulator();
    const int startVert = vac->getCount();
    vac->PreAlloc(6);
    const xlColor c = _statusColor;
    vac->AddVertex(px[0], py[0], pz[0], c);
    vac->AddVertex(px[1], py[1], pz[1], c);
    vac->AddVertex(px[2], py[2], pz[2], c);
    vac->AddVertex(px[2], py[2], pz[2], c);
    vac->AddVertex(px[3], py[3], pz[3], c);
    vac->AddVertex(px[0], py[0], pz[0], c);
    const int endVert = vac->getCount();
    solid->addStep([vac, startVert, endVert](xlGraphicsContext* ctx) {
        ctx->drawTriangles(vac, startVert, endVert - startVert);
    });
}

// The controller name, painted onto the front face of the enclosure. Drawn as
// world-space geometry rather than a screen-space overlay so it rotates and
// scales with the box like a real label, and is depth-sorted with everything
// else instead of floating at a fixed depth.
void ControllerObject::DrawLabel(IModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* solid, bool flat)
{
    if (!_showLabel || _controllerName.empty()) { return; }

    xlTexture*& tex = _fontTextures[preview->getName()];
    const xlFontInfo& font = xlFontInfo::FindFont(20);
    if (tex == nullptr) {
        tex = ctx->createTextureForFont(font);
        if (tex == nullptr) {
            _fontTextures.erase(preview->getName());
            return;
        }
    }

    // Work directly in object units: pick a glyph height as a fraction of the
    // face, then shrink it if the name would overrun the face width.
    float charH = BOX_HEIGHT * LABEL_HEIGHT_FRACTION;
    float factor = (float)font.getSize() / charH;
    float textW = font.widthOf(_controllerName, factor);
    const float maxW = BOX_WIDTH * LABEL_MAX_WIDTH_FRACTION;
    if (textW > maxW && textW > 0.0f) {
        charH *= maxW / textW;
        factor = (float)font.getSize() / charH;
        textW = font.widthOf(_controllerName, factor);
    }

    // Build the text-space -> world transform from three probe points, so the
    // object's full TRS chain (including any rotation) carries the label.
    // populate() lays glyphs out Y-down from the given baseline, hence -Y.
    const float faceZ = flat ? 0.0f : (FRONT_SURFACE_Z + FRONT_DECAL_OFFSET);
    auto probe = [this, flat](float x, float y, float z) {
        GetObjectScreenLocation().TranslatePoint(x, y, z);
        if (flat) z = 0.0f;
        return glm::vec3(x, y, z);
    };
    const glm::vec3 origin = probe(-textW * 0.5f, charH * 0.5f, faceZ);
    const glm::vec3 base   = probe(0.0f, 0.0f, faceZ);
    const glm::vec3 axisX  = probe(1.0f, 0.0f, faceZ) - base;
    const glm::vec3 axisY  = probe(0.0f, 1.0f, faceZ) - base;

    glm::mat4 toWorld(1.0f);
    toWorld[0] = glm::vec4(axisX, 0.0f);
    toWorld[1] = glm::vec4(-axisY, 0.0f);
    toWorld[2] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // text is planar; z unused
    toWorld[3] = glm::vec4(origin, 1.0f);

    const std::string label = _controllerName;
    const float baseline = charH;
    solid->addStep([tex, factor, baseline, label, toWorld](xlGraphicsContext* ctx) {
        const xlFontInfo& font = xlFontInfo::FindFont(20);
        xlVertexTextureAccumulator* vta = ctx->createVertexTextureAccumulator();
        font.populate(*vta, 0.0f, baseline, label, factor);
        if (vta->getCount() > 0) {
            ctx->PushMatrix();
            ctx->enableBlending();
            ctx->ApplyMatrix(toWorld);
            ctx->drawTexture(vta, tex, xlWHITE);
            ctx->PopMatrix();
        }
        delete vta;
    });
}
