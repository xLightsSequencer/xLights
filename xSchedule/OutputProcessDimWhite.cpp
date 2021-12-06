/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OutputProcessDimWhite.h"
#include "OutputProcessExcludeDim.h"
#include <wx/xml/xml.h>

OutputProcessDimWhite::OutputProcessDimWhite(OutputManager* outputManager, wxXmlNode* node) : OutputProcess(outputManager, node)
{
    _lastDim = -1;
    _nodes = wxAtol(node->GetAttribute("Nodes", "1"));
    _dim = wxAtol(node->GetAttribute("Dim", "100"));
    BuildDimTable();
}

OutputProcessDimWhite::OutputProcessDimWhite(const OutputProcessDimWhite& op) : OutputProcess(op)
{
    _nodes = op._nodes;
    _dim = op._dim;
    BuildDimTable();
}

OutputProcessDimWhite::OutputProcessDimWhite(OutputManager* outputManager) : OutputProcess(outputManager)
{
    _lastDim = -1;
    _nodes = 1;
    _dim = 100;
    BuildDimTable();
}

OutputProcessDimWhite::OutputProcessDimWhite(OutputManager* outputManager, std::string startChannel, size_t p1, size_t p2, const std::string& description) : OutputProcess(outputManager, startChannel, description)
{
    _lastDim = -1;
    _nodes = p1;
    _dim = p2;
}

wxXmlNode* OutputProcessDimWhite::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OPDimWhite");

    res->AddAttribute("Nodes", wxString::Format(wxT("%ld"), (long)_nodes));
    res->AddAttribute("Dim", wxString::Format(wxT("%d"), _dim));

    OutputProcess::Save(res);

    return res;
}

void OutputProcessDimWhite::BuildDimTable()
{
    if (_lastDim != _dim)
    {
        for (int i = 0; i < 256; i++)
        {
            _dimTable[i] = i * _dim / 100;
        }
    }
}

void OutputProcessDimWhite::Frame(uint8_t* buffer, size_t size, std::list<OutputProcess*>& processes)
{
    if (!_enabled) return;
    if (_dim == 100) return;

    size_t sc = GetStartChannelAsNumber();

    size_t nodes = std::min(_nodes, (size - (sc - 1)) / 3);

    auto ed = GetExcludeDim(processes, sc, sc + nodes * 3 - 1);

    if (ed.size() == 0) { // do the common easy stuff fast
        for (size_t i = 0; i < nodes; i++) {
            uint8_t* p = buffer + (sc - 1) + (i * 3);

            if (*p == *(p + 1) && *p == *(p + 2)) {
                *p = _dimTable[*p];
                *(p + 1) = *p;
                *(p + 2) = *p;
            }
        }
    }
    else {
        auto exclude = ed.begin();
        for (size_t i = 0; i < nodes; i++) {
            size_t c = (sc - 1) + (i * 3);

            while (exclude != ed.end() && c > (*exclude)->GetLastExcludeChannel() - 1) ++exclude;

            bool ex = (exclude != ed.end() && c >= (*exclude)->GetFirstExcludeChannel() - 1);

            if (!ex) {
                uint8_t* p = buffer + (sc - 1) + (i * 3);

                if (*p == *(p + 1) && *p == *(p + 2)) {
                    *p = _dimTable[*p];
                    *(p + 1) = *p;
                    *(p + 2) = *p;
                }
            }
        }
    }
}
