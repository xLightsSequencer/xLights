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
#include <wx/xml/xml.h>

class DmxModel;
class DmxMovingHeadComm;;
class DmxColorAbilityCMY;
class DmxColorAbilityRGB;
class DmxColorAbilityWheel;
class DmxMotor;
class Mesh;
class Model;
class Servo;

struct XmlDeserializingModelFactory {
    Model* Deserialize(wxXmlNode* node, xLightsFrame* xlights, bool importing);

private:

    void CommonDeserializeSteps(Model* model, wxXmlNode* node, xLightsFrame* xlights, bool importing);
    void DeserializeControllerConnection(Model* model, wxXmlNode* node);
    void DeserializeBaseObjectAttributes(Model* model, wxXmlNode* node, xLightsFrame* xlights, bool importing);
    void DeserializeCommonModelAttributes(Model* model, wxXmlNode* node, bool importing);
    void DeserializeSubModel(Model* model, wxXmlNode* node);
    void DeserializeAliases(Model* model, wxXmlNode* node);
    void DeserializeSuperStrings(Model* model, wxXmlNode* node);

    Model* DeserializeArches(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeCandyCane(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeChannelBlock(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeCircle(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeCube(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeCustom(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeIcicles(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeImage(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeMatrix(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeMultiPoint(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeSingleLine(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializePolyLine(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeSphere(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeSpinner(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeStar(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeTree(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeWindow(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeWreath(wxXmlNode* node, xLightsFrame* xlights, bool importing);

    // ************************************************************************************************************
    // **********************                        DMX Section                           ************************
    // ************************************************************************************************************
    void DeserializeDmxModel(DmxModel* dmx_model, wxXmlNode* node);
    void DeserializeBeamAbility(DmxModel* model, wxXmlNode* node);
    void DeserializePresetAbility(DmxModel* model, wxXmlNode* node);
    void DeserializeDynamicColorAbility(DmxModel* model, wxXmlNode* node);
    void DeserializeColorAbility(DmxModel* model, wxXmlNode* node);
    void DeserializeColorAbilityRGBAttributes(DmxColorAbilityRGB* ability, wxXmlNode* node);
    void DeserializeColorWheelAttributes(DmxColorAbilityWheel* ability, wxXmlNode* node);
    void DeserializeColorAbilityCMYAttributes(DmxColorAbilityCMY* ability, wxXmlNode* node);
    void DeserializeShutterAbility(DmxModel* model, wxXmlNode* node);
    void DeserializeDimmerAbility(DmxModel* model, wxXmlNode* node);
    void DeserializeDmxMotor(DmxMotor* motor, wxXmlNode* node);
    void DeserializeMesh(Mesh* mesh, wxXmlNode* node);
    void DeserializeServo(Servo* servo, wxXmlNode* node);
    void DeserializeDmxMovingHeadComm(DmxMovingHeadComm* model, wxXmlNode* node);

    Model* DeserializeDmxMovingHead(wxXmlNode* node, xLightsFrame* xlights, bool importing);
    Model* DeserializeDmxMovingHeadAdv(wxXmlNode *node, xLightsFrame* xlights, bool importing);
};
