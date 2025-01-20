
struct PlasmaData
{
    float Style;
    float state;
    float Line_Density;
    float time;
    float sin_time_5;
    float cos_time_3;
    float sin_time_2;
    uint32 width;
    uint32 height;

    uint8<4> colors[8];
    uint16 numColors;
};

uint8 plasma_channelBlend(float c1, float c2, float ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5);
}
uint8<4> plasma_getMultiColorBlend(const uniform PlasmaData &data, float n, bool circular) {
    if (data.numColors <= 1) {
        return data.colors[0];
    }
    uniform float nc = data.numColors;
    n = clamp(n, 0.0, 0.999999999f);
    float realidx = circular ? n * nc : n * (nc - 1);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % data.numColors;
    float ratio = realidx - ((float)(coloridx1));
    uint8<4> ret;
    uint8<4> c1 = data.colors[coloridx1];
    uint8<4> c2 = data.colors[coloridx2];
    ret.r = plasma_channelBlend(c1.r, c2.r,  ratio);
    ret.g = plasma_channelBlend(c1.g, c2.g,  ratio);
    ret.b = plasma_channelBlend(c1.b, c2.b,  ratio);
    ret.a = 255;
    return ret;
}
// reference: http://www.bidouille.org/prog/plasma

float plasmaCalc_vldpi(const uniform PlasmaData &data, float x, float y) {
    uniform float time = data.time;
    
    float rx = data.width > 1 ? (x / (float)(data.width - 1)) : 0.0; // rx is now in the range 0.0 to 1.0
    float rx2 = rx * rx;
    float cx = rx + 0.5f * data.sin_time_5;
    float cx2 = cx*cx;
    float sin_rx_time = sin(rx + time);

    // 1st equation
    float v = sin(rx * 10.0f + time);
    float ry = data.height > 1 ? (y/(float)(data.height - 1)) : 0;

    //  second equation
    v += sin(10.0f*(rx*data.sin_time_2+ry*data.cos_time_3)+time);

    //  third equation
    float cy=ry + 0.5*data.cos_time_3;
    v += sin(sqrt((data.Style*50.0f)*((cx2)+(cy*cy))+time));

    //    vec2 c = v_coords * u_k - u_k/2.0;
    v += sin_rx_time;
    v += sin((ry+time)/2.0f);
    v += sin((rx+ry+time)/2.0f);
    //   c += u_k/2.0 * vec2(buffer.sin (u_time/3.0), buffer.cos (u_time/2.0));
    v += sin(sqrt(rx2+ry*ry)+time);
    v = v/2.0f;
    // vec3 col = vec3(1, buffer.sin (PI*v), buffer.cos (PI*v));
    //   gl_FragColor = vec4(col*.5 + .5, 1);

    return v * data.Line_Density * PI;
}


export void PlasmaEffectStyle0(const uniform PlasmaData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    const uniform float pi3 = PI / 3.0;
    const uniform float pi2 = PI * 2.0;
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        float vldpi = plasmaCalc_vldpi(data, x, y);
        float h = (sin (vldpi + 2.0 * pi3) + 1.0) * 0.5;
        result[index] = plasma_getMultiColorBlend(data, h, false);
    }
}



export void PlasmaEffectStyle1(const uniform PlasmaData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    const uniform float pi3 = PI / 3.0;
    const uniform float pi2 = PI * 2.0;
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);
        float vldpi = plasmaCalc_vldpi(data, x, y);
        uint8<4> ret;
        ret.r = (sin(vldpi) + 1.0) * 255.0 / 2.0;
        ret.g = (cos(vldpi) + 1.0) * 255.0 / 2.0;
        ret.b = 0;
        ret.a = 255;
        result[index] = ret;
     }
}



export void PlasmaEffectStyle2(const uniform PlasmaData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    const uniform float pi3 = PI / 3.0;
    const uniform float pi2 = PI * 2.0;
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        float vldpi = plasmaCalc_vldpi(data, x, y);
        uint8<4> ret;
        ret.b = (sin(vldpi) + 1.0) * 255.0 / 2.0;
        ret.g = (cos(vldpi) + 1.0) * 255.0 / 2.0;
        ret.r = 1;
        ret.a = 255;
        result[index] = ret;
    }
}



export void PlasmaEffectStyle3(const uniform PlasmaData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    const uniform float pi3 = PI / 3.0;
    const uniform float pi2 = PI * 2.0;
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);

        float vldpi = plasmaCalc_vldpi(data, x, y);
        
        uint8<4> ret;
        ret.r = (sin(vldpi) + 1.0) * 255.0 / 2.0;
        ret.g = (sin(vldpi + 2.0 * pi3) + 1.0) * 255.0 / 2.0;
        ret.b = (sin(vldpi + 4.0 * pi3) + 1.0) * 255.0 / 2.0;
        ret.a = 255;
        result[index] = ret;
    }
}



export void PlasmaEffectStyle4(const uniform PlasmaData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[]) {
    const uniform float pi3 = PI / 3.0;
    const uniform float pi2 = PI * 2.0;
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;

    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);
        float vldpi = plasmaCalc_vldpi(data, x, y);

        uint8<4> ret;
        ret.r = (sin(vldpi) + 1.0) * 255.0 / 2.0;
        ret.g = ret.r;
        ret.b = ret.r;
        ret.a = 255;
        result[index] = ret;
    }
}
