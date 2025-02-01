export struct VideoData {
    uint32 width;
    uint32 height;
    int32 yoffset;
    int32 xoffset;
    int32 ytail;
    int32 xtail;
    bool transparentBlack;
    int32 transparentBlackLevel;
    int32 image_width;
    int32 image_height;
    int32 startx;
    int32 starty;
    int32 sampleSpacing;

    int8 ch;
    uint8* image;
    void* bufferData;
};

export void VideoEffectProcess(const uniform VideoData &data,
                                  uniform int startIdx, uniform int endIdx,
                                  uniform uint8<4> result[])
{
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;
    uniform float height = data.height;
    uniform int8 ch = data.ch;

    uint8<4> c;
    foreach (index = startIdx...endIdx) {
        float fidx = index;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);
        float newindex = x + data.startx + ((y + data.starty) * width);

        uint8* ptr = data.image + (data.image_height - 1 - y - data.yoffset) * (data.image_width * ch) + (data.xoffset * ch) + (x * ch);

        c.x = *(ptr);
        c.y = *(ptr + 1);
        c.z = *(ptr + 2);
        c.w = ch == 3 ? 255 : *(ptr + 3);
        if (data.transparentBlack) {
            int level = c.x + c.y + c.z;
            if (level <= data.transparentBlackLevel) {
                c.x = 0;
                c.y = 0;
                c.z = 0;
                c.w = 0;
            }
            result[newindex] = c;
        } else {
            result[newindex] = c;
        }
    }
}

export void VideoEffectProcessSample(const uniform VideoData &data,
                                        uniform int startIdx, uniform int endIdx, 
                                        uniform uint8<4> result[]) {
    uniform float width = data.width;
    uniform float rszw = 1.0 / width;
    uniform float height = data.height;
    uniform int8 ch = data.ch;

    uniform int imageWidth = data.image_width;
    uniform int imageHeight = data.image_height;
    uniform int startx = data.startx;
    uniform int starty = data.starty;

    uint8<4> c;
    foreach (i = startIdx ... endIdx) {
        float fidx = i;
        float y = (floor)(fidx * rszw + 0.00001);
        float x = fidx - (y * width);
        float newIndex = (y * width) + x;

        int curx = startx + x * data.sampleSpacing;
        int cury = starty + y * data.sampleSpacing;

        if (curx >= 0 && curx < imageWidth && cury >= 0 && cury < imageHeight) {

            uint8* ptr = data.image + ((imageHeight - 1 - cury) * imageWidth * ch) + (curx * ch);

            c.x = *(ptr);
            c.y = *(ptr + 1);
            c.z = *(ptr + 2);
            c.w = ch == 3 ? 255 : *(ptr + 3);
            result[newIndex] = c;
        }
    }
}
