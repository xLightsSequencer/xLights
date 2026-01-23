/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>

#include "BaseObject.h"
#include "ModelScreenLocation.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"

BaseObject::BaseObject()
: ModelXml(nullptr), changeCount(0), _active(true)
{
    //ctor
}

BaseObject::~BaseObject()
{
    //dtor
}

wxXmlNode* BaseObject::GetModelXml() const {
    wxASSERT(TRUE);  // This flags code to be deleted/reworked
    return this->ModelXml;
}

void BaseObject::SetName(std::string const& newname)
{
    name = newname;
}

void BaseObject::SetLayoutGroup(const std::string &grp, bool ignore_changes) {
    if (grp == layout_group) return;
    layout_group = grp;
    if (ignore_changes) return;
    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SetLayoutGroup");
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SetLayoutGroup");
    AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "SetLayoutGroup");
}

glm::vec3 BaseObject::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return GetBaseObjectScreenLocation().GetHandlePosition(handle);

    int i = GetBaseObjectScreenLocation().MoveHandle3D(preview, handle, ShiftKeyPressed, CtrlKeyPressed, mouseX, mouseY, latch, scale_z);
    if (i) {
        Setup();
    }
    IncrementChangeCount();
    return GetBaseObjectScreenLocation().GetHandlePosition(handle);
}

glm::vec3 BaseObject::MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) {
    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return GetBaseObjectScreenLocation().GetHandlePosition(handle);

    int i = GetBaseObjectScreenLocation().MoveHandle3D(scale, handle, rot, mov);
    if (i) {
        Setup();
    }
    IncrementChangeCount();
    return GetBaseObjectScreenLocation().GetHandlePosition(handle);
}

void BaseObject::SelectHandle(int handle) {
    GetBaseObjectScreenLocation().SelectHandle(handle);
}

void BaseObject::Lock(bool lock)
{
    GetBaseObjectScreenLocation().Lock(lock);
    IncrementChangeCount();
}

bool BaseObject::IsLocked() const
{
    return GetBaseObjectScreenLocation().IsLocked();
}

void BaseObject::AddASAPWork(uint32_t work, const std::string& from)
{
    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(work, from, this, nullptr, GetName());
}

void BaseObject::SetTop(float y) {

   if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetTop(y);
    IncrementChangeCount();
}

void BaseObject::SetBottom(float y) {

    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetBottom(y);
    IncrementChangeCount();
}

void BaseObject::SetLeft(float x) {

    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetLeft(x);
    IncrementChangeCount();
}

void BaseObject::SetRight(float x) {

    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetRight(x);
    IncrementChangeCount();
}

void BaseObject::SetFront(float z) {

    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetFront(z);
    IncrementChangeCount();
}

void BaseObject::SetBack(float z) {

    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetBack(z);
    IncrementChangeCount();
}

void BaseObject::SetWidth(float w, bool ignoreLock) {

    if (IsFromBase())
        return;
    if (!ignoreLock && GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetMWidth(w);
    IncrementChangeCount();
}

void BaseObject::SetDepth(float d, bool ignoreLock) {
    
    if (IsFromBase())
        return;
    if (!ignoreLock && GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetMDepth(d);
    IncrementChangeCount();
}

void BaseObject::SetHeight(float h, bool ignoreLock) {

    if (IsFromBase())
        return;
    if (!ignoreLock && GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetMHeight(h);
    Setup();
    IncrementChangeCount();
}

void BaseObject::SetHcenterPos(float pos) {

    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetHcenterPos(pos);
    IncrementChangeCount();
}

void BaseObject::SetVcenterPos(float pos) {

    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetVcenterPos(pos);
    IncrementChangeCount();
}

void BaseObject::SetDcenterPos(float pos) {

    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().SetDcenterPos(pos);
    IncrementChangeCount();
}

bool BaseObject::Rotate(ModelScreenLocation::MSLAXIS axis, float factor)
{
    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return false;

    bool b = GetBaseObjectScreenLocation().Rotate(axis, factor);
    IncrementChangeCount();
    return b;
}

void BaseObject::FlipHorizontal(bool ignoreLock) {
    if (IsFromBase())
        return;
    if (!ignoreLock && GetBaseObjectScreenLocation().IsLocked()) 
        return;
    
    GetBaseObjectScreenLocation().Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, 180.0);
    IncrementChangeCount();
}

void BaseObject::FlipVertical(bool ignoreLock) {
    if (IsFromBase())
        return;
    if (!ignoreLock && GetBaseObjectScreenLocation().IsLocked()) 
        return;
    
    GetBaseObjectScreenLocation().Rotate(ModelScreenLocation::MSLAXIS::X_AXIS, 180.0);
    IncrementChangeCount();
}

static inline bool checkNameAttributes(wxXmlNode* nn, wxXmlNode* cc) {
    if (nn->HasAttribute("name")) {
        return (cc->GetAttribute("name") == nn->GetAttribute("name"));
    } else if (nn->HasAttribute("Name")) {
        return (cc->GetAttribute("Name") == nn->GetAttribute("Name"));
    }
    return true;
}

bool BaseObject::IsXmlChanged(wxXmlNode* n) const
{
    for (wxXmlAttribute* a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
        if (!ModelXml->HasAttribute(a->GetName()) || ModelXml->GetAttribute(a->GetName()) != a->GetValue()) {
            if (a->GetName() != "StartChannel" || !ModelXml->HasAttribute("Controller")) {
                return true;
            }
        }
    }

    // This part assumes the nodes under the model only exist once or if they exist twice they have a "Name" attribute that distinguishes them
    // it also assumes one level of child nodes only
    for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
        bool found = false;
        for (wxXmlNode* cc = ModelXml->GetChildren(); cc != nullptr; cc = cc->GetNext()) {
            if (cc->GetName() == nn->GetName() && checkNameAttributes(nn, cc)) {
                found = true;
                for (wxXmlAttribute* a = cc->GetAttributes(); a != nullptr; a = a->GetNext()) {
                    if (!cc->HasAttribute(a->GetName()) || nn->GetAttribute(a->GetName()) != a->GetValue()) {
                        return true;
                    }
                }
            }
        }

        if (!found) {
            return true;
        }
    }

    return false;
}

float BaseObject::GetTop() {
    return GetBaseObjectScreenLocation().GetTop();
}

float BaseObject::GetBottom() {
    return GetBaseObjectScreenLocation().GetBottom();
}

float BaseObject::GetLeft() {
    return GetBaseObjectScreenLocation().GetLeft();
}

float BaseObject::GetRight() {
    return GetBaseObjectScreenLocation().GetRight();
}

float BaseObject::GetFront() {
    return GetBaseObjectScreenLocation().GetFront();
}

float BaseObject::GetBack() {
    return GetBaseObjectScreenLocation().GetBack();
}

float BaseObject::GetWidth() const {
    return GetBaseObjectScreenLocation().GetMWidth();
}

float BaseObject::GetHeight() const {
    return GetBaseObjectScreenLocation().GetMHeight();
}

float BaseObject::GetDepth() const {
    return GetBaseObjectScreenLocation().GetMDepth();
}

float BaseObject::GetRestorableMWidth() const {
    return GetBaseObjectScreenLocation().GetRestorableMWidth();
}

float BaseObject::GetRestorableMHeight() const {
    return GetBaseObjectScreenLocation().GetRestorableMHeight();
}

float BaseObject::GetRestorableMDepth() const {
    return GetBaseObjectScreenLocation().GetRestorableMDepth();
}

float BaseObject::GetHcenterPos() {
    return GetBaseObjectScreenLocation().GetHcenterPos();
}

float BaseObject::GetVcenterPos() {
    return GetBaseObjectScreenLocation().GetVcenterPos();
}

float BaseObject::GetDcenterPos() {
    return GetBaseObjectScreenLocation().GetDcenterPos();
}

void BaseObject::AddOffset(double deltax, double deltay, double deltaz) {
	if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

	GetBaseObjectScreenLocation().AddOffset(deltax, deltay, deltaz);
	IncrementChangeCount();
}

void BaseObject::RotateAboutPoint(glm::vec3 position, glm::vec3 angle) {
    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return;

    GetBaseObjectScreenLocation().RotateAboutPoint(position, angle);
    IncrementChangeCount();
    Setup();  // only needed when rotating PolyLine...hope to remove this later and do what's needed in the PolyLine rotate call
}

bool BaseObject::Scale(const glm::vec3& factor)
{
    bool return_value = false;
    if (GetBaseObjectScreenLocation().IsLocked() || IsFromBase()) return false;

    return_value = GetBaseObjectScreenLocation().Scale(factor);
    IncrementChangeCount();
    return return_value;
}

bool BaseObject::IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) {
    return  GetBaseObjectScreenLocation().IsContained(preview, x1, y1, x2, y2);
}

void BaseObject::SetActive(bool active) {
	_active = active; 

    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BaseObject::SetActive");
    AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BaseObject::SetActive");
    AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "BaseObject::SetActive"); // because the names are displayed differently
}

