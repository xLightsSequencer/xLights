/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>

#include "../XmlSerializer/FileSerializingVisitor.h"

#include "TreeModel.h"
#include "ModelScreenLocation.h"
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "../graphics/IModelPreview.h"
#include "CustomModel.h"
#include "../XmlSerializer/XmlNodeKeys.h"

TreeModel::TreeModel(const ModelManager &manager) : MatrixModel(manager)
{
    DisplayAs = DisplayAsType::Tree;
    screenLocation.SetSupportsZScaling(true);
    screenLocation.SetPreferredSelectionPlane(ModelScreenLocation::MSLPLANE::GROUND);
}

TreeModel::~TreeModel()
{
}

void TreeModel::InitModel() {
    if (_firstStrand < 0) {
        _firstStrand = 0;
    }
    if (_vMatrix) {
        InitVMatrix(_firstStrand);
    } else {
        InitHMatrix();
    }

    screenLocation.SetPerspective2D(_perspective);
    SetTreeCoord(_degrees);
    InitSingleChannelModel();
}

// initialize screen coordinates for tree
void TreeModel::SetTreeCoord(long _degrees)
{
    double bufferX, bufferY;
    if (BufferWi < 1)
        return;
    if (BufferHt < 1)
        return; // June 27,2013. added check to not divide by zero
    double RenderHt, RenderWi;
    if (_degrees > 0) {
        RenderHt = BufferHt * 3;
        RenderWi = ((double)RenderHt) / 1.8;

        double radians = toRadians(_degrees);
        double radius = RenderWi / 2.0;
        double topradius = radius;
        if (_botTopRatio != 0.0f) {
            topradius = radius / std::abs(_botTopRatio);
        }
        if (_botTopRatio < 0.0f) {
            std::swap(topradius, radius);
        }

        double StartAngle = -radians / 2.0;
        double AngleIncr = radians / double(BufferWi);
        if (_degrees < 350 && BufferWi > 1) {
            AngleIncr = radians / double(BufferWi - 1);
        }

        // shift a tiny bit to make the strands in back not line up exactly with the strands in front
        StartAngle += toRadians(_rotation);

        std::vector<float> yPos(BufferHt);
        std::vector<float> xInc(BufferHt);
        for (int x = 0; x < BufferHt; ++x) {
            yPos[x] = x;
            xInc[x] = 0;
        }
        if (_spiralRotations != 0.0f) {
            std::vector<float> lengths(10);
            float rgap = (radius - topradius) / 10.0;
            float total = 0;
            for (size_t x = 0; x < 10; ++x) {
                lengths[x] = 2.0 * M_PI * (radius - rgap * x) - rgap / 2.0;
                lengths[x] *= _spiralRotations / 10.0;
                lengths[x] = sqrt(lengths[x] * lengths[x] + (float)BufferHt / 10.0 * (float)BufferHt / 10.0);
                total += lengths[x];
            }
            int lights = 0;
            for (int x = 0; x < 10; x++) {
                lengths[x] /= total;
                lights += (int)std::round(lengths[x] * BufferHt);
            }
            int curSeg = 0;
            float lightsInSeg = std::round(lengths[0] * BufferHt);
            int curLightInSeg = 0;
            for (int x = 1; x < BufferHt; x++) {
                if (curLightInSeg >= lightsInSeg) {
                    curSeg++;
                    curLightInSeg = 0;
                    if (curSeg == 9) {
                        lightsInSeg = BufferHt - x;
                    } else {
                        lightsInSeg = std::round(lengths[curSeg] * BufferHt);
                    }
                }
                float ang = _spiralRotations * 2.0 * M_PI / 10.0;
                ang /= (float)lightsInSeg;
                yPos[x] = yPos[x - 1] + (BufferHt / 10.0 / lightsInSeg);
                xInc[x] = xInc[x - 1] + ang;
                curLightInSeg++;
            }
        }

        double topYoffset = 0.0; // std::abs(perspective * topradius * cos(M_PI));
        double ytop = RenderHt - topYoffset;
        double ybot = 0.0; // std::abs(perspective * radius * cos(M_PI));

        size_t NodeCount = GetNodeCount();
        for (size_t n = 0; n < NodeCount; n++) {
            size_t CoordCount = GetCoordCount(n);
            for (size_t c = 0; c < CoordCount; c++) {
                bufferX = Nodes[n]->Coords[c].bufX;
                bufferY = Nodes[n]->Coords[c].bufY;
                double angle = StartAngle + double(bufferX) * AngleIncr + xInc[bufferY];
                double xb = radius * sin(angle);
                double xt = topradius * sin(angle);
                double zb = radius * cos(angle);
                double zt = topradius * cos(angle);
                double yb = ybot;
                double yt = ytop;
                // double yb = ybot - perspective * radius * cos(angle);
                // double yt = ytop - perspective * topradius * cos(angle);
                double posOnString = 0.5;
                if (BufferHt > 1) {
                    posOnString = yPos[bufferY] / (double)(BufferHt - 1.0);
                }

                Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                Nodes[n]->Coords[c].screenY = yb + (yt - yb) * posOnString - ((double)RenderHt) / 2.0;
                Nodes[n]->Coords[c].screenZ = zb + (zt - zb) * posOnString;
            }
        }
    } else {
        double treeScale = _degrees == -1 ? 5.0 : 4.0;
        double botWid = BufferWi * treeScale;
        RenderHt = BufferHt * 2.0;
        RenderWi = (botWid + 2);

        double offset = 0.5;
        size_t NodeCount = GetNodeCount();
        for (size_t n = 0; n < NodeCount; n++) {
            size_t CoordCount = GetCoordCount(n);
            if (_degrees == -1) {
                for (size_t c = 0; c < CoordCount; c++) {
                    bufferX = Nodes[n]->Coords[c].bufX;
                    bufferY = Nodes[n]->Coords[c].bufY;

                    double xt = (bufferX + offset - BufferWi / 2.0) * 0.9;
                    double xb = (bufferX + offset - BufferWi / 2.0) * treeScale;
                    double h = std::sqrt(RenderHt * RenderHt + (xt - xb) * (xt - xb));

                    double posOnString = 0.5;
                    if (BufferHt > 1) {
                        posOnString = (bufferY / (double)(BufferHt - 1.0));
                    }

                    double newh = RenderHt * posOnString;
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * newh / h - ((double)RenderHt) / 2.0;

                    posOnString = 0;
                    if (BufferHt > 1) {
                        posOnString = ((bufferY - 0.33) / (double)(BufferHt - 1.0));
                    }

                    newh = RenderHt * posOnString;
                    NodeBaseClass::CoordStruct cs = Nodes[n]->Coords[c];
                    cs.screenX = xb + (xt - xb) * posOnString;
                    cs.screenY = RenderHt * newh / h - ((double)RenderHt) / 2.0;
                    Nodes[n]->Coords.push_back(cs);

                    posOnString = 1;
                    if (BufferHt > 1) {
                        posOnString = ((bufferY + 0.33) / (double)(BufferHt - 1.0));
                    }
                    newh = RenderHt * posOnString;
                    cs = Nodes[n]->Coords[c];
                    cs.screenX = xb + (xt - xb) * posOnString;
                    cs.screenY = RenderHt * newh / h - ((double)RenderHt) / 2.0;
                    Nodes[n]->Coords.push_back(cs);
                }

            } else {
                for (size_t c = 0; c < CoordCount; c++) {
                    bufferX = Nodes[n]->Coords[c].bufX;
                    bufferY = Nodes[n]->Coords[c].bufY;

                    double xt = (bufferX + offset - BufferWi / 2.0) * 0.9;
                    double xb = (bufferX + offset - BufferWi / 2.0) * treeScale;
                    double posOnString = 0.5;
                    if (BufferHt > 1) {
                        posOnString = (bufferY / (double)(BufferHt - 1.0));
                    }
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * posOnString - ((double)RenderHt) / 2.0;
                }
            }
        }
    }
    screenLocation.SetRenderSize(RenderWi, RenderHt, RenderWi);
}

#define SCALE_FACTOR_3D (1.1)

// Helper function to build 3D custom model data from node coordinates
static std::vector<std::vector<std::vector<int>>> BuildTreeCustomModelData(
    const std::vector<NodeBaseClassPtr>& nodes,
    int& width, int& height, int& depth)
{
    float minx = 99999;
    float miny = 99999;
    float minz = 99999;
    float maxx = -99999;
    float maxy = -99999;
    float maxz = -99999;

    for (auto& n : nodes) {
        minx = std::min(minx, n->Coords[0].screenX);
        miny = std::min(miny, n->Coords[0].screenY);
        minz = std::min(minz, n->Coords[0].screenZ);
        maxx = std::max(maxx, n->Coords[0].screenX);
        maxy = std::max(maxy, n->Coords[0].screenY);
        maxz = std::max(maxz, n->Coords[0].screenZ);
    }
    float w = maxx - minx;
    float h = maxy - miny;
    float d = maxz - minz;

    width = static_cast<int>(SCALE_FACTOR_3D * w + 1);
    height = static_cast<int>(SCALE_FACTOR_3D * h + 1);
    depth = static_cast<int>(SCALE_FACTOR_3D * d + 1);

    std::vector<std::vector<std::vector<int>>> data;
    for (int l = 0; l < depth; l++) {
        std::vector<std::vector<int>> layer;
        for (int r = height; r >= 0; r--) {
            std::vector<int> row;
            for (int c = 0; c < width; c++) {
                row.push_back(-1);
            }
            layer.push_back(row);
        }
        data.push_back(layer);
    }

    int i = 1;
    for (auto& n : nodes) {
        int xx = SCALE_FACTOR_3D * w * (n->Coords[0].screenX - minx) / w;
        int yy = (SCALE_FACTOR_3D * h) - (SCALE_FACTOR_3D * h * (n->Coords[0].screenY - miny) / h);
        int zz = SCALE_FACTOR_3D * d * (maxz - n->Coords[0].screenZ) / d;
        assert(xx >= 0 && xx < width);
        assert(yy >= 0 && yy < height);
        assert(zz >= 0 && zz < depth);
        assert(data[zz][yy][xx] == -1);
        data[zz][yy][xx] = i++;
    }

    return data;
}

void TreeModel::ExportAsCustomXModel3D(BaseSerializingVisitor& visitor) const
{
    // Build the 3D custom model data from node coordinates
    int width, height, depth;
    auto data = BuildTreeCustomModelData(Nodes, width, height, depth);

    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name", GetName());
    attrs.Add("CustomWidth", std::to_string(width));
    attrs.Add("CustomHeight", std::to_string(height));
    attrs.Add("StrandsPerString", std::to_string(_strandsPerString));
    attrs.Add("Depth", std::to_string(depth));
    attrs.Add("StringType", GetStringType());
    attrs.Add("Transparency", GetTransparency() ? "1" : "0");
    attrs.Add("PixelSize", std::to_string(GetPixelSize()));
    attrs.Add("Antialias", std::to_string((int)GetPixelStyle()));
    attrs.Add("StrandNames", GetStrandNames());
    attrs.Add("NodeNames", GetNodeNames());
    std::string pc = GetPixelCount();
    if (!pc.empty()) attrs.Add("PixelCount", pc);
    std::string pt = GetPixelType();
    if (!pt.empty()) attrs.Add("PixelType", pt);
    std::string psp = GetPixelSpacing();
    if (!psp.empty()) attrs.Add("PixelSpacing", psp);
    attrs.Add("CustomModel", CustomModel::ToCustomModel(data));
    attrs.Add("CustomModelCompressed", CustomModel::ToCompressed(data));
    attrs.Add("SourceVersion", xlights_version_string);
    visitor.AddSuperStrings(*this, attrs);

    visitor.WriteOpenTag("custommodel", attrs);
    visitor.WriteFacesAndStates(this);
    visitor.WriteSubmodels(this);
    visitor.WriteCloseTag();
}
