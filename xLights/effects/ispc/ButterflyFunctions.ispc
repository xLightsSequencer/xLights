
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
           
           
// optimized for when s and v are 1.0
inline uint8<4> h2rgb(float h) {
    float<3> rgb;
    h = clamp(h, 0.0, 1.0);
    float hue = h * 6.0;      // sector 0 to 5
    int i = (int)floor(hue);
    float f = hue - (float)i;   // fractional part of h
    switch (i) {
        case 6:
        case 0:
            rgb.r = 1.0;
            rgb.g = f;
            rgb.b = 0;
            break;
        case 1:
            rgb.r = (1.0 - f);
            rgb.g = 1.0;
            rgb.b = 0;
            break;
        case 2:
            rgb.r = 0;
            rgb.g = 1.0;
            rgb.b = f;
            break;
        case 3:
            rgb.r = 0;
            rgb.g = 1.0 - f;
            rgb.b = 1.0;
            break;
        case 4:
            rgb.r = f;
            rgb.g = 0;
            rgb.b = 1.0;
            break;
        default:    // case 5:
            rgb.r = 1.0;
            rgb.g = 0;
            rgb.b = 1.0 - f;
            break;
    }
    rgb *= 255.0;
    uint8<4> ret = {rgb.r, rgb.g, rgb.b, 255};
    return ret;
}

inline uint8 channelBlend(float c1, float c2, float ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5);
}

uint8<4> getMultiColorBlend(const uniform ButterflyData &data, float n, bool circular) {
    if (data.numColors <= 1) {
        return data.colors[0];
    }

    if (n >= 1.0) n = 0.99999;
    if (n < 0.0) n = 0.0;
    float nc = data.numColors;
    float realidx = circular ? n * nc : n * (nc - 1.0);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % data.numColors;
    float ratio = realidx - coloridx1;
    uint8<4> ret;
    uint8<4> c1 = data.colors[coloridx1];
    uint8<4> c2 = data.colors[coloridx2];
    ret.r = channelBlend(c1.r, c2.r,  ratio);
    ret.g = channelBlend(c1.g, c2.g,  ratio);
    ret.b = channelBlend(c1.b, c2.b,  ratio);
    ret.a = 255;
    return ret;
}


export void ButterflyEffectStyle1(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    uniform float sz = data.height + data.width;
    uniform float rsz = rcp(sz) * pi2;
    uniform float offset = (uniform float)data.offset;
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        float x2 = x * x;
        float y2 = y * y;
        float n = abs((x2 - y2) * sin(offset + ((x+y)*rsz)));
        float d = x2 + y2;
        float h = d > 0.001 ? clamp(n/d, 0.0, 1.0) : 0.0;
        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = h2rgb(h);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}

export void ButterflyEffectStyle2(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;
    uniform int maxframe = data.height * 2;
    uniform int frame = (data.height * data.curState / 200) % maxframe;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        float f = (frame < data.height) ? frame + 1 : maxframe - frame;
        float x1= (x - (float)(data.width)/2.0)/f;
        float y1= (y - (float)(data.height)/2.0)/f;
        float h = sqrt(x1*x1 + y1*y1);
        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = h2rgb(h);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}


export void ButterflyEffectStyle3(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;
    uniform int maxframe = data.height * 2;
    uniform int frame = (data.height * data.curState / 200) % maxframe;
    
    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        float f = (frame < maxframe/2) ? frame + 1 : maxframe - frame;
        f = f * 0.1 + (float)(data.height)/60.0;
        float x1 = (x-((float)data.width)/2.0)/f;
        float y1 = (y-((float)data.height)/2.0)/f;
        float h = sin(x1) * cos(y1);

        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = h2rgb(h);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}


export void ButterflyEffectStyle4(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {

    uniform float width = data.width;
    uniform float rszw = 1.0 / width;
    uniform float sz = data.height + data.width;
    uniform float rsz = rcp(sz) * pi2;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        float n = ((x*x - y*y) * sin(data.offset + ((x+y)*rsz)));
        float d = x*x + y*y;

        float h = d>0.001 ? n/d : 0.0;
        float intpart = floor(h);
        float fractpart = h - intpart;
        h = fractpart;
        if (h < 0) h = 1.0 + h;

        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = h2rgb(h);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}


export void ButterflyEffectStyle5(const uniform ButterflyData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {

    uniform float width = data.width;
    uniform float rszw = 1.0 / width;
    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        int yi = (int)y;
        int xi = (int)x;

        //  This section is to fix the colors on pixels at {0,1} and {1,0}
        if (xi == 0 && yi == 1) y += 1.0;
        if (xi == 1 && yi == 0) x += 1.0;

        float n = abs((x*x - y*y) * sin(data.offset + ((x+y)*pi2 / ((float)(data.height*data.width)))));
        float d = x*x + y*y;
        float h=d>0.001 ? n/d : 0.0;

        if (data.chunks <= 1 || (int)(h*data.chunks) % data.skip != 0) {
            if (data.colorScheme == 0) {
                result[index] = h2rgb(h);
            } else {
                result[index] = getMultiColorBlend(data, h, false);
            }
        }
    }
}


export void ButterflyEffectPlasmaStyles(const uniform ButterflyData &data,
                                    uniform int startIdx, uniform int endIdx,
                                    uniform uint8<4> result[]) {
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;
    uniform float invh = rcp((uniform float)data.height);
    uniform float invw = rcp((uniform float)data.width);
    uniform float time = data.plasmaTime;
    uniform const float onehalf = 1.0 / 2.0;
    uniform const float onethird = 1.0 / 3.0;
    uniform float halfTime = time / 2.0;
    uniform float thirdTime = time / 3.0;
    uniform float fifthTime = time / 5.0;
    uniform float chunks = data.chunks;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);
        float v = 0;

        float rx = x * invw - 0.5;
        float ry = y * invh - 0.5;

        // 1st equation
        v = sin(rx * 10.0 + time);
                
        //  second equation
        v += sin(10.0 * (rx * sin(halfTime) + ry * cos(thirdTime)) + time);
                
        //  third equation
        float cx = rx + 0.5 * sin(fifthTime);
        float cy = ry + 0.5 * cos(thirdTime);
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
                color.r = (sin(v * chunks * pi) + 1.0) * 128.0;
                color.g= (cos(v * chunks * pi) + 1.0) * 128.0;
                color.b = 0;
            break;
            case 7:
                color.r = 1;
                color.g = (cos(v * chunks * pi) + 1.0) * 128.0;
                color.b = (sin(v * chunks * pi) + 1.0) * 128.0;
                break;
            case 8:
                color.r = (sin(v * chunks * pi) + 1.0) * 128.0;
                color.g= (sin(v * chunks * pi + 2.0 * pi * onethird) + 1.0) * 128.0;
                color.b =(sin(v * chunks * pi + 4.0 * pi * onethird) + 1.0) * 128.0;
                break;
            case 9:
                color.r = color.g = color.b = (sin(v * chunks * pi) + 1.0) * 128.0;
                break;
            case 10:
                if (data.numColors >= 2) {
                    float h = sin(v * chunks * pi + 2.0 * pi * onethird) + 0.5;
                    color = getMultiColorBlend(data, h, false);
                } else {
                    color.r = 0; color.g = 0; color.b = 0;
                }
                break;
        }
        result[index] = color;
    }
}
