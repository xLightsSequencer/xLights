#ifndef OUTPUTPROCESSREMAP_H
#define OUTPUTPROCESSREMAP_H

#include "OutputProcess.h"

class OutputProcessRemap : public OutputProcess
{
    size_t _to;
    size_t _channels;

    public:

        OutputProcessRemap(wxXmlNode* node);
        OutputProcessRemap();
        OutputProcessRemap(const OutputProcessRemap& op);
        OutputProcessRemap(size_t _startChannel, size_t p1, size_t p2, const std::string& description);
        virtual ~OutputProcessRemap() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(wxByte* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _to; }
        virtual size_t GetP2() const override { return _channels; }
        virtual std::string GetType() const { return "Remap"; }
};

#endif
