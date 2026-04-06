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

#include "XmlNodeKeys.h"
#include <pugixml.hpp>

class DmxModel;
class DmxMovingHeadComm;;
class DmxColorAbilityCMY;
class DmxColorAbilityRGB;
class DmxColorAbilityWheel;
class DmxImage;
class DmxMotor;
class Mesh;
class Model;
class ModelManager;
class Servo;

struct XmlDeserializingModelFactory {
    Model* Deserialize(pugi::xml_node node, ModelManager& modelManager, bool importing);

private:

    void CommonDeserializeSteps(Model* model, pugi::xml_node node, ModelManager& modelManager, bool importing);
    void DeserializeControllerConnection(Model* model, pugi::xml_node node);
    void DeserializeBaseObjectAttributes(Model* model, pugi::xml_node node, ModelManager& modelManager, bool importing);
    void DeserializeCommonModelAttributes(Model* model, pugi::xml_node node, ModelManager& modelManager, bool importing);
    void DeserializeCommonModelChildElements(Model* model, pugi::xml_node node, ModelManager& modelManager, bool importing);
    void DeserializeSubModel(Model* model, pugi::xml_node node);
    void DeserializeAliases(Model* model, pugi::xml_node node);
    void DeserializeSuperStrings(Model* model, pugi::xml_node node);
    void DeserializeDimmingCurve(Model* model, pugi::xml_node node);

    Model* DeserializeArches(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeCandyCane(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeChannelBlock(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeCircle(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeCube(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeCustom(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeIcicles(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeImage(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeMatrix(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeMultiPoint(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeSingleLine(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializePolyLine(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeSphere(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeSpinner(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeStar(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeTree(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeWindow(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeWreath(pugi::xml_node node, ModelManager& modelManager, bool importing);

    // Model Groups
    Model* DeserializeModelGroup(pugi::xml_node node, ModelManager& modelManager, bool importing);

    // ************************************************************************************************************
    // **********************                        DMX Section                           ************************
    // ************************************************************************************************************
    void DeserializeDmxModel(DmxModel* dmx_model, pugi::xml_node node);
    void DeserializeBeamAbility(DmxModel* model, pugi::xml_node node);
    void DeserializePresetAbility(DmxModel* model, pugi::xml_node node);
    void DeserializeDynamicColorAbility(DmxModel* model, pugi::xml_node node);
    void DeserializeColorAbility(DmxModel* model, pugi::xml_node node);
    void DeserializeColorAbilityRGBAttributes(DmxColorAbilityRGB* ability, pugi::xml_node node);
    void DeserializeColorWheelAttributes(DmxColorAbilityWheel* ability, pugi::xml_node node);
    void DeserializeColorAbilityCMYAttributes(DmxColorAbilityCMY* ability, pugi::xml_node node);
    void DeserializeShutterAbility(DmxModel* model, pugi::xml_node node);
    void DeserializeDimmerAbility(DmxModel* model, pugi::xml_node node);
    void DeserializeDmxImage(DmxImage* img, pugi::xml_node node);
    void DeserializeDmxMotor(DmxMotor* motor, pugi::xml_node node);
    void DeserializeMesh(Mesh* mesh, pugi::xml_node node);
    void DeserializeServo(Servo* servo, pugi::xml_node node);
    void DeserializeDmxMovingHeadComm(DmxMovingHeadComm* model, pugi::xml_node node);

    Model* DeserializeDmxGeneral(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeDmxFloodArea(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeDmxFloodlight(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeDmxMovingHead(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeDmxMovingHeadAdv(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeDmxServo3d(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeDmxServo(pugi::xml_node node, ModelManager& modelManager, bool importing);
    Model* DeserializeDmxSkull(pugi::xml_node node, ModelManager& modelManager, bool importing);
};
