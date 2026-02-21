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

namespace XmlNodeKeys {
    // Model Node Names
    constexpr auto ModelsNodeName      = "models";
    constexpr auto ModelNodeName       = "model";
    constexpr auto ExportedAttribute   = "exported";
    constexpr auto RGBEffectsAttribute = "rgb_effects";
    constexpr auto DimmingCurveName    = "dimmingCurve";

    // View Object Node Names
    constexpr auto ViewObjectsNodeName = "view_objects";
    constexpr auto ViewObjectNodeName  = "view_object";
    

    // Common BaseObject Attributes
    constexpr auto NameAttribute        = "name";
    constexpr auto StateNameAttribute   = "Name";
    constexpr auto DisplayAsAttribute   = "DisplayAs";
    constexpr auto LayoutGroupAttribute = "LayoutGroup";
    constexpr auto FromBaseAttribute    = "FromBase";

    // Shared (by some) Attributes
    constexpr auto ArcAttribute            = "Arc";
    constexpr auto AlternateNodesAttribute = "AlternateNodes";
    constexpr auto BrightnessAttribute     = "Brightness";      //should fix
    constexpr auto DCBrightnessAttribute   = "brightness";      //should fix
    constexpr auto LayerSizesAttribute     = "LayerSizes";
    constexpr auto StarSizesAttribute      = "starSizes";
    constexpr auto ZigZagAttribute         = "ZigZag";      //fix it
    constexpr auto CZigZagAttribute        = "zigZag";      //fix it
    constexpr auto CustomColorsAttribute   = "CustomColors";
    constexpr auto TypeAttribute           = "type";
    constexpr auto StateTypeAttribute      = "Type";
    constexpr auto CReverseAttribute       = "reverse";     //fix it
    constexpr auto ReverseAttribute        = "Reverse";     //fix it
    constexpr auto PointDataAttribute      = "PointData";

    // Common Model Attributes
    constexpr auto StartSideAttribute       = "StartSide";
    constexpr auto DirAttribute             = "Dir";
    constexpr auto Parm1Attribute           = "parm1";
    constexpr auto Parm2Attribute           = "parm2";
    constexpr auto Parm3Attribute           = "parm3";
    constexpr auto AdvancedAttribute        = "Advanced";
    constexpr auto AntialiasAttribute       = "Antialias";
    constexpr auto PixelSizeAttribute       = "PixelSize";
    constexpr auto StringTypeAttribute      = "StringType";
    constexpr auto RGBWHandleAttribute      = "RGBWHandling";
    constexpr auto TransparencyAttribute    = "Transparency";
    constexpr auto BTransparencyAttribute   = "BlackTransparency";
    constexpr auto StartChannelAttribute    = "StartChannel";
    constexpr auto NodeNamesAttribute       = "NodeNames";
    constexpr auto StrandNamesAttribute     = "StrandNames";
    constexpr auto ShadowModelAttribute     = "ShadowModelFor";
    constexpr auto ControllerAttribute      = "Controller";
    constexpr auto xlightsVersionAttr       = "SourceVersion";
    constexpr auto versionNumberAttribute   = "versionNumber";
    constexpr auto ActiveAttribute          = "Active";
    constexpr auto FromBaaseAttribute       = "FromBase";
    constexpr auto DescriptionAttribute     = "Description";
    constexpr auto TagColourAttribute       = "TagColour";
    constexpr auto XLVersionAttribute       = "xLightsVersion";
    constexpr auto SettingsAttribute        = "settings";
    constexpr auto CustomColorAttribute     = "CustomColor";
    constexpr auto ModelBrightnessAttribute = "ModelBrightness";
    constexpr auto LowDefinitionAttribute   = "LowDefinition";
    constexpr auto ModelChainAttribute      = "ModelChain";
    constexpr auto AliasesAttribute         = "Aliases";
    constexpr auto AliasNodeName            = "alias";
    constexpr auto StrandsAttribute         = "Strands";

    // Common SubModel Attributes
    constexpr auto SubModelNodeName        = "subModel";
    constexpr auto LayoutAttribute         = "layout";
    constexpr auto SMTypeAttribute         = "type";
    constexpr auto BufferStyleAttribute    = "bufferstyle";
    constexpr auto SubBufferAttribute      = "subBuffer";
    constexpr auto LineAttribute           = "line";

    // ModelGroup
    constexpr auto GroupNodeName            = "modelGroup";
    constexpr auto mgSelectedAttribute      = "selected";
    constexpr auto mgGridSizeAttribute      = "GridSize";
    constexpr auto mgCentreMinxAttribute    = "centreMinx";
    constexpr auto mgCentreMinyAttribute    = "centreMiny";
    constexpr auto mgCentreMaxxAttribute    = "centreMaxx";
    constexpr auto mgCentreMaxyAttribute    = "centreMaxy";
    constexpr auto mgModelsAttribute        = "models";
    constexpr auto mgCentrexAttribute       = "centrex";
    constexpr auto mgCentreyAttribute       = "centrey";
    constexpr auto mgCentreDefinedAttribute = "centreDefined";
    constexpr auto mgxCentreOffsetAttribute = "XCentreOffset";
    constexpr auto mgyCentreOffsetAttribute = "YCentreOffset";
    constexpr auto mgDefaultCameraAttribute = "DefaultCamera";

    // Size/Position Attributes
    constexpr auto WorldPosXAttribute = "WorldPosX";
    constexpr auto WorldPosYAttribute = "WorldPosY";
    constexpr auto WorldPosZAttribute = "WorldPosZ";
    constexpr auto ScaleXAttribute    = "ScaleX";
    constexpr auto ScaleYAttribute    = "ScaleY";
    constexpr auto ScaleZAttribute    = "ScaleZ";
    constexpr auto RotateXAttribute   = "RotateX";
    constexpr auto RotateYAttribute   = "RotateY";
    constexpr auto RotateZAttribute   = "RotateZ";
    constexpr auto WidthAttribute     = "Width";
    constexpr auto HeightAttribute    = "Height";
    constexpr auto DepthAttribute     = "Depth";
    constexpr auto OffsetXAttribute   = "OffsetX";
    constexpr auto OffsetYAttribute   = "OffsetY";
    constexpr auto OffsetZAttribute   = "OffsetZ";

    // Dimensions Attributes
    constexpr auto DimNodeName        = "dimensions";
    constexpr auto DimUnitsAttribute  = "units";
    constexpr auto DimWidthAttribute  = "width";
    constexpr auto DimHeightAttribute = "height";
    constexpr auto DimDepthAttribute  = "depth";

    // Model Screen Location Attributes
    constexpr auto LockedAttribute = "Locked";

    // TwoPoint Screen Location Attributes
    constexpr auto X2Attribute = "X2";
    constexpr auto Y2Attribute = "Y2";
    constexpr auto Z2Attribute = "Z2";

    // ThreePoint Screen Location Attributes
    constexpr auto AngleAttribute = "Angle";
    constexpr auto ShearAttribute = "Shear";

    // DMX Moving Head Attributes
    constexpr auto DmxColorTypeAttribute   = "DmxColorType";
    constexpr auto DmxBeamYOffsetAttribute = "DmxBeamYOffset";
    constexpr auto DmxBeamLengthAttribute  = "DmxBeamLength";
    constexpr auto DmxBeamWidthAttribute   = "DmxBeamWidth";    // old MH model
    constexpr auto DmxStyleAttribute       = "DmxStyle";        // old MH model
    constexpr auto HideBodyAttribute       = "HideBody";        // old MH model

    // DmxColorAbilityRGB Attributes
    constexpr auto DmxRedChannelAttribute   = "DmxRedChannel";
    constexpr auto DmxGreenChannelAttribute = "DmxGreenChannel";
    constexpr auto DmxBlueChannelAttribute  = "DmxBlueChannel";
    constexpr auto DmxWhiteChannelAttribute = "DmxWhiteChannel";

    // DmxColorAbilityWheel Attributes
    constexpr auto DmxColorWheelChannelAttribute = "DmxColorWheelChannel";
    constexpr auto DmxDimmerChannelAttribute     = "DmxDimmerChannel";
    constexpr auto DmxColorWheelColorAttribute   = "DmxColorWheelColor";
    constexpr auto DmxColorWheelDelayAttribute   = "DmxColorWheelDelay";
    constexpr auto DmxColorWheelDMXAttribute     = "DmxColorWheelDMX";

    // DmxPresetAbility Attributes
    constexpr auto DmxPresetChannelAttribute = "DmxPresetChannel";
    constexpr auto DmxPresetValueAttribute   = "DmxPresetValue";
    constexpr auto DmxPresetDescAttribute    = "DmxPresetDesc";

    // DmxShutterAbility Attributes
    constexpr auto DmxShutterChannelAttribute = "DmxShutterChannel";
    constexpr auto DmxShutterOpenAttribute    = "DmxShutterOpen";
    constexpr auto DmxShutterOnValueAttribute = "DmxShutterOnValue";

    // DmxDimmerAbility Attributes
    constexpr auto MhDimmerChannelAttribute = "MhDimmerChannel";

    // DmxColorAbilityCMY Attributes
    constexpr auto DmxCyanChannelAttribute    = "DmxCyanChannel";
    constexpr auto DmxMagentaChannelAttribute = "DmxMagentaChannel";
    constexpr auto DmxYellowChannelAttribute  = "DmxYellowChannel";

    // DmxMovingHeadComm Attributes
    constexpr auto DmxFixtureAttribute = "DmxFixture";

    // DmxMotor Attributes
    constexpr auto ChannelCoarseAttribute = "ChannelCoarse";
    constexpr auto ChannelFineAttribute   = "ChannelFine";
    constexpr auto MinLimitAttribute      = "MinLimit";
    constexpr auto MaxLimitAttribute      = "MaxLimit";
    constexpr auto RangeOfMotionAttribute = "RangeOfMotion";
    constexpr auto OrientZeroAttribute    = "OrientZero";
    constexpr auto OrientHomeAttribute    = "OrientHome";
    constexpr auto SlewLimitAttribute     = "SlewLimit";
    constexpr auto UpsideDownAttribute    = "UpsideDown";

    // Servo Model
    //  TBC

    // Controller Attributes
    constexpr auto CtrlConnectionName       = "ControllerConnection";
    constexpr auto ProtocolAttribute        = "Protocol";
    constexpr auto ProtocolSpeedAttribute   = "Speed";
    constexpr auto PortAttribute            = "Port";
    constexpr auto StartNullAttribute       = "nullNodes";
    constexpr auto EndNullAttribute         = "endNullNodes";
    constexpr auto GammaAttribute           = "gamma";
    constexpr auto ColorOrderAttribute      = "colorOrder";
    constexpr auto GroupCountAttribute      = "groupCount";

    // Smart Remote Attributes
    constexpr auto SmartRemoteAttribute     = "SmartRemote";
    constexpr auto SmartRemoteTypeAttribute = "SmartRemoteType";
    constexpr auto SRCascadeOnPortAttribute = "SRCascadeOnPort";
    constexpr auto SRMaxCascadeAttribute    = "SRMaxCascade";
    constexpr auto SmartRemoteTsAttribute   = "ts";

    // Mesh Attributes
    constexpr auto ObjFileAttribute    = "ObjFile";
    constexpr auto MeshOnlyAttribute   = "MeshOnly";

    // Arch Attributes
    constexpr auto HollowAttribute = "Hollow";
    constexpr auto GapAttribute    = "Gap";
    constexpr auto ArchesSkewAttribute = "ArchesSkew";

    // Candy Canes
    constexpr auto CCHeightAttribute  = "CandyCaneHeight";
    constexpr auto CCReverseAttribute = "CandyCaneReverse";
    constexpr auto CCSticksAttribute  = "CandyCaneSticks";
    constexpr auto CCSkewAttribute  = "CandyCaneSkew";

    // Channel Block Model
    constexpr auto ChannelColorAttribute =  "ChannelProperties.ChannelColor";

    // Circle Model
    constexpr auto InsideOutAttribute   = "InsideOut";
    constexpr auto CircleSizesAttribute = "circleSizes";

    // Cube
    constexpr auto StyleAttribute          = "Style";
    constexpr auto CubeStartAttribute      = "Start";
    constexpr auto CubeStringsAttribute    = "Strings";
    constexpr auto StrandPerLineAttribute  = "StrandPerLine";
    constexpr auto StrandPerLayerAttribute = "StrandPerLayer";

    // Custom Model
    constexpr auto CustomModelAttribute    = "CustomModel";
    constexpr auto CustomModelCmpAttribute = "CustomModelCompressed";
    constexpr auto NodesAttribute          = "Nodes";
    constexpr auto PixelTypeAttribute      = "PixelType";
    constexpr auto PixelSpacingAttribute   = "PixelSpacing";
    constexpr auto PixelAttribute          = "Pixel";
    constexpr auto BkgLightnessAttribute   = "CustomBkgLightness";
    constexpr auto BkgImageAttribute       = "CustomBkgImage";
    constexpr auto BkgAttribute            = "Bkg";
    constexpr auto CMDepthAttribute        = "Depth";
    constexpr auto CustomStringsAttribute  = "CustomStrings";

    // Dimming Curves
    constexpr auto DimmingNodeName = "dimmingCurve";

    // Image Model
    constexpr auto ImageAttribute         = "Image";
    constexpr auto BlackAttribute         = "Black";
    constexpr auto ChainAttribute         = "Chain";
    constexpr auto WhiteAsAlphaAttribute  = "WhiteAsAlpha";
    constexpr auto OffBrightnessAttribute = "OffBrightness";

    // Icicles Model
    constexpr auto DropPatternAttribute = "DropPattern";

    // Matrix
    constexpr auto VertMatrixAttribute    = "Vertical";
    constexpr auto NoZigZagAttribute      = "NoZig";

    // MultiPoint Model
    constexpr auto MultiStringsAttribute  = "MultiStrings";

    // Poly Line Model
    constexpr auto NumPointsAttribute    = "NumPoints";
    constexpr auto cPointDataAttribute   = "cPointData";
    constexpr auto SegsExpandedAttribute = "SegsExpanded";
    constexpr auto ModelHeightAttribute  = "ModelHeight";
    constexpr auto PolyStringsAttribute  = "PolyStrings";

    // Sphere
    constexpr auto DegreesAttribute  = "Degrees";
    constexpr auto StartLatAttribute = "StartLatitude";
    constexpr auto EndLatAttribute   = "EndLatitude";

    // Spinner Model
    constexpr auto StartAngleAttribute = "StartAngle";
    constexpr auto HallowAttribute     = "Hollow";
    constexpr auto ArcAngleAttribute   = "Arc";
    constexpr auto AlternateAttribute  = "Alternate";

    // Star Model
    constexpr auto StarStartLocationAttribute = "StarStartLocation";
    constexpr auto StarRatioAttribute         = "starRatio";
    constexpr auto StarCenterPercentAttribute = "starCenterPercent";

    // Tree Model
    constexpr auto TreeBottomTopRatioAttribute  = "TreeBottomTopRatio";
    constexpr auto TreePerspectiveAttribute     = "TreePerspective";
    constexpr auto TreeRotationAttribute        = "TreeRotation";
    constexpr auto TreeSpiralRotationsAttribute = "TreeSpiralRotations";
    constexpr auto StrandDirAttribute           = "StrandDir";
    constexpr auto exportFirstStrandAttribute   = "exportFirstStrand";
    constexpr auto TreeTypeAttribute            = "TreeType";
    constexpr auto TreeDegreesAttribute         = "TreeDegrees";
    
    // Window Frame Model
    constexpr auto RotationAttribute = "Rotation";

    // States
    constexpr auto StateNodeName = "stateInfo";
    
    // Faces
    constexpr auto FaceNodeName = "faceInfo";

    // WIP
    // constexpr auto Attribute = "";

    //Layout Groups
    constexpr auto BackgroundImageAttribute      = "backgroundImage";
    constexpr auto BackgroundBrightnessAttribute = "backgroundBrightness";
    constexpr auto BackgroundAlphaAttribute      = "backgroundAlpha";
    constexpr auto ScaleImageAttribute           = "scaleImage";
     
    // View_Object
    constexpr auto GridLineSpacingAttribute = "GridLineSpacing";
    constexpr auto GridWidthAttribute       = "GridWidth";
    constexpr auto GridHeightAttribute      = "GridHeight";
    constexpr auto UnitsAttribute           = "Units";
    constexpr auto LengthAttribute          = "Length";
    constexpr auto TerrainLineAttribute     = "TerrianLineSpacing";     //fix spelling in v8
    constexpr auto TerrainWidthAttribute    = "TerrianWidth";           //fix spelling in v8
    constexpr auto TerrainDepthAttribute    = "TerrianDepth";           //fix spelling in v8
    constexpr auto TerrainBrushAttribute    = "TerrianBrushSize";       //fix spelling in v8
    constexpr auto GridColorAttribute       = "gridColor";
    constexpr auto HideGridAttribute        = "HideGrid";
    constexpr auto HideImageAttribute       = "HideImage";
    constexpr auto ObjAttribute             = "ObjFile";

    //Settings
    constexpr auto ValueAttribute = "value";

    //Colors
    //Already does it in ColorManager.cpp ColorManager::Save()
    constexpr auto RedAttribute   = "Red";
    constexpr auto GreenAttribute = "Green";
    constexpr auto BlueAttribute  = "Blue";

    // Effect Version
    constexpr auto VersionAttribute = "version";

    // Previews
    constexpr auto ModelsAttribute = "models";

    // Group - already done

    // Perspectives
    constexpr auto CurrentAttribute  = "current";

    // ViewPoint
    constexpr auto posXAttribute     = "posX";
    constexpr auto posYAttribute     = "posY";
    constexpr auto posZAttribute     = "posZ";
    constexpr auto angleXAttribute   = "angleX";
    constexpr auto angleYAttribute   = "angleY";
    constexpr auto angleZAttribute   = "angleZ";
    constexpr auto distanceAttribute = "distance";
    constexpr auto zoomAttribute     = "zoom";
    constexpr auto panXAttribute     = "panx";
    constexpr auto panYAttribute     = "pany";
    constexpr auto panZAttribute     = "panz";
    constexpr auto zoomXAttribute    = "zoom_corrx";
    constexpr auto zoomYAttribute    = "zoom_corry";
    constexpr auto is3DAttribute     = "is_ed";

    // Model Types
    constexpr auto ArchesType           = "Arches";
    constexpr auto CandyCaneType        = "Candy Canes";
    constexpr auto ChannelBlockType     = "Channel Block";
    constexpr auto CircleType           = "Circle";
    constexpr auto CubeType             = "Cube";
    constexpr auto CustomType           = "Custom";
    constexpr auto DmxMovingHeadType    = "DmxMovingHead";
    constexpr auto DmxMovingHeadAdvType = "DmxMovingHeadAdv";
    constexpr auto DmxFloodAreaType     = "DmxFloodArea";
    constexpr auto DmxFloodlightType    = "DmxFloodlight";
    constexpr auto DmxGeneralType       = "DmxGeneral";
    constexpr auto DmxServoType         = "DmxServo";
    constexpr auto DmxServo3dType       = "DmxServo3d";
    constexpr auto DmxSkullType         = "DmxSkull";
    constexpr auto IciclesType          = "Icicles";
    constexpr auto ImageType            = "Image";
    constexpr auto MatrixType           = "Matrix";
    constexpr auto MultiPointType       = "MultiPoint";
    constexpr auto SingleLineType       = "Single Line";
    constexpr auto PolyLineType         = "Poly Line";
    constexpr auto SphereType           = "Sphere";
    constexpr auto SpinnerType          = "Spinner";
    constexpr auto StarType             = "Star";
    constexpr auto TreeType             = "Tree";
    constexpr auto WindowType           = "Window Frame";
    constexpr auto WreathType           = "Wreath";

    // ViewObject Types
    constexpr auto GridlinesType    = "Gridlines";
    constexpr auto TerrainType      = "Terrian";
    constexpr auto MeshType         = "Mesh";
    constexpr auto RulerType        = "Ruler";

    //Extra Types
    constexpr auto EffectsType      = "effects";
    constexpr auto ViewsType        = "views";
    constexpr auto PalettesType     = "palettes";
    constexpr auto LayoutGroupsType = "layoutGroups";
    constexpr auto PerspectivesType = "perspectives";
    constexpr auto SettingsType     = "settings";
    constexpr auto ColorsType       = "colors";
    constexpr auto ViewPointsType   = "Viewpoints";
}; // end namespace XmlNodeKeys
