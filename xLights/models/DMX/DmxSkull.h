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
#include "Servo.h"
#include "Mesh.h"

class DmxSkull : public DmxModel
{
public:
    DmxSkull(const ModelManager& manager);
    virtual ~DmxSkull();

    enum SERVO_TYPE {
        JAW,
        PAN,
        TILT,
        NOD,
        EYE_UD,
        EYE_LR
    };

    virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                  xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                  const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                  bool highlightFirst = false, int highlightpixel = 0,
                                  float *boundingBox = nullptr) override;
    virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;
    void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool is3d, bool active, const xlColor *c);


    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual void DisableUnusedProperties(wxPropertyGridInterface* grid) override;

    bool Is16Bit() const { return _16bit; }
    bool HasJaw() const { return has_jaw; }
    bool HasPan() const { return has_pan; }
    bool HasTilt() const { return has_tilt; }
    bool HasNod() const { return has_nod; }
    bool HasEyeLR() const { return has_eye_lr; }
    bool HasEyeUD() const { return has_eye_ud; }
    bool HasColor() const { return has_color; }
    void SetSkulltronix() { setup_skulltronix = true; }

    Servo* CreateServo(const std::string& name);
    Mesh* CreateMesh(const std::string& name, bool add_path = false);

    int GetEyeBrightnessChannel() { return eye_brightness_channel; }
    int GetPanChannel() const
    {
        return pan_servo == nullptr ? 0 : pan_servo->GetChannel();
    }
    int GetPanMinLimit() const
    {
        return pan_servo == nullptr ? 0 : pan_servo->GetMinLimit();
    }
    int GetPanMaxLimit() const
    {
        return pan_servo == nullptr ? 0 : pan_servo->GetMaxLimit();
    }
    int GetTiltChannel() const
    {
        return tilt_servo == nullptr ? 0 : tilt_servo->GetChannel();
    }
    int GetTiltMinLimit() const
    {
        return tilt_servo == nullptr ? 0 : tilt_servo->GetMinLimit();
    }
    int GetTiltMaxLimit() const
    {
        return tilt_servo == nullptr ? 0 : tilt_servo->GetMaxLimit();
    }
    int GetNodChannel() const
    {
        return nod_servo == nullptr ? 0 : nod_servo->GetChannel();
    }
    int GetNodMinLimit() const
    {
        return nod_servo == nullptr ? 0 : nod_servo->GetMinLimit();
    }
    int GetNodMaxLimit() const
    {
        return nod_servo == nullptr ? 0 : nod_servo->GetMaxLimit();
    }
    int GetJawChannel() const
    {
        return jaw_servo == nullptr ? 0 : jaw_servo->GetChannel();
    }
    int GetJawMinLimit() const
    {
        return jaw_servo == nullptr ? 0 : jaw_servo->GetMinLimit();
    }
    int GetJawMaxLimit() const
    {
        return jaw_servo == nullptr ? 0 : jaw_servo->GetMaxLimit();
    }
    int GetEyeUDChannel() const
    {
        return eye_ud_servo == nullptr ? 0 : eye_ud_servo->GetChannel();
    }
    int GetEyeUDMinLimit() const
    {
        return eye_ud_servo == nullptr ? 0 : eye_ud_servo->GetMinLimit();
    }
    int GetEyeUDMaxLimit() const
    {
        return eye_ud_servo == nullptr ? 0 : eye_ud_servo->GetMaxLimit();
    }
    int GetEyeLRChannel() const
    {
        return eye_lr_servo == nullptr ? 0 : eye_lr_servo->GetChannel();
    }
    int GetEyeLRMinLimit() const
    {
        return eye_lr_servo == nullptr ? 0 : eye_lr_servo->GetMinLimit();
    }
    int GetEyeLRMaxLimit() const
    {
        return eye_lr_servo == nullptr ? 0 : eye_lr_servo->GetMaxLimit();
    }

    int GetDefaultOrient(SERVO_TYPE type) const { return default_orient[type]; }
    void SetEyeBrightnessChannel(int val) { eye_brightness_channel = val; }
    void SetJawOrient(int val) { jaw_orient = val; }
    void SetPanOrient(int val) { pan_orient = val; }
    void SetTiltOrient(int val) { tilt_orient = val; }
    void SetNodOrient(int val) { nod_orient = val; }
    void SetEyeUDOrient(int val) { eye_ud_orient = val; }
    void SetEyeLROrient(int val) { eye_lr_orient = val; }
    void SetHasJaw(bool val) {has_jaw = val; }
    void SetHasPan(bool val) {has_pan = val; }
    void SetHasTilt(bool val) {has_tilt = val; }
    void SetHasNod(bool val) {has_nod = val; }
    void SetHasEyeUD(bool val) {has_eye_ud = val; }
    void SetHasEyeLR(bool val) {has_eye_lr = val; }
    void SetHasColor(bool val) {has_color = val; }
    void SetIs16Bit(bool val) { _16bit = val; }
    void SetMeshOnly(bool val) { mesh_only = val; }

    virtual void GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const override;

    virtual std::vector<std::string> GenerateNodeNames() const override;
protected:
    virtual void InitModel() override;

    float GetServoPos(Servo* _servo, bool active);

    virtual std::list<std::string> CheckModelSettings() override;

    std::unique_ptr<Mesh> head_mesh = nullptr;
    std::unique_ptr<Mesh> jaw_mesh = nullptr;
    std::unique_ptr<Mesh> eye_l_mesh = nullptr;
    std::unique_ptr<Mesh> eye_r_mesh = nullptr;
    std::unique_ptr<Servo> pan_servo = nullptr;
    std::unique_ptr<Servo> tilt_servo = nullptr;
    std::unique_ptr<Servo> nod_servo = nullptr;
    std::unique_ptr<Servo> jaw_servo = nullptr;
    std::unique_ptr<Servo> eye_ud_servo = nullptr;
    std::unique_ptr<Servo> eye_lr_servo = nullptr;
    bool _16bit = true;
    wxString obj_path = "";
    int eye_brightness_channel {0};
    int default_channels[6];
    int default_min_limit[6];
    int default_max_limit[6];
    float default_orient[6];
    int jaw_orient;
    int pan_orient;
    int tilt_orient;
    int nod_orient;
    int eye_ud_orient;
    int eye_lr_orient;
    bool has_jaw {true};
    bool has_pan {true};
    bool has_tilt{true};
    bool has_nod{true};
    bool has_eye_ud{true};
    bool has_eye_lr{true};
    bool has_color{true};
    bool setup_skulltronix {false};
    bool mesh_only {false};
    float default_range_of_motion[6];
    std::string default_node_names;

private:
    void SetupSkulltronix();
    void SetupServo(Servo* _servo, int channel, float min_limit, float max_limit, float range_of_motion, bool _16bit);
    std::unique_ptr<Servo> CreateServo(const std::string& name, int type, const std::string& style);
    std::unique_ptr<Mesh> CreateMesh(const std::string& name, const std::string& objfile);
};

