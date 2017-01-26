#ifndef OUTPUTPROCESSDIM_H
#define OUTPUTPROCESSDIM_H

#include "OutputProcess.h"

class OutputProcessDim : public OutputProcess
{
    size_t _channels;
    int _dim;

public:

    OutputProcessDim(wxXmlNode* node);
    OutputProcessDim();
    OutputProcessDim(const OutputProcessDim& op);
    OutputProcessDim(size_t _startChannel, size_t p1, size_t p2, const std::string& description);
    virtual ~OutputProcessDim() {}
    virtual wxXmlNode* Save() override;
    virtual void Frame(wxByte* buffer, size_t size) override;
    virtual size_t GetP1() const override { return _channels; }
    virtual size_t GetP2() const override { return _dim; }
    virtual std::string GetType() const { return "Dim"; }
};

#endif
