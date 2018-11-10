#ifndef OUTPUTPROCESSSET_H
#define OUTPUTPROCESSSET_H

#include "OutputProcess.h"

class OutputProcessSet : public OutputProcess
{
    size_t _channels;
    int _value;

    public:

        OutputProcessSet(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessSet(OutputManager* outputManager);
        OutputProcessSet(const OutputProcessSet& op);
        OutputProcessSet(OutputManager* outputManager, std::string _startChannel, size_t p1, size_t p2, const std::string& description);
        virtual ~OutputProcessSet() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(wxByte* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _channels; }
        virtual size_t GetP2() const override { return _value; }
        virtual std::string GetType() const override { return "Set"; }
};

#endif
