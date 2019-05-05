#include <wx/xml/xml.h>

#include "BaseObject.h"
#include "ModelScreenLocation.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"

BaseObject::BaseObject()
: ModelXml(nullptr), changeCount(0)
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

void BaseObject::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (GetBaseObjectScreenLocation().IsLocked()) return;

    int i = GetBaseObjectScreenLocation().MoveHandle3D(preview, handle, ShiftKeyPressed, CtrlKeyPressed, mouseX, mouseY, latch, scale_z);
    GetBaseObjectScreenLocation().Write(ModelXml);
    if (i) {
        SetFromXml(ModelXml);
    }
    IncrementChangeCount();
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
    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(work, from);
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

void BaseObject::SetWidth(float w) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetMWidth(w);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetDepth(float d) {
    
    if (GetBaseObjectScreenLocation().IsLocked()) return;

    GetBaseObjectScreenLocation().SetMDepth(d);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
}

void BaseObject::SetHeight(float h) {

    if (GetBaseObjectScreenLocation().IsLocked()) return;

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

bool BaseObject::Scale(float f) {
    if (GetBaseObjectScreenLocation().IsLocked()) return false;
    
    bool b = GetBaseObjectScreenLocation().Scale(f);
    GetBaseObjectScreenLocation().Write(ModelXml);
    IncrementChangeCount();
    return b;
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

void BaseObject::AddOffset(double deltax, double deltay, double deltaz) {

	if (GetBaseObjectScreenLocation().IsLocked()) return;

	GetBaseObjectScreenLocation().AddOffset(deltax, deltay, deltaz);
	GetBaseObjectScreenLocation().Write(ModelXml);
	IncrementChangeCount();
}
