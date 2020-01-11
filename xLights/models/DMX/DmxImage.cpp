#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include <algorithm>

#include "DmxImage.h"
#include "../../UtilFunctions.h"
#include "../../ModelPreview.h"
#include "../../xLightsMain.h"

#include <log4cpp/Category.hh>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

DmxImage::DmxImage(wxXmlNode* node, wxString _name)
 : node_xml(node), base_name(_name), width(1), height(1),
    controls_size(false), obj_exists(false), show_empty(false)
{
}

DmxImage::~DmxImage()
{
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }
}

void DmxImage::Init(BaseObject* base, bool set_size, bool show_empty_) {

    _imageFile = FixFile("", node_xml->GetAttribute("Image", ""));

    offset_x = wxAtof(node_xml->GetAttribute("OffsetX", "0.0"));
    offset_y = wxAtof(node_xml->GetAttribute("OffsetY", "0.0"));
    offset_z = wxAtof(node_xml->GetAttribute("OffsetZ", "0.0"));

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

    show_empty = show_empty_;
    controls_size = set_size;
    if (controls_size) {
        base->GetBaseObjectScreenLocation().SetRenderSize(width * scalex, height * scaley, 10.0f);
    }
}

void DmxImage::AddTypeProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));

    wxPGProperty* prop = grid->Append(new wxImageFileProperty("Image", base_name + "Image", _imageFile));
    prop->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg|All files (*.*)|*.*");

    prop = grid->Append(new wxFloatProperty("Offset X", base_name + "OffsetX", offset_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Y", base_name + "OffsetY", offset_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Z", base_name + "OffsetZ", offset_z));
    prop->SetAttribute("Precision", 2);
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
}

int DmxImage::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    if (base_name + "Image" == name) {
        for (auto it = _images.begin(); it != _images.end(); ++it) {
            delete it->second;
        }
        _images.clear();
        _imageFile = event.GetValue().GetString();
        obj_exists = false;
        base->GetBaseObjectScreenLocation().SetScaleMatrix(glm::vec3(1.0f, 1.0f, 1.0f));  // reset scale when new image is loaded
        node_xml->DeleteAttribute("Image");
        node_xml->AddAttribute("Image", _imageFile);
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxServo::OnPropertyGridChange::StaticImage");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxServo::OnPropertyGridChange::StaticImage");
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
        offset_x = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetX");
        node_xml->AddAttribute("OffsetX", wxString::Format("%6.4f", offset_x));
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
        offset_y = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetY");
        node_xml->AddAttribute("OffsetY", wxString::Format("%6.4f", offset_y));
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
        offset_z = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetZ");
        node_xml->AddAttribute("OffsetZ", wxString::Format("%6.4f", offset_z));
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

void DmxImage::Draw(BaseObject* base, ModelPreview* preview, DrawGLUtils::xlAccumulator& va,
                    glm::mat4& base_matrix, glm::mat4& motion_matrix,
                    int transparency, float brightness,
                    int pivot_offset_x, int pivot_offset_y, bool use_pivot)
{
    bool exists = false;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (wxFileExists(_imageFile)) {
            logger_base.debug("Loading image model %s file %s for preview %s.",
                (const char*)base->GetName().c_str(),
                (const char*)_imageFile.c_str(),
                (const char*)preview->GetName().c_str());
            _images[preview->GetName().ToStdString()] = new Image(_imageFile);

            width = (_images[preview->GetName().ToStdString()])->width;
            height = (_images[preview->GetName().ToStdString()])->height;
            if (controls_size) {
                base->GetBaseObjectScreenLocation().SetRenderSize(width * scalex, height * scaley, 10.0f);
            }
            exists = true;
        }
    }
    else {
        exists = true;
    }

    float x1 = -0.5f * width;
    float x2 = -0.5f * width;
    float x3 = 0.5f * width;
    float x4 = 0.5f * width;
    float y1 = -0.5f * height;
    float y2 = 0.5f * height;
    float y3 = 0.5f * height;
    float y4 = -0.5f * height;
    float z1 = 0.0f;
    float z2 = 0.0f;
    float z3 = 0.0f;
    float z4 = 0.0f;

    if (controls_size) {
        base->GetBaseObjectScreenLocation().UpdateBoundingBox(width, height);  // FIXME: Modify to only call this when position changes
    }

    if (exists) {
        obj_exists = true;
        glm::mat4 Identity = glm::mat4(1.0f);
        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::quat rotate_quat = glm::quat_cast(rx * ry * rz);
        glm::mat4 translationMatrix = glm::translate(Identity, glm::vec3(offset_x, offset_y, offset_z));
        glm::mat4 pivotToZero = glm::translate(Identity, glm::vec3(-pivot_offset_x, -pivot_offset_y, 0.0f));
        glm::mat4 pivotBack = glm::translate(Identity, glm::vec3(pivot_offset_x, pivot_offset_y, 0.0f));
        glm::mat4 m = translationMatrix * glm::toMat4(rotate_quat) * scalingMatrix;
        m = base_matrix * m * pivotBack * motion_matrix * pivotToZero;

        glm::vec4 v = m * glm::vec4(glm::vec3(x1, y1, z1), 1.0f);
        x1 = v.x; y1 = v.y; z1 = v.z;
        v = m * glm::vec4(glm::vec3(x2, y2, z2), 1.0f);
        x2 = v.x; y2 = v.y; z2 = v.z;
        v = m * glm::vec4(glm::vec3(x3, y3, z3), 1.0f);
        x3 = v.x; y3 = v.y; z3 = v.z;
        v = m * glm::vec4(glm::vec3(x4, y4, z4), 1.0f);
        x4 = v.x; y4 = v.y; z4 = v.z;

        Image* image = _images[preview->GetName().ToStdString()];

        float tx1 = 0;
        float tx2 = image->tex_coord_x;

        //DrawGLUtils::xl3Accumulator& va = transparency == 0 ? va3 : tva3;

        va.PreAllocTexture(6);
        va.AddTextureVertex(x1, y1, z1, tx1, -0.5 / (image->textureHeight));
        va.AddTextureVertex(x4, y4, z4, tx2, -0.5 / (image->textureHeight));
        va.AddTextureVertex(x2, y2, z2, tx1, image->tex_coord_y);
        va.AddTextureVertex(x2, y2, z2, tx1, image->tex_coord_y);
        va.AddTextureVertex(x4, y4, z4, tx2, -0.5 / (image->textureHeight));
        va.AddTextureVertex(x3, y3, z3, tx2, image->tex_coord_y);
        int alpha = (100.0 - transparency) * 255.0 / 100.0;
        va.FinishTextures(GL_TRIANGLES, image->getID(), alpha, brightness);


        if (use_pivot) {
            xlColor pink = xlColor(255, 0, 255);
            float x1 = pivot_offset_x;
            float y1 = pivot_offset_y;
            float z1 = 0.0f;
            glm::vec4 v = m * glm::vec4(glm::vec3(x1, y1, 0.0f), 1.0f);
            x1 = v.x; y1 = v.y; z1 = v.z + 0.01;
            float pscale = base_matrix[0].x;
            va.AddTrianglesCircle(x1, y1, z1, 10.0 * pscale, xlBLACK);
            va.AddTrianglesCircle(x1, y1, z1+0.01, 9.0 * pscale, pink);
            va.Finish(GL_TRIANGLES);
            va.AddVertex(x1 - 7 * pscale, y1 + 7 * pscale, z1+0.02, xlBLACK);
            va.AddVertex(x1 + 7 * pscale, y1 - 7 * pscale, z1+0.02, xlBLACK);
            va.AddVertex(x1 - 7 * pscale, y1 - 7 * pscale, z1+0.02, xlBLACK);
            va.AddVertex(x1 + 7 * pscale, y1 + 7 * pscale, z1+0.02, xlBLACK);
            va.Finish(GL_LINES);
        }
    }
    else if (show_empty) {
        float x1 = -0.5f * width * scalex;
        float x2 = -0.5f * width * scalex;
        float x3 = 0.5f * width * scalex;
        float x4 = 0.5f * width * scalex;
        float y1 = -0.5f * height * scaley;
        float y2 = 0.5f * height * scaley;
        float y3 = 0.5f * height * scaley;
        float y4 = -0.5f * height * scaley;
        float z1 = 0.0f;
        float z2 = 0.0f;
        float z3 = 0.0f;
        float z4 = 0.0f;

        base->GetBaseObjectScreenLocation().TranslatePoint(x1, y1, z1);
        base->GetBaseObjectScreenLocation().TranslatePoint(x2, y2, z2);
        base->GetBaseObjectScreenLocation().TranslatePoint(x3, y3, z3);
        base->GetBaseObjectScreenLocation().TranslatePoint(x4, y4, z4);

        va.AddVertex(x1, y1, z1, *wxRED);
        va.AddVertex(x2, y2, z2, *wxRED);
        va.AddVertex(x2, y2, z2, *wxRED);
        va.AddVertex(x3, y3, z3, *wxRED);
        va.AddVertex(x3, y3, z3, *wxRED);
        va.AddVertex(x4, y4, z4, *wxRED);
        va.AddVertex(x4, y4, z4, *wxRED);
        va.AddVertex(x1, y1, z1, *wxRED);
        va.AddVertex(x1, y1, z1, *wxRED);
        va.AddVertex(x3, y3, z3, *wxRED);
        va.AddVertex(x2, y2, z2, *wxRED);
        va.AddVertex(x4, y4, z4, *wxRED);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 5.0f);
    }
}
