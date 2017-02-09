#ifndef OUTPUTPROCESSREVERSE_H
#define OUTPUTPROCESSREVERSE_H

#include "OutputProcess.h"

class OutputProcessReverse : public OutputProcess
{
    size_t _nodes;

    public:

        OutputProcessReverse(wxXmlNode* node);
        OutputProcessReverse();
        OutputProcessReverse(const OutputProcessReverse& op);
        OutputProcessReverse(size_t _startChannel, size_t p1, size_t p2, const std::string& description);
        virtual ~OutputProcessReverse() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(wxByte* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _nodes; }
        virtual size_t GetP2() const override { return 0; }
        virtual std::string GetType() const override { return "Reverse"; }
};

#endif
