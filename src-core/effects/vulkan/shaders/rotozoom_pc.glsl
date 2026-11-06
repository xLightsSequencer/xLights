// RotoZoomData push-constant block — must match RotoZoomData in
// VulkanEffectDataTypes.h (and RotoZoomData in MetalEffectDataTypes.h).
layout(push_constant) uniform PC {
    uint width;
    uint height;

    float offset;
    float xrotation;
    int xpivot;
    float yrotation;
    int ypivot;

    float zrotation;
    float zoom;
    float zoomquality;
    int pivotpointx;
    int pivotpointy;

    float xsine;
    float xpivotPx;
    float ysine;
    float ypivotPx;
    float zcos;
    float zsin;
    float zxoff;
    float zyoff;
    float zinc;
} data;

