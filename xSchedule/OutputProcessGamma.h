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

#include "OutputProcess.h"

class OutputProcessGamma : public OutputProcess
{
    size_t _nodes;
    float _gamma;
    float _gammaR;
    float _gammaG;
    float _gammaB;

    uint8_t _gammaData[256];
    uint8_t _gammaDataR[256];
    uint8_t _gammaDataG[256];
    uint8_t _gammaDataB[256];

    void BuildGammaData(uint8_t data[], float gamma);
    void BuildGammaData();

public:

    OutputProcessGamma(OutputManager* outputManager, wxXmlNode* node);
    OutputProcessGamma(OutputManager* outputManager);
    OutputProcessGamma(const OutputProcessGamma& op);
    OutputProcessGamma(OutputManager* outputManager, std::string _startChannel, size_t p1, float gamma, float gammaR, float gammaG, float gammaB, const std::string& description);
    virtual ~OutputProcessGamma() {}
    virtual wxXmlNode* Save() override;
    virtual void Frame(uint8_t* buffer, size_t size) override;
    virtual size_t GetP1() const override { return _nodes; }
    virtual size_t GetP2() const override { return 0; }
    virtual std::string GetType() const override { return "Gamma"; }
    std::string GetGammaSettings() const { return wxString::Format("%.2f,%.2f,%.2f,%.2f", _gamma, _gammaR, _gammaG, _gammaB).ToStdString(); }
};

