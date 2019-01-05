#ifndef OUTPUTPROCESSDIMWHITE_H
#define OUTPUTPROCESSDIMWHITE_H

#include "OutputProcess.h"

class OutputProcessDimWhite : public OutputProcess
{
    size_t _nodes;
    int _dim;
    int _lastDim;
    uint8_t _dimTable[256];

    void BuildDimTable();

    public:

        OutputProcessDimWhite(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessDimWhite(OutputManager* outputManager);
        OutputProcessDimWhite(const OutputProcessDimWhite& op);
        OutputProcessDimWhite(OutputManager* outputManager, std::string _startChannel, size_t p1, size_t p2, const std::string& description);
        virtual ~OutputProcessDimWhite() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(uint8_t* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _nodes; }
        virtual size_t GetP2() const override { return _dim; }
        virtual std::string GetType() const override { return "Dim White"; }
};

#endif
