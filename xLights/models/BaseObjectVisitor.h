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

class ArchesModel;
class CandyCaneModel;
class ChannelBlockModel;
class CircleModel;
class CubeModel;
class CustomModel;
class DmxMovingHead;
class DmxMovingHeadAdv;
class ImageModel;
class IciclesModel;
class MatrixModel;
class SingleLineModel;
class PolyLineModel;
class SphereModel;
class SpinnerModel;
class StarModel;
class TreeModel;
class WindowFrameModel;
class WreathModel;

struct BaseObjectVisitor {
    virtual void Visit(const ArchesModel& arch) = 0;
    virtual void Visit(const CandyCaneModel& cc) = 0;
    virtual void Visit(const ChannelBlockModel& channelblock) = 0;
    virtual void Visit(const CircleModel& circle) = 0;
    virtual void Visit(const CubeModel& cube) = 0;
    virtual void Visit(const CustomModel& custom) = 0;
    virtual void Visit(const DmxMovingHead& moving_head) = 0;
    virtual void Visit(const DmxMovingHeadAdv& moving_head) = 0;
    virtual void Visit(const ImageModel& image) = 0;
    virtual void Visit(const IciclesModel& icicles) = 0;
    virtual void Visit(const MatrixModel& matrix) = 0;
    virtual void Visit(const SingleLineModel& singleline) = 0;
    virtual void Visit(const PolyLineModel& polyline) = 0;
    virtual void Visit(const SphereModel& sphere) = 0;
    virtual void Visit(const SpinnerModel& spinner) = 0;
    virtual void Visit(const StarModel& start) = 0;
    virtual void Visit(const TreeModel& tree) = 0;
    virtual void Visit(const WindowFrameModel& window) = 0;
    virtual void Visit(const WreathModel& wreath) = 0;

    virtual ~BaseObjectVisitor() {
    }
};
