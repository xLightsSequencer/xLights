#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ViewObject.h"
#include "TwoPointScreenLocation.h"

class ModelPreview;

#define RULER_UNITS_INCHES 5
#define RULER_UNITS_FEET 4
#define RULER_UNITS_YARDS 3
#define RULER_UNITS_MM 2
#define RULER_UNITS_CM 1
#define RULER_UNITS_M 0

class RulerObject : public ObjectWithScreenLocation<TwoPointScreenLocation>
{
public:
    RulerObject(wxXmlNode *node, const ViewObjectManager &manager);
    virtual ~RulerObject();

    virtual void InitModel() override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

    int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;

	static RulerObject* GetRuler() { return __rulerObject; }
	static int GetUnits() { if (__rulerObject != nullptr) return __rulerObject->_units; return RULER_UNITS_M; }
	static std::string GetUnitDescription();
    static float Measure(glm::vec3 p1, glm::vec3 p2);
    static float Measure(float length);
    static float UnMeasure(float length);
    static float MeasureWidth(glm::vec3 p1, glm::vec3 p2);
	static float MeasureHeight(glm::vec3 p1, glm::vec3 p2);
	static float MeasureDepth(glm::vec3 p1, glm::vec3 p2);
    static std::string MeasureDescription(float length);
    static std::string PrescaledMeasureDescription(float length);
    static std::string MeasureLengthDescription(glm::vec3 p1, glm::vec3 p2);
	static std::string MeasureWidthDescription(glm::vec3 p1, glm::vec3 p2);
	static std::string MeasureHeightDescription(glm::vec3 p1, glm::vec3 p2);
	static std::string MeasureDepthDescription(glm::vec3 p1, glm::vec3 p2);
    float ConvertDimension(const std::string& units, float measure);

protected:

    float GetPerUnit() const;

private:
    float _realLength = 1;
	int _units = RULER_UNITS_M;
	static RulerObject* __rulerObject;
};
