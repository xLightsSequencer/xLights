#ifndef VIEWPOINTMGR_H
#define VIEWPOINTMGR_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

class wxXmlDocument;
class wxXmlNode;

class PreviewCamera
{
public:
    friend class ViewpointMgr;

    PreviewCamera(bool is_3d_);
    virtual ~PreviewCamera();
    PreviewCamera(const PreviewCamera &cam);
    PreviewCamera& operator= (const PreviewCamera& rhs);
    glm::mat4& GetViewMatrix();

    float GetPosX() { return posX; }
    float GetPosY() { return posY; }
    float GetAngleX() { return angleX; }
    float GetAngleY() { return angleY; }
    float GetDistance() { return distance; }
    float GetZoom() { return zoom; }
    float GetPanX() { return panx; }
    float GetPanY() { return pany; }
    float GetPanZ() { return panz; }
    float GetZoomCorrX() { return zoom_corrx; }
    float GetZoomCorrY() { return zoom_corry; }
    float GetIs3D() { return is_3d; }
    const std::string& GetName() { return name; }
    const long GetMenuId() { return menu_id; }
    const long GetDeleteMenuId() { return deletemenu_id; }

    void SetPosX(float value) { posX = value; mat_valid = false; }
    void SetPosY(float value) { posY = value; mat_valid = false; }
    void SetAngleX(float value) { angleX = value; mat_valid = false; }
    void SetAngleY(float value) { angleY = value; mat_valid = false; }
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
    float angleX;
    float angleY;
    float distance;
    float zoom;
    float panx;
    float pany;
    float panz;
    float zoom_corrx;
    float zoom_corry;
    bool is_3d;
    std::string name;
    const long menu_id;
    const long deletemenu_id;
    bool mat_valid;
    glm::mat4 view_matrix;
};

class ViewpointMgr
{
public:
    ViewpointMgr();
    virtual ~ViewpointMgr();

    void Save(wxXmlDocument* doc);
    void Load(wxXmlNode* vp_node);

    void AddCamera( std::string name, PreviewCamera* current_camera, bool is_3d );
    int GetNum2DCameras() { return previewCameras2d.size(); }
    int GetNum3DCameras() { return previewCameras3d.size(); }
    PreviewCamera* GetCamera2D(int i) { return previewCameras2d[i]; }
    void DeleteCamera2D(int i);
    PreviewCamera* GetCamera3D(int i) { return previewCameras3d[i]; }
    void DeleteCamera3D(int i);
    PreviewCamera* GetNamedCamera3D(const std::string& name);
    void Clear();
    bool IsNameUnique(const std::string& name, bool is_3d);

protected:

private:
    wxXmlNode* Save() const;

    std::vector<PreviewCamera*> previewCameras3d;
    std::vector<PreviewCamera*> previewCameras2d;

};

#endif // VIEWPOINTMGR_H
