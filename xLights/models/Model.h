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
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "BoxedScreenLocation.h"
#include "ModelScreenLocation.h"
#include "PWMOutput.h"
#include "PolyPointScreenLocation.h"
#include "ThreePointScreenLocation.h"
#include "TwoPointScreenLocation.h"

#include "BaseObject.h"
#include "../Color.h"
#include "../UtilFunctions.h"
#include <wx/gdicmn.h>
#include <wx/propgrid/props.h>

class wxProgressDialog;
class DimmingCurve;
class wxXmlNode;
class ModelPreview;
class wxArrayString;
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class ModelScreenLocation;
class ModelManager;
class xLightsFrame;
class OutputManager;
class wxPGProperty;
class ControllerCaps;
class NodeBaseClass;
class xlGraphicsProgram;
class xlVertexIndexedColorAccumulator;
class xlVertexColorAccumulator;
class xlVertexAccumulator;
class xlGraphicsContext;
typedef std::unique_ptr<NodeBaseClass> NodeBaseClassPtr;

// convert to Structs someday
using FaceStateData = std::map<std::string, std::map<std::string, std::string>>;
using FaceStateNodes = std::map<std::string, std::map<std::string, std::list<int>>>;

#define NO_CONTROLLER "No Controller"
#define USE_START_CHANNEL "Use Start Channel"

enum {
    // GRIDCHANGE_REFRESH_DISPLAY = 0x0001,
    // GRIDCHANGE_MARK_DIRTY = 0x0002,
    // GRIDCHANGE_REBUILD_PROP_GRID = 0x0004,
    // GRIDCHANGE_REBUILD_MODEL_LIST = 0x0008,
    // GRIDCHANGE_UPDATE_ALL_MODEL_LISTS = 0x0010,
    GRIDCHANGE_SUPPRESS_HOLDSIZE = 0x00020,

    // GRIDCHANGE_MARK_DIRTY_AND_REFRESH = 0x0003
};

// only allows 0-9,- characters
class wxDropPatternProperty : public wxStringProperty {
public:
    wxDropPatternProperty(const wxString& label = wxPG_LABEL,
                          const wxString& name = wxPG_LABEL,
                          const wxString& value = wxEmptyString) :
        wxStringProperty(label, name, value) {
    }
    virtual bool ValidateValue(wxVariant& value, wxPGValidationInfo& validationInfo) const override;
};

class Model : public BaseObject {
    friend class LayoutPanel;
    friend class SubModel;

public:
    enum class PIXEL_STYLE {
        PIXEL_STYLE_SQUARE,
        PIXEL_STYLE_SMOOTH,
        PIXEL_STYLE_SOLID_CIRCLE,
        PIXEL_STYLE_BLENDED_CIRCLE
    };

    static wxArrayString CONTROLLER_COLORORDER;

    Model(const ModelManager& manager);
    virtual ~Model();
    static wxArrayString GetLayoutGroups(const ModelManager& mm);
    static std::string SafeModelName(const std::string& name) {
        wxString n(Trim(name).c_str());
        n.Replace(",", "", true);
        n.Replace("~", "", true);
        n.Replace("!", "", true);
        n.Replace(";", "", true);
        n.Replace("<", "", true);
        n.Replace(">", "", true);
        n.Replace("\"", "", true);
        n.Replace("\'", "", true);
        n.Replace("&", "", true);
        n.Replace(":", "", true);
        n.Replace("|", "", true);
        // n.Replace("#", "", true);
        n.Replace("@", "", true);
        n.Replace("/", "", true);
        n.Replace("\\", "", true);
        n.Replace("\t", "", true);
        n.Replace("\r", "", true);
        n.Replace("\n", "", true);
        // Other characters I could remove
        // $%^*()?|][{}`.
        return n.ToStdString();
    }

    [[nodiscard]] bool RenameController(const std::string& oldName, const std::string& newName);
    [[nodiscard]] virtual std::string GetFullName() const {
        return name;
    }
    void Rename(std::string const& newName);
    [[nodiscard]] int GetNumStrings() const {
        return parm1;
    }
    [[nodiscard]] PIXEL_STYLE GetPixelStyle() const {
        return _pixelStyle;
    }
    void SetPixelStyle(PIXEL_STYLE style);
    [[nodiscard]] static std::string GetPixelStyleDescription(PIXEL_STYLE pixelStyle);
    [[nodiscard]] virtual int GetNumPhysicalStrings() const;
    ControllerCaps* GetControllerCaps() const;
    Controller* GetController() const;

    [[nodiscard]] std::string GetModelStartChannel() const {
        return ModelStartChannel;
    }
    [[nodiscard]] const std::string GetStartSide() const {
        return _startSide;
    }
    [[nodiscard]] const std::string GetDirection() const {
        return _dir;
    }
    [[nodiscard]] long GetParm1() const {
        return parm1;
    }
    [[nodiscard]] long GetParm2() const {
        return parm2;
    }
    [[nodiscard]] long GetParm3() const {
        return parm3;
    }
    [[nodiscard]] int GetTransparency() const {
        return transparency;
    }
    [[nodiscard]] const std::string GetNodeNames() const {
        return _nodeNamesString;
    }
    [[nodiscard]] const std::string GetStrandNames() const {
        return _strandNamesString;
    }

    void SetDirection(const std::string dir) {
        _dir = dir;
    }
    void SetStartSide(const std::string start_side) {
        _startSide = start_side;
    }

    [[nodiscard]] virtual bool SupportsChangingStringCount() const {
        return false;
    };
    [[nodiscard]] virtual bool ChangeStringCount(long count, std::string& message) {
        return false;
    };

    std::string description;
    xlColor customColor;
    DimmingCurve* modelDimmingCurve = nullptr;
    int _controller = 0; // this is used to pass the selected controller name between property create and property change only

    [[nodiscard]] int GetPixelSize() const {
        return pixelSize;
    }
    void SetPixelSize(int size);
    void SetTransparency(int t);
    void SetBlackTransparency(int t);
    void ApplyDimensions(const std::string& units, float width, float height, float depth, float& min_x, float& max_x, float& min_y, float& max_y);
    void ExportDimensions(wxFile& f) const;

    [[nodiscard]] virtual bool AllNodesAllocated() const {
        return true;
    }
    static void ParseFaceInfo(wxXmlNode* fiNode, FaceStateData& faceInfo);
    static void WriteFaceInfo(wxXmlNode* fiNode, const FaceStateData& faceInfo);
    wxString SerialiseFace() const;
    wxString SerialiseState() const;
    wxString SerialiseGroups() const;
    wxString SerialiseConnection() const;
    void AddModelGroups(wxXmlNode* n, int w, int h, const wxString& name, bool& merge, bool& ask);

    void UpdateFaceInfoNodes();
    void UpdateStateInfoNodes();

    static void ParseStateInfo(wxXmlNode* fiNode, FaceStateData& stateInfo);
    static void WriteStateInfo(wxXmlNode* fiNode, const FaceStateData& stateInfo, bool customColours = false);

    [[nodiscard]] virtual FaceStateData const& GetFaceInfo() const {
        return faceInfo;
    };
    [[nodiscard]] virtual FaceStateNodes const& GetFaceInfoNodes() const {
        return faceInfoNodes;
    };
    [[nodiscard]] virtual FaceStateData const& GetStateInfo() const {
        return stateInfo;
    };
    [[nodiscard]] virtual FaceStateNodes const& GetStateInfoNodes() const {
        return stateInfoNodes;
    };

    virtual void SetFaceInfo(FaceStateData const& info) {
        faceInfo = info;
    };
    virtual void SetFaceInfoNodes(FaceStateNodes const& nodes) {
        faceInfoNodes = nodes;
    };
    virtual void SetStateInfo(FaceStateData const& info) {
        stateInfo = info;
    };
    virtual void SetStateInfoNodes(FaceStateNodes const& nodes) {
        stateInfoNodes = nodes;
    };

    void AddFace(wxXmlNode* n);
    void AddState(wxXmlNode* n);
    void AddSubmodel(wxXmlNode* n);
    void AddModelAliases(wxXmlNode* n);
    void ImportExtraModels(wxXmlNode* n, xLightsFrame* xlights, ModelPreview* modelPreview, const std::string& layoutGroup);
    [[nodiscard]] Model* CreateDefaultModelFromSavedModelNode(Model* model, ModelPreview* modelPreview, wxXmlNode* node, xLightsFrame* xlights, const std::string& startChannel, bool& cancelled) const;

    [[nodiscard]] wxString SerialiseSubmodel() const;
    [[nodiscard]] wxString SerialiseAliases() const;
    [[nodiscard]] virtual wxString CreateBufferAsSubmodel() const;
    bool importAliases = false;
    bool skipImportAliases = false;

    [[nodiscard]] std::map<std::string, std::map<std::string, std::string>> GetDimmingInfo() const;
    [[nodiscard]] virtual std::list<std::string> CheckModelSettings() override;
    [[nodiscard]] virtual const std::vector<std::string>& GetBufferStyles() const {
        return DEFAULT_BUFFER_STYLES;
    };
    [[nodiscard]] virtual const std::string AdjustBufferStyle(const std::string& style) const;
    virtual void GetBufferSize(const std::string& type, const std::string& camera, const std::string& transform, int& BufferWi, int& BufferHi, int& BufferDp, int stagger) const;
    virtual void InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform,
                                       std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int& BufferDp, int stagger, bool deep = false) const;
    void InitRenderBufferNodes3DFromScreenCoords(std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int& BufferDp) const;
    [[nodiscard]] const ModelManager& GetModelManager() const {
        return modelManager;
    }
    [[nodiscard]] virtual bool SupportsXlightsModel() {
        return false;
    }
    [[nodiscard]] static Model* GetXlightsModel(Model* model, std::string& last_model, xLightsFrame* xlights, bool& cancelled, bool download, wxProgressDialog* prog, int low, int high, ModelPreview* modelPreview);
    [[nodiscard]] bool ImportXlightsModel(std::string const& filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y);
    [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) {
        return true;
    }
    virtual void ExportXlightsModel() {
    }
    virtual void ImportModelChildren(wxXmlNode* root, xLightsFrame* xlights, wxString const& newname, float& min_x, float& max_x, float& min_y, float& max_y);
    [[nodiscard]] bool FourChannelNodes() const;
    [[nodiscard]] bool FiveChannelNodes() const;
    [[nodiscard]] std::list<std::string> GetShadowedBy() const;

    void SetStartChannel(std::string const& startChannel);
    void ReloadModelXml() override {
        GetModelScreenLocation().Reload();
        SetFromXml(ModelXml, zeroBased);
    }

    static const std::vector<std::string> DEFAULT_BUFFER_STYLES;

    [[nodiscard]] int GetDefaultBufferWi() const {
        return BufferWi;
    }
    [[nodiscard]] int GetDefaultBufferHt() const {
        return BufferHt;
    }
    [[nodiscard]] virtual bool IsDMXModel() const {
        return false;
    }

    void SetProperty(wxString const& property, wxString const& value, bool apply = false);
    virtual void AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    void GetSerialProtocolSpeeds(const std::string& protocol, wxArrayString& cp, int& idx) const;
    void GetControllerProtocols(wxArrayString& cp, int& idx);
    virtual void AddControllerProperties(wxPropertyGridInterface* grid);
    virtual void UpdateControllerProperties(wxPropertyGridInterface* grid);
    virtual void DisableUnusedProperties(wxPropertyGridInterface* grid){};
    virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override{};
    virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override{};
    virtual void AddSizeLocationProperties(wxPropertyGridInterface* grid) override;
    virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override;
    virtual void OnPropertyGridChanging(wxPropertyGridInterface* grid, wxPropertyGridEvent& event){};
    [[nodiscard]] virtual int OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
        return 0;
    };
    virtual void OnPropertyGridItemCollapsed(wxPropertyGridInterface* grid, wxPropertyGridEvent& event){};
    virtual void OnPropertyGridItemExpanded(wxPropertyGridInterface* grid, wxPropertyGridEvent& event){};
    [[nodiscard]] virtual std::string GetDimension() const override;
    [[nodiscard]] virtual bool IsNodeFirst(int n) const {
        return n == 0;
    }
    /**
     * Returns a combination of:
     *     0x0001  -  Refresh displays
     *     0x0002  -  Mark settings as "dirty"/"changed"
     *     0x0004  -  Rebuild the property grid
     *     0x0008  -  Rebuild the model list
     *     0x0010  -  Update all model lists
     */
    [[nodiscard]] virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);
    [[nodiscard]] virtual const ModelScreenLocation& GetModelScreenLocation() const = 0;
    [[nodiscard]] virtual ModelScreenLocation& GetModelScreenLocation() = 0;
    [[nodiscard]] bool HasIndividualStartChannels() const;
    [[nodiscard]] wxString GetIndividualStartChannel(size_t s) const;

    [[nodiscard]] bool IsNodeInBufferRange(size_t nodeNum, int x1, int y1, int x2, int y2);

    static void ApplyTransparency(xlColor& color, int transparency, int blackTransparency);

protected:
    void AdjustStringProperties(wxPropertyGridInterface* grid, int newNum);
    [[nodiscard]] std::string ComputeStringStartChannel(int x);
    void ApplyTransform(const std::string& transform,
                        std::vector<NodeBaseClassPtr>& Nodes,
                        int& bufferWi, int& bufferHi, int& bufferDp, int startNode = 0) const;
    void AdjustForTransform(const std::string& transform,
                            int& bufferWi, int& bufferHi, int& bufferDp) const;
    void DumpBuffer(std::vector<NodeBaseClassPtr>& newNodes, int bufferWi, int bufferHi, int bufferDp) const;

    // size of the default buffer
    int BufferHt = 0;
    int BufferWi = 0;
    int BufferDp = 0;
    std::vector<NodeBaseClassPtr> Nodes;
    const ModelManager& modelManager;

    [[nodiscard]] int FindNodeAtXY(int bufx, int bufy);
    virtual void InitModel() {
    }
    [[nodiscard]] virtual int CalcCannelsPerString();
    virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString);
    void RecalcStartChannels();

    void SetBufferSize(int NewHt, int NewWi);
    void SetNodeCount(size_t NumStrings, size_t NodesPerString, const std::string& rgbOrder);
    void CopyBufCoord2ScreenCoord();

    [[nodiscard]] bool FindCustomModelScale(int scale) const;

    [[nodiscard]] wxString ExportSuperStringColors() const;
    void ImportSuperStringColours(wxXmlNode* root);

    void SetLineCoord();
    [[nodiscard]] std::string GetNextName();

    PIXEL_STYLE _pixelStyle = PIXEL_STYLE::PIXEL_STYLE_SMOOTH;
    int pixelSize = 2;
    int transparency = 0;
    int blackTransparency = 0;
    wxColour modelTagColour = *wxBLACK;
    uint8_t _lowDefFactor = 100;
    std::string _startSide = "B";
    std::string _dir = "L";

    int StrobeRate = 0; // 0 = no strobing
    bool zeroBased = false;

    std::vector<std::string> strandNames;
    std::vector<std::string> nodeNames;
    std::string _nodeNamesString;
    std::string _strandNamesString;
    long parm1 = 0;     /* Number of strings in the model or number of arches or canes (except for frames & custom) */
    long parm2 = 0;     /* Number of nodes per string in the model or number of segments per arch or cane (except for frames & custom) */
    long parm3 = 0;     /* Number of strands per string in the model or number of lights per arch or cane segment (except for frames & custom) */
    bool IsLtoR = true; // true = left to right, false = right to left
    std::vector<int32_t> stringStartChan;
    bool isBotToTop = true;
    std::string StringType; // RGB Nodes, 3 Channel RGB, Single Color Red, Single Color Green, Single Color Blue, Single Color White
    int rgbwHandlingType = 0;
    std::vector<xlColor> superStringColours;

    std::vector<Model*> subModels;
    void ParseSubModel(wxXmlNode* subModelNode);
    void ColourClashingChains(wxPGProperty* p);
    [[nodiscard]] uint32_t ApplyLowDefinition(uint32_t val) const;

    FaceStateData faceInfo;
    FaceStateNodes faceInfoNodes;
    FaceStateData stateInfo;
    FaceStateNodes stateInfoNodes;

public:
    [[nodiscard]] bool IsControllerConnectionValid() const;
    [[nodiscard]] wxXmlNode* GetControllerConnection() const;
    [[nodiscard]] std::string GetControllerConnectionString() const;
    [[nodiscard]] std::string GetControllerConnectionRangeString() const;
    [[nodiscard]] std::string GetControllerConnectionPortRangeString() const;
    [[nodiscard]] std::string GetControllerConnectionAttributeString() const;
    void ReplaceIPInStartChannels(const std::string& oldIP, const std::string& newIP);
    [[nodiscard]] static std::string DecodeSmartRemote(int sr);

    void SetTagColour(wxColour colour);
    [[nodiscard]] wxColour GetTagColour() const {
        return modelTagColour;
    }
    [[nodiscard]] int32_t GetStringStartChan(int x) const;
    void SaveSuperStringColours();
    void SetSuperStringColours(int count);
    void SetSuperStringColour(int index, xlColor c);
    void AddSuperStringColour(xlColor c, bool saveToXml = true);
    void SetShadowModelFor(const std::string& shadowFor);
    void SetControllerName(const std::string& controllerName);
    void SetControllerProtocol(const std::string& protocol);
    void SetControllerSerialProtocolSpeed(int speed);
    void SetControllerPort(int port);
    void SetControllerBrightness(int brightness);
    void ClearControllerBrightness();
    [[nodiscard]] bool IsControllerBrightnessSet() const;
    [[nodiscard]] bool IsShadowModel() const;
    [[nodiscard]] std::string GetShadowModelFor() const;
    [[nodiscard]] std::string GetControllerName() const;
    [[nodiscard]] std::string GetControllerProtocol() const;
    [[nodiscard]] int GetControllerProtocolSpeed() const;
    [[nodiscard]] int GetControllerBrightness() const;
    [[nodiscard]] int GetControllerDMXChannel() const;
    [[nodiscard]] int GetSmartRemote() const;
    [[nodiscard]] bool GetSRCascadeOnPort() const;
    [[nodiscard]] int GetSRMaxCascade() const;
    [[nodiscard]] std::vector<std::string> GetSmartRemoteTypes() const;
    [[nodiscard]] std::string GetSmartRemoteType() const;
    [[nodiscard]] int GetSmartRemoteTypeIndex(const std::string& srType) const;
    [[nodiscard]] std::string GetSmartRemoteTypeName(int idx) const;
    [[nodiscard]] int GetSmartRemoteCount() const;
    [[nodiscard]] int GetControllerStartNulls() const;
    [[nodiscard]] int GetControllerEndNulls() const;
    [[nodiscard]] wxString GetControllerColorOrder() const;
    [[nodiscard]] int GetControllerGroupCount() const;
    [[nodiscard]] float GetControllerGamma() const;
    [[nodiscard]] virtual bool Supports3DBuffer() const {
        return false;
    }
    void SetControllerStartNulls(int nulls);
    void SetControllerEndNulls(int nulls);
    void SetControllerColorOrder(wxString const& color);
    void SetControllerGroupCount(int grouping);
    void SetControllerGamma(float gamma);

    [[nodiscard]] bool IsAlias(const std::string& alias, bool oldnameOnly = false) const;
    void AddAlias(const std::string& alias);
    void DeleteAlias(const std::string& alias);
    [[nodiscard]] std::list<std::string> GetAliases() const;
    void SetAliases(std::list<std::string>& aliases);

    void GetPortSR(int string, int& outport, int& outsr) const;
    [[nodiscard]] char GetSmartRemoteLetter() const;
    [[nodiscard]] char GetSmartRemoteLetterForString(int string = 1) const;
    [[nodiscard]] int GetSortableSmartRemote() const;
    [[nodiscard]] int GetSmartTs() const;
    [[nodiscard]] int GetSmartRemoteForString(int string = 1) const;
    [[nodiscard]] int GetControllerPort(int string = 1) const;
    void SetModelChain(const std::string& modelChain);
    void SetSmartRemote(int sr);
    void SetSRCascadeOnPort(bool cascade);
    void SetSRMaxCascade(int max);
    void SetSmartRemoteType(const std::string& type);
    void SetControllerDMXChannel(int ch);
    [[nodiscard]] std::string GetModelChain() const;
    [[nodiscard]] const std::vector<Model*>& GetSubModels() const {
        return subModels;
    }
    [[nodiscard]] Model* GetSubModel(const std::string& name) const;
    [[nodiscard]] std::string GenerateUniqueSubmodelName(const std::string suggested) const;
    [[nodiscard]] int GetNumSubModels() const {
        return subModels.size();
    }
    [[nodiscard]] Model* GetSubModel(int i) const {
        return i < (int)subModels.size() ? subModels[i] : nullptr;
    }
    void RemoveSubModel(const std::string& name);
    [[nodiscard]] std::list<int> ParseFaceNodes(std::string channels);

    [[nodiscard]] bool IsPixelProtocol() const;
    [[nodiscard]] bool IsSerialProtocol() const;
    [[nodiscard]] bool IsMatrixProtocol() const;
    [[nodiscard]] bool IsLEDPanelMatrixProtocol() const;
    [[nodiscard]] bool IsVirtualMatrixProtocol() const;
    [[nodiscard]] bool IsPWMProtocol() const;

    [[nodiscard]] virtual std::vector<PWMOutput> GetPWMOutputs() const;

    [[nodiscard]] static wxArrayString GetSmartRemoteValues(int smartRemoteCount);

    [[nodiscard]] unsigned long GetChangeCount() const {
        return changeCount;
    }

    std::string rgbOrder;
    bool SingleNode = false;    // true for dumb strings and single channel strings
    bool SingleChannel = false; // true for traditional single-color strings

    std::string ModelStartChannel{ "" };
    bool CouldComputeStartChannel = false;
    bool Overlapping = false;
    std::string _pixelCount{ "" };
    std::string _pixelType{ "" };
    std::string _pixelSpacing{ "" };

    void SetFromXml(wxXmlNode* ModelNode, bool zeroBased = false) override;
    [[nodiscard]] virtual bool ModelRenamed(const std::string& oldName, const std::string& newName);
    [[nodiscard]] uint32_t GetNodeCount() const;
    [[nodiscard]] NodeBaseClass* GetNode(uint32_t node) const;
    [[nodiscard]] uint32_t GetChanCount() const;
    [[nodiscard]] uint32_t GetActChanCount() const;
    [[nodiscard]] int GetChanCountPerNode() const;
    [[nodiscard]] uint32_t GetCoordCount(size_t nodenum) const;
    [[nodiscard]] int GetNodeStringNumber(size_t nodenum) const;
    void UpdateXmlWithScale() override;
    void SetPosition(double posx, double posy);
    [[nodiscard]] std::string GetChannelInStartChannelFormat(OutputManager* outputManager, uint32_t channel);
    [[nodiscard]] std::string GetLastChannelInStartChannelFormat(OutputManager* outputManager);
    [[nodiscard]] std::string GetFirstChannelInStartChannelFormat(OutputManager* outputManager);
    [[nodiscard]] std::string GetStartChannelInDisplayFormat(OutputManager* outputManager);
    [[nodiscard]] bool IsValidStartChannelString() const;
    [[nodiscard]] virtual uint32_t GetFirstChannel() const;
    [[nodiscard]] virtual uint32_t GetLastChannel() const;
    [[nodiscard]] uint32_t GetNumChannels() const;
    [[nodiscard]] uint32_t GetNodeNumber(size_t nodenum) const;
    [[nodiscard]] uint32_t GetNodeNumber(int bufY, int bufX) const;
    [[nodiscard]] bool UpdateStartChannelFromChannelString(std::map<std::string, Model*>& models, std::list<std::string>& used);
    [[nodiscard]] int GetNumberFromChannelString(const std::string& sc) const;
    [[nodiscard]] int GetNumberFromChannelString(const std::string& sc, bool& valid, std::string& dependsonmodel) const;

    virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx,
                                      xlGraphicsProgram* solidProgram, xlGraphicsProgram* transparentProgram, bool is_3d = false,
                                      const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                      bool highlightFirst = false, int highlightpixel = 0,
                                      float* boundingBox = nullptr);
    virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize);

    [[nodiscard]] virtual int NodeRenderOrder() {
        return 0;
    }
    [[nodiscard]] float GetPreviewDimScale(ModelPreview* preview, int& w, int& h);
    void GetScreenLocation(float& sx, float& sy, const NodeBaseClass::CoordStruct& it2, int w, int h, float scale);
    [[nodiscard]] bool GetScreenLocations(ModelPreview* preview, std::map<int, std::pair<float, float>>& coords);
    [[nodiscard]] wxString GetNodeNear(ModelPreview* preview, wxPoint pt, bool flip);
    [[nodiscard]] std::vector<int> GetNodesInBoundingBox(ModelPreview* preview, wxPoint start, wxPoint end);
    [[nodiscard]] bool IsMultiCoordsPerNode() const;

    [[nodiscard]] virtual bool CleanupFileLocations(xLightsFrame* frame) override;
    [[nodiscard]] std::list<std::string> GetFaceFiles(const std::list<std::string>& facesUsed, bool all = false, bool includeFaceName = false) const;
    [[nodiscard]] glm::vec3 MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY);
    [[nodiscard]] int GetSelectedHandle();
    [[nodiscard]] int GetNumHandles();
    [[nodiscard]] int GetSelectedSegment();
    [[nodiscard]] bool SupportsCurves();
    [[nodiscard]] bool HasCurve(int segment);
    void SetCurve(int segment, bool create);
    void AddHandle(ModelPreview* preview, int mouseX, int mouseY);
    virtual void InsertHandle(int after_handle, float zoom, int scale);
    virtual void DeleteHandle(int handle);

    [[nodiscard]] bool HasState(std::string const& state) const;

    [[nodiscard]] bool HitTest(ModelPreview* preview, glm::vec3& ray_origin, glm::vec3& ray_direction);
    [[nodiscard]] const std::string& GetStringType() const {
        return StringType;
    }
    [[nodiscard]] virtual int NodesPerString() const;
    [[nodiscard]] virtual int NodesPerString(int string) const {
        return NodesPerString();
    }
    [[nodiscard]] virtual int MapPhysicalStringToLogicalString(int string) const {
        return string;
    }
    [[nodiscard]] virtual int GetLightsPerNode() const {
        return 1;
    } // default to one unless a model supports this
    [[nodiscard]] wxCursor InitializeLocation(int& handle, wxCoord x, wxCoord y, ModelPreview* preview);

    [[nodiscard]] int32_t NodeStartChannel(size_t nodenum) const;
    [[nodiscard]] int32_t NodeEndChannel(size_t nodenum) const;
    [[nodiscard]] const std::string& NodeType(size_t nodenum) const;
    [[nodiscard]] virtual int MapToNodeIndex(int strand, int node) const;

    void GetNodeChannelValues(size_t nodenum, unsigned char* buf);
    void SetNodeChannelValues(size_t nodenum, const unsigned char* buf);
    [[nodiscard]] xlColor GetNodeColor(size_t nodenum) const;
    [[nodiscard]] virtual const xlColor& GetNodeMaskColor(size_t nodenum) const;
    void SetNodeColor(size_t nodenum, const xlColor& c);
    [[nodiscard]] wxChar GetChannelColorLetter(wxByte chidx);
    [[nodiscard]] std::string GetRGBOrder() const {
        return rgbOrder;
    }
    [[nodiscard]] static char EncodeColour(const xlColor& c);
    [[nodiscard]] char GetAbsoluteChannelColorLetter(int32_t absoluteChannel); // absolute channel may or may not be in this model ... in which case a ' ' is returned
    [[nodiscard]] std::string GetControllerPortSortString() const;

    [[nodiscard]] virtual std::string ChannelLayoutHtml(OutputManager* outputManager);
    virtual void ExportAsCustomXModel() const;
    [[nodiscard]] virtual std::string GetStartLocation() const;
    [[nodiscard]] bool IsCustom();
    [[nodiscard]] virtual bool SupportsExportAsCustom() const = 0;
    [[nodiscard]] virtual bool SupportsExportAsCustom3D() const {
        return false;
    }
    virtual void ExportAsCustomXModel3D() const {
    }
    [[nodiscard]] virtual bool SupportsWiringView() const = 0;
    [[nodiscard]] size_t GetChannelCoords(wxArrayString& choices); // wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3);
    [[nodiscard]] static bool ParseFaceElement(const std::string& str, std::vector<wxPoint>& first_xy);
    [[nodiscard]] static bool ParseStateElement(const std::string& str, std::vector<wxPoint>& first_xy);
    [[nodiscard]] virtual bool SupportsLowDefinitionRender() const {
        return false;
    }
    [[nodiscard]] std::string GetNodeXY(const std::string& nodenumstr);
    [[nodiscard]] std::string GetNodeXY(int nodeinx);

    void GetNodeCoords(int nodeidx, std::vector<wxPoint>& pts);
    void GetNode3DScreenCoords(int nodeidx, std::vector<std::tuple<float, float, float>>& pts);

    [[nodiscard]] bool GetIsLtoR() const {
        return IsLtoR;
    }
    [[nodiscard]] bool GetIsBtoT() const {
        return isBotToTop;
    }
    [[nodiscard]] virtual int GetStrandLength(int strand) const;

    float _savedWidth{ 0.0F };
    float _savedHeight{ 0.0F };
    float _savedDepth{ 0.0F };
    void SaveDisplayDimensions();
    void RestoreDisplayDimensions();

    void ClearIndividualStartChannels();

    void GetMinScreenXY(float& minx, float& miny) const;
    [[nodiscard]] virtual int GetNumStrands() const {
        return 1;
    }
    [[nodiscard]] std::string GetStrandName(size_t x, bool def = false) const {
        if (x < strandNames.size()) {
            return strandNames[x];
        }
        if (def) {
            return std::string("Strand ") + std::to_string(x + 1);
        }
        return "";
    }

    [[nodiscard]] virtual int GetMappedStrand(int strand) const {
        return strand;
    }

    [[nodiscard]] virtual std::string GetNodeName(size_t x, bool def = false) const {
        if (x < nodeNames.size()) {
            return nodeNames[x];
        }
        if (def) {
            return std::string("Node ") + std::to_string(x + 1);
        }
        return "";
    }

    [[nodiscard]] static std::string StartChanAttrName(int idx) {
        return std::string("String") + std::to_string(idx + 1); // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
    }
    // returns true for models that only have 1 string and where parm1 does NOT represent the # of strings
    [[nodiscard]] static bool HasOneString(const std::string& DispAs) {
        return (DispAs == "Window Frame" || DispAs == "Custom" || DispAs == "Cube");
    }
    // true for dumb strings and traditional strings
    [[nodiscard]] bool HasSingleNode(const std::string& StrType) const {
        if (StrType == "Node Single Color")
            return false;
        if (StrType == "Superstring")
            return true;
        static std::string Nodes(" Nodes");
        if (Nodes.size() > StrType.size())
            return false;
        return StrType.find(Nodes) == std::string::npos;
    }
    // true for traditional strings
    [[nodiscard]] /*static */ bool HasSingleChannel(const std::string& StrType) const {
        return GetNodeChannelCount(StrType) == 1 && StrType != "Node Single Color";
    }
    [[nodiscard]] /*static */ size_t GetNodeChannelCount(const std::string& nodeType) const;

    // Methods to support layer sizes
    int layerSizeMenu{ -1 }; // when a layer size is right clicked on this holds the layer that was clicked on
    [[nodiscard]] virtual bool ModelSupportsLayerSizes() const {
        return false;
    }
    [[nodiscard]] std::vector<int> GetLayerSizes() const {
        return layerSizes;
    }
    void SetLayerSizeCount(int count) {
        size_t oldCount = layerSizes.size();
        layerSizes.resize(count);
        // If it has grown initialise everything to 1
        for (size_t i = oldCount; i < layerSizes.size(); i++) {
            layerSizes[i] = 1;
        }
    }
    [[nodiscard]] size_t GetLayerSizesTotalNodes() const {
        size_t count = 0;
        for (const auto it : layerSizes) {
            count += it;
        }
        return count;
    }
    void DeleteLayerSize(size_t layer) {
        if (GetLayerSizeCount() <= layer)
            return;
        auto layers = layerSizes;
        layerSizes.resize(0);
        for (size_t i = 0; i < layer; i++) {
            layerSizes.push_back(layers[i]);
        }
        for (size_t i = layer + 1; i < layers.size(); i++) {
            layerSizes.push_back(layers[i]);
        }
    }
    void InsertLayerSizeBefore(size_t layer) {
        auto layers = layerSizes;
        layerSizes.resize(0);
        for (size_t i = 0; i < layer; i++) {
            layerSizes.push_back(layers[i]);
        }
        layerSizes.push_back(1);
        for (size_t i = layer; i < layers.size(); i++) {
            layerSizes.push_back(layers[i]);
        }
    }
    size_t GetLayerSizeCount() const {
        return layerSizes.size();
    }
    void SetLayerSize(int layer, int size) {
        if (GetLayerSizeCount() > layer && size != 0) {
            layerSizes[layer] = size;
        }
        // else wxASSERT(false);
    }
    [[nodiscard]] int GetLayerSize(int layer) const {
        if (GetLayerSizeCount() > layer) {
            return layerSizes[layer];
        }
        wxASSERT(false);
        return 0;
    }
    void AddLayerSizeProperty(wxPropertyGridInterface* grid);
    [[nodiscard]] bool HandleLayerSizePropertyChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);
    [[nodiscard]] std::string SerialiseLayerSizes() const {
        std::string res;
        for (const auto it : layerSizes) {
            if (res != "")
                res += ",";
            res += std::to_string(it);
        }
        return res;
    }
    [[nodiscard]] bool ContainsChannel(uint32_t startChannel, uint32_t endChannel) const;
    [[nodiscard]] bool ContainsChannel(int strand, uint32_t startChannel, uint32_t endChannel) const;
    [[nodiscard]] bool ContainsChannel(const std::string& submodelName, uint32_t startChannel, uint32_t endChannel) const;

    virtual void OnLayerSizesChange(bool countChanged) {
    }
    static const long ID_LAYERSIZE_DELETE;
    static const long ID_LAYERSIZE_INSERT;
    void HandlePropertyGridRightClick(wxPropertyGridEvent& event, wxMenu& mnu) override;
    void HandlePropertyGridContextMenu(wxCommandEvent& event) override;

    // reverse is used for conversion scenarios where the old format was reversed
    void DeserialiseLayerSizes(std::string const& ls, bool reverse) {
        layerSizes.resize(0);
        auto lss = wxSplit(ls, ',');
        if (reverse) {
            for (auto it = lss.rbegin(); it != lss.rend(); ++it) {
                int l = wxAtoi(*it);
                if (l > 0)
                    layerSizes.push_back(l);
            }
        } else {
            for (const auto& it : lss) {
                int l = wxAtoi(it);
                if (l > 0)
                    layerSizes.push_back(l);
            }
        }
    }
    [[nodiscard]] uint32_t GetChannelForNode(int strandIndex, int node) const;

    [[nodiscard]] std::string GetAttributesAsJSON() const;

protected:
    std::vector<int> layerSizes; // inside to outside
    unsigned int maxVertexCount = 0;

    class PreviewGraphicsCacheInfo {
    public:
        PreviewGraphicsCacheInfo() {
        }
        virtual ~PreviewGraphicsCacheInfo();
        xlGraphicsProgram* program = nullptr;
        xlVertexIndexedColorAccumulator* vica = nullptr;
        xlVertexColorAccumulator* vca = nullptr;
        xlVertexAccumulator* va = nullptr;

        int width = 0;
        int height = 0;
        int renderWi = 0;
        int renderHi = 0;
        int modelChangeCount = 0;
        bool isTransparent = false;
        float boundingBox[6] = { 0 };
    };
    std::map<std::string, PreviewGraphicsCacheInfo*> uiCaches;
    virtual void deleteUIObjects();
};

template<class ScreenLocation>
class ModelWithScreenLocation : public Model {
public:
    [[nodiscard]] virtual const ModelScreenLocation& GetModelScreenLocation() const {
        return screenLocation;
    }
    [[nodiscard]] virtual ModelScreenLocation& GetModelScreenLocation() {
        return screenLocation;
    }
    [[nodiscard]] virtual const ModelScreenLocation& GetBaseObjectScreenLocation() const {
        return screenLocation;
    }
    [[nodiscard]] virtual ModelScreenLocation& GetBaseObjectScreenLocation() {
        return screenLocation;
    }

protected:
    ModelWithScreenLocation(const ModelManager& manager) :
        Model(manager) {
    }
    virtual ~ModelWithScreenLocation() {
    }
    ScreenLocation screenLocation;
};
