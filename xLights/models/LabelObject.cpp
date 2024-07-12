/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "LabelObject.h"
#include "UtilFunctions.h"
#include "ModelPreview.h"
#include "../xLightsMain.h"

#include <log4cpp/Category.hh>

LabelObject::LabelObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), _label("")
{
    SetFromXml(node);
}

LabelObject::~LabelObject()
{
}

void LabelObject::InitModel() {
	_label = ModelXml->GetAttribute("Label", "");

    if (ModelXml->HasAttribute("LabelColor")) {
        _color = xlColor(ModelXml->GetAttribute("LabelColor", "#00FF00").ToStdString());
    }

    screenLocation.SetRenderSize(1, 1, 10.0f);
}

void LabelObject::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {

    std::string l = _label;
    Replace(l, "\n", "\\n");

	wxPGProperty *p = grid->Append(new wxStringProperty("Label", "Label", l));

    p = grid->Append(new wxColourProperty("Label Color", "LabelColor", _color.asWxColor()));
}

int LabelObject::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{

    if ("LabelColor" == event.GetPropertyName()) {
        wxPGProperty* p = grid->GetPropertyByName("LabelColor");
        wxColour c;
        c << p->GetValue();
        _color = c;
        ModelXml->DeleteAttribute("LabelColor");
        ModelXml->AddAttribute("LabelColor", _color);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LabelObject::OnPropertyGridChange::LabelColor");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LabelObject::OnPropertyGridChange::LabelColor");
        return 0;
    }
    else if (event.GetPropertyName() == "Label") {
        _label = event.GetPropertyValue().GetString();
        Replace(_label, "\\n", "\n");
        ModelXml->DeleteAttribute("Label");
        ModelXml->AddAttribute("Label", _label);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "LabelObject::OnPropertyGridChange::Label");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "LabelObject::OnPropertyGridChange::Label");
        return 0;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

bool LabelObject::Draw(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* solid, xlGraphicsProgram* transparent, bool allowSelected) {
    if( !IsActive() ) { return true; }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool exists = false;

    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    float x = -0.5;
    float y = 0.5;
    float z = 0;
    GetObjectScreenLocation().TranslatePoint(x, y, z);

    float x2 = 0.5;
    float y2 = -0.5;
    float z2 = 0;
    GetObjectScreenLocation().TranslatePoint(x2, y2, z2);

    int lines = 1 + CountChar(_label, '\n');
    float lineHeight = (y - y2) / lines;

    if (_label != "") {
        xlVertexTextureAccumulator* texts = ctx->createVertexTextureAccumulator()->SetName("Labels");

        float factor = preview->translateToBacking(1.0);
        int toffset = 0;
        float fontSize = xlFontInfo::ComputeFontSize(toffset, lineHeight, factor);
        if (fontSize > 88) fontSize = 88;
        if (fontSize < 6) fontSize = 6;

        if (_curFontSize != fontSize || _fontTexture == nullptr) {
            if (_fontTexture) {
                delete _fontTexture;
            }
            _curFontSize = fontSize;
            const xlFontInfo& font = xlFontInfo::FindFont(_curFontSize);
            _fontTexture = ctx->createTextureForFont(font);
        }

        // I cant get 3d to work yet as it doesnt support the same force color feature.
        // There are colours but these colour the whole texture ... which is not what i want.

        // this does not handle colour

        const xlFontInfo &font = xlFontInfo::FindFont(_curFontSize);
        font.populate(*texts, x + toffset, y, _label, factor, true, z);
        ctx->drawTexture(texts->Flush(), _fontTexture, _color);
        texts->Reset();
    }

    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }

    return true;
}
