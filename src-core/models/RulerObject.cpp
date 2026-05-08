/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <spdlog/fmt/fmt.h>
#include "RulerObject.h"
#include "../graphics/IModelPreview.h"
#include "../graphics/xlGraphicsContext.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "Model.h"
#include "../graphics/xlGraphicsAccumulators.h"

RulerObject* RulerObject::__rulerObject = nullptr;

RulerObject::RulerObject(const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager)
{
    DisplayAs = DisplayAsType::Ruler;
	__rulerObject = this;

    // we need to call this so we can use it to measure in 2D
    screenLocation.PrepareToDraw(false, false);
}

RulerObject::~RulerObject()
{
	__rulerObject = nullptr;
}

void RulerObject::InitModel() {
    float dx = screenLocation.GetX2();
    float dy = screenLocation.GetY2();
    float dz = screenLocation.GetZ2();
    screenLocation.SetRenderSize(std::abs(dx), std::abs(dy), std::abs(dz));
}


bool RulerObject::Draw(IModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected) {
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

    if ((Selected() || Highlighted()) && allowSelected) {
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
    return fmt::format("{:.2f}{}", Measure(p1,p2), GetUnitDescription());
}

std::string RulerObject::MeasureWidthDescription(glm::vec3 p1, glm::vec3 p2)
{
    if (__rulerObject == nullptr) return "";
    return fmt::format("{:.2f}{}", MeasureWidth(p1,p2), GetUnitDescription());
}

std::string RulerObject::MeasureHeightDescription(glm::vec3 p1, glm::vec3 p2)
{
    if (__rulerObject == nullptr) return "";
    return fmt::format("{:.2f}{}", MeasureHeight(p1,p2), GetUnitDescription());
}

std::string RulerObject::MeasureDepthDescription(glm::vec3 p1, glm::vec3 p2)
{
    if (__rulerObject == nullptr) return "";
    return fmt::format("{:.2f}{}", MeasureDepth(p1,p2), GetUnitDescription());
}

float RulerObject::Convert(const std::string& fromUnits, int toUnits, float measure) {
    std::string tu = "";
    switch (toUnits) {
    case RULER_UNITS_INCHES:
        tu = "i";
        break;
    case RULER_UNITS_FEET:
        tu = "f";
        break;
    case RULER_UNITS_YARDS:
        tu = "y";
        break;
    case RULER_UNITS_MM:
        tu = "mm";
        break;
    case RULER_UNITS_CM:
        tu = "cm";
        break;
    case RULER_UNITS_M:
        tu = "m";
        break;
    }

    return Convert(fromUnits, tu, measure);
}

float RulerObject::Convert(int fromUnits, const std::string& toUnits, float measure) {
    std::string fu = "";
    switch (fromUnits) {
    case RULER_UNITS_INCHES:
        fu = "i";
        break;
    case RULER_UNITS_FEET:
        fu = "f";
        break;
    case RULER_UNITS_YARDS:
        fu = "y";
        break;
    case RULER_UNITS_MM:
        fu = "mm";
        break;
    case RULER_UNITS_CM:
        fu = "cm";
        break;
    case RULER_UNITS_M:
        fu = "m";
        break;
    }

    return Convert(fu, toUnits, measure);
}

float RulerObject::Convert(const std::string& fromUnits, const std::string& toUnits, float measure) {
    if (fromUnits == toUnits)
        return measure;

    if (fromUnits == "m") {
        if (toUnits == "cm")
            return measure * 100;
        if (toUnits == "mm")
            return measure * 1000;
        if (toUnits == "i")
            return measure / 0.0254f;
        if (toUnits == "f")
            return measure / (12 * 0.0254f);
        if (toUnits == "y")
            return measure / (36 * 0.0254f);
    } else if (fromUnits == "cm") {
        if (toUnits == "m")
            return measure / 100;
        if (toUnits == "mm")
            return measure * 10;
        if (toUnits == "i")
            return measure / 2.54f;
        if (toUnits == "f")
            return measure / (12 * 2.54f);
        if (toUnits == "y")
            return measure / (36 * 2.54f);
    } else if (fromUnits == "mm") {
        if (toUnits == "m")
            return measure / 1000;
        if (toUnits == "cm")
            return measure / 10;
        if (toUnits == "i")
            return measure / 25.4f;
        if (toUnits == "f")
            return measure / (12 * 25.4f);
        if (toUnits == "y")
            return measure / (36 * 25.4f);
    } else if (fromUnits == "i") {
        if (toUnits == "m")
            return measure * 0.0254f;
        if (toUnits == "cm")
            return measure * 2.54f;
        if (toUnits == "mm")
            return measure * 25.4f;
        if (toUnits == "f")
            return measure / 12;
        if (toUnits == "y")
            return measure / 36;
    } else if (fromUnits == "f") {
        if (toUnits == "m")
            return measure * 0.0254f * 12;
        if (toUnits == "cm")
            return measure * 2.54f * 12;
        if (toUnits == "mm")
            return measure * 25.4f * 12;
        if (toUnits == "i")
            return measure * 12;
        if (toUnits == "y")
            return measure / 3;
    } else if (fromUnits == "y") {
        if (toUnits == "m")
            return measure * 0.0254f * 36;
        if (toUnits == "cm")
            return measure * 2.54f * 36;
        if (toUnits == "mm")
            return measure * 25.4f * 36;
        if (toUnits == "i")
            return measure * 36;
        if (toUnits == "f")
            return measure * 3;
    }

    return 0.0f;
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
    // Compute distance directly from stored x2/y2/z2 offsets so this works
    // in 2D mode where Draw()/PrepareToDraw() is never called on view objects
    // and the cached origin/point2 members are stale (zeroed from construction).
    float dx = screenLocation.GetX2();
    float dy = screenLocation.GetY2();
    float dz = screenLocation.GetZ2();

    float den = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (den == 0) return 1;
    if (_realLength == 0) return 1;

    return _realLength / den;
}

std::string RulerObject::MeasureDescription(float length)
{
    if (__rulerObject == nullptr) return "";
    return fmt::format("{:.2f}{}", Measure(length), GetUnitDescription());
}

std::string RulerObject::PrescaledMeasureDescription(float length)
{
    if (__rulerObject == nullptr) return "";
    return fmt::format("{:.2f}{}", length, GetUnitDescription());
}
