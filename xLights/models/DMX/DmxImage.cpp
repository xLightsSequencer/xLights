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
#include <wx/sstream.h>

#include <algorithm>

#include "DmxModel.h"
#include "DmxImage.h"
#include "../../UtilFunctions.h"
#include "../../ExternalHooks.h"
#include "../../ModelPreview.h"
#include "../../xLightsMain.h"

#include <log4cpp/Category.hh>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

static const float OFFSET_SCALE = 100.0f;

DmxImage::DmxImage(wxXmlNode* node, wxString _name)
 : node_xml(node), _imageFile(""), width(1), height(1),
    obj_exists(false), image_selected(false),
    offset_x(0.0f), offset_y(0.0f), offset_z(0.0f),
    scalex(1.0f), scaley(1.0f), scalez(1.0f),
    rotatex(0.0f), rotatey(0.0f), rotatez(0.0f), base_name(_name)
{
}

DmxImage::~DmxImage()
{
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }
}

void DmxImage::SetOffsetZ(float value, BaseObject* base)
{
    offset_z = value;
    node_xml->DeleteAttribute("OffsetZ");
    node_xml->AddAttribute("OffsetZ", wxString::Format("%6.4f", offset_z));
    base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::SetOffsetZ");
    base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::SetOffsetZ");
    base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::SetOffsetZ");
    base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::SetOffsetZ");
}

void DmxImage::SetScaleX(float value, BaseObject* base)
{
    scalex = value;
    node_xml->DeleteAttribute("ScaleX");
    node_xml->AddAttribute("ScaleX", wxString::Format("%6.4f", scalex));
    base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::SetScaleX");
    base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::SetScaleX");
    base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::SetScaleX");
    base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::SetScaleX");
}

void DmxImage::SetScaleY(float value, BaseObject* base)
{
    scaley = value;
    node_xml->DeleteAttribute("ScaleY");
    node_xml->AddAttribute("ScaleY", wxString::Format("%6.4f", scaley));
    base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::SetScaleY");
    base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::SetScaleY");
    base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::SetScaleY");
    base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::SetScaleY");
}

void DmxImage::Init(BaseObject* base) {

    _imageFile = FixFile("", node_xml->GetAttribute("Image", ""));
    if (_imageFile != node_xml->GetAttribute("Image", "")) {
        node_xml->DeleteAttribute("Image");
        node_xml->AddAttribute("Image", _imageFile);
    }

    offset_x = wxAtof(node_xml->GetAttribute("OffsetX", "0.0")) / OFFSET_SCALE;
    offset_y = wxAtof(node_xml->GetAttribute("OffsetY", "0.0")) / OFFSET_SCALE;
    offset_z = wxAtof(node_xml->GetAttribute("OffsetZ", "0.0")) / OFFSET_SCALE;

    scalex = wxAtof(node_xml->GetAttribute("ScaleX", "1.0"));
    scaley = wxAtof(node_xml->GetAttribute("ScaleY", "1.0"));
    scalez = wxAtof(node_xml->GetAttribute("ScaleZ", "1.0"));

    if (scalex < 0) {
        scalex = 1.0f;
    }
    if (scaley < 0) {
        scaley = 1.0f;
    }
    if (scalez < 0) {
        scalez = 1.0f;
    }

    rotatex = wxAtof(node_xml->GetAttribute("RotateX", "0.0f"));
    rotatey = wxAtof(node_xml->GetAttribute("RotateY", "0.0f"));
    rotatez = wxAtof(node_xml->GetAttribute("RotateZ", "0.0f"));

    if (rotatex < -180.0f || rotatex > 180.0f) {
        rotatex = 0.0f;
    }
    if (rotatey < -180.0f || rotatey > 180.0f) {
        rotatey = 0.0f;
    }
    if (rotatez < -180.0f || rotatez > 180.0f) {
        rotatez = 0.0f;
    }
}

void DmxImage::AddTypeProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));

    wxPGProperty* prop = grid->Append(new wxImageFileProperty("Image", base_name + "Image", _imageFile));
    prop->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                           ";*.webp"
                                           "|All files (*.*)|*.*");

    prop = grid->Append(new wxFloatProperty("Offset X", base_name + "OffsetX", offset_x * OFFSET_SCALE));
    prop->SetAttribute("Precision", 1);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Y", base_name + "OffsetY", offset_y * OFFSET_SCALE));
    prop->SetAttribute("Precision", 1);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Z", base_name + "OffsetZ", offset_z * OFFSET_SCALE));
    prop->SetAttribute("Precision", 1);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleX", base_name + "ScaleX", scalex));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleY", base_name + "ScaleY", scaley));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleZ", base_name + "ScaleZ", scalez));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateX", base_name + "RotateX", rotatex));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateY", base_name + "RotateY", rotatey));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateZ", base_name + "RotateZ", rotatez));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");

    grid->Collapse(base_name + "Properties");
}

int DmxImage::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    if (base_name + "Image" == name) {
        for (auto it = _images.begin(); it != _images.end(); ++it) {
            delete it->second;
        }
        _images.clear();
        _imageFile = event.GetValue().GetString();
        ObtainAccessToURL(_imageFile);
        obj_exists = false;
        image_selected = true;
        node_xml->DeleteAttribute("Image");
        node_xml->AddAttribute("Image", _imageFile);
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::Image");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::Image");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::Image");
        return 0;
    }
    else if (!locked && base_name + "ScaleX" == name) {
        scalex = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleX");
        node_xml->AddAttribute("ScaleX", wxString::Format("%6.4f", scalex));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ScaleX");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "ScaleY" == name) {
        scaley = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleY");
        node_xml->AddAttribute("ScaleY", wxString::Format("%6.4f", scaley));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ScaleY");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "ScaleZ" == name) {
        scalez = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleZ");
        node_xml->AddAttribute("ScaleZ", wxString::Format("%6.4f", scalez));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ScaleZ");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetX" == name) {
        offset_x = event.GetValue().GetDouble() / OFFSET_SCALE;
        node_xml->DeleteAttribute("OffsetX");
        node_xml->AddAttribute("OffsetX", wxString::Format("%6.4f", offset_x * OFFSET_SCALE));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ModelX");
        return 0;
    }
    else if (locked && base_name + "OffsetX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetY" == name) {
        offset_y = event.GetValue().GetDouble() / OFFSET_SCALE;
        node_xml->DeleteAttribute("OffsetY");
        node_xml->AddAttribute("OffsetY", wxString::Format("%6.4f", offset_y * OFFSET_SCALE));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ModelY");
        return 0;
    }
    else if (locked && base_name + "OffsetY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetZ" == name) {
        offset_z = event.GetValue().GetDouble() / OFFSET_SCALE;
        node_xml->DeleteAttribute("OffsetZ");
        node_xml->AddAttribute("OffsetZ", wxString::Format("%6.4f", offset_z * OFFSET_SCALE));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::ModelZ");
        return 0;
    }
    else if (locked && base_name + "OffsetZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateX" == name) {
        rotatex = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateX");
        node_xml->AddAttribute("RotateX", wxString::Format("%4.8f", rotatex));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (locked && base_name + "RotateX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateY" == name) {
        rotatey = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateY");
        node_xml->AddAttribute("RotateY", wxString::Format("%4.8f", rotatey));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::RotateY");
        return 0;
    }
    else if (locked && base_name + "RotateY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateZ" == name) {
        rotatez = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateZ");
        node_xml->AddAttribute("RotateZ", wxString::Format("%4.8f", rotatez));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxImage::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxImage::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxImage::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxImage::OnPropertyGridChange::RotateZ");
        return 0;
    }
    else if (locked && base_name + "RotateZ" == name) {
        event.Veto();
        return 0;
    }

    return -1;
}

void DmxImage::Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *pg,
                    glm::mat4 &motion_matrix,
                    int transparency, float brightness, bool only_image,
                    float pivot_offset_x, float pivot_offset_y, bool rotation, bool use_pivot)
{
    bool exists = false;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (FileExists(_imageFile)) {
            logger_base.debug("Loading image model %s file %s for preview %s.",
                (const char*)base->GetName().c_str(),
                (const char*)_imageFile.c_str(),
                (const char*)preview->GetName().c_str());
            wxImage img(_imageFile);
            if (img.IsOk()) {
                xlTexture *t = preview->getCurrentGraphicsContext()->createTexture(img);
                _images[preview->GetName().ToStdString()] = t;
                width = img.GetWidth();
                height = img.GetHeight();
                exists = true;
                obj_exists = true;
            }
        }
    } else {
        exists = true;
    }


    if (exists) {
        xlTexture* image = _images[preview->GetName().ToStdString()];

        glm::mat4 Identity = glm::mat4(1.0f);
        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::quat rotate_quat = glm::quat_cast(rx * ry * rz);
        glm::mat4 translationMatrix = glm::translate(Identity, glm::vec3(offset_x, offset_y, offset_z));
        glm::mat4 m = translationMatrix * glm::toMat4(rotate_quat) * scalingMatrix;
        if (rotation) {
            glm::mat4 pivotToZero = glm::translate(Identity, glm::vec3(-pivot_offset_x, -pivot_offset_y, 0.0f));
            glm::mat4 pivotBack = glm::translate(Identity, glm::vec3(pivot_offset_x, pivot_offset_y, 0.0f));
            m = m * pivotBack * motion_matrix * pivotToZero;
        } else {
            m = m * motion_matrix;
        }
        
        auto vac = pg->getAccumulator();
        int start = vac->getCount();
        if (use_pivot && rotation) {
            glm::vec3 scale = base->GetBaseObjectScreenLocation().GetScaleMatrix();
            float mw = std::min(4.0f / scale.x, 4.0f / scale.y);
            float mw2 = std::min(3.0f / scale.x, 3.0f / scale.y);

            
            xlColor pink = xlColor(255, 0, 255);
            //stack z coordinates
            vac->AddCircleAsTriangles(0, 0, 0.001f, mw, xlBLACK);
            vac->AddCircleAsTriangles(0, 0, 0.002f, mw2, pink);
            vac->AddVertex(0.0-mw/2, 0.0-mw/2, 0.003f, xlBLACK);
            vac->AddVertex(0.0+mw/2, 0.0+mw/2, 0.003f, xlBLACK);
            vac->AddVertex(0.0-mw/2, 0.0+mw/2, 0.003f, xlBLACK);
            vac->AddVertex(0.0+mw/2, 0.0-mw/2, 0.003f, xlBLACK);
        }

        int end = vac->getCount();
        int alpha = (100.0 - transparency) * 255.0 / 100.0;
        pg->addStep([=](xlGraphicsContext *ctx) {
            ctx->PushMatrix();
            ctx->ApplyMatrix(m);
            ctx->drawTexture(image,
                             -0.5, 0.5, 0.5, -0.5,
                             0.0, 0.0, 1.0, 1.0,
                             true, 100, alpha);
            if (end != start) {
                if (rotation) {
                    ctx->Translate(pivot_offset_x, pivot_offset_y, 0.0f);
                }
                ctx->drawTriangles(vac, start, end - 4 - start);
                ctx->drawLines(vac, end - 4, 4);
            }
            ctx->PopMatrix();
        });

    } else if (only_image) {
        DmxModel::DrawInvalid(pg, nullptr, false, false);
    }
}

void DmxImage::Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const
{
    wxString res = "";

    wxXmlNode* child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == base_name) {
            wxXmlDocument new_doc;
            new_doc.SetRoot(new wxXmlNode(*child));
            wxStringOutputStream stream;
            new_doc.Save(stream);
            wxString s = stream.GetString();
            s = s.SubString(s.Find("\n") + 1, s.Length()); // skip over xml format header
            int index = s.Find(show_dir);
            while (index != wxNOT_FOUND) {
                s = s.SubString(0, index - 1) + s.SubString(index + show_dir.Length() + 1, s.Length());
                index = s.Find(show_dir);
            }
            res += s;
        }
        child = child->GetNext();
    }

    if (res != "")
    {
        f.Write(res);
    }
}

// Serialise for input
void DmxImage::Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const
{
    wxXmlNode* node = nullptr;
    for (wxXmlNode* n = model_xml->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == base_name)
        {
            node = n;
            break;
        }
    }

    if (node != nullptr) {
        // add new attributes from import
        for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == base_name)
            {
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext())
                {
                    wxString s = a->GetValue();
                    if (a->GetName() == "Image") {
                        s = show_dir + wxFileName::GetPathSeparator() + s;
                    }
                    if (node->HasAttribute(a->GetName())) {
                        node->DeleteAttribute(a->GetName());
                    }
                    node->AddAttribute(a->GetName(), s);
                }
                return;
            }
        }
    }
}
