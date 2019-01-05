#ifndef OUTPUTPROCESSTHREETOFOUR_H
#define OUTPUTPROCESSTHREETOFOUR_H

#include "OutputProcess.h"

class OutputProcessThreeToFour : public OutputProcess
{
    size_t _nodes;
	std::string _colourOrder;

    public:

        OutputProcessThreeToFour(OutputManager* outputManager, wxXmlNode* node);
        OutputProcessThreeToFour(OutputManager* outputManager);
        OutputProcessThreeToFour(const OutputProcessThreeToFour& op);
        OutputProcessThreeToFour(OutputManager* outputManager, std::string _startChannel, size_t p1, std::string colourOrder, const std::string& description);
        virtual ~OutputProcessThreeToFour() {}
        virtual wxXmlNode* Save() override;
        virtual void Frame(uint8_t* buffer, size_t size) override;
        virtual size_t GetP1() const override { return _nodes; }
        std::string GetColourOrder() const { return _colourOrder; }
        virtual size_t GetP2() const override { return 0; }
        virtual std::string GetType() const override { return "Three To Four"; }
};

#endif
