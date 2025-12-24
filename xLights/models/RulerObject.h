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

    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

    [[nodiscard]] int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

    [[nodiscard]] virtual bool Draw(ModelPreview* preview, xlGraphicsContext* ctx, xlGraphicsProgram* solid, xlGraphicsProgram* transparent, bool allowSelected = false) override;

	[[nodiscard]] static RulerObject* GetRuler() {
        return __rulerObject;
    }
    [[nodiscard]] static int GetUnits() {
        if (__rulerObject != nullptr)
            return __rulerObject->_units;
        return RULER_UNITS_M;
    }
    [[nodiscard]] static std::string GetUnitDescription();
    [[nodiscard]] static float Measure(glm::vec3 p1, glm::vec3 p2);
    [[nodiscard]] static float Measure(float length);
    [[nodiscard]] static float UnMeasure(float length);
    [[nodiscard]] static float MeasureWidth(glm::vec3 p1, glm::vec3 p2);
    [[nodiscard]] static float MeasureHeight(glm::vec3 p1, glm::vec3 p2);
    [[nodiscard]] static float MeasureDepth(glm::vec3 p1, glm::vec3 p2);
    [[nodiscard]] static std::string MeasureDescription(float length);
    [[nodiscard]] static std::string PrescaledMeasureDescription(float length);
    [[nodiscard]] static std::string MeasureLengthDescription(glm::vec3 p1, glm::vec3 p2);
    [[nodiscard]] static std::string MeasureWidthDescription(glm::vec3 p1, glm::vec3 p2);
    [[nodiscard]] static std::string MeasureHeightDescription(glm::vec3 p1, glm::vec3 p2);
    [[nodiscard]] static std::string MeasureDepthDescription(glm::vec3 p1, glm::vec3 p2);
    [[nodiscard]] float ConvertDimension(const std::string& units, float measure);
    [[nodiscard]] static float Convert(const std::string& fromUnits, int toUnits, float measure);
    [[nodiscard]] static float Convert(int fromUnits, const std::string& toUnits, float measure);
    [[nodiscard]] static float Convert(const std::string& fromUnits, const std::string& toUnits, float measure);

    protected:

    [[nodiscard]] float GetPerUnit() const;

private:
    float _realLength = 1;
	int _units = RULER_UNITS_M;
	static RulerObject* __rulerObject;
};
