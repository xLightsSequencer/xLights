/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "render/ViewpointMgr.h"
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "UtilFunctions.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

void PreviewCamera::Reset()
{
    posX = 0.0f;
    posY = 0.0f;
    posZ = 0.0f;
    angleX = 20.0f;
    angleY = 5.0f;
    angleZ = 0.0f;
    distance = -2000.0f;
    zoom = 1.0f;
    panx = 0.0f;
    pany = 0.0f;
    panz = 0.0f;
    zoom_corrx = 0.0f;
    zoom_corry = 0.0f;
    mat_valid = false;
}

PreviewCamera::PreviewCamera(bool is_3d_)
: posX(0.0f), posY(0.0f), posZ(0.0f), angleX(20.0f), angleY(5.0f), angleZ(0.0f), distance(-2000.0f), zoom(1.0f),
  panx(0.0f), pany(0.0f), panz(0.0f), zoom_corrx(0.0f), zoom_corry(0.0f), is_3d(is_3d_),
  name("Name Unspecified"), mat_valid(false)
{
}

PreviewCamera::~PreviewCamera()
{
}

// Copy constructor
PreviewCamera::PreviewCamera(const PreviewCamera &cam)
: posX(cam.posX), posY(cam.posY), posZ(cam.posZ), angleX(cam.angleX), angleY(cam.angleY), angleZ(cam.angleZ), distance(cam.distance), zoom(cam.zoom),
  panx(cam.panx), pany(cam.pany), panz(cam.panz), zoom_corrx(cam.zoom_corrx), zoom_corry(cam.zoom_corry), is_3d(cam.is_3d),
  name(cam.name), mat_valid(false)
{
}

PreviewCamera& PreviewCamera::operator= (const PreviewCamera& rhs)
{
    if (this == &rhs) return *this;   // Gracefully handle self assignment
    posX = rhs.posX;
    posY = rhs.posY;
    posZ = rhs.posZ;
    angleX = rhs.angleX;
    angleY = rhs.angleY;
    angleZ = rhs.angleZ;
    distance = rhs.distance;
    zoom = rhs.zoom;
    panx = rhs.panx;
    pany = rhs.pany;
    panz = rhs.panz;
    zoom_corrx = rhs.zoom_corrx;
    zoom_corry = rhs.zoom_corry;
    is_3d = rhs.is_3d;
    mat_valid = false;
    return *this;
}

glm::mat4& PreviewCamera::GetViewMatrix()
{
    if (!mat_valid) {
        glm::mat4 ViewTranslatePan = glm::translate(glm::mat4(1.0f), glm::vec3(posX + panx, posY + pany, posZ + panz));
        glm::mat4 ViewTranslateDistance = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, distance * zoom));
        glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 ViewRotateZ = glm::rotate(glm::mat4(1.0f), glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));
        view_matrix = ViewTranslateDistance * ViewRotateX * ViewRotateY * ViewRotateZ * ViewTranslatePan;
        mat_valid = true;
    }
    return view_matrix;
}

void PreviewCamera::SetAngleX(float value)
{
    angleX = value;
    if (angleX < 0.0f) {
        angleX += 360.0f;
    }
    if (angleX >= 360.0f) {
        angleX -= 360.0f;
    }
    mat_valid = false;
}

void PreviewCamera::SetAngleY(float value)
{
    angleY = value;
    if (angleY < 0.0f) {
        angleY += 360.0f;
    }
    if (angleY >= 360.0f) {
        angleY -= 360.0f;
    }
    mat_valid = false;
}
void PreviewCamera::SetAngleZ(float value)
{
    angleZ = value;
    if (angleZ < 0.0f) {
        angleZ += 360.0f;
    }
    if (angleZ >= 360.0f) {
        angleZ -= 360.0f;
    }
    mat_valid = false;
}

ViewpointMgr::ViewpointMgr()
{
    //ctor
}

ViewpointMgr::~ViewpointMgr() = default;

void ViewpointMgr::DeleteCamera3D(int i)
{
    if ((size_t)i >= previewCameras3d.size()) return;
    previewCameras3d.erase(previewCameras3d.begin() + i);
}

void ViewpointMgr::DeleteCamera2D(int i)
{
    if ((size_t)i >= previewCameras2d.size()) return;
    previewCameras2d.erase(previewCameras2d.begin() + i);
}

PreviewCamera* ViewpointMgr::GetNamedCamera3D(const std::string& name)
{
    for (auto& cam : previewCameras3d) {
        if (cam->name == name) {
            return cam.get();
        }
    }
    return nullptr;
}

bool ViewpointMgr::IsNameUnique(const std::string& name, bool is_3d)
{
    auto& cameras = is_3d ? previewCameras3d : previewCameras2d;
    for (auto& cam : cameras) {
        if (cam->name == name) {
            return false;
        }
    }
    return true;
}

void ViewpointMgr::Clear()
{
    previewCameras2d.clear();
    previewCameras3d.clear();
}

void ViewpointMgr::AddCamera( std::string name, PreviewCamera* current_camera, bool is_3d )
{
    std::unique_ptr<PreviewCamera> new_camera;
    if (current_camera != nullptr) {
        new_camera = std::make_unique<PreviewCamera>(*current_camera);
    }
    else {
        new_camera = std::make_unique<PreviewCamera>(is_3d);
    }
    new_camera->name = name;
    if (new_camera->name == "") {
        new_camera->name = "...";  // avoid exception that occurs if menu name is blank
    }
    if (is_3d) {
        previewCameras3d.push_back(std::move(new_camera));
    }
    else {
        previewCameras2d.push_back(std::move(new_camera));
    }
}

void ViewpointMgr::SaveCameraToVisitor(BaseSerializingVisitor& visitor, const PreviewCamera* camera,
                                        const std::string& nodename, const std::string& nameOverride) const
{
    using F = BaseSerializingVisitor;
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name", XmlSafe(nameOverride.empty() ? camera->name : nameOverride));
    attrs.Add("posX", F::FloatToString(camera->posX));
    attrs.Add("posY", F::FloatToString(camera->posY));
    attrs.Add("posZ", F::FloatToString(camera->posZ));
    attrs.Add("angleX", F::FloatToString(camera->angleX));
    attrs.Add("angleY", F::FloatToString(camera->angleY));
    attrs.Add("angleZ", F::FloatToString(camera->angleZ));
    attrs.Add("distance", F::FloatToString(camera->distance));
    attrs.Add("zoom", F::FloatToString(camera->zoom));
    attrs.Add("panx", F::FloatToString(camera->panx));
    attrs.Add("pany", F::FloatToString(camera->pany));
    attrs.Add("panz", F::FloatToString(camera->panz));
    attrs.Add("zoom_corrx", F::FloatToString(camera->zoom_corrx));
    attrs.Add("zoom_corry", F::FloatToString(camera->zoom_corry));
    attrs.Add("is_3d", std::to_string(static_cast<int>(camera->is_3d)));
    visitor.WriteOpenTag(nodename, attrs, /*selfClose=*/true);
}

void ViewpointMgr::Save(BaseSerializingVisitor& visitor) const
{
    BaseSerializingVisitor::AttrCollector emptyAttrs;
    visitor.WriteOpenTag("Viewpoints", emptyAttrs);

    for (const auto& cam : previewCameras2d) {
        SaveCameraToVisitor(visitor, cam.get(), "Camera");
    }
    for (const auto& cam : previewCameras3d) {
        SaveCameraToVisitor(visitor, cam.get(), "Camera");
    }
    if (_defaultCamera2D) {
        SaveCameraToVisitor(visitor, _defaultCamera2D.get(), "DefaultCamera2D", "DEFAULT2D");
    }
    if (_defaultCamera3D) {
        SaveCameraToVisitor(visitor, _defaultCamera3D.get(), "DefaultCamera3D", "DEFAULT3D");
    }

    visitor.WriteCloseTag();
}

std::unique_ptr<PreviewCamera> ViewpointMgr::CreateCameraFromNode(pugi::xml_node c)
{
    std::string name = UnXmlSafe(c.attribute("name").as_string(""));
    if (name.empty()) {
        // This is so Gils early file still loads correctly
        name = c.name();
    }
    bool is_3d = c.attribute("is_3d").as_int(0) != 0;
    auto new_camera = std::make_unique<PreviewCamera>(is_3d);
    new_camera->name = name;
    new_camera->posX = c.attribute("posX").as_float(0.0f);
    new_camera->posY = c.attribute("posY").as_float(0.0f);
    new_camera->posZ = c.attribute("posZ").as_float(0.0f);
    new_camera->angleX = c.attribute("angleX").as_float(0.0f);
    new_camera->angleY = c.attribute("angleY").as_float(0.0f);
    if (!c.attribute("angleZ").empty()) {
        new_camera->angleZ = c.attribute("angleZ").as_float(0.0f);
    }
    new_camera->distance = c.attribute("distance").as_float(0.0f);
    new_camera->zoom = c.attribute("zoom").as_float(0.0f);
    new_camera->panx = c.attribute("panx").as_float(0.0f);
    new_camera->pany = c.attribute("pany").as_float(0.0f);
    new_camera->panz = c.attribute("panz").as_float(0.0f);
    new_camera->zoom_corrx = c.attribute("zoom_corrx").as_float(0.0f);
    new_camera->zoom_corry = c.attribute("zoom_corry").as_float(0.0f);

    return new_camera;
}

void ViewpointMgr::Load(pugi::xml_node vp_node)
{
	if (vp_node)
	{
        previewCameras2d.clear();
        previewCameras3d.clear();
        _defaultCamera2D.reset();
        _defaultCamera3D.reset();
        for (pugi::xml_node c = vp_node.first_child(); c; c = c.next_sibling())
        {
            std::string_view cname = c.name();
            if (cname == "Camera") {
                auto new_camera = CreateCameraFromNode(c);
                if (new_camera->is_3d) {
                    previewCameras3d.push_back(std::move(new_camera));
                }
                else {
                    previewCameras2d.push_back(std::move(new_camera));
                }
            }
            else if (cname == "DefaultCamera2D") {
                _defaultCamera2D = CreateCameraFromNode(c);
            }
            else if (cname == "DefaultCamera3D") {
                _defaultCamera3D = CreateCameraFromNode(c);
            }
        }
	}
}
