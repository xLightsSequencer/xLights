#include "OutputProcessGamma.h"
#include <wx/xml/xml.h>

OutputProcessGamma::OutputProcessGamma(wxXmlNode* node) : OutputProcess(node)
{
    _nodes = wxAtol(node->GetAttribute("Nodes", "1"));
    _gamma = wxAtof(node->GetAttribute("Gamma", "1.0"));
    _gammaR = wxAtof(node->GetAttribute("GammaR", "1.0"));
    _gammaG = wxAtof(node->GetAttribute("GammaG", "1.0"));
    _gammaB = wxAtof(node->GetAttribute("GammaB", "1.0"));
    BuildGammaData();
}

OutputProcessGamma::OutputProcessGamma(const OutputProcessGamma& op) : OutputProcess(op)
{
    _nodes = op._nodes;
    _gamma = op._gamma;
    _gammaR = op._gammaR;
    _gammaG = op._gammaG;
    _gammaB = op._gammaB;
    BuildGammaData();
}

OutputProcessGamma::OutputProcessGamma() : OutputProcess()
{
    _nodes = 1;
    _gamma = 1.0;
    _gammaR = 1.0;
    _gammaG = 1.0;
    _gammaB = 1.0;
    BuildGammaData();
}

OutputProcessGamma::OutputProcessGamma(size_t _startChannel, size_t p1, float gamma, float gammaR, float gammaG, float gammaB, const std::string& description) : OutputProcess(_startChannel, description)
{
    _nodes = p1;
    _gamma = gamma;
    _gammaR = gammaR;
    _gammaG = gammaG;
    _gammaB = gammaB;
    BuildGammaData();
}

wxXmlNode* OutputProcessGamma::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPGamma");

    res->AddAttribute("Nodes", wxString::Format(wxT("%ld"), (long)_nodes));
    res->AddAttribute("Gamma", wxString::Format(wxT("%.2f"), _gamma));
    res->AddAttribute("GammaR", wxString::Format(wxT("%.2f"), _gammaR));
    res->AddAttribute("GammaG", wxString::Format(wxT("%.2f"), _gammaG));
    res->AddAttribute("GammaB", wxString::Format(wxT("%.2f"), _gammaB));

    OutputProcess::Save(res);

    return res;
}

void OutputProcessGamma::BuildGammaData()
{
    BuildGammaData(_gammaData, _gamma);
    BuildGammaData(_gammaDataR, _gammaR);
    BuildGammaData(_gammaDataG, _gammaG);
    BuildGammaData(_gammaDataB, _gammaB);
}

void OutputProcessGamma::BuildGammaData(wxByte data[], float gamma)
{
    for (int x = 0; x < 256; x++) {
        float i = x;
        i = 255 * pow(i / 255.0, gamma);
        if (i > 255) {
            i = 255;
        }
        if (i < 0) {
            i = 0;
        }
        data[x] = i;
    }
}

void OutputProcessGamma::Frame(wxByte* buffer, size_t size)
{
    if (!_enabled) return;
    if (_gamma == 1.0) return;
    if (_gamma == 0.00 && _gammaR == 1.0 && _gammaG == 1.0 && _gammaB == 1.0) return;

    size_t nodes = std::min(_nodes, (size - (_startChannel - 1)) / 3);

    for (int i = 0; i < nodes; i++)
    {
        wxByte* p = buffer + (_startChannel - 1) + (i * 3);

        if (_gamma != 0.0)
        {
            *p = _gammaData[*p];
            *(p+1) = _gammaData[*(p+1)];
            *(p+2) = _gammaData[*(p+2)];
        }
        else
        {
            *p = _gammaDataR[*p];
            *(p + 1) = _gammaDataG[*(p + 1)];
            *(p + 2) = _gammaDataB[*(p + 2)];
        }
    }
}
