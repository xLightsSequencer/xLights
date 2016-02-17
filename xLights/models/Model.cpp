#include "Model.h"

#include <wx/xml/xml.h>
#include <wx/tokenzr.h>

#include "../xLightsMain.h" //for Preview and Other model collections
#include "../Color.h"
#include "../DrawGLUtils.h"
#include "../DimmingCurve.h"


#define RECT_HANDLE_WIDTH           6
#define BOUNDING_RECT_OFFSET        8

static inline void TranslatePointDoubles(double radians,double x, double y,double &x1, double &y1) {
    x1 = cos(radians)*x-(sin(radians)*y);
    y1 = sin(radians)*x+(cos(radians)*y);
}

inline double toRadians(long degrees) {
    return 2.0*M_PI*double(degrees)/360.0;
}

inline long toDegrees(double radians) {
    return (radians/(2*M_PI))*360.0;
}

const std::vector<std::string> Model::DEFAULT_BUFFER_STYLES {"Default", "Per Preview", "Rotate CC 90",
    "Rotate CW 90", "Rotate 180", "Flip Vertical", "Flip Horizontal", "Single Line"};

Model::Model() : modelDimmingCurve(nullptr) {
}

Model::~Model() {
    if (modelDimmingCurve != nullptr) {
        delete modelDimmingCurve;
    }
}

bool Model::IsMyDisplay(wxXmlNode* ModelNode)
{
    return ModelNode->GetAttribute(wxT("MyDisplay"),wxT("0")) == wxT("1");
}
void Model::SetMyDisplay(wxXmlNode* ModelNode,bool NewValue)
{
    ModelNode->DeleteAttribute(wxT("MyDisplay"));
    ModelNode->AddAttribute(wxT("MyDisplay"), NewValue ? wxT("1") : wxT("0"));
}

int Model::GetNumStrands() const {
    wxStringTokenizer tkz(DisplayAs, " ");
    wxString token = tkz.GetNextToken();
    if (DisplayAs == wxT("Vert Matrix") || DisplayAs == wxT("Horiz Matrix")) {
        if (SingleChannel) {
            return 1;
        }
        return parm1*parm3;
    } else
        return 1;
}



wxXmlNode* Model::GetModelXml() const {
    return this->ModelXml;
}

int Model::GetNumberFromChannelString(std::string sc) {
    int output = 1;
    if (sc.find(":") != std::string::npos) {
        output = wxAtoi(sc.substr(0, sc.find(":")));
        sc = sc.substr(sc.find(":") + 1);
    }
    int returnChannel = wxAtoi(sc);
    if (output > 1) {
        returnChannel = ModelNetInfo->CalcAbsChannel(output - 1, returnChannel - 1) + 1;
    }
    return returnChannel;
}

void Model::SetFromXml(wxXmlNode* ModelNode, const NetInfoClass &netInfo, bool zb) {


    wxString tempstr,channelstr;
    long degrees, StartChannel;

    zeroBased = zb;
    ModelXml=ModelNode;
    ModelNetInfo = &netInfo;
    StrobeRate=0;
    Nodes.clear();

    isMyDisplay = IsMyDisplay(ModelNode);

    name=ModelNode->GetAttribute("name").ToStdString();
    DisplayAs=ModelNode->GetAttribute("DisplayAs").ToStdString();
    StringType=ModelNode->GetAttribute("StringType").ToStdString();
    SingleNode=HasSingleNode(StringType);
    SingleChannel=HasSingleChannel(StringType);
    rgbOrder = SingleNode ? "RGB" : StringType.substr(0, 3);

    tempstr=ModelNode->GetAttribute("parm1");
    tempstr.ToLong(&parm1);
    tempstr=ModelNode->GetAttribute("parm2");
    tempstr.ToLong(&parm2);
    tempstr=ModelNode->GetAttribute("parm3");
    tempstr.ToLong(&parm3);
    tempstr=ModelNode->GetAttribute("StrandNames");
    strandNames.clear();
    while (tempstr.size() > 0) {
        std::string t2 = tempstr.ToStdString();
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        } else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        strandNames.push_back(t2);
    }
    tempstr=ModelNode->GetAttribute("NodeNames");
    nodeNames.clear();
    while (tempstr.size() > 0) {
        std::string t2 = tempstr.ToStdString();
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        } else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        nodeNames.push_back(t2);
    }

    StartChannel = GetNumberFromChannelString(ModelNode->GetAttribute("StartChannel","1").ToStdString());
    tempstr=ModelNode->GetAttribute("Dir");
    IsLtoR=tempstr != "R";
    if (ModelNode->HasAttribute("StartSide")) {
        tempstr=ModelNode->GetAttribute("StartSide");
        isBotToTop = (tempstr == "B");
    } else {
        isBotToTop=true;
    }
    customColor = xlColor(ModelNode->GetAttribute("CustomColor", "#000000").ToStdString());

    long n;
    tempstr=ModelNode->GetAttribute("Antialias","1");
    tempstr.ToLong(&n);
    pixelStyle = n;
    tempstr=ModelNode->GetAttribute("PixelSize","2");
    tempstr.ToLong(&n);
    pixelSize = n;
    tempstr=ModelNode->GetAttribute("Transparency","0");
    tempstr.ToLong(&n);
    transparency = n;
    blackTransparency = wxAtoi(ModelNode->GetAttribute("BlackTransparency","0"));

    MyDisplay=IsMyDisplay(ModelNode);

    tempstr=ModelNode->GetAttribute("offsetXpct","0");
    tempstr.ToDouble(&offsetXpct);
    if(offsetXpct<0 || offsetXpct>1) {
        offsetXpct = .5;
    }
    tempstr=ModelNode->GetAttribute("offsetYpct","0");
    tempstr.ToDouble(&offsetYpct);
    if(offsetYpct<0 || offsetYpct>1) {
        offsetYpct = .5;
    }
    tempstr=ModelNode->GetAttribute("PreviewScale");
    singleScale = false;
    if (tempstr == "") {
        PreviewScaleX = wxAtof(ModelNode->GetAttribute("PreviewScaleX", "0.3333"));
        PreviewScaleY = wxAtof(ModelNode->GetAttribute("PreviewScaleY", "0.3333"));
    } else {
        singleScale = true;
        tempstr.ToDouble(&PreviewScaleX);
        tempstr.ToDouble(&PreviewScaleY);
    }
    if(PreviewScaleX<0 || PreviewScaleX>1) {
        PreviewScaleX = .33;
    }
    if(PreviewScaleY<0 || PreviewScaleY>1) {
        PreviewScaleY = .33;
    }
    tempstr=ModelNode->GetAttribute("PreviewRotation","0");
    tempstr.ToLong(&degrees);

    PreviewRotation=degrees;
    ModelStartChannel = ModelNode->GetAttribute("StartChannel");

    // calculate starting channel numbers for each string
    size_t NumberOfStrings= HasOneString(DisplayAs) ? 1 : parm1;
    int ChannelsPerString=CalcCannelsPerString();

    SetStringStartChannels(zeroBased, NumberOfStrings, StartChannel, ChannelsPerString);

    InitModel();

    SetModelCoord(PreviewRotation);
    size_t NodeCount=GetNodeCount();
    for(size_t i=0; i<NodeCount; i++) {
        Nodes[i]->sparkle = rand() % 10000;
    }

    wxXmlNode *f = ModelNode->GetChildren();
    faceInfo.clear();
    while (f != nullptr) {
        if ("faceInfo" == f->GetName()) {
            std::string name = f->GetAttribute("Name", "SingleNode").ToStdString();
            std::string type = f->GetAttribute("Type", "SingleNode").ToStdString();
            if (name == "") {
                name = type;
                f->DeleteAttribute("Name");
                f->AddAttribute("Name", type);
            }
            if (!(type == "SingleNode" || type == "NodeRange" || type == "Matrix")) {
                if (type == "Coro") {
                    type = "SingleNode";
                } else {
                    type = "Matrix";
                }
                f->DeleteAttribute("Type");
                f->AddAttribute("Type", type);
            }

            wxXmlAttribute *att = f->GetAttributes();
            while (att != nullptr) {
                if (att->GetName() != "Name") {
                    faceInfo[name][att->GetName().ToStdString()] = att->GetValue();
                }
                att = att->GetNext();
            }
        } else if ("dimmingCurve" == f->GetName()) {
            modelDimmingCurve = DimmingCurve::createFromXML(f);
        }
        f = f->GetNext();
    }

    if (ModelNode->HasAttribute("ModelBrightness") && modelDimmingCurve == nullptr) {
        int b = wxAtoi(ModelNode->GetAttribute("ModelBrightness"));
        if (b != 0) {
            modelDimmingCurve = DimmingCurve::createBrightnessGamma(b, 1.0);
        }
    }
}
int Model::CalcCannelsPerString() {
    int ChannelsPerString = parm2*GetNodeChannelCount(StringType);
    if (SingleChannel)
        ChannelsPerString=1;
    else if (SingleNode)
        ChannelsPerString=GetNodeChannelCount(StringType);

    return ChannelsPerString;
}

void Model::SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) {
    std::string tempstr = ModelXml->GetAttribute("Advanced", "0").ToStdString();
    bool HasIndividualStartChans=tempstr == "1";
    stringStartChan.clear();
    stringStartChan.resize(NumberOfStrings);
    for (int i=0; i<NumberOfStrings; i++) {
        tempstr = StartChanAttrName(i);
        if (!zeroBased && HasIndividualStartChans && ModelXml->HasAttribute(tempstr)) {
            stringStartChan[i] = GetNumberFromChannelString(ModelXml->GetAttribute(tempstr, "1").ToStdString())-1;
        } else {
            stringStartChan[i] = (zeroBased? 0 : StartChannel-1) + i*ChannelsPerString;
        }
    }
}

void Model::InitModel() {
    // initialize model based on the DisplayAs value
    wxStringTokenizer tkz(DisplayAs, " ");
    wxString token = tkz.GetNextToken();
    if (DisplayAs == "Vert Matrix") {
        InitVMatrix();
        CopyBufCoord2ScreenCoord();
    } else if (DisplayAs == "Horiz Matrix") {
        InitHMatrix();
        CopyBufCoord2ScreenCoord();
    } else {
        wxMessageBox(DisplayAs + " is not a valid model type for model " + name);
        DisplayAs = "Vert Matrix";
        InitVMatrix();  //init something to avoid some crashes
    }
}


void Model::GetNodeChannelValues(size_t nodenum, unsigned char *buf) {
    Nodes[nodenum]->GetForChannels(buf);
}
void Model::SetNodeChannelValues(size_t nodenum, const unsigned char *buf) {
    Nodes[nodenum]->SetFromChannels(buf);
}
xlColor Model::GetNodeColor(size_t nodenum) const {
    xlColor color;
    Nodes[nodenum]->GetColor(color);
    return color;
}


// only valid for rgb nodes and dumb strings (not traditional strings)
wxChar Model::GetChannelColorLetter(wxByte chidx) {
    return rgbOrder[chidx];
}

int Model::GetLastChannel() {
    int LastChan=0;
    size_t NodeCount=GetNodeCount();
    for(size_t idx=0; idx<NodeCount; idx++) {
        LastChan=std::max(LastChan,Nodes[idx]->ActChan + Nodes[idx]->GetChanCount() - 1);
    }
    return LastChan;
}

void Model::SetOffset(double xPct, double yPct) {
    offsetXpct=xPct;
    offsetYpct=yPct;
}


void Model::AddOffset(double xPct, double yPct) {
    offsetXpct+=xPct;
    offsetYpct+=yPct;
}


void Model::SetScale(double x, double y) {
    PreviewScaleX = x;
    PreviewScaleY = y;
    singleScale = false;
}

void Model::GetScales(double &x, double &y) {
    x = PreviewScaleX;
    y = PreviewScaleY;
}

int Model::GetPreviewRotation() {
    return PreviewRotation;
}
// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void Model::InitVMatrix(int firstExportStrand) {
    int y,x,idx,stringnum,segmentnum,yincr;
    if (parm3 > parm2) {
        parm3 = parm2;
    }
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(PixelsPerStrand,NumStrands);
    SetNodeCount(parm1,PixelsPerString, rgbOrder);
    SetRenderSize(PixelsPerStrand,NumStrands);

    // create output mapping
    if (SingleNode) {
        x=0;
        for (size_t n=0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            y=0;
            yincr=1;
            for (size_t c=0; c<PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX=IsLtoR ? x : NumStrands-x-1;
                Nodes[n]->Coords[c].bufY=y;
                y+=yincr;
                if (y < 0 || y >= PixelsPerStrand) {
                    yincr=-yincr;
                    y+=yincr;
                    x++;
                }
            }
        }
    } else {
        std::vector<int> strandStartChan;
        strandStartChan.clear();
        strandStartChan.resize(NumStrands);
        for (int x = 0; x < NumStrands; x++) {
            stringnum = x / parm3;
            segmentnum = x % parm3;
            strandStartChan[x] = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * 3;
        }
        if (firstExportStrand > 0 && firstExportStrand < NumStrands) {
            int offset = strandStartChan[firstExportStrand];
            for (int x = 0; x < NumStrands; x++) {
                strandStartChan[x] = strandStartChan[x] - offset;
                if (strandStartChan[x] < 0) {
                    strandStartChan[x] += (PixelsPerStrand * NumStrands * 3);
                }
            }
        }

        for (x=0; x < NumStrands; x++) {
            stringnum = x / parm3;
            segmentnum = x % parm3;
            for(y=0; y < PixelsPerStrand; y++) {
                idx=stringnum * PixelsPerString + segmentnum * PixelsPerStrand + y;
                Nodes[idx]->ActChan = strandStartChan[x] + y*3;
                Nodes[idx]->Coords[0].bufX=IsLtoR ? x : NumStrands-x-1;
                Nodes[idx]->Coords[0].bufY= isBotToTop == (segmentnum % 2 == 0) ? y:PixelsPerStrand-y-1;
                Nodes[idx]->StringNum=stringnum;
            }
        }
    }
}


// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void Model::InitHMatrix() {
    int y,x,idx,stringnum,segmentnum,xincr;
    if (parm3 > parm2) {
        parm3 = parm2;
    }
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(NumStrands,PixelsPerStrand);
    SetNodeCount(parm1,PixelsPerString,rgbOrder);
    SetRenderSize(NumStrands,PixelsPerStrand);

    // create output mapping
    if (SingleNode) {
        y=0;
        for (size_t n=0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            x=0;
            xincr=1;
            for (size_t c=0; c<PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX=x;
                Nodes[n]->Coords[c].bufY=isBotToTop ? y :NumStrands-y-1;
                x+=xincr;
                if (x < 0 || x >= PixelsPerStrand) {
                    xincr=-xincr;
                    x+=xincr;
                    y++;
                }
            }
        }
    } else {
        for (y=0; y < NumStrands; y++) {
            stringnum=y / parm3;
            segmentnum=y % parm3;
            for(x=0; x<PixelsPerStrand; x++) {
                idx=stringnum * PixelsPerString + segmentnum * PixelsPerStrand + x;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand*3 + x*3;
                Nodes[idx]->Coords[0].bufX=IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand-x-1 : x;
                Nodes[idx]->Coords[0].bufY= isBotToTop ? y :NumStrands-y-1;
                Nodes[idx]->StringNum=stringnum;
            }
        }
    }
}




// initialize screen coordinates
// parm1=Number of Strings/Arches
// parm2=Pixels Per String/Arch
void Model::SetLineCoord() {
    double x,y;
    float idx=0;
    size_t NodeCount=GetNodeCount();
    int numlights=parm1*parm2;
    double half=numlights/2;
    SetRenderSize(numlights*2,numlights);

    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            x=idx;
            x=IsLtoR ? x - half : half - x;
            y=0;
            Nodes[n]->Coords[c].screenX=x;
            Nodes[n]->Coords[c].screenY=y + numlights;
            idx++;
        }
    }
}



void Model::SetBufferSize(int NewHt, int NewWi) {
    BufferHt=NewHt;
    BufferWi=NewWi;
}

void Model::SetRenderSize(int NewHt, int NewWi) {
    RenderHt=NewHt;
    RenderWi=NewWi;
}

// not valid for Frame or Custom
int Model::NodesPerString() {
    return SingleNode ? 1 : parm2;
}

int Model::NodeStartChannel(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->ActChan: 0; //avoid memory access error if no nods -DJ
}

const std::string &Model::NodeType(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->GetNodeType(): NodeBaseClass::RGB; //avoid memory access error if no nods -DJ
}

NodeBaseClass* Model::createNode(int ns, const std::string &StringType, size_t NodesPerString, const std::string &rgbOrder) {
    if (StringType=="Single Color Red" || StringType == "R") {
        return new NodeClassRed(ns, NodesPerString);
    } else if (StringType=="Single Color Green" || StringType == "G") {
        return new NodeClassGreen(ns,NodesPerString);
    } else if (StringType=="Single Color Blue" || StringType == "B") {
        return new NodeClassBlue(ns,NodesPerString);
    } else if (StringType=="Single Color White" || StringType == "W") {
        return new NodeClassWhite(ns,NodesPerString);
    } else if (StringType[0] == '#') {
        return new NodeClassCustom(ns,NodesPerString, xlColor(StringType));
    } else if (StringType=="Strobes White 3fps") {
        return new NodeClassWhite(ns,NodesPerString);
    } else if (StringType=="4 Channel RGBW" || StringType == "RGBW") {
        return new NodeClassRGBW(ns,NodesPerString);
    }
    return new NodeBaseClass(ns,1,rgbOrder);
}


void Model::GetBufferSize(const std::string &type, int &bufferWi, int &bufferHi) const {
    if (type == "Rotate CC 90" || type == "Rotate CW 90") {
        bufferHi = this->BufferWi;
        bufferWi = this->BufferHt;
    } else if (type == "Single Line") {
        bufferHi = 1;
        bufferWi = Nodes.size();
    } else {
        //if (type == "Per Preview") {
        //default is to go ahead and build the full node buffer
        std::vector<NodeBaseClassPtr> newNodes;
        InitRenderBufferNodes(type, newNodes, bufferWi, bufferHi);
    }
}

static inline void SetCoords(NodeBaseClass::CoordStruct &it2, int x, int y) {
    it2.bufX = x;
    it2.bufY = y;
}

void Model::InitRenderBufferNodes(const std::string &type, std::vector<NodeBaseClassPtr> &newNodes, int &bufferWi, int &bufferHi) const {
    for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
        newNodes.push_back(NodeBaseClassPtr(it->get()->clone()));
    }
    if (type == "Default") {
        bufferHi = this->BufferHt;
        bufferWi = this->BufferWi;
    } else if (type == "Rotate 180") {
        bufferHi = this->BufferHt;
        bufferWi = this->BufferWi;
        for (auto it = newNodes.begin(); it != newNodes.end(); it++) {
            for (auto it2 = it->get()->Coords.begin(); it2 != it->get()->Coords.end(); it2++) {
                SetCoords(*it2, bufferWi - it2->bufX - 1, bufferHi - it2->bufY - 1);
            }
        }
    } else if (type == "Flip Vertical") {
        bufferHi = this->BufferHt;
        bufferWi = this->BufferWi;
        for (auto it = newNodes.begin(); it != newNodes.end(); it++) {
            for (auto it2 = it->get()->Coords.begin(); it2 != it->get()->Coords.end(); it2++) {
                SetCoords(*it2, it2->bufX, bufferHi - it2->bufY - 1);
            }
        }
    } else if (type == "Flip Horizontal") {
        bufferHi = this->BufferHt;
        bufferWi = this->BufferWi;
        for (auto it = newNodes.begin(); it != newNodes.end(); it++) {
            for (auto it2 = it->get()->Coords.begin(); it2 != it->get()->Coords.end(); it2++) {
                SetCoords(*it2, bufferWi - it2->bufX - 1, it2->bufY);
            }
        }
    } else if (type == "Rotate CC 90") {
        bufferHi = this->BufferWi;
        bufferWi = this->BufferHt;
        for (auto it = newNodes.begin(); it != newNodes.end(); it++) {
            for (auto it2 = it->get()->Coords.begin(); it2 != it->get()->Coords.end(); it2++) {
                SetCoords(*it2, this->BufferHt - it2->bufY - 1, it2->bufX);
            }
        }
    } else if (type == "Rotate CW 90") {
        bufferHi = this->BufferWi;
        bufferWi = this->BufferHt;
        for (auto it = newNodes.begin(); it != newNodes.end(); it++) {
            for (auto it2 = it->get()->Coords.begin(); it2 != it->get()->Coords.end(); it2++) {
                SetCoords(*it2, it2->bufY, this->BufferWi - it2->bufX - 1);
            }
        }
    } else if (type == "Single Line") {
        bufferHi = 1;
        bufferWi = newNodes.size();
        int cnt = 0;
        for (auto it = newNodes.begin(); it != newNodes.end(); it++) {
            for (auto it2 = it->get()->Coords.begin(); it2 != it->get()->Coords.end(); it2++) {
                SetCoords(*it2, cnt, 0);
                cnt++;
            }
        }
    } else if (type == "Per Preview") {
        double maxX = -1000000;
        double minX = 1000000;
        double maxY = -1000000;
        double minY = 1000000;


        double sx,sy;

        double w = previewW;
        double h = previewH;
        double scalex = w / RenderWi * PreviewScaleX;
        double scaley = h / RenderHt * PreviewScaleY;
        double radians=toRadians(PreviewRotation);
        double w1 = int(offsetXpct*w);
        double h1 = int(offsetYpct*h);

        for (auto it = newNodes.begin(); it != newNodes.end(); it++) {
            for (auto it2 = it->get()->Coords.begin(); it2 != it->get()->Coords.end(); it2++) {
                sx = it2->screenX;
                sy = it2->screenY;

                sx = (sx*scalex);
                sy = (sy*scaley);
                TranslatePointDoubles(radians,sx,sy,sx,sy);
                sx += w1;
                sy += h1;

                if (sx > maxX) {
                    maxX = sx;
                }
                if (sx < minX) {
                    minX = sx;
                }
                if (sy > maxY) {
                    maxY = sy;
                }
                if (sy < minY) {
                    minY = sy;
                }
            }
        }

        for (auto it = newNodes.begin(); it != newNodes.end(); it++) {
            for (auto it2 = it->get()->Coords.begin(); it2 != it->get()->Coords.end(); it2++) {
                sx = it2->screenX;
                sy = it2->screenY;

                sx = (sx*scalex);
                sy = (sy*scaley);
                TranslatePointDoubles(radians,sx,sy,sx,sy);
                sx += w1;
                sy += h1;

                SetCoords(*it2, sx - minX, sy - minY);
            }
        }
        bufferHi = maxY - minY + 1;
        bufferWi = maxX - minX + 1;
    } else {
        bufferHi = this->BufferHt;
        bufferWi = this->BufferWi;
    }
}


std::string Model::GetNextName() {
    if (nodeNames.size() > Nodes.size()) {
        return nodeNames[Nodes.size()];
    }
    return "";
}
// set size of Nodes vector and each Node's Coords vector
void Model::SetNodeCount(size_t NumStrings, size_t NodesPerString, const std::string &rgbOrder) {
    size_t n;
    if (SingleNode) {
        if (StringType=="Single Color Red") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRed(n,NodesPerString, GetNextName())));
        } else if (StringType=="Single Color Green") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassGreen(n,NodesPerString, GetNextName())));
        } else if (StringType=="Single Color Blue") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassBlue(n,NodesPerString, GetNextName())));
        } else if (StringType=="Single Color White") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n,NodesPerString, GetNextName())));
        } else if (StringType=="Strobes White 3fps") {
            StrobeRate=7;  // 1 out of every 7 frames
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n,NodesPerString, GetNextName())));
        } else if (StringType=="Single Color Custom") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassCustom(n,NodesPerString, customColor, GetNextName())));
        } else if (StringType=="4 Channel RGBW") {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n,NodesPerString, GetNextName())));
        } else {
            // 3 Channel RGB
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n,NodesPerString, "RGB", GetNextName())));
        }
    } else if (NodesPerString == 0) {
        Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(0, 0, rgbOrder, GetNextName())));
    } else {
        size_t numnodes=NumStrings*NodesPerString;
        for(n=0; n<numnodes; n++)
            Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n/NodesPerString, 1, rgbOrder, GetNextName())));
    }
}

int Model::GetNodeChannelCount(const std::string & nodeType) {
    if (nodeType.compare(0, 12, "Single Color") == 0) {
        return 1;
    } else if (nodeType == "Strobes White 3fps") {
        return 1;
    } else if (nodeType == "4 Channel RGBW") {
        return 4;
    }
    return 3;
}


void Model::Rotate(int degrees) {
    PreviewRotation=degrees;
    SetLineCoord();
}

int Model::GetRotation() {
    return PreviewRotation;
}


// returns a number where the first node is 1
int Model::GetNodeNumber(size_t nodenum) {
    if (nodenum >= Nodes.size()) return 0;
    //if (Nodes[nodenum].bufX < 0) return 0;
    int sn=Nodes[nodenum]->StringNum;
    return (Nodes[nodenum]->ActChan - stringStartChan[sn]) / 3 + sn*NodesPerString() + 1;
}

size_t Model::GetNodeCount() const {
    return Nodes.size();
}

int Model::GetChanCount() const {
    size_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    int min = 999999999;
    int max = 0;

    for (int x = 0; x < NodeCnt; x++) {
        int i = Nodes[x]->ActChan;
        if (i < min) {
            min = i;
        }
        i += Nodes[x]->GetChanCount();
        if (i > max) {
            max = i;
        }
    }
    return max - min;
}
int Model::GetChanCountPerNode() const {
    size_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    return Nodes[0]->GetChanCount();
}
size_t Model::GetCoordCount(size_t nodenum) {
    return nodenum < Nodes.size() ? Nodes[nodenum]->Coords.size() : 0;
}

void Model::GetNodeCoords(int nodeidx, std::vector<wxPoint> &pts) {
    for (int x = 0; x < Nodes[nodeidx]->Coords.size(); x++) {
        pts.push_back(wxPoint(Nodes[nodeidx]->Coords[x].bufX, Nodes[nodeidx]->Coords[x].bufY));
    }
}

bool Model::IsCustom(void) {
    return (DisplayAs == "Custom");
}

//convert # to AA format so it matches Custom Model grid display:
//this makes it *so* much easier to visually compare with Custom Model grid display
//A - Z == 1 - 26
//AA - AZ == 27 - 52
//BA - BZ == 53 - 78
//etc
static wxString AA(int x) {
    wxString retval;
    --x;
    //    if (x >= 26 * 26) { retval += 'A' + x / (26 * 26); x %= 26 * 26; }
    if (x >= 26) {
        retval += 'A' + x / 26 - 1;
        x %= 26;
    }
    retval += 'A' + x;
    return retval;
}
//add just the node#s to a choice list:
//NO add parsed info to choice list or check list box:
size_t Model::GetChannelCoords(wxArrayString& choices) { //wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3)
    //    if (choices1) choices1->Clear();
    //    if (choices2) choices2->Clear();
    //    if (choices3) choices3->Clear();
    //    if (choices1) choices1->Append(wxT("0: (none)"));
    //    if (choices2) choices2->Append(wxT("0: (none)"));
    //    if (choices3) choices3->Append(wxT("0: (none)"));
    size_t NodeCount = GetNodeCount();
    for (size_t n = 0; n < NodeCount; n++) {
        wxString newstr;
        //        debug(10, "model::node[%d/%d]: #coords %d, ach# %d, str %d", n, NodeCount, Nodes[n]->Coords.size(), Nodes[n]->StringNum, Nodes[n]->ActChan);
        if (Nodes[n]->Coords.empty()) continue;
        //        newstr = wxString::Format(wxT("%d"), GetNodeNumber(n));
        //        choices.Add(newstr);
        choices.Add(GetNodeXY(n));
        //        if (choices1) choices1->Append(newstr);
        //        if (choices2) choices2->Append(newstr);
        //        if (choices3)
        //        {
        //            wxArrayString strary;
        //            choices3->InsertItems(strary, choices3->GetCount() + 0);
        //        }
    }
    return choices.GetCount(); //choices1? choices1->GetCount(): 0) + (choices2? choices2->GetCount(): 0);
}
//get parsed node info:
std::string Model::GetNodeXY(const std::string& nodenumstr) {
    long nodenum;
    size_t NodeCount = GetNodeCount();
    try {
        nodenum = std::stod(nodenumstr);
        for (size_t inx = 0; inx < NodeCount; inx++) {
            if (Nodes[inx]->Coords.empty()) continue;
            if (GetNodeNumber(inx) == nodenum) return GetNodeXY(inx);
        }
    } catch ( ... ) {

    }
    return nodenumstr; //not found?
}
std::string Model::GetNodeXY(int nodeinx) {
    if ((nodeinx < 0) || (nodeinx >= GetNodeCount())) return "";
    if (Nodes[nodeinx]->Coords.empty()) return "";
    if (GetCoordCount(nodeinx) > 1) //show count and first + last coordinates
        if (IsCustom())
            return wxString::Format(wxT("%d: %d# @%s%d-%s%d"), GetNodeNumber(nodeinx), GetCoordCount(nodeinx), AA(Nodes[nodeinx]->Coords.front().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.front().bufY, AA(Nodes[nodeinx]->Coords.back().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.back().bufY).ToStdString(); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
        else
            return wxString::Format(wxT("%d: %d# @(%d,%d)-(%d,%d"), GetNodeNumber(nodeinx), GetCoordCount(nodeinx), Nodes[nodeinx]->Coords.front().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.front().bufY, Nodes[nodeinx]->Coords.back().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.back().bufY).ToStdString(); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
        else //just show singleton
            if (IsCustom())
                return wxString::Format(wxT("%d: @%s%d"), GetNodeNumber(nodeinx), AA(Nodes[nodeinx]->Coords.front().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.front().bufY).ToStdString();
            else
                return wxString::Format(wxT("%d: @(%d,%d)"), GetNodeNumber(nodeinx), Nodes[nodeinx]->Coords.front().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.front().bufY).ToStdString();
}

//extract first (X,Y) from string formatted above:
bool Model::ParseFaceElement(const std::string& multi_str, std::vector<wxPoint>& first_xy) {
    //    first_xy->x = first_xy->y = 0;
    //    first_xy.clear();
    wxStringTokenizer wtkz(multi_str, "+");
    while (wtkz.HasMoreTokens()) {
        wxString str = wtkz.GetNextToken();
        if (str.empty()) continue;
        if (str.Find('@') == wxNOT_FOUND) continue; //return false;

        wxString xystr = str.AfterFirst('@');
        if (xystr.empty()) continue; //return false;
        long xval = 0, yval = 0;
        if (xystr[0] == '(') {
            xystr.Remove(0, 1);
            if (!xystr.BeforeFirst(',').ToLong(&xval)) continue; //return false;
            if (!xystr.AfterFirst(',').BeforeFirst(')').ToLong(&yval)) continue; //return false;
        } else {
            int parts = 0;
            while (!xystr.empty() && (xystr[0] >= 'A') && (xystr[0] <= 'Z')) {
                xval *= 26;
                xval += xystr[0] - 'A' + 1;
                xystr.Remove(0, 1);
                parts |= 1;
            }
            while (!xystr.empty() && (xystr[0] >= '0') && (xystr[0] <= '9')) {
                yval *= 10;
                yval += xystr[0] - '0';
                xystr.Remove(0, 1);
                parts |= 2;
            }
            if (parts != 3) continue; //return false;
            if (!xystr.empty() && (xystr[0] != '-')) continue; //return false;
        }
        wxPoint newxy(xval, yval);
        first_xy.push_back(newxy);
    }

    return !first_xy.empty(); //true;
}


std::string Model::ChannelLayoutHtml() {
    size_t NodeCount=GetNodeCount();
    size_t i,idx;
    int n,x,y,s;
    wxString bgcolor;
    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi,0);
    bool IsCustom = DisplayAs == "Custom";
    std::string direction;
    if (IsCustom) {
        direction="n/a";
    } else if (!IsLtoR) {
        if(!isBotToTop)
            direction="Top Right";
        else
            direction="Bottom Right";
    } else {
        if (!isBotToTop)
            direction="Top Left";
        else
            direction="Bottom Left";
    }

    std::string html = "<html><body><table border=0>";
    html+="<tr><td>Name:</td><td>"+name+"</td></tr>";
    html+="<tr><td>Display As:</td><td>"+DisplayAs+"</td></tr>";
    html+="<tr><td>String Type:</td><td>"+StringType+"</td></tr>";
    html+="<tr><td>Start Corner:</td><td>"+direction+"</td></tr>";
    html+=wxString::Format("<tr><td>Total nodes:</td><td>%d</td></tr>",NodeCount);
    html+=wxString::Format("<tr><td>Height:</td><td>%d</td></tr>",BufferHt);
    html+="</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";

    int Ibufx,Ibufy;

    if (BufferHt == 1) {
        // single line or arch
        html+="<tr>";
        for(i=1; i<=NodeCount; i++) {
            n=IsLtoR ? i : NodeCount-i+1;
            s=Nodes[n-1]->StringNum+1;
            bgcolor=s%2 == 1 ? "#ADD8E6" : "#90EE90";
            html += wxString::Format("<td bgcolor='"+bgcolor+"'>n%ds%d</td>",n,s);
        }
        html+="</tr>";
    } else if (BufferHt > 1) {
        // horizontal or vertical matrix or frame
        for(i=0; i<NodeCount; i++) {

            Ibufx = Nodes[i]->Coords[0].bufX;
            Ibufy = Nodes[i]->Coords[0].bufY;
            idx=Nodes[i]->Coords[0].bufY * BufferWi + Nodes[i]->Coords[0].bufX;
            if (idx < chmap.size()) chmap[idx]=i + 1;
        }
        for(y=BufferHt-1; y>=0; y--) {
            html+="<tr>";
            for(x=0; x<BufferWi; x++) {
                n=chmap[y*BufferWi+x];
                if (n==0) {
                    html+="<td></td>";
                } else {
                    s=Nodes[n-1]->StringNum+1;
                    bgcolor=s%2 == 1 ? "#ADD8E6" : "#90EE90";
                    html+=wxString::Format("<td bgcolor='"+bgcolor+"'>n%ds%d</td>",n,s);
                }
            }
            html+="</tr>";
        }
    } else {
        html+="<tr><td>Error - invalid height</td></tr>";
    }
    html+="</table></body></html>";
    return html;
}


// initialize screen coordinates
void Model::CopyBufCoord2ScreenCoord() {
    size_t NodeCount=GetNodeCount();
    int xoffset=BufferWi/2;
    int yoffset=BufferHt/2;
    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].screenX = Nodes[n]->Coords[c].bufX - xoffset;
            Nodes[n]->Coords[c].screenY = Nodes[n]->Coords[c].bufY - yoffset;
        }
    }
    SetRenderSize(BufferHt,BufferWi);
}

void Model::UpdateXmlWithScale() {
    ModelXml->DeleteAttribute("offsetXpct");
    ModelXml->DeleteAttribute("offsetYpct");
    ModelXml->DeleteAttribute("PreviewScale");
    ModelXml->DeleteAttribute("PreviewScaleX");
    ModelXml->DeleteAttribute("PreviewScaleY");
    ModelXml->DeleteAttribute("PreviewRotation");
    ModelXml->DeleteAttribute("StartChannel");
    if (ModelXml->HasAttribute("versionNumber"))
        ModelXml->DeleteAttribute("versionNumber");
    ModelXml->AddAttribute("offsetXpct", wxString::Format("%6.4f",offsetXpct));
    ModelXml->AddAttribute("offsetYpct", wxString::Format("%6.4f",offsetYpct));
    if (singleScale) {
        ModelXml->AddAttribute("PreviewScale", wxString::Format("%6.4f",PreviewScaleX));
    } else {
        ModelXml->AddAttribute("PreviewScaleX", wxString::Format("%6.4f",PreviewScaleX));
        ModelXml->AddAttribute("PreviewScaleY", wxString::Format("%6.4f",PreviewScaleY));
    }
    ModelXml->AddAttribute("PreviewRotation", wxString::Format("%d",PreviewRotation));
    ModelXml->AddAttribute("versionNumber", "1");
    ModelXml->AddAttribute("StartChannel", ModelStartChannel);
}

void Model::AddToWholeHouseModel(int w, int h, std::vector<int>& xPos,std::vector<int>& yPos,
                                      std::vector<int>& actChannel, std::vector<std::string>& nodeTypes) {
    size_t NodeCount=Nodes.size();
    double sx,sy;

    if (singleScale) {
        //we now have the virtual size so we can flip to non-single scale
        singleScale = false;
        if (RenderHt > RenderWi) {
            PreviewScaleX = double(RenderWi) * double(h) / (double(w) * RenderHt) * PreviewScaleY;
        } else {
            PreviewScaleY = double(RenderHt) * double(w) / (double(h) * RenderWi) * PreviewScaleX;
        }
    }
    double scalex = double(w) / RenderWi * PreviewScaleX;
    double scaley = double(h) / RenderHt * PreviewScaleY;
    double radians=toRadians(PreviewRotation);

    double w1 = int(offsetXpct*w);
    double h1 = int(offsetYpct*h);

    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        std::string type = Nodes[n]->GetNodeType();
        int channel = Nodes[n]->ActChan;
        for(size_t c=0; c < CoordCount; c++) {
            sx=Nodes[n]->Coords[c].screenX;
            sy=Nodes[n]->Coords[c].screenY;

            sx = (sx*scalex);
            sy = (sy*scaley);
            TranslatePointDoubles(radians,sx,sy,sx,sy);
            sx += w1;
            sy += h1;

            xPos.push_back(sx);
            yPos.push_back(sy);
            actChannel.push_back(channel);
            nodeTypes.push_back(type);
        }
    }
}

bool Model::IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) {
    SetMinMaxModelScreenCoordinates(preview);
    int xs = x1<x2?x1:x2;
    int xf = x1>x2?x1:x2;
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;

    if (mMinScreenX>=xs && mMaxScreenX<=xf && mMinScreenY>=ys && mMaxScreenY<=yf) {
        return true;
    } else {
        return false;
    }
}


bool Model::HitTest(ModelPreview* preview,int x,int y) {
    int y1 = preview->GetVirtualCanvasHeight()-y;
    SetMinMaxModelScreenCoordinates(preview);
    if (x>=mMinScreenX && x<=mMaxScreenX && y1>=mMinScreenY && y1 <= mMaxScreenY) {
        return true;
    } else {
        return false;
    }
}

wxCursor Model::GetResizeCursor(int cornerIndex) {
    int angleState;
    //LeftTop and RightBottom
    switch(cornerIndex) {
            // Left top when PreviewRotation = 0
        case 0:
            angleState = (int)(PreviewRotation/22.5);
            break;
            // Right Top
        case 1:
            angleState = ((int)(PreviewRotation/22.5)+4)%16;
            break;
            // Right Bottom
        case 2:
            angleState = ((int)(PreviewRotation/22.5)+8)%16;
            break;
            // Right Bottom
        default:
            angleState = ((int)(PreviewRotation/22.5)+12)%16;
            break;
    }
    switch(angleState) {
        case 0:
            return wxCURSOR_SIZENWSE;
        case 1:
            return wxCURSOR_SIZEWE;
        case 2:
            return wxCURSOR_SIZEWE;
        case 3:
            return wxCURSOR_SIZENESW;
        case 4:
            return wxCURSOR_SIZENESW;
        case 5:
            return wxCURSOR_SIZENS;
        case 6:
            return wxCURSOR_SIZENS;
        case 7:
            return wxCURSOR_SIZENWSE;
        case 8:
            return wxCURSOR_SIZENWSE;
        case 9:
            return wxCURSOR_SIZEWE;
        case 10:
            return wxCURSOR_SIZEWE;
        case 11:
            return wxCURSOR_SIZENESW;
        case 12:
            return wxCURSOR_SIZENESW;
        case 13:
            return wxCURSOR_SIZENS;
        case 14:
            return wxCURSOR_SIZENS;
        default:
            return wxCURSOR_SIZENWSE;
    }

}

int Model::CheckIfOverHandles(ModelPreview* preview, wxCoord x,wxCoord y) {
    int status;
    if (x>mHandlePosition[0].x && x<mHandlePosition[0].x+RECT_HANDLE_WIDTH &&
        y>mHandlePosition[0].y && y<mHandlePosition[0].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(GetResizeCursor(0));
        status = OVER_L_TOP_HANDLE;
    } else if (x>mHandlePosition[1].x && x<mHandlePosition[1].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[1].y && y<mHandlePosition[1].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(GetResizeCursor(1));
        status = OVER_R_TOP_HANDLE;
    } else if (x>mHandlePosition[2].x && x<mHandlePosition[2].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[2].y && y<mHandlePosition[2].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(GetResizeCursor(2));
        status = OVER_R_BOTTOM_HANDLE;
    } else if (x>mHandlePosition[3].x && x<mHandlePosition[3].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[3].y && y<mHandlePosition[3].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(GetResizeCursor(3));
        status = OVER_R_BOTTOM_HANDLE;
    } else if (x>mHandlePosition[4].x && x<mHandlePosition[4].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[4].y && y<mHandlePosition[4].y+RECT_HANDLE_WIDTH) {
        preview->SetCursor(wxCURSOR_HAND);
        status = OVER_ROTATE_HANDLE;
    }

    else {
        preview->SetCursor(wxCURSOR_DEFAULT);
        status = OVER_NO_HANDLE;
    }
    return status;
}

// display model using colors stored in each node
// used when preview is running
void Model::DisplayModelOnWindow(ModelPreview* preview, const xlColor *c, bool allowSelected) {
    size_t NodeCount=Nodes.size();
    double sx,sy;
    xlColor color;
    if (c != NULL) {
        color = *c;
    }
    int w, h;
    //int vw, vh;
    //preview->GetSize(&w, &h);
    preview->GetVirtualCanvasSize(w, h);

    if (pixelStyle == 1) {
        glEnable(GL_POINT_SMOOTH);
    }
    if (pixelSize != 2) {
        glPointSize(preview->calcPixelSize(pixelSize));
    }

    if (singleScale) {
        //we now have the virtual size so we can flip to non-single scale
        singleScale = false;
        if (RenderHt > RenderWi) {
            PreviewScaleX = double(RenderWi) * double(h) / (double(w) * RenderHt) * PreviewScaleY;
        } else {
            PreviewScaleY = double(RenderHt) * double(w) / (double(h) * RenderWi) * PreviewScaleX;
        }
    }

    double scalex=double(w) / RenderWi * PreviewScaleX;
    double scaley=double(h) / RenderHt * PreviewScaleY;

    double radians=toRadians(PreviewRotation);

    int w1 = int(offsetXpct*w);
    int h1 = int(offsetYpct*h);

    bool started = false;

    int first = 0; int last = NodeCount;
    int buffFirst = -1; int buffLast = -1;
    bool left = true;
    while (first < last) {
        int n = 0;
        if (left) {
            n = first;
            first++;
            if (buffFirst == -1) {
                buffFirst = Nodes[n]->Coords[0].bufX;
            }
            if (first < NodeCount && buffFirst != Nodes[first]->Coords[0].bufX) {
                left = false;
            }
        } else {
            last--;
            n = last;
            if (buffLast == -1) {
                buffLast = Nodes[n]->Coords[0].bufX;
            }
            if (last > 0 && buffFirst != Nodes[last - 1]->Coords[0].bufX) {
                left = true;
            }
        }
        if (c == NULL) {
            Nodes[n]->GetColor(color);
            if (modelDimmingCurve != nullptr) {
                modelDimmingCurve->reverse(color);
            }
            if (StrobeRate) {
                int r = rand() % 5;
                if (r != 0) {
                    color = xlBLACK;
                }
            }
        }
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            // draw node on screen
            sx=Nodes[n]->Coords[c].screenX;;
            sy=Nodes[n]->Coords[c].screenY;
            sx = (sx*scalex);
            sy = (sy*scaley);
            TranslatePointDoubles(radians,sx,sy,sx,sy);
            sx += w1;
            sy += h1;

            if (pixelStyle < 2) {
                started = true;
                DrawGLUtils::AddVertex(sx,sy,color, color == xlBLACK ? blackTransparency : transparency);
            } else {
                int trans = transparency;
                if (color == xlBLACK) {
                    trans = blackTransparency;
                }
                DrawGLUtils::DrawCircle(color, sx, sy, pixelSize / 2,
                                        trans, pixelStyle == 2 ? transparency : 100);
            }
        }
    }
    if (started) {
        DrawGLUtils::End(GL_POINTS);
    }
    if (pixelStyle == 1) {
        glDisable(GL_POINT_SMOOTH);
    }
    if (pixelSize != 2) {
        glPointSize(preview->calcPixelSize(2));
    }


    if (Selected && c != NULL && allowSelected) {
        //Draw bounding rectangle
        // Upper Left Handle
        sx =  (-RenderWi*scalex/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
        sy = (RenderHt*scaley/2) + BOUNDING_RECT_OFFSET;
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = sx + w1;
        sy = sy + h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        mHandlePosition[0].x = sx;
        mHandlePosition[0].y = sy;
        // Upper Right Handle
        sx =  (RenderWi*scalex/2) + BOUNDING_RECT_OFFSET;
        sy = (RenderHt*scaley/2) + BOUNDING_RECT_OFFSET;
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = sx + w1;
        sy = sy + h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        mHandlePosition[1].x = sx;
        mHandlePosition[1].y = sy;
        // Lower Right Handle
        sx =  (RenderWi*scalex/2) + BOUNDING_RECT_OFFSET;
        sy = (-RenderHt*scaley/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = sx + w1;
        sy = sy + h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        mHandlePosition[2].x = sx;
        mHandlePosition[2].y = sy;
        // Lower Left Handle
        sx =  (-RenderWi*scalex/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
        sy = (-RenderHt*scaley/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = sx + w1;
        sy = sy + h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        mHandlePosition[3].x = sx;
        mHandlePosition[3].y = sy;

        // Draw rotation handle square
        sx = -RECT_HANDLE_WIDTH/2;
        sy = ((RenderHt*scaley/2) + 50);
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx += w1;
        sy += h1;
        DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
        // Save rotate handle
        mHandlePosition[4].x = sx;
        mHandlePosition[4].y = sy;
        // Draw rotation handle from center to 25 over rendered height
        sx = 0;
        sy = ((RenderHt*scaley/2) + 50);
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx += w1;
        sy += h1;
        DrawGLUtils::DrawLine(xlWHITE,255,w1,h1,sx,sy,1.0);
    }
}

void Model::DisplayEffectOnWindow(ModelPreview* preview, double pointSize) {
    xlColor color;
    int w, h;


    preview->GetSize(&w, &h);

    double scaleX = double(w) * 0.95 / RenderWi;
    double scaleY = double(h) * 0.95 / RenderHt;
    double scale=scaleY < scaleX ? scaleY : scaleX;

    bool success = preview->StartDrawing(pointSize);

    if (pixelStyle == 1) {
        glEnable(GL_POINT_SMOOTH);
    }
    if (pixelSize != 2) {
        glPointSize(preview->calcPixelSize(pixelSize));
    }

    if(success) {
        // layer calculation and map to output
        size_t NodeCount=Nodes.size();
        DrawGLUtils::PreAlloc(NodeCount);
        double sx,sy;
        bool started = false;
        int first = 0; int last = NodeCount;
        int buffFirst = -1; int buffLast = -1;
        bool left = true;
        while (first < last) {
            int n = 0;
            if (left) {
                n = first;
                first++;
                if (buffFirst == -1) {
                    buffFirst = Nodes[n]->Coords[0].bufX;
                }
                if (first < NodeCount && buffFirst != Nodes[first]->Coords[0].bufX) {
                    left = false;
                }
            } else {
                last--;
                n = last;
                if (buffLast == -1) {
                    buffLast = Nodes[n]->Coords[0].bufX;
                }
                if (last > 0 && buffFirst != Nodes[last - 1]->Coords[0].bufX) {
                    left = true;
                }
            }

            Nodes[n]->GetColor(color);
            if (modelDimmingCurve != nullptr) {
                modelDimmingCurve->reverse(color);
            }
            if (StrobeRate) {
                int r = rand() % 5;
                if (r != 0) {
                    color = xlBLACK;
                }
            }
            size_t CoordCount=GetCoordCount(n);
            for(size_t c=0; c < CoordCount; c++) {
                // draw node on screen
                sx=Nodes[n]->Coords[c].screenX;
                sy=Nodes[n]->Coords[c].screenY;

                double newsy = ((sy*scale)+(h/2));
                if (pixelStyle < 2) {
                    DrawGLUtils::AddVertex((sx*scale)+(w/2), newsy, color, color == xlBLACK ? blackTransparency : transparency);
                    started = true;
                } else {
                    int trans = transparency;
                    if (color == xlBLACK) {
                        trans = blackTransparency;
                    }
                    DrawGLUtils::DrawCircle(color, (sx*scale)+(w/2), newsy, pixelSize,
                                            trans, pixelStyle == 2 ? transparency : 100);
                }
            }
        }
        if (started) {
            DrawGLUtils::End(GL_POINTS);
        }
        preview->EndDrawing();
    }
    if (pixelStyle == 1) {
        glDisable(GL_POINT_SMOOTH);
    }
    if (pixelSize != 2) {
        glPointSize(preview->calcPixelSize(2));
    }
}


void Model::SetModelCoord(int degrees) {
    PreviewRotation=degrees;
}

void Model::SetMinMaxModelScreenCoordinates(ModelPreview* preview) {
    int w, h;
    preview->GetVirtualCanvasSize(w, h);
    SetMinMaxModelScreenCoordinates(w, h);
}
void Model::SetMinMaxModelScreenCoordinates(int w, int h) {
    previewW = w;
    previewH = h;

    size_t NodeCount=Nodes.size();
    double sx,sy;

    if (singleScale) {
        //we now have the virtual size so we can flip to non-single scale
        singleScale = false;
        if (RenderHt > RenderWi) {
            PreviewScaleX = double(RenderWi) * double(h) / (double(w) * RenderHt) * PreviewScaleY;
        } else {
            PreviewScaleY = double(RenderHt) * double(w) / (double(h) * RenderWi) * PreviewScaleX;
        }
    }

    double scalex = double(w) / RenderWi * PreviewScaleX;
    double scaley = double(h) / RenderHt * PreviewScaleY;
    double radians=toRadians(PreviewRotation);

    double w1 = int(offsetXpct*w);
    double h1 = int(offsetYpct*h);

    mMinScreenX = w;
    mMinScreenY = h;
    mMaxScreenX = 0;
    mMaxScreenY = 0;
    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            // draw node on screen
            sx=Nodes[n]->Coords[c].screenX;
            sy=Nodes[n]->Coords[c].screenY;

            sx = (sx*scalex);
            sy = (sy*scaley);
            TranslatePointDoubles(radians,sx,sy,sx,sy);
            sx += w1;
            sy += h1;

            if (sx<mMinScreenX) {
                mMinScreenX = sx;
            }
            if (sx>mMaxScreenX) {
                mMaxScreenX = sx;
            }
            if (sy<mMinScreenY) {
                mMinScreenY = sy;
            }
            if (sy>mMaxScreenY) {
                mMaxScreenY = sy;
            }
        }
    }
    // Set minimum bounding rectangle
    if(mMaxScreenY-mMinScreenY<4) {
        mMaxScreenY+=2;
        mMinScreenY-=2;
    }
    if(mMaxScreenX-mMinScreenX<4) {
        mMaxScreenX+=2;
        mMinScreenX-=2;
    }
}

void Model::ResizeWithHandles(ModelPreview* preview,int mouseX,int mouseY) {
    int w, h;
    // Get Center Point
    preview->GetVirtualCanvasSize(w, h);
    double w1 = offsetXpct* double(w);
    double h1 = offsetYpct* double(h);
    // Get mouse point in model space/ not screen space
    double sx,sy;
    sx = double(mouseX)-w1;
    sy = double(h-mouseY)-h1;
    double radians=-toRadians(PreviewRotation); // negative angle to reverse translation
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = fabs(sx) - RECT_HANDLE_WIDTH;
    sy = fabs(sy) - RECT_HANDLE_WIDTH;
    SetScale( (double)(sx*2.0)/double(w), (double)(sy*2.0)/double(h));
}

void Model::RotateWithHandles(ModelPreview* preview, bool ShiftKeyPressed, int mouseX,int mouseY) {
    int w, h;
    preview->GetVirtualCanvasSize(w, h);
    int w1 = int(offsetXpct*w);
    int h1 = int(offsetYpct*h);
    // Get mouse point in screen space where center of model is origin.
    int sx,sy;
    sx = mouseX-w1;
    sy = (h-mouseY)-h1;
    //Calculate angle of mouse from center.
    float tan = (float)sx/(float)sy;
    int angle = -toDegrees((double)atan(tan));
    if(sy>=0) {
        PreviewRotation = angle;
    } else if (sx<=0) {
        PreviewRotation = 90+(90+angle);
    } else {
        PreviewRotation = -90-(90-angle);
    }
    if(ShiftKeyPressed) {
        PreviewRotation = (int)(PreviewRotation/5) * 5;
    }
}

void Model::SetTop(ModelPreview* preview,int y) {
    SetMinMaxModelScreenCoordinates(preview);
    int h = preview->GetVirtualCanvasHeight();
    int screenCenterY = h*offsetYpct;
    int newCenterY = screenCenterY + (y-mMaxScreenY);
    offsetYpct = ((float)newCenterY/(float)h);
}

void Model::SetBottom(ModelPreview* preview,int y) {
    SetMinMaxModelScreenCoordinates(preview);
    int h = preview->GetVirtualCanvasHeight();
    int screenCenterY = h*offsetYpct;
    int newCenterY = screenCenterY + (y-mMinScreenY);
    offsetYpct = ((float)newCenterY/(float)h);
}

void Model::SetLeft(ModelPreview* preview,int x) {
    SetMinMaxModelScreenCoordinates(preview);
    int w = preview->GetVirtualCanvasWidth();
    int screenCenterX = w*offsetXpct;
    int newCenterX = screenCenterX + (x-mMinScreenX);
    offsetXpct = ((float)newCenterX/(float)w);
}

void Model::SetRight(ModelPreview* preview,int x) {
    SetMinMaxModelScreenCoordinates(preview);
    int w = preview->GetVirtualCanvasWidth();
    int screenCenterX = w*offsetXpct;
    int newCenterX = screenCenterX + (x-mMaxScreenX);
    offsetXpct = ((float)newCenterX/(float)w);
}

void Model::SetHcenterOffset(float offset) {
    offsetXpct = offset;
}

void Model::SetVcenterOffset(float offset) {
    offsetYpct = offset;
}

int Model::GetTop(ModelPreview* preview) {
    SetMinMaxModelScreenCoordinates(preview);
    return mMaxScreenY;
}

int Model::GetBottom(ModelPreview* preview) {
    SetMinMaxModelScreenCoordinates(preview);
    return mMinScreenY;
}

int Model::GetLeft(ModelPreview* preview) {
    SetMinMaxModelScreenCoordinates(preview);
    return mMinScreenX;
}

int Model::GetRight(ModelPreview* preview) {
    SetMinMaxModelScreenCoordinates(preview);
    return mMaxScreenX;
}

float Model::GetHcenterOffset() {
    return offsetXpct;
}

float Model::GetVcenterOffset() {
    return offsetYpct;
}

int Model::GetStrandLength(int strand) const {
    return GetNodeCount() / GetNumStrands();
}

int Model::MapToNodeIndex(int strand, int node) const {
    if ((DisplayAs == wxT("Vert Matrix") || DisplayAs == wxT("Horiz Matrix")) && SingleChannel) {
        return node;
    }
    if (GetNumStrands() == 1) {
        return node;
    }
    if (SingleNode) {
        return strand;
    }
    return (strand * parm2 / parm3) + node;
}


void Model::SetModelStartChan(const std::string &start_channel) {
    ModelStartChannel = start_channel;
}



