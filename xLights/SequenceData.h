//
//  SequenceData.h
//  xLights
//
//  Created by Daniel Kulp on 1/12/15.
//  Copyright (c) 2015 Daniel Kulp. All rights reserved.
//

#ifndef xLights_SequenceData_h
#define xLights_SequenceData_h


class FrameData {
public:
    FrameData(int nc, unsigned char *d) {
        numChannels = nc;
        data = d;
        zero = 0;
    }
    
    unsigned char &operator[](int channel) {
        zero = 0;
        return channel < numChannels ? data[channel] : zero;
    }
private:
    unsigned char zero;
    int numChannels;
    unsigned char* data;
};

class SequenceData {
public:
    SequenceData();
    virtual ~SequenceData();
    
    void init(int numChannels, int numFrames, int frameTime);
    

    int TotalTime() { return numFrames * frameTime; }
    bool OK(int frame, int channel) { return frame < numFrames && channel < numChannels; }
    
    FrameData operator[](int frame);
    
    int NumChannels() { return numChannels;}
    int NumFrames() { return numFrames;}
    int FrameTime() { return frameTime;}

private:
    unsigned char *invalidData;
    unsigned char *data;
    int numChannels;
    int numFrames;
    int frameTime;
};
#endif
