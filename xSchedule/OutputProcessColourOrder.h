#ifndef OUTPUTPROCESSCOLOURORDER_H
#define OUTPUTPROCESSCOLOURORDER_H

#include "OutputProcess.h"

class OutputProcessColourOrder : public OutputProcess
{
    size_t _nodes;
    int _colourOrder;

    public:

        OutputProcessColourOrder(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessColourOrder(OutputManager* outputManager);
        OutputProcessColourOrder(const OutputProcessColourOrder& op);
        OutputProcessColourOrder(OutputManager* outputManager, std::string _startChannel, size_t p1, size_t p2, const std::string& description);
        virtual ~OutputProcessColourOrder() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(wxByte* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _nodes; }
        virtual size_t GetP2() const override { return _colourOrder; }
        virtual std::string GetType() const override { return "Colour Order"; }
};

#endif
