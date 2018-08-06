#ifndef VIEWPOINTMGR_H
#define VIEWPOINTMGR_H

#include <string>
#include <vector>

class wxXmlDocument;
class wxXmlNode;

class PreviewCamera
{
public:
    PreviewCamera(bool is_3d_);
    virtual ~PreviewCamera();
    PreviewCamera(const PreviewCamera &cam);
    PreviewCamera& operator= (const PreviewCamera& rhs);

    float posX;
    float posY;
    float angleX;
    float angleY;
    float distance;
    float zoom;
    float panx;
    float pany;
    float zoom_corrx;
    float zoom_corry;
    bool is_3d;
    std::string name;
    const long menu_id;
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
    PreviewCamera* GetCamera3D(int i) { return previewCameras3d[i]; }

protected:

private:
    wxXmlNode* Save() const;

    std::vector<PreviewCamera*> previewCameras3d;
    std::vector<PreviewCamera*> previewCameras2d;

};

#endif // VIEWPOINTMGR_H
