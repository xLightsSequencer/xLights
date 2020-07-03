/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
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
    return this->ModelXml;
}

void BaseObject::SetLayoutGroup(const std::string &grp) {
    if (grp != ModelXml->GetAttribute("LayoutGroup", "xyzzy_kw"))
    {
        layout_group = grp;
        ModelXml->DeleteAttribute("LayoutGroup");
        ModelXml->AddAttribute("LayoutGroup", grp);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SetLayoutGroup");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SetLayoutGroup");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "SetLayoutGroup");
    }
}

void BaseObject::EnableLayoutGroupProperty(wxPropertyGridInterface* grid, bool enable)
{
    if (grid->GetProperty("ModelLayoutGroup") != nullptr)
    {
        grid->GetProperty("ModelLayoutGroup")->Enable(enable);
    }
}

glm::vec3 BaseObject::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (GetBaseObjectScreenLocation().IsLocked()) return GetBaseObjectScreenLocation().GetHandlePosition(handle);

    int i = GetBaseObjectScreenLocation().MoveHandle3D(preview, handle, ShiftKeyPressed, CtrlKeyPressed, mouseX, mouseY, latch, scale_z);
    GetBaseObjectScreenLocation().Write(ModelXml);
    if (i) {
        SetFromXml(ModelXml);
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
    GetModelXml()->DeleteAttribute("Locked");
    if (lock)
    {
        GetModelXml()->AddAttribute("Locked", "1");
    }
    IncrementChangeCount();
}

void BaseObject::AddASAPWork(uint32_t work, const std::string& from)
{
    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(work, from, this, nullptr, GetName());
}

void BaseObject::SetTop(float y) {

   if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetTop(y);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetBottom(float y) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetBottom(y);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetLeft(float x) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetLeft(x);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetRight(float x) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetRight(x);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetFront(float z) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetFront(z);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetBack(float z) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetBack(z);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetWidth(float w, bool ignoreLock) {

    if (!ignoreLock && GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetMWidth(w);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetDepth(float d, bool ignoreLock) {
    
    if (!ignoreLock && GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetMDepth(d);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetHeight(float h, bool ignoreLock) {

    if (!ignoreLock && GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetMHeight(h);
    GetBaseObjectScreenLocation().Write(ModelXml);
    SetFromXml(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetHcenterPos(float pos) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetHcenterPos(pos);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetVcenterPos(float pos) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetVcenterPos(pos);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetDcenterPos(float pos) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetDcenterPos(pos);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

bool BaseObject::Rotate(int axis, float factor) {
    if (GetBaseObjectScreenLocation().IsLocked()) return false;

    bool b = GetBaseObjectScreenLocation().Rotate(axis, factor);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
    return b;
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
	if (GetBaseObjectScreenLocation().IsLocked()) return;

	GetBaseObjectScreenLocation().AddOffset(deltax, deltay, deltaz);
	GetBaseObjectScreenLocation().Write(ModelXml);
	IncrementChangeCount();
}

void BaseObject::RotateAboutPoint(glm::vec3 position, glm::vec3 angle) {
    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().RotateAboutPoint(position, angle);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
    SetFromXml(ModelXml);  // only needed when rotating PolyLine...hope to remove this later and do what's needed in the PolyLine rotate call
}

bool BaseObject::Scale(const glm::vec3& factor)
{
    bool return_value = false;
    if (GetBaseObjectScreenLocation().IsLocked()) return false;

    return_value = GetBaseObjectScreenLocation().Scale(factor);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
    return return_value;
}

bool BaseObject::IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) {
    return  GetBaseObjectScreenLocation().IsContained(preview, x1, y1, x2, y2);
}

void BaseObject::SetActive(bool active) {
	_active = active; 
    ModelXml->DeleteAttribute("Active");
    if (active) {
        ModelXml->AddAttribute("Active", "1");
    }
    else
    {
        ModelXml->AddAttribute("Active", "0");
    }
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BaseObject::SetActive");
    AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BaseObject::SetActive");
}

