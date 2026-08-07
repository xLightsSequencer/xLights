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
} data;

const float XL_PI = 3.1415926535897932384626433832;
const float XL_PI2 = XL_PI * 2.0;
