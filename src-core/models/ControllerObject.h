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

#include <map>
#include <memory>
#include <string>

#include "Color.h"
#include "ViewObject.h"
#include "BoxedScreenLocation.h"

class IModelPreview;
class xlTexture;
class xlMesh;

// Which surface is drawing, for the purposes of ControllerObject visibility.
// Only the layout editor's own preview is ever set to a LayoutEditor* value;
// house preview, sequencer preview and the per-layout-group preview panes stay
// None, so they only ever show objects set to Always.
enum class ControllerObjectContext {
    None,
    LayoutEditor,
    LayoutEditorControllerTab
};

// A physical stand-in for a controller in the layout preview, so a user can
// place a box where the controller actually sits in their display.
//
// Bound to a controller by name. Every controller object draws the SAME generic
// enclosure - appearance is a constant of the type, never per-vendor. See
// plans/controller-layout-objects.md section 3.1 for why, before adding any
// artwork hook here.
class ControllerObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
public:
    // When the object is drawn. Default Off so existing shows are unaffected.
    enum class Visibility {
        Off,           // never
        ControllerTab, // only while the Controllers page of the layout tab is active
        LayoutPanel,   // anywhere in the layout editor, but not during playback
        Always         // every preview, including house preview and playback
    };

    ControllerObject(const ViewObjectManager& manager);
    virtual ~ControllerObject();

    virtual void InitModel() override;

    virtual bool Draw(IModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* solid, xlGraphicsProgram* transparent, bool allowSelected = false) override;

    // The bound controller. The object's name is derived from this and kept in
    // sync by ViewObjectManager::RenameController - see ObjectNameFor().
    void SetControllerName(const std::string& name);
    const std::string& GetControllerName() const { return _controllerName; }

    void SetVisibility(Visibility v) { _visibility = v; }
    Visibility GetVisibility() const { return _visibility; }

    // The name label is off by default - in a dense layout a label per
    // controller is more clutter than help, and the box position is usually
    // enough to identify it.
    void SetShowLabel(bool b) { _showLabel = b; }
    bool GetShowLabel() const { return _showLabel; }

    // Body tint, driven from the controller's ping state by the UI layer. Core
    // has no notion of pinging, so this is pushed in rather than pulled.
    void SetStatusColor(const xlColor& c) { _statusColor = c; }
    const xlColor& GetStatusColor() const { return _statusColor; }

    static std::string VisibilityToString(Visibility v);
    static Visibility VisibilityFromString(const std::string& s);

    // Visibility policy. Lives here rather than in the preview so both the
    // desktop canvas and any future non-wx preview apply the same rules.
    // Callers check this at their draw-loop call site - Draw() itself has no
    // notion of which preview it is beyond IModelPreview::getName().
    static bool ShouldDraw(Visibility v, ControllerObjectContext ctx);
    bool ShouldDrawIn(ControllerObjectContext ctx) const { return ShouldDraw(_visibility, ctx); }

    // The object name is always derived from the controller name, never
    // user-edited. The base-show merge matches objects on their name attribute,
    // so letting the two drift would let base's copy arrive as a second object
    // bound to the same controller. See plans/controller-layout-objects.md 6.3.
    static std::string ObjectNameFor(const std::string& controllerName);

    void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

protected:

private:
    // `flat` collapses everything onto the canvas plane for the 2D preview,
    // whose projection clips anything with real depth.
    void DrawLabel(IModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* solid, bool flat);
    void DrawFallbackBox(xlGraphicsProgram* solid);
    void DrawFlatBody(xlGraphicsProgram* solid);
    void DrawStatusLed(xlGraphicsProgram* solid, bool flat);

    std::string _controllerName;
    Visibility _visibility { Visibility::Off };
    bool _showLabel { false };
    xlColor _statusColor { 96, 96, 104 };

    // The one generic enclosure, shared by every controller. Loaded lazily on
    // first draw; a null mesh after the attempt means the fallback box is used
    // for the rest of the session.
    std::unique_ptr<xlMesh> _mesh;
    bool _meshLoadAttempted { false };

    // Font atlas is per graphics context, so it is cached per preview name the
    // same way ImageObject caches its textures.
    std::map<std::string, xlTexture*> _fontTextures;
};
