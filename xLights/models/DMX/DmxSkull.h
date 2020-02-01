#ifndef DMXSKULL_H
#define DMXSKULL_H

#include "DmxModel.h"
#include "DmxColorAbility.h"
#include "Servo.h"

class Mesh;

class DmxSkull : public DmxModel, public DmxColorAbility
{
    public:
        DmxSkull(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
        virtual ~DmxSkull();

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active) override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

        virtual bool HasColorAbility() override { return true; }
        bool Is16Bit() const { return _16bit; }
        bool IsSkulltronix() const { return is_skulltronix; }
        bool HasJaw() const { return has_jaw; }
        bool HasPan() const { return has_pan; }
        bool HasTilt() const { return has_tilt; }
        bool HasNod() const { return has_nod; }
        bool HasEyeLR() const { return has_eye_lr; }
        bool HasEyeUD() const { return has_eye_ud; }
        bool HasColor() const { return has_color; }
        void SetSkulltronix() { setup_skulltronix = true; }

        int GetEyeBrightnessChannel() { return eye_brightness_channel; }
        int GetPanChannel() { return pan_servo->GetChannel(); }
        int GetPanMinLimit() { return pan_servo->GetMinLimit(); }
        int GetPanMaxLimit() { return pan_servo->GetMaxLimit(); }
        int GetTiltChannel() { return tilt_servo->GetChannel(); }
        int GetTiltMinLimit() { return tilt_servo->GetMinLimit(); }
        int GetTiltMaxLimit() { return tilt_servo->GetMaxLimit(); }
        int GetNodChannel() { return nod_servo->GetChannel(); }
        int GetNodMinLimit() { return nod_servo->GetMinLimit(); }
        int GetNodMaxLimit() { return nod_servo->GetMaxLimit(); }
        int GetJawChannel() { return jaw_servo->GetChannel(); }
        int GetJawMinLimit() { return jaw_servo->GetMinLimit(); }
        int GetJawMaxLimit() { return jaw_servo->GetMaxLimit(); }
        int GetEyeUDChannel() { return eye_ud_servo->GetChannel(); }
        int GetEyeUDMinLimit() { return eye_ud_servo->GetMinLimit(); }
        int GetEyeUDMaxLimit() { return eye_ud_servo->GetMaxLimit(); }
        int GetEyeLRChannel() { return eye_lr_servo->GetChannel(); }
        int GetEyeLRMinLimit() { return eye_lr_servo->GetMinLimit(); }
        int GetEyeLRMaxLimit() { return eye_lr_servo->GetMaxLimit(); }

    protected:
        enum SERVO_TYPE {
            JAW,
            PAN,
            TILT,
            NOD,
            EYE_UD,
            EYE_LR
        };

        virtual void InitModel() override;
        void DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va2, DrawGLUtils::xl3Accumulator& va3, const xlColor* c, float& sx, float& sy, bool active, bool is_3d);

        void FixObjFile(wxXmlNode* node, const std::string& objfile);
        void AddServo(Servo** _servo, const std::string& name, int type, const std::string& style);
        void AddMesh(Mesh** _mesh, const std::string& name, const std::string& objfile, bool set_size);
        float GetServoPos(Servo* _servo, bool active);

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        Mesh* head_mesh = nullptr;
        Mesh* jaw_mesh = nullptr;
        Mesh* eye_l_mesh = nullptr;
        Mesh* eye_r_mesh = nullptr;
        Servo* pan_servo = nullptr;
        Servo* tilt_servo = nullptr;
        Servo* nod_servo = nullptr;
        Servo* jaw_servo = nullptr;
        Servo* eye_ud_servo = nullptr;
        Servo* eye_lr_servo = nullptr;
        bool _16bit = true;
        wxString obj_path;
        int eye_brightness_channel;
        int default_channels[6];
        int default_min_limit[6];
        int default_max_limit[6];
        float default_orient[6];
        int pan_orient;
        int tilt_orient;
        int nod_orient;
        int eye_ud_orient;
        int eye_lr_orient;
        bool has_jaw = true;
        bool has_pan = true;
        bool has_tilt = true;
        bool has_nod = true;
        bool has_eye_ud = true;
        bool has_eye_lr = true;
        bool has_color = true;
        bool is_skulltronix = false;
        bool setup_skulltronix = false;
        bool mesh_only = false;
        float default_range_of_motion[6];
        std::string default_node_names;

    private:
        void SetupSkulltronix();
        void SetupServo(Servo* _servo, int channel, float min_limit, float max_limit, float range_of_motion, bool _16bit);
};
#endif // DMXSKULL_H
