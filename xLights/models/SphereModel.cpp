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

#include "SphereModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"
#include "CustomModel.h"
#include "../XmlSerializer/XmlNodeKeys.h"

#include <log4cpp/Category.hh>

SphereModel::SphereModel(const ModelManager &manager) : MatrixModel(manager)
{
    DisplayAs = DisplayAsType::Sphere;
    screenLocation.SetSupportsZScaling(true);
    screenLocation.SetStartOnXAxis(true);
}

SphereModel::~SphereModel()
{
}

void SphereModel::InitModel() {
    InitVMatrix(0);
    screenLocation.SetPerspective2D(0.1f);
    SetSphereCoord();
    InitSingleChannelModel();
}

void SphereModel::SetSphereCoord() {

    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (BufferWi < 1) return;
    if (BufferHt < 1) return; // June 27,2013. added check to not divide by zero

    // BufferWi is the number of "spokes" and thus should not have any impact on the
    // shape of the sphere.   We'll use the "max" of the two so we have
    // the highest number of pixels to work  with
    double RenderMx = ((double)std::max(BufferHt, BufferWi)) / 1.8;

    double radians = toRadians(360);
    double radius = RenderMx / 2.0;

    //logger_base.debug("Buffer %d,%d Render %f,%f Radius %f,%f",
    //    BufferWi, BufferHt,
    //    (float)RenderWi, (float)RenderHt,
    //    (float)Hradius, (float)Vradius);

    double remove = toRadians((360.0 - _sphereDegrees));
    double fudge = toRadians((360.0 - _sphereDegrees) / (double)BufferWi);
    double HStartAngle = radians / 4.0 + 0.003 - remove / 2.0;
    double HAngleIncr = (-radians + remove - fudge) / (double)BufferWi;

    //logger_base.debug("Horizontal Start %d: +%f x %d",
    //    (int)toDegrees(HStartAngle), (float)toDegrees(HAngleIncr), BufferWi);

    double VStartAngle = toRadians(_startLatitude-90);
    double VAngleIncr = (toRadians(-1 * _startLatitude) + toRadians(_endLatitude)) / (BufferHt-1);

    //logger_base.debug("Vertical Start %d: +%f x %d",
    //    (int)toDegrees(VStartAngle), (float)toDegrees(VAngleIncr), BufferHt);

    size_t NodeCount = GetNodeCount();
    for (size_t n = 0; n<NodeCount; n++) {
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; c++) {
            double bufferX = Nodes[n]->Coords[c].bufX;
            double bufferY = Nodes[n]->Coords[c].bufY;
            double hangle = HStartAngle + bufferX * HAngleIncr;
            double vangle = VStartAngle + bufferY * VAngleIncr;

            double sv = sin(vangle);
            Nodes[n]->Coords[c].screenX = radius * cos(hangle) * sv;
            Nodes[n]->Coords[c].screenZ = radius * sin(hangle) * sv;
            Nodes[n]->Coords[c].screenY = radius * cos(vangle);

            //logger_base.debug("%d: %d,%d -> hangle %d vangle %d -> %f,%f,%f",
            //    n,
            //    (int)bufferX, (int)bufferY,
            //    (int)toDegrees(hangle), (int)toDegrees(vangle),
            //    Nodes[n]->Coords[c].screenX, Nodes[n]->Coords[c].screenY, Nodes[n]->Coords[c].screenZ);
        }
    }
    screenLocation.SetRenderSize(RenderMx, RenderMx, RenderMx);
}

void SphereModel::ExportAsCustomXModel3D(BaseSerializingVisitor& visitor) const
{
    float minx = 99999;
    float miny = 99999;
    float minz = 99999;
    float maxx = -99999;
    float maxy = -99999;
    float maxz = -99999;

    for (auto& n : Nodes) {
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

    int scaleFactor3D = 2;
    while (!Find3DCustomModelScale(scaleFactor3D, minx, miny, minz, w, h, d)) {
        ++scaleFactor3D;
        if (scaleFactor3D > 20) {
            scaleFactor3D = 2;
            break;
        }
    }

    std::vector<std::vector<std::vector<int>>> data;
    for (int l = 0; l < BufferWi * scaleFactor3D + 1; l++) {
        std::vector<std::vector<int>> layer;
        for (int r = BufferHt * scaleFactor3D + 1; r >= 0; r--) {
            std::vector<int> row;
            for (int c = 0; c < BufferWi * scaleFactor3D + 1; c++) {
                row.push_back(-1);
            }
            layer.push_back(row);
        }
        data.push_back(layer);
    }

    int i = 1;
    for (auto& n : Nodes) {
        int xx = (scaleFactor3D * (float)BufferWi) * (n->Coords[0].screenX - minx) / w;
        int yy = (scaleFactor3D * (float)BufferHt) - (scaleFactor3D * (float)BufferHt * (n->Coords[0].screenY - miny) / h);
        int zz = (scaleFactor3D * (float)BufferWi) * (n->Coords[0].screenZ - minz) / d;
        assert(xx >= 0 && xx < scaleFactor3D * BufferWi + 1);
        assert(yy >= 0 && yy < scaleFactor3D * BufferHt + 1);
        assert(zz >= 0 && zz < scaleFactor3D * BufferWi + 1);
        assert(data[zz][yy][xx] == -1);
        data[zz][yy][xx] = i++;
    }

    int dim = scaleFactor3D * BufferWi + 1;
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("name", GetName());
    attrs.Add("CustomWidth", std::to_string(dim));
    attrs.Add("CustomHeight", std::to_string(scaleFactor3D * BufferHt + 1));
    attrs.Add("StrandsPerString", std::to_string(_strandsPerString));
    attrs.Add("Depth", std::to_string(dim));
    attrs.Add("StringType", GetStringType());
    attrs.Add("Transparency", GetTransparency() ? "1" : "0");
    attrs.Add("PixelSize", std::to_string(GetPixelSize()));
    attrs.Add("Antialias", std::to_string((int)GetPixelStyle()));
    attrs.Add("StrandNames", GetStrandNames());
    attrs.Add("NodeNames", GetNodeNames());
    attrs.Add("StartLatitude", std::to_string(GetStartLatitude()));
    attrs.Add("EndLatitude", std::to_string(GetEndLatitude()));
    attrs.Add("Degrees", std::to_string(GetSphereDegrees()));
    attrs.Add("AlternateNodes", std::to_string(HasAlternateNodes()));
    attrs.Add("NoZig", std::to_string(IsNoZigZag()));
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
    visitor.WriteDimensionsElement(*this);
    visitor.WriteCloseTag();
}

bool SphereModel::Find3DCustomModelScale(int scale, float minx, float miny, float minz, float w, float h, float d) const
{
    size_t nodeCount = GetNodeCount();
    if (nodeCount <= 1) {
        return true;
    }
    for (int i = 0; i < nodeCount; ++i) {
        for (int j = i + 1; j < nodeCount; ++j) {
            int x1 = (scale * (float)BufferWi) * (Nodes[i]->Coords[0].screenX - minx) / w;
            int y1 = (scale * (float)BufferHt) - (scale * (float)BufferHt * (Nodes[i]->Coords[0].screenY - miny) / h);
            int z1 = (scale * (float)BufferWi) * (Nodes[i]->Coords[0].screenZ - minz) / d;

            int x2 = (scale * (float)BufferWi) * (Nodes[j]->Coords[0].screenX - minx) / w;
            int y2 = (scale * (float)BufferHt) - (scale * (float)BufferHt * (Nodes[j]->Coords[0].screenY - miny) / h);
            int z2 = (scale * (float)BufferWi) * (Nodes[j]->Coords[0].screenZ - minz) / d;

            if (x1 == x2 && y1 == y2 && z1 == z2) {
                return false;
            }
        }
    }
    return true;
}
