#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxModel.h"

class DmxImage;
class Servo;

class DmxServo : public DmxModel {
public:
    DmxServo(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
    virtual ~DmxServo();

    virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                      const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                      bool highlightFirst = false, int highlightpixel = 0,
                                      float *boundingBox = nullptr) override;
    virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual std::list<std::string> CheckModelSettings() override;

    Servo* GetAxis(int num) { return num < num_servos ? servos[num] : servos[0]; }
    int GetNumServos() { return num_servos; }

protected:
    virtual void InitModel() override;
    void Clear();

    void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *program, const xlColor* c, bool active);
    virtual void ExportXlightsModel() override;
    virtual void ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

    int transparency;
    float brightness;

private:
    bool update_node_names;
    int num_servos;
    bool _16bit;
    std::vector<DmxImage*> static_images;
    std::vector<DmxImage*> motion_images;
    std::vector<Servo*> servos;
};
