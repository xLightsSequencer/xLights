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

#include "../models/DMX/DmxMovingHeadComm.h"
#include "ViewObject.h"
#include <wx/xml/xml.h>

class DmxBeamAbility;
class DmxColorAbility;
class DmxColorAbilityCMY;
class DmxColorAbilityRGB;
class DmxColorAbilityWheel;
class DmxDimmerAbility;
class DmxImage;
class DmxFloodArea;
class DmxFloodlight;
class DmxGeneral;
class DmxMotor;
class DmxPresetAbility;
class DmxShutterAbility;
class Mesh;
class Servo;

struct XmlSerializingVisitor : BaseObjectVisitor {
    XmlSerializingVisitor(wxXmlNode* parentNode) :
        parentNode(parentNode) {
    }

    void Visit(const ArchesModel& model) override;
    void Visit(const CandyCaneModel& model) override;
    void Visit(const CircleModel& model) override;
    void Visit(const ChannelBlockModel& model) override;
    void Visit(const CubeModel& model) override;
    void Visit(const CustomModel& model) override;
    void Visit(const IciclesModel& model) override;
    void Visit(const ImageModel& model) override;
    void Visit(const MatrixModel& model) override;
    void Visit(const MultiPointModel& model) override;
    void Visit(const SingleLineModel& model) override;
    void Visit(const PolyLineModel& model) override;
    void Visit(const SphereModel& model) override;
    void Visit(const SpinnerModel& model) override;
    void Visit(const StarModel& model) override;
    void Visit(const TreeModel& model) override;
    void Visit(const WindowFrameModel& model) override;
    void Visit(const WreathModel& model) override;
    void Visit(const DmxFloodArea& model) override;
    void Visit(const DmxFloodlight& model) override;
    void Visit(const DmxGeneral& model) override;
    void Visit(const DmxMovingHeadAdv& model) override;
    void Visit(const DmxMovingHead& model) override;
    void Visit(const DmxServo& model) override;
    void Visit(const DmxServo3d& model) override;
    void Visit(const DmxSkull& model) override;

    void Visit(const GridlinesObject& object) override;
    void Visit(const ImageObject& object) override;
    void Visit(const MeshObject& object) override;
    void Visit(const TerrainObject& object) override;
    void Visit(const RulerObject& object) override;

private:
    wxXmlNode* parentNode;

    void SortAttributes(wxXmlNode* input);

    void AddBaseObjectAttributes(const BaseObject& base, wxXmlNode* node);
    void AddCommonModelAttributes(const Model& model, wxXmlNode* node);

    void AddModelScreenLocationAttributes(const BaseObject& base, wxXmlNode* node);
    void AddTwoPointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node);
    void AddThreePointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node);
    void AddPolyPointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node);
    void AddMultiPointScreenLocationAttributes(const BaseObject& base, wxXmlNode* node);

    void AddDmxMotorAttributes(const DmxMotor* motor, wxXmlNode* node);
    void AddMeshAttributes(const Mesh* mesh, wxXmlNode* node);
    void AddServoAttributes(const Servo* servo, wxXmlNode* node);
    void AddDmxImageAttributes(const DmxImage* img, wxXmlNode* node);
    void AddDmxMovingHeadCommAttributes(const DmxMovingHeadComm& model, wxXmlNode* node);
    void AddDmxModelAttributes(const DmxModel& dmx_model, wxXmlNode* node);
    
    void AddColorAbilityAttributes(const DmxColorAbility* color_ability, wxXmlNode* node);
    void AddColorAbilityRGBAttributes(const DmxColorAbilityRGB* colors, wxXmlNode* node);
    void AddColorWheelAttributes(const DmxColorAbilityWheel* colors, wxXmlNode* node);
    void AddColorAbilityCMYAttributes(const DmxColorAbilityCMY* colors, wxXmlNode* node);
    void AddBeamAbilityAttributes(const DmxBeamAbility* beam, wxXmlNode* node);
    void AddPresetAbilityAttributes(const DmxPresetAbility* presets, wxXmlNode* node);
    void AddShutterAbilityAttributes(const DmxShutterAbility* shutter, wxXmlNode* node);
    void AddDimmerAbilityAttributes(const DmxDimmerAbility* dimmer, wxXmlNode* node);
    void AddCustomModel(wxXmlNode* xmlNode, const CustomModel& m);
    void AddFacesandStates(wxXmlNode* node, const Model* m);
    void AddAliases(wxXmlNode* node, const std::list<std::string>& aliases);
    void AddDimmingCurve(wxXmlNode* node, const Model* m);
    void AddSubmodels(wxXmlNode* node, const Model* m);
    void AddGroups(wxXmlNode* node, const Model* m);
    void AddControllerConnection(wxXmlNode* node, const Model* m);
    void AddDimensions(wxXmlNode* node, const Model* m);
    void AddSuperStrings(Model const& model, wxXmlNode* node);
    void AddOtherElements(wxXmlNode* xmlNode, const Model* m);

    [[nodiscard]] wxXmlNode* CommonVisitSteps(const Model& model);
    [[nodiscard]] wxXmlNode* CommonObjectVisitSteps(const ViewObject& object);
};
