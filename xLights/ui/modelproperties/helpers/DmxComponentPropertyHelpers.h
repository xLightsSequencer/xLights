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

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class Servo;
class DmxMotor;
class Mesh;
class DmxImage;

namespace DmxComponentPropertyHelpers {

void AddServoProperties(wxPropertyGridInterface* grid, const Servo& servo, bool pwm);
int OnServoPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, Servo& servo, BaseObject* base, bool locked);

void AddMotorProperties(wxPropertyGridInterface* grid, const DmxMotor& motor);
int OnMotorPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxMotor& motor, BaseObject* base, bool locked);

void AddMeshProperties(wxPropertyGridInterface* grid, const Mesh& mesh);
int OnMeshPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, Mesh& mesh, BaseObject* base, bool locked);

void AddImageProperties(wxPropertyGridInterface* grid, const DmxImage& image);
int OnImagePropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxImage& image, BaseObject* base, bool locked);

} // namespace DmxComponentPropertyHelpers
