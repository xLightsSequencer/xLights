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

// BaseSerializingVisitor is an intermediate abstract base class sitting between
// BaseObjectVisitor and the two concrete serializers (StringSerializingVisitor
// and XmlSerializingVisitor).  It owns all shared Visit() implementations,
// attribute-collecting helpers, and child-element writers.  Concrete subclasses
// only need to override the two output primitives WriteOpenTag / WriteCloseTag.

#include "../models/BaseObjectVisitor.h"
#include "../models/DMX/DmxMovingHeadComm.h"
#include "../models/ViewObject.h"

#include <string>
#include <vector>
#include <utility>
#include <list>
#include <initializer_list>

class ControllerConnection;
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
class ModelGroup;
class Servo;

struct BaseSerializingVisitor : BaseObjectVisitor {

    // ---------------------------------------------------------------------------
    // Collected attributes for a pending element.
    // Public so that external helpers (StringSerializer, TabSequence, etc.) can
    // construct instances.
    // ---------------------------------------------------------------------------
    struct AttrCollector {
        std::vector<std::pair<std::string, std::string>> attrs;

        AttrCollector() = default;
        AttrCollector(std::initializer_list<std::pair<std::string, std::string>> init)
            : attrs(init) {}

        void Add(const char* name, const std::string& value) {
            attrs.emplace_back(name, value);
        }
        void Add(const std::string& name, const std::string& value) {
            attrs.emplace_back(name, value);
        }
    };

    // ---------------------------------------------------------------------------
    // Output primitives — must be overridden by concrete subclasses.
    // WriteOpenTag with selfClose=true writes a self-closing element (<foo/>).
    // WriteOpenTag with selfClose=false opens an element; a matching
    // WriteCloseTag must follow.
    // ---------------------------------------------------------------------------
    virtual void WriteOpenTag(const std::string& name, const AttrCollector& attrs,
                              bool selfClose = false) = 0;
    virtual void WriteOpenTag(const std::string& name) = 0;
    virtual void WriteCloseTag(const std::string& name) = 0;
    virtual void WriteBodyText(const std::string& txt) = 0;

    // ---------------------------------------------------------------------------
    // Concrete Visit() overrides — implemented once in BaseSerializingVisitor.cpp
    // ---------------------------------------------------------------------------
    void Visit(const ArchesModel& model) override;
    void Visit(const CandyCaneModel& model) override;
    void Visit(const CircleModel& model) override;
    void Visit(const ChannelBlockModel& model) override;
    void Visit(const CubeModel& model) override;
    void Visit(const CustomModel& model) override;
    void Visit(const IciclesModel& model) override;
    void Visit(const ImageModel& model) override;
    void Visit(const MatrixModel& model) override;
    void Visit(const ModelGroup& model) override;
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
    void Visit(const ControllerConnection& cc) override;

    // Shared utilities
    static std::string FloatToString(float f);
protected:
    explicit BaseSerializingVisitor(bool exporting = false) : forExport(exporting) {}
    ~BaseSerializingVisitor() override = default;

    bool forExport;

    // Attribute sorting — sorts attrs in-place using the same priority rules
    // as the original visitors: "name" first, "DisplayAs" second, "CustomModel"
    // last, remainder case-insensitive alphabetical.
    static void SortAttributes(AttrCollector& collector);

    // ---------------------------------------------------------------------------
    // Attribute-collecting helpers (take AttrCollector& instead of wxXmlNode*)
    // ---------------------------------------------------------------------------
    void AddBaseObjectAttributes(const BaseObject& base, AttrCollector& attrs);
    void AddCommonModelAttributes(const Model& model, AttrCollector& attrs);
    void AddSuperStrings(const Model& model, AttrCollector& attrs);

    void AddModelScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs);
    void AddTwoPointScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs);
    void AddThreePointScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs);
    void AddPolyPointScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs);
    void AddMultiPointScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs);
    void AddBoxedScreenLocationAttributes(const BaseObject& base, AttrCollector& attrs);

    void AddColorAbilityAttributes(const DmxColorAbility* color_ability, AttrCollector& attrs);
    void AddColorAbilityRGBAttributes(const DmxColorAbilityRGB* colors, AttrCollector& attrs);
    void AddColorWheelAttributes(const DmxColorAbilityWheel* colors, AttrCollector& attrs);
    void AddColorAbilityCMYAttributes(const DmxColorAbilityCMY* colors, AttrCollector& attrs);
    void AddBeamAbilityAttributes(const DmxBeamAbility* beam, AttrCollector& attrs);
    void AddPresetAbilityAttributes(const DmxPresetAbility* presets, AttrCollector& attrs);
    void AddShutterAbilityAttributes(const DmxShutterAbility* shutter, AttrCollector& attrs);
    void AddDimmerAbilityAttributes(const DmxDimmerAbility* dimmer, AttrCollector& attrs);
    void AddDmxModelAttributes(const DmxModel& dmx_model, AttrCollector& attrs);
    // Note: AddDmxMovingHeadCommAttributes collects ONLY parent-level attributes.
    // Motor child elements must be written separately in the Visit method.
    void AddDmxMovingHeadCommAttributes(const DmxMovingHeadComm& model, AttrCollector& attrs);

    // ---------------------------------------------------------------------------
    // Child-element writers (use WriteOpenTag / WriteCloseTag)
    // ---------------------------------------------------------------------------
    void WriteDmxMotorElement(const DmxMotor* motor);
    void WriteMeshElement(const Mesh* mesh);
    void WriteServoElement(const Servo* servo);
    void WriteDmxImageElement(const DmxImage* img);
    void WriteFacesAndStates(const Model* m);
    void WriteControllerConnectionElement(const Model* m);
    void WriteDimmingCurve(const Model* m);
    void WriteAliases(const std::list<std::string>& aliases);
    void WriteSubmodels(const Model* m);
    void WriteOtherElements(const Model* m);

    // Common preamble helpers
    void CommonVisitSteps(const Model& model, AttrCollector& attrs);
    void CommonObjectVisitSteps(const ViewObject& object, AttrCollector& attrs);
};
