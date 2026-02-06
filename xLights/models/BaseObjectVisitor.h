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
class DmxFloodArea;
class DmxFloodlight;
class DmxGeneral;
class DmxMovingHead;
class DmxMovingHeadAdv;
class DmxServo;
class DmxServo3d;
class DmxSkull;
class ImageModel;
class IciclesModel;
class MatrixModel;
class MultiPointModel;
class SingleLineModel;
class PolyLineModel;
class SphereModel;
class SpinnerModel;
class StarModel;
class TreeModel;
class WindowFrameModel;
class WreathModel;

struct BaseObjectVisitor {
    virtual void Visit(const ArchesModel& model) = 0;
    virtual void Visit(const CandyCaneModel& model) = 0;
    virtual void Visit(const ChannelBlockModel& model) = 0;
    virtual void Visit(const CircleModel& model) = 0;
    virtual void Visit(const CubeModel& model) = 0;
    virtual void Visit(const CustomModel& model) = 0;
    virtual void Visit(const DmxFloodArea& model) = 0;
    virtual void Visit(const DmxFloodlight& model) = 0;
    virtual void Visit(const DmxGeneral& model) = 0;
    virtual void Visit(const DmxMovingHead& model) = 0;
    virtual void Visit(const DmxMovingHeadAdv& model) = 0;
    virtual void Visit(const DmxServo& model) = 0;
    virtual void Visit(const DmxServo3d& model) = 0;
    virtual void Visit(const DmxSkull& model) = 0;
    virtual void Visit(const ImageModel& model) = 0;
    virtual void Visit(const IciclesModel& model) = 0;
    virtual void Visit(const MatrixModel& model) = 0;
    virtual void Visit(const MultiPointModel& model) = 0;
    virtual void Visit(const SingleLineModel& model) = 0;
    virtual void Visit(const PolyLineModel& model) = 0;
    virtual void Visit(const SphereModel& model) = 0;
    virtual void Visit(const SpinnerModel& model) = 0;
    virtual void Visit(const StarModel& model) = 0;
    virtual void Visit(const TreeModel& model) = 0;
    virtual void Visit(const WindowFrameModel& model) = 0;
    virtual void Visit(const WreathModel& model) = 0;

    virtual ~BaseObjectVisitor() {
    }
};
