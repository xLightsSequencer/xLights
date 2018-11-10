#ifndef OUTPUTPROCESSREMAP_H
#define OUTPUTPROCESSREMAP_H

#include "OutputProcess.h"

class OutputProcessRemap : public OutputProcess
{
    size_t _to;
    size_t _channels;

    public:

        OutputProcessRemap(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessRemap(OutputManager* outputManager);
        OutputProcessRemap(const OutputProcessRemap& op);
        OutputProcessRemap(OutputManager* outputManager, std::string _startChannel, size_t p1, size_t p2, const std::string& description);
        virtual ~OutputProcessRemap() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(uint8_t* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _to; }
        virtual size_t GetP2() const override { return _channels; }
        virtual std::string GetType() const override { return "Remap"; }
};

#endif
