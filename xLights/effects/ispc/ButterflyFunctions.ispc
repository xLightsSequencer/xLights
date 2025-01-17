
struct ButterflyData
{
    uint8<4> colors[8];

    float offset;
    int chunks;
    int skip;
    int curState;

    uint16 colorScheme;
    uint16 width;
    uint16 height;
    uint16 numColors;

    float plasmaTime;
    int plasmaStyle;
};


const uniform float pi = 3.14159;
const uniform float pi2 = 3.14159*2.0;
           
inline uint8<4> hsv2rgb(float16 h, float16 s, float16 v) {
    float16<3> rgb;
    h = clamp(h, 0.0f16, 1.0f16);
    s = clamp(s, 0.0f16, 1.0f16);
    v = clamp(v, 0.0f16, 1.0f16);
    if (0.0f == s) {
        rgb.r = v;
        rgb.g = v;
        rgb.b = v;
    } else { // not grey
        float16 hue = h * 6.0f16;      // sector 0 to 5
        int i = (int)floor(hue);
        float16 f = hue - (float16)i;   // fractional part of h
        float16 p = v * (1.0f16 - s);

        switch (i) {
            case 6:
            case 0:
                rgb.r = v;
                rgb.g = v * (1.0f16 - s * (1.0f16 - f));
                rgb.b = p;
                break;
            case 1:
                rgb.r = v * (1.0f16 - s * f);
                rgb.g = v;
                rgb.b = p;
                break;
            case 2:
                rgb.r = p;
                rgb.g = v;
                rgb.b = v * (1.0f16 - s * (1.0f16 - f));
                break;
            case 3:
                rgb.r = p;
                rgb.g = v * (1.0f16 - s * f);
                rgb.b = v;
                break;
            case 4:
                rgb.r = v * (1.0f16 - s * (1.0f16 - f));
                rgb.g = p;
                rgb.b = v;
                break;
            default:    // case 5:
                rgb.r = v;
                rgb.g = p;
                rgb.b = v * (1.0f16 - s * f);
                break;
        }
    }
    rgb *= 255.0;
    uint8<4> ret = {rgb.r, rgb.g, rgb.b, 255};
    return ret;
}

inline uint8 channelBlend(float16 c1, float16 c2, float16 ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5f16);
}

uint8<4> getMultiColorBlend(const uniform ButterflyData &data, float16 n, bool circular) {
    if (data.numColors <= 1) {
        return data.colors[0];
    }

    if (n >= 1.0) n = 0.99999f16;
    if (n < 0.0) n = 0.0f16;
    float16 nc = data.numColors;
    float16 realidx = circular ? n * nc : n * (nc - 1.0f16);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % data.numColors;
    float16 ratio = realidx - coloridx1;
    uint8<4> ret;
    ret.r = channelBlend(data.colors[coloridx1].r, data.colors[coloridx2].r,  ratio);
    ret.g = channelBlend(data.colors[coloridx1].g, data.colors[coloridx2].g,  ratio);
    ret.b = channelBlend(data.colors[coloridx1].b, data.colors[coloridx2].b,  ratio);
    ret.a = 255;
    return ret;
}



export void ButterflyEffectStyle1(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    uniform float sz = data.height + data.width;
    uniform float rsz = rcp(sz) * pi2;
        
    foreach (index = startIdx...endIdx) {
        int32 y = index / data.width;
        int32 x = index - (y * data.width);

        float x2 = x * x;
        float y2 = y * y;
        float n = abs((x2 - y2) * sin(data.offset + ((x+y)*rsz)));
        float d = x2 + y2;
        float h = d > 0.001 ? n/d : 0.0;
        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = hsv2rgb(h, 1.0, 1.0);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}

export void ButterflyEffectStyle2(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
                                  
    foreach (index = startIdx...endIdx) {
        int32 y = index / data.width;
        int32 x = index - (y * data.width);

        int maxframe = data.height * 2;
        int frame = (data.height * data.curState / 200) % maxframe;

        float f = (frame < data.height) ? frame + 1 : maxframe - frame;
        float x1= ((float)(x) - (float)(data.width)/2.0)/f;
        float y1= ((float)(y) - (float)(data.height)/2.0)/f;
        float h = sqrt(x1*x1 + y1*y1);

        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = hsv2rgb(h, 1.0, 1.0);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}


export void ButterflyEffectStyle3(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    foreach (index = startIdx...endIdx) {
        int32 y = index / data.width;
        int32 x = index - (y * data.width);
        int maxframe = data.height * 2;
        int frame = (data.height * data.curState / 200) % maxframe;

        float f = (frame < maxframe/2) ? frame + 1 : maxframe - frame;
        f = f * 0.1 + (float)(data.height)/60.0;
        float x1 = (x-((float)data.width)/2.0)/f;
        float y1 = (y-((float)data.height)/2.0)/f;
        float h = sin(x1) * cos(y1);

        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = hsv2rgb(h, 1.0, 1.0);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}


export void ButterflyEffectStyle4(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {

    uniform float sz = data.height + data.width;
    uniform float rsz = rcp(sz) * pi2;

    foreach (index = startIdx...endIdx) {
        int32 y = index / data.width;
        int32 x = index - (y * data.width);

        float n = ((x*x - y*y) * sin(data.offset + ((x+y)*rsz)));
        float d = x*x + y*y;

        float h = d>0.001 ? n/d : 0.0;
        float intpart = floor(h);
        float fractpart = h - intpart;
        h = fractpart;
        if (h < 0) h = 1.0 + h;

        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = hsv2rgb(h, 1.0, 1.0);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}


export void ButterflyEffectStyle5(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {

    foreach (index = startIdx...endIdx) {
        int32 y = index / data.width;
        int32 x = index - (y * data.width);

        //  This section is to fix the colors on pixels at {0,1} and {1,0}
        if (x == 0 && y == 1) y++;
        if (x == 1 && y == 0) x++;

        float n = abs((x*x - y*y) * sin(data.offset + ((x+y)*pi2 / ((float)(data.height*data.width)))));
        float d = x*x + y*y;
        float h=d>0.001 ? n/d : 0.0;

        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = hsv2rgb(h, 1.0, 1.0);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}


export void ButterflyEffectPlasmaStyles(const uniform ButterflyData &data,
                                    uniform int startIdx, uniform int endIdx,
                                    uniform uint8<4> result[]) {
    uniform float16 invh = rcp((uniform float16)data.height);
    uniform float16 invw = rcp((uniform float16)data.width);
    uniform float16 time = data.plasmaTime;
    uniform const float16 onehalf = 1.0 / 2.0;
    uniform const float16 onethird = 1.0 / 3.0;
    uniform float16 halfTime = time / 2.0;
    uniform float16 thirdTime = time / 3.0;
    uniform float16 fifthTime = time / 5.0;
    uniform float16 chunks = data.chunks;

    foreach (index = startIdx...endIdx) {
        int32 y = index / data.width;
        int32 x = index - (y * data.width);
        float16 v = 0;

        float16 rx = ((float16)x) * invw - 0.5f16;
        float16 ry = ((float16)y) * invh - 0.5f16;

        // 1st equation
        v = sin(rx * 10.0f16 + time);
                
        //  second equation
        v += sin(10.0f16 * (rx * sin(halfTime) + ry * cos(thirdTime)) + time);
                
        //  third equation
        float cx = rx + 0.5f16 * sin(fifthTime);
        float cy = ry + 0.5f16 * cos(thirdTime);
        v += sin(sqrt(100*((cx*cx)+(cy*cy))+1+time));
                                
        v += sin(rx + time);
        v += sin((ry + time) * onehalf);
        v += sin((rx + ry + time) * onehalf);
        
        v += sin(sqrt(rx * rx + ry * ry + 1.0) + time);
        v = v * onehalf;
                
        uint8<4> color;
        color.a = 255;
        switch (data.plasmaStyle) {
            case 6:
                color.r = (sin(v * chunks * pi) + 1.0f16) * 128.0f16;
                color.g= (cos(v * chunks * pi) + 1.0f16) * 128.0f16;
                color.b = 0;
            break;
            case 7:
                color.r = 1;
                color.g = (cos(v * chunks * pi) + 1.0f16) * 128.0f16;
                color.b = (sin(v * chunks * pi) + 1.0f16) * 128.0f16;
                break;
            case 8:
                color.r = (sin(v * chunks * pi) + 1.0f16) * 128.0f16;
                color.g= (sin(v * chunks * pi + 2.0f16 * pi * onethird) + 1.0f16) * 128.0f16;
                color.b =(sin(v * chunks * pi + 4.0f16 * pi * onethird) + 1.0f16) * 128.0f16;
                break;
            case 9:
                color.r = color.g = color.b = (sin(v * chunks * pi) + 1.0f16) * 128.0f16;
                break;
            case 10:
                if (data.numColors >= 2) {
                    float16 h = sin(v * chunks * pi + 2.0f16 * pi * onethird) + 0.5f16;
                    color = getMultiColorBlend(data, h, false);
                } else {
                    color.r = 0; color.g = 0; color.b = 0;
                }
                break;
        }
        result[index] = color;

    }
}
