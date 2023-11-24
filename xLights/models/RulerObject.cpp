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
#include <wx/propgrid/advprops.h>

#include "RulerObject.h"
#include "ModelPreview.h"
#include "Model.h"
#include "../graphics/xlGraphicsAccumulators.h"

RulerObject* RulerObject::__rulerObject = nullptr;

RulerObject::RulerObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager)
{
	__rulerObject = this;
    SetFromXml(node);

    // we need to call this so we can use it to measure in 2D
    screenLocation.PrepareToDraw(false, false);
}

RulerObject::~RulerObject()
{
	__rulerObject = nullptr;
}

void RulerObject::InitModel() {
    if (ModelXml->HasAttribute("Units")) {
        _units = wxAtoi(ModelXml->GetAttribute("Units"));
    }
    if (ModelXml->HasAttribute("Length")) {
        _realLength = wxAtof(ModelXml->GetAttribute("Length"));
    }
    auto start = screenLocation.GetPoint1();
    auto end = screenLocation.GetPoint2();
    //screenLocation.SetRenderSize(1.0, 1.0, 1.0);
    screenLocation.SetRenderSize(std::abs(start.x - end.x), std::abs(start.y - end.y), std::abs(start.z - end.z));  // FIXME: Modify to only call this when position changes
}

static const char *UNITS_VALUES[] = {
    "Meters", "Centimeters", "Millimeters", "Yards", "Feet",
    "Inches"};
static wxArrayString RULER_UNITS(6, UNITS_VALUES);

void RulerObject::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{

	wxPGProperty* p = grid->Append(new wxEnumProperty("Units", "Units", RULER_UNITS, wxArrayInt(), _units));

    p = grid->Append(new wxFloatProperty("Real Length", "Length", _realLength));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.5);
    p->SetAttribute("Min", 0.01);
    p->SetEditor("SpinCtrl");
}

int RulerObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("Units" == event.GetPropertyName()) {
        _units = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Units");
        ModelXml->AddAttribute("Units", wxString::Format("%d", _units));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "RulerObject::OnPropertyGridChange::Units");
        return 0;
    }
    else if ("Length" == event.GetPropertyName()) {
        _realLength = event.GetPropertyValue().GetDouble();
        if (_realLength < 0.01) _realLength = 0.01f;
        ModelXml->DeleteAttribute("Length");
        ModelXml->AddAttribute("Length", wxString::Format("%f", _realLength));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "GridlinesObject::OnPropertyGridChange::GridWidth");
        return 0;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

bool RulerObject::Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    if (!IsActive()) { return true; }

    xlColor colour = xlColor(255,0,255);

    auto start = screenLocation.GetPoint1();
    auto end = screenLocation.GetPoint2();
    
    screenLocation.SetRenderSize(std::abs(start.x - end.x), std::abs(start.y - end.y), std::abs(start.z - end.z));
    
    auto vac = transparent->getAccumulator();
    int startVert = vac->getCount();
    
    vac->AddVertex(start.x, start.y, start.z, colour);
    vac->AddVertex(end.x, end.y, end.z, colour);
    transparent->addStep([transparent, vac, startVert](xlGraphicsContext *ctx) {
        ctx->drawLines(vac, startVert, 2);
    });

    GetObjectScreenLocation().SetScaleMatrix(glm::vec3(1.0, 1.0, 1.0));
    static_cast<TwoPointScreenLocation&>(screenLocation).UpdateBoundingBox();

    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(solid, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), true, IsFromBase());
    }
    return true;
}


std::string RulerObject::GetUnitDescription()
{
	if (__rulerObject == nullptr) return "m";
	switch(__rulerObject->_units)
	{
		case RULER_UNITS_INCHES: return "in";
		case RULER_UNITS_FEET: return "ft";
        case RULER_UNITS_YARDS: return "yds";
        case RULER_UNITS_MM: return "mm";
        case RULER_UNITS_CM: return "cm";
        case RULER_UNITS_M: return "m";
	}
	return "m";
}

float RulerObject::Measure(float length)
{
    if (__rulerObject == nullptr) return 0.0;
    return length * __rulerObject->GetPerUnit();
}

float RulerObject::UnMeasure(float length)
{
    if (__rulerObject == nullptr)
        return 0.0;
    return length / __rulerObject->GetPerUnit();
}

float RulerObject::Measure(glm::vec3 p1, glm::vec3 p2)
{
	if (__rulerObject == nullptr) return 0.0;
	float res = std::sqrt(
						(p2.x - p1.x) * (p2.x - p1.x) +
						(p2.y - p1.y) * (p2.y - p1.y) +
						(p2.z - p1.z) * (p2.z - p1.z)
					) * __rulerObject->GetPerUnit();
    return res;
}

float RulerObject::MeasureWidth(glm::vec3 p1, glm::vec3 p2)
{
	if (__rulerObject == nullptr) return 0.0;
	return std::abs(p2.x - p1.x) * __rulerObject->GetPerUnit();
}

float RulerObject::MeasureHeight(glm::vec3 p1, glm::vec3 p2)
{
	if (__rulerObject == nullptr) return 0.0;
	return std::abs(p2.y - p1.y) * __rulerObject->GetPerUnit();
}

float RulerObject::MeasureDepth(glm::vec3 p1, glm::vec3 p2)
{
	if (__rulerObject == nullptr) return 0.0;
	return std::abs(p2.z - p1.z) * __rulerObject->GetPerUnit();
}

std::string RulerObject::MeasureLengthDescription(glm::vec3 p1, glm::vec3 p2)
{
    if (__rulerObject == nullptr) return "";
    return wxString::Format("%0.02f%s", Measure(p1,p2), GetUnitDescription()).ToStdString();
}

std::string RulerObject::MeasureWidthDescription(glm::vec3 p1, glm::vec3 p2)
{
    if (__rulerObject == nullptr) return "";
    return wxString::Format("%0.02f%s", MeasureWidth(p1,p2), GetUnitDescription()).ToStdString();
}

std::string RulerObject::MeasureHeightDescription(glm::vec3 p1, glm::vec3 p2)
{
    if (__rulerObject == nullptr) return "";
    return wxString::Format("%0.02f%s", MeasureHeight(p1,p2), GetUnitDescription()).ToStdString();
}

std::string RulerObject::MeasureDepthDescription(glm::vec3 p1, glm::vec3 p2)
{
    if (__rulerObject == nullptr) return "";
    return wxString::Format("%0.02f%s", MeasureDepth(p1,p2), GetUnitDescription()).ToStdString();
}

float RulerObject::ConvertDimension(const std::string& units, float measure)
{
    if (units == "m") // metres
    {
        switch (_units) {
        case RULER_UNITS_INCHES:
            return measure / 0.0254f;
        case RULER_UNITS_FEET:
            return measure / (12 * 0.0254f);
        case RULER_UNITS_YARDS:
            return measure / (36 * 0.0254f);
        case RULER_UNITS_MM:
            return measure * 1000;
        case RULER_UNITS_CM:
            return measure * 100;
        case RULER_UNITS_M:
            break;
        }
    } else if (units == "cm") // centimeters
    {
        switch (_units) {
        case RULER_UNITS_INCHES:
            return measure / 2.54f;
        case RULER_UNITS_FEET:
            return measure / (12 * 2.54f);
        case RULER_UNITS_YARDS:
            return measure / (36 * 2.54f);
        case RULER_UNITS_MM:
            return measure * 10;
        case RULER_UNITS_CM:
            break;
        case RULER_UNITS_M:
            return measure / 100;
        }
    } else if (units == "mm") // millimeters
    {
        switch (_units) {
        case RULER_UNITS_INCHES:
            return measure / 25.4f;
        case RULER_UNITS_FEET:
            return measure / (12 * 25.4f);
        case RULER_UNITS_YARDS:
            return measure / (36 * 25.4f);
        case RULER_UNITS_MM:
            break;
        case RULER_UNITS_CM:
            return measure / 10;
        case RULER_UNITS_M:
            return measure / 1000;
        }
    } else if (units == "i") // inches
    {
        switch (_units) {
        case RULER_UNITS_INCHES:
            break;
        case RULER_UNITS_FEET:
            return measure / 12;
        case RULER_UNITS_YARDS:
            return measure / 36;
        case RULER_UNITS_MM:
            return measure * 25.4f;
        case RULER_UNITS_CM:
            return measure * 2.54f;
        case RULER_UNITS_M:
            return measure * 0.0254f;
        }
    } else if (units == "f") // feet
    {
        switch (_units) {
        case RULER_UNITS_INCHES:
            return measure * 12;
        case RULER_UNITS_FEET:
            break;
        case RULER_UNITS_YARDS:
            return measure / 3;
        case RULER_UNITS_MM:
            return measure * 12 * 25.4f;
        case RULER_UNITS_CM:
            return measure * 12 * 2.54f;
        case RULER_UNITS_M:
            return measure * 12 * 0.0254f;
        }
    } else if (units == "y") // yards
    {
        switch (_units) {
        case RULER_UNITS_INCHES:
            return measure * 36;
        case RULER_UNITS_FEET:
            return measure * 3;
        case RULER_UNITS_YARDS:
            break;
        case RULER_UNITS_MM:
            return measure * 36 * 25.4f;
        case RULER_UNITS_CM:
            return measure * 36 * 2.54f;
        case RULER_UNITS_M:
            return measure * 36 * 0.0254f;
        }
    }

    return measure;
}

float RulerObject::GetPerUnit() const
{
    auto p1 = static_cast<TwoPointScreenLocation>(screenLocation).GetPoint1();
    auto p2 = static_cast<TwoPointScreenLocation>(screenLocation).GetPoint2();

    float den = std::sqrt(
        (p2.x - p1.x) * (p2.x - p1.x) +
        (p2.y - p1.y) * (p2.y - p1.y) +
        (p2.z - p1.z) * (p2.z - p1.z)
    );

    if (den == 0) return 1;
    if (_realLength == 0) return 1;

    return _realLength / den;
}

std::string RulerObject::MeasureDescription(float length)
{
    if (__rulerObject == nullptr) return "";
    return wxString::Format("%0.02f%s", Measure(length), GetUnitDescription()).ToStdString();
}

std::string RulerObject::PrescaledMeasureDescription(float length)
{
    if (__rulerObject == nullptr) return "";
    return wxString::Format("%0.02f%s", length, GetUnitDescription()).ToStdString();
}
