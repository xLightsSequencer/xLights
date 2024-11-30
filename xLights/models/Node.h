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

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "../Color.h"
#include "../utils/string_utils.h"

#define NODE_RGB_CHAN_CNT 3
#define NODE_RGBW_CHAN_CNT 4
#define NODE_RGBWW_CHAN_CNT 5
#define NODE_SINGLE_COLOR_CHAN_CNT 1

class Model;

class NodeBaseClass {
protected:
    // color values in rgb order
    uint8_t c[3] = { 0, 0, 0 };
    // color channel offsets, rgb would be 0,1,2
    uint8_t offsets[3] = { 0, 1, 2 };
    uint16_t chanCnt = 3;

public:
    // buffer and screen coordinates for displayed nodes
    struct CoordStruct {
        int bufX, bufY;
        float screenX = 0, screenY = 0, screenZ = 0;
    };
    
    class CoordVec {
    public:
        class CoordIterator {
        private:
            CoordVec &coords;
            uint32_t pos;
        public:
            explicit CoordIterator(CoordVec &c, int p) : coords(c), pos(p) {}
            CoordStruct& operator*() const {
                return coords[pos];
            }
            //overload pre increment operator
            CoordIterator& operator++() {
                pos++;
                return *this;
            }
            // overload post increment operator
            CoordIterator operator++(int) {
                CoordIterator ret(coords, pos);
                ++pos;
                return ret;
            }
            bool operator==(const CoordIterator& iter) const {
                return this->pos == iter.pos;
            }
            bool operator!=(const CoordIterator& iter) const {
                return this->pos != iter.pos;
            }
        };

        
        CoordVec() {}
        CoordVec(const CoordVec &c) {
            if (c._size <= 1) {
                coords = &first;
                first = c.first;
                _size = c._size;
            } else {
                _size = 0;
                coords = &first;
                resize(c._size);
                memcpy(coords, c.coords, _size * sizeof(CoordStruct));
            }
        }
        CoordVec(CoordVec &&c) : _size(c._size) {
            if (_size <= 1) {
                coords = &first;
                first = c.first;
            } else {
                coords = c.coords;
                c.coords = &c.first;
                c._size = 0;
            }
        }
        ~CoordVec() {
            if (coords != &first) {
                free(coords);
            }
        }
        
        bool empty() const { return _size == 0; }
        uint32_t size() const { return _size; }
        
        void resize(uint32_t i) {
            if (i <= 1 && (coords != &first)) {
                first = coords[0];
                free(coords);
                coords = &first;
            } else if (i > 1) {
                if (_size <= 1) {
                    coords = (CoordStruct*)malloc(i * sizeof(CoordStruct));
                    coords[0] = first;
                } else {
                    // need to expand
                    coords = (CoordStruct*)realloc(coords, i * sizeof(CoordStruct));
                }
                for (size_t x = _size; x < i; ++x) {
                    coords[x].screenX = coords[x].screenY = coords[x].screenZ = 0;
                }
            }
            _size = i;
        }
        void push_back(const CoordStruct &c) {
            uint32_t sz = _size;
            if (sz == 0) {
                _size = 1;
            } else {
                resize(sz + 1);
            }
            coords[sz] = c;
        }
        void push_front(CoordStruct c) {
            uint32_t sz = _size;
            if (sz == 0) {
                _size = 1;
            } else {
                resize(sz + 1);
                while (sz > 0) {
                    coords[sz] = coords[sz - 1];
                    --sz;
                }
            }
            coords[0] = c;
        }
        CoordStruct &back() {
            return coords[_size - 1];
        }
        CoordStruct &front() {
            return coords[0];
        }

        const CoordStruct& operator[](uint32_t idx) const {
            return coords[idx];
        }
        CoordStruct& operator[](uint32_t idx) {
            return coords[idx];
        }
        
        CoordIterator begin() {
            return CoordIterator(*this, 0);
        }
        CoordIterator end() {
            return CoordIterator(*this, _size);
        }

    private:
        uint32_t _size = 0;
        CoordStruct first;
        CoordStruct *coords = &first;
    };

    uint32_t ActChan = 0;   // 0 is the first channel
    uint32_t StringNum = 0; // node is part of this string (0 is the first string)
    CoordVec Coords;
    std::string* name = nullptr;
    const Model* model = nullptr;

    NodeBaseClass() {
        chanCnt = NODE_RGB_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = 1;
        offsets[2] = 2;
    }
    NodeBaseClass(const NodeBaseClass& c) :
        ActChan(c.ActChan), StringNum(c.StringNum), Coords(c.Coords), name(nullptr), chanCnt(c.chanCnt), model(c.model) {
        if (c.name != nullptr) {
            name = new std::string(*(c.name));
        }
        for (int x = 0; x < 3; x++) {
            this->offsets[x] = c.offsets[x];
            this->c[x] = c.c[x];
        }
    }

    NodeBaseClass(int StringNumber, size_t NodesPerString) {
        StringNum = StringNumber;
        Coords.resize(NodesPerString);
        chanCnt = NODE_RGB_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = 1;
        offsets[2] = 2;
    }

    NodeBaseClass(int StringNumber, size_t NodesPerString, const std::string& rgbOrder, const std::string& n = xlEMPTY_STRING) {
        StringNum = StringNumber;
        Coords.resize(NodesPerString);
        chanCnt = NODE_RGB_CHAN_CNT;
        offsets[0] = rgbOrder.find('R');
        offsets[1] = rgbOrder.find('G');
        offsets[2] = rgbOrder.find('B');
        if (n != xlEMPTY_STRING) {
            name = new std::string(n);
        } else {
            name = nullptr;
        }
    }
    virtual ~NodeBaseClass() {
        if (name != nullptr) {
            delete name;
        }
    }
    virtual NodeBaseClass* clone() const {
        return new NodeBaseClass(*this);
    }

    // only for use in initializing the custom model
    void AddBufCoord(unsigned short x, unsigned short y) {
        CoordStruct c;
        c.bufX = x;
        c.bufY = y;
        c.screenZ = 0;
        Coords.push_back(c);
    }

    // does this node exist within this channel range
    bool ContainsChannel(uint32_t startChannel, uint32_t endChannel) {
        return !(endChannel < ActChan || startChannel > ActChan + GetChanCount() - 1);
    }

    virtual void SetColor(const xlColor& color) {
        c[0] = color.red;
        c[1] = color.green;
        c[2] = color.blue;
    }

    virtual void SetFromChannels(const unsigned char* buf) {
        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                c[x] = buf[offsets[x]];
            }
        }
    }

    virtual void GetForChannels(unsigned char* buf) const {
        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                buf[offsets[x]] = c[x];
            }
        }
    }

    virtual const std::string& GetNodeType() const;

    uint32_t GetChanCount() const {
        return chanCnt;
    }

    bool IsVisible() const {
        return !Coords.empty();
    }

    void SetName(const std::string& n) {
        if (name != nullptr) {
            delete name;
            name = nullptr;
        }
        if (n != "") {
            name = new std::string(n);
        }
    }

    const std::string& GetName() const {
        if (name == nullptr) {
            return xlEMPTY_STRING;
        }
        return *name;
    }

    virtual void GetColor(xlColor& color) const {
        color.Set(c[0], c[1], c[2]);
    }

    virtual const xlColor& GetMaskColor() const {
        return xlWHITE;
    }

    static const std::string RED;
    static const std::string GREEN;
    static const std::string BLUE;
    static const std::string WHITE;

    static const std::string RGB;
    static const std::string RBG;
    static const std::string GBR;
    static const std::string GRB;
    static const std::string BRG;
    static const std::string BGR;

    static const std::string WRGB;
    static const std::string WRBG;
    static const std::string WGBR;
    static const std::string WGRB;
    static const std::string WBRG;
    static const std::string WBGR;

    static const std::string RGBW;
    static const std::string RBGW;
    static const std::string GBRW;
    static const std::string GRBW;
    static const std::string BRGW;
    static const std::string BGRW;

    static const std::string RGBWW;
};

class NodeClassRed : public NodeBaseClass {
public:
    NodeClassRed(int StringNumber, size_t NodesPerString, const std::string& n = xlEMPTY_STRING) :
        NodeBaseClass(StringNumber, NodesPerString) {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = offsets[2] = 255;
        SetName(n);
    }
    virtual void GetColor(xlColor& color) const override {
        color.Set(c[0], 0, 0);
    }
    virtual const std::string& GetNodeType() const override {
        return RED;
    }
    virtual NodeBaseClass* clone() const override {
        return new NodeClassRed(*this);
    }
    virtual const xlColor& GetMaskColor() const override {
        return xlRED;
    }
};

class NodeClassGreen : public NodeBaseClass {
public:
    NodeClassGreen(int StringNumber, size_t NodesPerString, const std::string& n = xlEMPTY_STRING) :
        NodeBaseClass(StringNumber, NodesPerString) {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[1] = 0;
        offsets[0] = offsets[2] = 255;
        SetName(n);
    }
    virtual void GetColor(xlColor& color) const override {
        color.Set(0, c[1], 0);
    }
    virtual const std::string& GetNodeType() const override {
        return GREEN;
    }
    virtual NodeBaseClass* clone() const override {
        return new NodeClassGreen(*this);
    }
    virtual const xlColor& GetMaskColor() const override {
        return xlGREEN;
    }
};

class NodeClassBlue : public NodeBaseClass {
public:
    NodeClassBlue(int StringNumber, size_t NodesPerString, const std::string& n = xlEMPTY_STRING) :
        NodeBaseClass(StringNumber, NodesPerString) {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[2] = 0;
        offsets[0] = offsets[1] = 255;
        SetName(n);
    }
    virtual void GetColor(xlColor& color) const override {
        color.Set(0, 0, c[2]);
    }
    virtual const std::string& GetNodeType() const override {
        return BLUE;
    }
    virtual NodeBaseClass* clone() const override {
        return new NodeClassBlue(*this);
    }
    virtual const xlColor& GetMaskColor() const override {
        return xlBLUE;
    }
};

class MaskedNodeBaseClass : public NodeBaseClass {
protected:
    xlColor _maskColor;

public:
    MaskedNodeBaseClass(const MaskedNodeBaseClass& c) :
        NodeBaseClass(c), _maskColor(c._maskColor) {
    }
    MaskedNodeBaseClass(int StringNumber, size_t NodesPerString, const xlColor& c) :
        NodeBaseClass(StringNumber, NodesPerString), _maskColor(c) {
    }

    virtual const xlColor& GetMaskColor() const override {
        return _maskColor;
    }
    void SetMaskColor(const xlColor& c) {
        _maskColor = c;
    }
};

class NodeClassCustom : public MaskedNodeBaseClass {
public:
    NodeClassCustom(int StringNumber, size_t NodesPerString, const xlColor& c, const std::string& n = xlEMPTY_STRING) :
        MaskedNodeBaseClass(StringNumber, NodesPerString, c) {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = offsets[2] = 255;
        SetName(n);
        hsv = c.asHSV();
        type = c;
    }
    NodeClassCustom(const NodeClassCustom& c) :
        MaskedNodeBaseClass(c), hsv(c.hsv), type(c.type) {
    }

    void SetCustomColor(xlColor& c) {
        hsv = c.asHSV();
        type = c;
    }

    virtual void GetColor(xlColor& color) const override {
        HSVValue hsv2 = hsv;
        hsv2.value = c[0];
        hsv2.value /= 255.0;
        color = hsv2;
    }

    virtual const std::string& GetNodeType() const override {
        return type;
    }

    virtual void SetColor(const xlColor& color) override {
        HSVValue hsv2 = color.asHSV();

        if (std::abs((double)(hsv2.hue - hsv.hue)) < 0.01) {
            // in the right hue
            c[0] = hsv2.value * 255.0;
        } else if (hsv2.hue < 0.01 && hsv2.saturation < 0.01) {
            // white/black
            c[0] = hsv2.value * 255.0;
        } else {
            c[0] = 0;
        }
    }
    virtual NodeBaseClass* clone() const override {
        return new NodeClassCustom(*this);
    }

private:
    HSVValue hsv;
    std::string type;
};

class NodeClassIntensity : public MaskedNodeBaseClass {
public:
    NodeClassIntensity(int StringNumber, size_t NodesPerString, const xlColor& c, const std::string& n = xlEMPTY_STRING) :
        MaskedNodeBaseClass(StringNumber, NodesPerString, c) {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = offsets[2] = 255;
        SetName(n);
        hsv = c.asHSV();
        type = c;
    }
    NodeClassIntensity(const NodeClassIntensity& c) :
        MaskedNodeBaseClass(c), hsv(c.hsv), type(c.type) {
    }

    void SetCustomColor(xlColor& c) {
        hsv = c.asHSV();
        type = c;
    }

    virtual void GetColor(xlColor& color) const override {
        HSVValue hsv2 = hsv;
        hsv2.value = c[0];
        hsv2.value /= 255.0;
        color = hsv2;
    }

    virtual const std::string& GetNodeType() const override {
        return type;
    }

    virtual void SetColor(const xlColor& color) override {
        HSVValue hsv2 = color.asHSV();
        c[0] = hsv2.value * 255.0;
    }
    virtual NodeBaseClass* clone() const override {
        return new NodeClassIntensity(*this);
    }

private:
    HSVValue hsv;
    std::string type;
};

class NodeClassWhite : public NodeBaseClass {
public:
    NodeClassWhite(int StringNumber, size_t NodesPerString, const std::string& n = xlEMPTY_STRING) :
        NodeBaseClass(StringNumber, NodesPerString) {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        SetName(n);
    }

    virtual void GetColor(xlColor& color) const override {
        uint8_t cmin = std::min(c[0], std::min(c[1], c[2]));
        color.Set(cmin, cmin, cmin);
    }
    virtual void SetFromChannels(const unsigned char* buf) override {
        c[0] = c[1] = c[2] = buf[0];
    }
    virtual void GetForChannels(unsigned char* buf) const override {
        buf[0] = std::min(c[0], std::min(c[1], c[2]));
    }
    virtual const std::string& GetNodeType() const override {
        return WHITE;
    }
    virtual NodeBaseClass* clone() const override {
        return new NodeClassWhite(*this);
    }
};

class NodeClassRGBWW : public NodeBaseClass {
public:
    NodeClassRGBWW(int StringNumber, size_t NodesPerString, const std::string& rgbOrder, int rgbwtype, const std::string& n = xlEMPTY_STRING) :
        NodeBaseClass(StringNumber, NodesPerString, rgbOrder) {
        chanCnt = NODE_RGBWW_CHAN_CNT;
        SetName(n);
        wOffset = 0;
        wIndex = 3;
        rgbwHandling = rgbwtype;
    }
    NodeClassRGBWW(const NodeClassRGBWW& c) :
        NodeBaseClass(c), wOffset(c.wOffset), wIndex(c.wIndex), rgbwHandling(c.rgbwHandling) {
    }

    virtual void SetFromChannels(const unsigned char* buf) override;
    virtual void GetForChannels(unsigned char* buf) const override;
    virtual const std::string& GetNodeType() const override;

    virtual NodeBaseClass* clone() const override {
        return new NodeClassRGBWW(*this);
    }

private:
    uint8_t wOffset;
    uint8_t wIndex;
    uint8_t rgbwHandling;
};

class NodeClassRGBW : public NodeBaseClass {
public:
    NodeClassRGBW(int StringNumber, size_t NodesPerString, const std::string& rgbOrder, bool whiteLast, int rgbwtype, const std::string& n = xlEMPTY_STRING) :
        NodeBaseClass(StringNumber, NodesPerString, rgbOrder) {
        chanCnt = NODE_RGBW_CHAN_CNT;
        SetName(n);
        wOffset = whiteLast ? 0 : 1;
        wIndex = whiteLast ? 3 : 0;
        rgbwHandling = rgbwtype;
    }
    NodeClassRGBW(const NodeClassRGBW& c) :
        NodeBaseClass(c), wOffset(c.wOffset), wIndex(c.wIndex), rgbwHandling(c.rgbwHandling) {
    }

    virtual void SetFromChannels(const unsigned char* buf) override;
    virtual void GetForChannels(unsigned char* buf) const override;
    virtual const std::string& GetNodeType() const override;

    virtual NodeBaseClass* clone() const override {
        return new NodeClassRGBW(*this);
    }

private:
    uint8_t wOffset;
    uint8_t wIndex;
    uint8_t rgbwHandling;
};

class NodeClassSuperString : public NodeBaseClass {
public:
    NodeClassSuperString(int StringNumber, size_t NodesPerString, const std::vector<xlColor>& superStringColours, int rgbwtype, const std::string& n = xlEMPTY_STRING) :
        NodeBaseClass(StringNumber, NodesPerString, "RGB") {
        chanCnt = superStringColours.size();
        SetName(n);
        _superStringColours = superStringColours;
        rgbwHandling = rgbwtype;
    }
    NodeClassSuperString(const NodeClassSuperString& c) :
        NodeBaseClass(c), _superStringColours(c._superStringColours), rgbwHandling(c.rgbwHandling) {
    }

    virtual void SetFromChannels(const unsigned char* buf) override;
    virtual void GetForChannels(unsigned char* buf) const override;
    virtual NodeBaseClass* clone() const override {
        return new NodeClassSuperString(*this);
    }

private:
    std::vector<xlColor> _superStringColours;
    uint8_t rgbwHandling;
};

typedef std::unique_ptr<NodeBaseClass> NodeBaseClassPtr;
