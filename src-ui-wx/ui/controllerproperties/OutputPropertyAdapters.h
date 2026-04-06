#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <list>
#include <memory>

class Output;
class Controller;
class ModelManager;
class OutputModelManager;
class wxPGProperty;
class wxPropertyGrid;
class wxPropertyGridEvent;

class OutputPropertyAdapter {
public:
    explicit OutputPropertyAdapter(Output& output) : _output(output) {}
    virtual ~OutputPropertyAdapter() = default;

    virtual void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {}
    virtual void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) {}
    virtual void RemoveProperties(wxPropertyGrid* propertyGrid) {}
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) { return false; }
    virtual void HandleExpanded(wxPropertyGridEvent& event, bool expanded) {}

    static std::unique_ptr<OutputPropertyAdapter> Create(Output& output);

protected:
    Output& _output;
};

class E131Output;
class E131OutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit E131OutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
private:
    E131Output& _e131;
};

class ArtNetOutput;
class ArtNetOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit ArtNetOutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
private:
    ArtNetOutput& _artnet;
};

class DDPOutput;
class DDPOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit DDPOutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
private:
    DDPOutput& _ddp;
};

class KinetOutput;
class KinetOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit KinetOutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
private:
    KinetOutput& _kinet;
};

class ZCPPOutput;
class ZCPPOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit ZCPPOutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
private:
    ZCPPOutput& _zcpp;
};

class TwinklyOutput;
class TwinklyOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit TwinklyOutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
private:
    TwinklyOutput& _twinkly;
};

class OPCOutput;
class OPCOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit OPCOutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
private:
    OPCOutput& _opc;
};

class xxxEthernetOutput;
class xxxEthernetOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit xxxEthernetOutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
private:
    xxxEthernetOutput& _xxx;
};

class xxxSerialOutput;
class xxxSerialOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit xxxSerialOutputPropertyAdapter(Output& output);
    void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    void RemoveProperties(wxPropertyGrid* propertyGrid) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
private:
    xxxSerialOutput& _xxxSerial;
};

class LOROptimisedOutput;
class LOROptimisedOutputPropertyAdapter : public OutputPropertyAdapter {
public:
    explicit LOROptimisedOutputPropertyAdapter(Output& output);
    void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty* before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
    void HandleExpanded(wxPropertyGridEvent& event, bool expanded) override;
private:
    LOROptimisedOutput& _lor;
};
