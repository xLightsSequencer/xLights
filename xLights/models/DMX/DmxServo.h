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

#include "DmxModel.h"

class DmxImage;
class Servo;

class DmxServo : public DmxModel {
public:
    DmxServo(const ModelManager &manager);
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

    Servo* GetAxis(int num) { return num < num_servos ? servos[num].get() : servos[0].get(); }
    Servo* GetServo(int num) const { return num < num_servos ? servos[num].get() : servos[0].get(); }
    DmxImage* GetStaticImage(int num) const { return num < num_servos ? static_images[num].get() : static_images[0].get(); }
    DmxImage* GetMotionImage(int num) const { return num < num_servos ? motion_images[num].get() : motion_images[0].get(); }

    int GetNumServos() const { return num_servos; }
    bool Is16Bit() const { return _16bit; }
    float GetBrightness() const { return brightness; }
    int GetTransparency() const { return transparency; }

    void SetNumServos(int val);
    void SetIs16Bit(bool val) { _16bit = val; }
    void SetBrightness(float val) {brightness = val; }
    void SetTransparency(int val) {transparency = val; }

    DmxImage* CreateStaticImage(const std::string& name, int idx);
    DmxImage* CreateMotionImage(const std::string& name, int idx);
    Servo* CreateServo(const std::string& name, int idx);

    void GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const override;

    void Accept(BaseObjectVisitor &visitor) const override { return visitor.Visit(*this); }

protected:
    virtual void InitModel() override;
    void Clear();

    void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *program, const xlColor* c, bool active);

    int transparency {0};
    float brightness {100};

private:
    bool update_node_names {false};
    int num_servos {1};
    bool _16bit {true};
    std::vector<std::unique_ptr<DmxImage>> static_images;
    std::vector<std::unique_ptr<DmxImage>> motion_images;
    std::vector<std::unique_ptr<Servo>> servos;
};
