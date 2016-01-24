//
//  Node.h
//  xLights
//
//  Created by Daniel Kulp on 1/12/16.
//  Copyright © 2016 Daniel Kulp. All rights reserved.
//

#ifndef Node_h
#define Node_h

#include <vector>
#include <string>
#include <cmath>
#include <memory>

#include <../Color.h>

#include <wx/string.h>

#define NODE_RGB_CHAN_CNT           3
#define NODE_RGBW_CHAN_CNT          4
#define NODE_SINGLE_COLOR_CHAN_CNT  1

class NodeBaseClass
{
private:

    // buffer and screen coordinates for displayed nodes
    struct CoordStruct
    {
        unsigned short bufX, bufY;
        float screenX, screenY;
    };

protected:
    // color values in rgb order
    uint8_t c[3];
    // color channel offsets, rgb would be 0,1,2
    uint8_t offsets[3];
    unsigned short chanCnt;

public:
    int sparkle;
    int ActChan = 0;   // 0 is the first channel
    int StringNum; // node is part of this string (0 is the first string)
    std::vector<CoordStruct> Coords;
    std::vector<CoordStruct> OrigCoords;
    std::string *name = nullptr;

    NodeBaseClass()
    {
        chanCnt=NODE_RGB_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = 1;
        offsets[2] = 2;
    }

    NodeBaseClass(int StringNumber, size_t NodesPerString)
    {
        StringNum=StringNumber;
        Coords.resize(NodesPerString);
        chanCnt=NODE_RGB_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = 1;
        offsets[2] = 2;
    }
    NodeBaseClass(int StringNumber, size_t NodesPerString, const std::string &rgbOrder, const std::string &n = "")
    {
        StringNum=StringNumber;
        Coords.resize(NodesPerString);
        chanCnt=NODE_RGB_CHAN_CNT;
        offsets[0]=rgbOrder.find('R');
        offsets[1]=rgbOrder.find('G');
        offsets[2]=rgbOrder.find('B');
        if (n != "") {
            name = new std::string(n);
        } else {
            name = nullptr;
        }
    }

    // only for use in initializing the custom model
    void AddBufCoord(unsigned short x, unsigned short y)
    {
        CoordStruct c;
        c.bufX=x;
        c.bufY=y;
        Coords.push_back(c);
    }

    virtual void SetColor(const xlColor& color)
    {
        c[0]=color.red;
        c[1]=color.green;
        c[2]=color.blue;
    }

    virtual void SetFromChannels(const unsigned char *buf) {
        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                c[x] = buf[offsets[x]];
            }
        }
    }
    virtual void GetForChannels(unsigned char *buf) {
        for (int x = 0; x < 3; x++) {
            if (offsets[x] != 255) {
                buf[offsets[x]] = c[x];
            }
        }
    }
    virtual std::string GetNodeType() const {
        std::string nt("   ");
        nt[offsets[0]] = 'R';
        nt[offsets[1]] = 'G';
        nt[offsets[2]] = 'B';
        return nt;
    }

    int GetChanCount()
    {
        return chanCnt;
    }
    bool IsVisible()
    {
        return Coords.size() > 0;
    }

    bool OrigCoordsSaved()
    {
        return Coords.size() == OrigCoords.size();
    }
    void SaveCoords()
    {
        OrigCoords = Coords;
    }
    void SetName(const std::string &n) {
        if (name != nullptr) {
            delete name;
            name = nullptr;
        }
        if (n != "") {
            name = new std::string(n);
        }
    }
    std::string GetName() {
        if (name == nullptr) {
            return "";
        }
        return *name;
    }

    virtual ~NodeBaseClass()
    {
        if (name != nullptr) {
            delete name;
        }
    }

    virtual void GetColor(xlColor& color)
    {
        color.Set(c[0],c[1],c[2]);
    }
};

class NodeClassRed : public NodeBaseClass
{
public:
    NodeClassRed(int StringNumber, size_t NodesPerString, const std::string &n = "") : NodeBaseClass(StringNumber,NodesPerString)
    {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = offsets[2] = 255;
        SetName(n);
    }
    virtual void GetColor(xlColor& color)
    {
        color.Set(c[0],0,0);
    }
    virtual std::string GetNodeType() {
        return "R";
    }

};

class NodeClassGreen : public NodeBaseClass
{
public:
    NodeClassGreen(int StringNumber, size_t NodesPerString, const std::string &n = "") : NodeBaseClass(StringNumber,NodesPerString)
    {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[1] = 0;
        offsets[0] = offsets[2] = 255;
        SetName(n);
    }
    virtual void GetColor(xlColor& color)
    {
        color.Set(0,c[1],0);
    }
    virtual std::string GetNodeType() {
        return "G";
    }
};

class NodeClassBlue : public NodeBaseClass
{
public:
    NodeClassBlue(int StringNumber, size_t NodesPerString, const std::string &n = "") : NodeBaseClass(StringNumber,NodesPerString)
    {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[2] = 0;
        offsets[0] = offsets[1] = 255;
        SetName(n);
    }
    virtual void GetColor(xlColor& color)
    {
        color.Set(0,0,c[2]);
    }
    virtual std::string GetNodeType() {
        return "B";
    }
};
class NodeClassCustom : public NodeBaseClass
{
public:
    NodeClassCustom(int StringNumber, size_t NodesPerString, const xlColor &c, const std::string &n = "") : NodeBaseClass(StringNumber,NodesPerString)
    {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        offsets[0] = 0;
        offsets[1] = offsets[2] = 255;
        SetName(n);
        hsv = c.asHSV();
    }
    virtual void GetColor(xlColor& color)
    {
        HSVValue hsv2 = hsv;
        hsv2.value=c[0];
        hsv2.value /= 255.0;
        color = hsv2;
    }
    virtual std::string GetNodeType() {
        return xlColor(hsv);
    }
    virtual void SetColor(const xlColor& color)
    {
        HSVValue hsv2 = color.asHSV();

        if (std::abs((double)(hsv2.hue - hsv.hue)) < 0.01) {
            //in the right hue
            c[0]=hsv2.value * 255.0;
        } else if (hsv2.hue < 0.01 && hsv2.saturation < 0.01) {
            //white/black
            c[0]=hsv2.value * 255.0;
        } else {
            c[0] = 0;
        }
    }
private:
    HSVValue hsv;
};

class NodeClassWhite : public NodeBaseClass
{
public:
    NodeClassWhite(int StringNumber, size_t NodesPerString, const std::string &n = "") : NodeBaseClass(StringNumber,NodesPerString)
    {
        chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        SetName(n);
    }

    virtual void GetColor(xlColor& color)
    {
        uint8_t cmin=std::min(c[0],std::min(c[1],c[2]));
        color.Set(cmin,cmin,cmin);
    }
    virtual void SetFromChannels(const unsigned char *buf) {
        c[0] = c[1] = c[2] = buf[0];
    }
    virtual void GetForChannels(unsigned char *buf) {
        buf[0] = std::min(c[0],std::min(c[1],c[2]));
    }
    virtual std::string GetNodeType() {
        return "W";
    }
};
class NodeClassRGBW : public NodeBaseClass
{
public:
    NodeClassRGBW(int StringNumber, size_t NodesPerString, const std::string &n = "") : NodeBaseClass(StringNumber,NodesPerString)
    {
        chanCnt = NODE_RGBW_CHAN_CNT;
        SetName(n);
    }
    virtual void SetFromChannels(const unsigned char *buf) {
        if (buf[3] != 0) {
            c[0] = c[1] = c[2] = buf[3];
        } else {
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    c[x] = buf[offsets[x]];
                }
            }
        }
    }
    virtual void GetForChannels(unsigned char *buf) {
        if (c[0] == c[1] && c[1] == c[2]) {
            buf[0] = buf[1] = buf[2] = 0;
            buf[3] = c[0];
        } else {
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    buf[offsets[x]] = c[x];
                }
            }
            buf[3] = 0;
        }
    }
    virtual std::string GetNodeType() {
        return "RGBW";
    }
};

typedef std::unique_ptr<NodeBaseClass> NodeBaseClassPtr;


#endif /* Node_h */
