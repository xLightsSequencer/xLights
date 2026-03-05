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
#include "../../graphics/xlGraphicsAccumulators.h"
#include <glm/glm.hpp>

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;

class ModelPreview;

class DmxImage
{
public:
    DmxImage(std::string _name);
    virtual ~DmxImage();

    const float OFFSET_SCALE = 100.0f;

    void Init(BaseObject* base);
    bool GetExists() { return obj_exists; }

    void AddTypeProperties(wxPropertyGridInterface* grid);
    void UpdateTypeProperties(wxPropertyGridInterface* grid) {}

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

    void Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *pg,
              glm::mat4 &motion_matrix,
              int transparency, float brightness, bool only_image,
              float pivot_offset_x, float pivot_offset_y, bool rotation, bool use_pivot);

    bool ImageSelected() const { return image_selected; }
    void ClearImageSelected() { image_selected = false; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    std::string GetName() const { return base_name; }
    std::string GetImageFile() const { return _imageFile; }
    float GetScaleX() const { return scalex; }
    float GetScaleY() const { return scaley; }
    float GetScaleZ() const { return scalez; }
    float GetRotateX() const { return rotatex; }
    float GetRotateY() const { return rotatey; }
    float GetRotateZ() const { return rotatez; }
    float GetOffsetX() const { return offset_x; }
    float GetOffsetY() const { return offset_y; }
    float GetOffsetZ() const { return offset_z; }

    void SetImageFile(const std::string& file) { _imageFile = file; }
    void SetScaleX(float val) { scalex = val; }
    void SetScaleY(float val) { scaley = val; }
    void SetScaleZ(float val) { scalez = val; }
    void SetRotateX(float val) { rotatex = val; }
    void SetRotateY(float val) { rotatey = val; }
    void SetRotateZ(float val) { rotatez = val; }
    void SetOffsetX(float val) { offset_x = val / OFFSET_SCALE; }
    void SetOffsetY(float val) { offset_y = val / OFFSET_SCALE; }
    void SetOffsetZ(float val) { offset_z = val / OFFSET_SCALE; }

protected:

private:
    wxXmlNode* node_xml;
    std::string _imageFile {""};
    int width {1};
    int height {1};
    std::map<std::string, xlTexture*> _images;
    bool obj_exists {false};
    bool image_selected {false};

    float offset_x {0.0f};
    float offset_y {0.0f};
    float offset_z {0.0f};
    float scalex {1.0f};
    float scaley {1.0f};
    float scalez {1.0f};
    float rotatex {0.0f};
    float rotatey {0.0f};
    float rotatez {0.0f};
    wxString base_name;
};

