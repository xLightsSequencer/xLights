
#include "SpacialColors.ispc.h"

struct PinwheelData {
    uint32 width;
    uint32 height;
    
    int32 pinwheel_arms;
    int32 xc_adj;
    int32 yc_adj;
    int32 degrees_per_arm;
    int32 pinwheel_twist;
    int32 max_radius;
    int32 poffset;
    int32 pw3dType;
    int32 pinwheel_rotation;

    float tmax;
    float pos;
    
    int32 allowAlpha;
    uint32 numColors;
    uint8<4> *colorsAsColor;
    float<3> *colorsAsHSV;
    bool      colorIsSpacial[8];
    uint32    *colorarray;
    
    void*    bufferData;
};

inline uint8<4> hsv2rgb(const float<3> &hsv, uint8 alph = 255) {
    float red, green, blue;

    if (0.0f == hsv.y) {
        // Grey
        red = hsv.z;
        green = hsv.z;
        blue = hsv.z;
    } else { // not grey
        float hue = hsv.x * 6.0;      // sector 0 to 5
        int i = (int)floor(hue);
        float f = hue - i;          // fractional part of h
        float p = hsv.z * (1.0 - hsv.y);

        switch (i) {
            case 0:
                red = hsv.z;
                green = hsv.z * (1.0 - hsv.y * (1.0 - f));
                blue = p;
                break;

            case 1:
                red = hsv.z * (1.0 - hsv.y * f);
                green = hsv.z;
                blue = p;
                break;

            case 2:
                red = p;
                green = hsv.z;
                blue = hsv.z * (1.0 - hsv.y * (1.0 - f));
                break;

            case 3:
                red = p;
                green = hsv.z * (1.0 - hsv.y * f);
                blue = hsv.z;
                break;

            case 4:
                red = hsv.z * (1.0 - hsv.y * (1.0 - f));
                green = p;
                blue = hsv.z;
                break;

            default:    // case 5:
                red = hsv.z;
                green = p;
                blue = hsv.z * (1.0 - hsv.y * f);
                break;
        }
    }
    uint8<4> ret = {red * 255, green * 255, blue * 255, alph};
    return ret;
}

export void PinwheelEffectStyle0(const uniform PinwheelData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[])
{
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;
    uniform float sz = data.height + data.width;
    uniform float rsz = rcp(sz) * PI * 2.0;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        float y1 = y - data.yc_adj - ((float)data.height / 2.0f);
        float x1 = x - data.xc_adj - ((float)data.width / 2.0f);
        float r = sqrt((x1*x1) + (y1*y1));
        
        
        float<3> hsv;
        if (r <= data.max_radius) {
            float degrees_twist = (r / data.max_radius) * data.pinwheel_twist;
            float theta = (atan2(x1, y1) * 180 / 3.14159) + degrees_twist;
            if (data.pinwheel_rotation == 1) { // do we have CW rotation
                theta = data.pos + theta + (data.tmax / 2) + data.poffset;
            } else {
                theta = data.pos - theta + (data.tmax / 2) + data.poffset;
            }

            theta = theta + 540.0;
            int t2 = (int)theta % data.degrees_per_arm;
            if (t2 <= data.tmax) {
                float round = (float)t2 / (float)data.tmax;
                t2 = abs(t2 - (data.tmax / 2)) * 2;
                int ColorIdx2 = ((int)((theta / data.degrees_per_arm))) % data.pinwheel_arms;
                uint8<4> color = data.colorsAsColor[ColorIdx2];
                if (data.colorIsSpacial[ColorIdx2]) {
                    color = getSpacialColor(data.bufferData, data.colorarray[ColorIdx2],
                                            data.xc_adj + data.width / 2,
                                            data.yc_adj + data.height / 2,
                                            x, y, round, data.max_radius);
                    if (!data.allowAlpha) {
                        hsv = data.colorsAsHSV[ColorIdx2];
                    }
                } else if (!data.allowAlpha) {
                    hsv = data.colorsAsHSV[ColorIdx2];
                }
                switch (data.pw3dType) {
                    case 1:
                        if (data.allowAlpha) {
                            color.a = 255.0 * ((data.tmax - t2) / data.tmax);
                        } else {
                            hsv.z = hsv.z * ((data.tmax - t2) / data.tmax);
                            color = hsv2rgb(hsv);
                        }
                        break;
                    case 2:
                        if (data.allowAlpha) {
                            color.a = 255.0 * ((t2) / data.tmax);
                        } else {
                            hsv.z = hsv.z * ((t2) / data.tmax);
                            color = hsv2rgb(hsv);
                        }
                        break;
                    case 3:
                        if (data.allowAlpha) {
                            color.a = 255.0 * (1.0 - round);
                        } else {
                            hsv.z = hsv.z * (1.0 - round);
                            color = hsv2rgb(hsv);
                        }
                        break;
                    default:
                        break;
                }
                result[index] = color;
            }
        }
    }
}
