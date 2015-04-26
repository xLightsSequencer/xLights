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
    
    const unsigned char *operator[](int channel) const {
        const unsigned char* cdata = data;
        return channel < numChannels ? &cdata[channel] : &constzero;
    }
private:
    unsigned char zero;
    int numChannels;
    unsigned char* data;
    
    static const unsigned char constzero;
};

class SequenceData {
public:
    SequenceData();
    virtual ~SequenceData();
    
    void init(int numChannels, int numFrames, int frameTime);
    

    int TotalTime() const { return numFrames * frameTime; }
    bool OK(int frame, int channel) const { return frame < numFrames && channel < numChannels; }
    
    FrameData operator[](int frame);
    const FrameData operator[](int frame) const;
    
    int NumChannels() const { return numChannels;}
    int NumFrames() const { return numFrames;}
    int FrameTime() const { return frameTime;}

private:
    SequenceData(const SequenceData&);  //make sure we cannot "copy" these
    SequenceData &operator=(const SequenceData& rgb);
    
    unsigned char *invalidData;
    unsigned char *data;
    int numChannels;
    int numFrames;
    int frameTime;
};
#endif
