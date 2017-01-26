#ifndef OUTPUTPROCESSDIMWHITE_H
#define OUTPUTPROCESSDIMWHITE_H

#include "OutputProcess.h"

class OutputProcessDimWhite : public OutputProcess
{
    size_t _nodes;
    int _dim;

    public:

        OutputProcessDimWhite(wxXmlNode* node);
        OutputProcessDimWhite();
        OutputProcessDimWhite(const OutputProcessDimWhite& op);
        OutputProcessDimWhite(size_t _startChannel, size_t p1, size_t p2);
        virtual ~OutputProcessDimWhite() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(wxByte* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _nodes; }
        virtual size_t GetP2() const override { return _dim; }
        virtual std::string GetType() const { return "Dim White"; }
};

#endif
