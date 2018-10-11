#ifndef OUTPUTPROCESSSUSTAIN_H
#define OUTPUTPROCESSSUSTAIN_H

#include "OutputProcess.h"

class OutputProcessSustain : public OutputProcess
{
    size_t _channels;
    wxByte* _save;

    public:

        OutputProcessSustain(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessSustain(OutputManager* outputManager);
        OutputProcessSustain(const OutputProcessSustain& op);
        OutputProcessSustain(OutputManager* outputManager, std::string _startChannel, size_t p1, const std::string& description);
        virtual ~OutputProcessSustain();
        virtual wxXmlNode* Save() override;
        virtual void Frame(wxByte* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _channels; }
        virtual size_t GetP2() const override { return 0; }
        virtual std::string GetType() const override { return "Sustain"; }
};

#endif
