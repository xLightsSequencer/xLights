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

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <pugixml.hpp>

#include "XmlSerializer/BaseSerializingVisitor.h"

class ViewpointMgr;

class PreviewCamera
{
public:
    friend class ViewpointMgr;

    void Reset();

    PreviewCamera(bool is_3d_);
    virtual ~PreviewCamera();
    PreviewCamera(const PreviewCamera &cam);
    PreviewCamera& operator= (const PreviewCamera& rhs);
    glm::mat4& GetViewMatrix();

    float GetPosX() const { return posX; }
    float GetPosY() const { return posY; }
    float GetPosZ() const { return posZ; }
    float GetAngleX() const { return angleX; }
    float GetAngleY() const { return angleY; }
    float GetAngleZ() const { return angleZ; }
    float GetDistance() const { return distance; }
    float GetZoom() const { return zoom; }
    float GetPanX() const { return panx; }
    float GetPanY() const { return pany; }
    float GetPanZ() const { return panz; }
    float GetZoomCorrX() const { return zoom_corrx; }
    float GetZoomCorrY() const { return zoom_corry; }
    float GetIs3D() const { return is_3d; }
    const std::string& GetName() const { return name; }

    void SetPosX(float value) { posX = value; mat_valid = false; }
    void SetPosY(float value) { posY = value; mat_valid = false; }
    void SetAngleX(float value);
    void SetAngleY(float value);
    void SetAngleZ(float value);
    void SetDistance(float value) { distance = value; mat_valid = false; }
    void SetZoom(float value) { zoom = value; mat_valid = false; }
    void SetPanX(float value) { panx = value; mat_valid = false; }
    void SetPanY(float value) { pany = value; mat_valid = false; }
    void SetPanZ(float value) { panz = value; mat_valid = false; }
    void SetZoomCorrX(float value) { zoom_corrx = value; mat_valid = false; }
    void SetZoomCorrY(float value) { zoom_corry = value; mat_valid = false; }
    void SetIs3D(bool value) { is_3d = value; mat_valid = false; }
    void SetName(std::string name_) { name = name_; mat_valid = false; }

protected:
    float posX;
    float posY;
    float posZ;
    float angleX;
    float angleY;
    float angleZ;
    float distance;
    float zoom;
    float panx;
    float pany;
    float panz;
    float zoom_corrx;
    float zoom_corry;
    bool is_3d;
    std::string name;
    bool mat_valid;
    glm::mat4 view_matrix;
};

class ViewpointMgr
{
public:
    ViewpointMgr();
    virtual ~ViewpointMgr();

    void Save(BaseSerializingVisitor& visitor) const;
    void Load(pugi::xml_node vp_node);

    void SetDefaultCamera2D(PreviewCamera* current_camera) {
        _defaultCamera2D = std::make_unique<PreviewCamera>(*current_camera);
    }
    void SetDefaultCamera3D(PreviewCamera* current_camera) {
        _defaultCamera3D = std::make_unique<PreviewCamera>(*current_camera);
    }
    void ClearDefault3DCamera() { _defaultCamera3D.reset(); }
    void AddCamera( std::string name, PreviewCamera* current_camera, bool is_3d );
    int GetNum2DCameras() const { return static_cast<int>(previewCameras2d.size()); }
    int GetNum3DCameras() const { return static_cast<int>(previewCameras3d.size()); }
    PreviewCamera* GetCamera2D(int i) { return previewCameras2d[i].get(); }
    PreviewCamera* GetDefaultCamera2D() { return _defaultCamera2D.get(); }
    PreviewCamera* GetDefaultCamera3D() { return _defaultCamera3D.get(); }
    void DeleteCamera2D(int i);
    PreviewCamera* GetCamera3D(int i) { return previewCameras3d[i].get(); }
    void DeleteCamera3D(int i);
    PreviewCamera* GetNamedCamera3D(const std::string& name);
    void Clear();
    bool IsNameUnique(const std::string& name, bool is_3d);

private:
    std::unique_ptr<PreviewCamera> CreateCameraFromNode(pugi::xml_node node);
    void SaveCameraToVisitor(BaseSerializingVisitor& visitor, const PreviewCamera* camera,
                             const std::string& nodename, const std::string& nameOverride = "") const;

    std::vector<std::unique_ptr<PreviewCamera>> previewCameras3d;
    std::vector<std::unique_ptr<PreviewCamera>> previewCameras2d;
    std::unique_ptr<PreviewCamera> _defaultCamera2D;
    std::unique_ptr<PreviewCamera> _defaultCamera3D;
};
