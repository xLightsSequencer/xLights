#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <list>
#include <map>

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
};

#endif
