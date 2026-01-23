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

class Mesh;
class Servo;

class DmxServo3d : public DmxModel
{
public:
    DmxServo3d(const ModelManager& manager);
    virtual ~DmxServo3d();

    virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                      const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                      bool highlightFirst = false, int highlightpixel = 0,
                                      float *boundingBox = nullptr) override;
    virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual std::list<std::string> CheckModelSettings() override;

    const Servo* GetAxis(int num) { return num < num_servos ? servos[num].get() : servos[0].get(); }
    int GetNumServos() const { return num_servos; }
    int GetNumStatic() const { return num_static; }
    int GetNumMotion() const { return num_motion; }
    void UpdateNodeNames() { update_node_names = true; }
    void UpdateBits() { update_bits = true; }
    bool Is16Bit() const { return _16bit; }

    void SetNumServos(int val);
    void SetNumStatic(int val);
    void SetNumMotion(int val);
    void SetIs16Bit(bool val) { _16bit = val; }
    void SetBrightness(float val) {brightness = val; }

    Mesh* CreateStaticMesh(const std::string& name, int idx);
    Mesh* CreateMotionMesh(const std::string& name, int idx);
    Servo* CreateServo(const std::string& name, int idx);
    void SetMeshLink(int idx, int val) { mesh_links[idx] = val; }
    void SetServoLink(int idx, int val) { servo_links[idx] = val; }

    void GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const override;

protected:
    virtual void InitModel() override;
    void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool active);

    float brightness = 100.0f;

private:
    static const int SUPPORTED_SERVOS = 24;

    bool update_node_names = false;
    bool update_bits = false;
    int num_servos = 1;
    int num_static = 1;
    int num_motion = 1;
    bool _16bit = true;
    bool show_pivot = false;
    std::vector<std::unique_ptr<Mesh>> static_meshs;
    std::vector<std::unique_ptr<Mesh>> motion_meshs;
    std::vector<std::unique_ptr<Servo>> servos;
    int servo_links[SUPPORTED_SERVOS];
    int mesh_links[SUPPORTED_SERVOS];
};

