#ifndef CONTROLLER_H
#define CONTROLLER_H

// Controllers are defined in .xcontroller xml files in the controllers directory under the directory containing xlights.exe
// One controller per file
//
// <Controller>
//   <Brand>Name of the controller manufacturer</Brand>
//   <Model>Identity of the model ... together with brand these must be unique</Model>
//   <MaxInputUniverses>Maximum number of input universes this controller can receive</MaxInputUniverses>
//   <MaxChannelsPerUniverse>Maximum number of channels an input universe can contain. Typically 510 or 512 but can be higher for some serial controllers.</MaxChannerlsPerUniverse>
//   <SupportedProtocols>
//      <Protocol>Valid xlights protocol this controller supports. See outputs/Output.h for a list of values</Protocol>
//      <Protocol>...</Protocol>
//   </SupportedProtocols>
//   <Outputs>
//      <String>
//        <Count>Number of string outputs</Count>
//        <MaxChannels>Maximum channels supported per string output</MaxChannels>
//      </String>
//      <Serial>
//        <Count>Number of serial outputs</Count>
//        <MaxChannels>Maximum channels supported per serial output</MaxChannels>
//      </Serial>
//   </Outputs>
// </Controller>

//
// ****** NOTE AT THIS TIME THIS CLASS EXISTS BUT IT HAS NOT BEEN INTEGRATED
// ****** CONCEPTUALLY THIS MAKES IT POSSIBLE FOR US TO BE SMARTER IN VALIDATING 
// ****** CONTROLLER CONFIGURATION
//

#include <list>
#include <map>
#include <string>

class wxXmlNode;

class Controller
{
    static std::list<Controller> __controllers;
    std::string _brand;
    std::string _model;
    int _maxUniverses;
    int _maxChannelsPerUniverse;
    std::map<std::string, int> _outputs;
    std::map<std::string, int> _channelsPerOutput;
    std::list<std::string> _supportedProtocols;

    public:
    static std::list<std::string> GetControllerTypes();
    static Controller* GetController(const std::string& id);
    static void LoadControllers();

    Controller(wxXmlNode* node);
    int GetMaxUniverses() const { return _maxUniverses; }
    int GetMaxChannelsPerUniverse() const { return _maxChannelsPerUniverse; }
    int GetOutputCount(const std::string& type) const;
    int GetChannelsPerOutput(const std::string& type) const;
    std::string GetBrand() const { return _brand; }
    std::string GetModel() const { return _model; }
    std::string GetId() const { return _model + " " + _brand; }
    bool SupportsProtocol(const std::string& protocol);
};

#endif
