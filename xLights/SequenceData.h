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
    void Zero()
    {
        memset(data, 0x00, numChannels);
    }
    FrameData(unsigned int nc, unsigned char *d) {
        numChannels = nc;
        data = d;
        zero = 0;
    }
    
    unsigned char &operator[](unsigned int channel) {
        zero = 0;
        return channel < numChannels ? data[channel] : zero;
    }
    
    const unsigned char *operator[](unsigned int channel) const {
        const unsigned char* cdata = data;
        return channel < numChannels ? &cdata[channel] : &constzero;
    }
private:
    unsigned char zero;
    unsigned int numChannels;
    unsigned char* data;
    
    static const unsigned char constzero;
};

class SequenceData {
public:
    SequenceData();
    virtual ~SequenceData();
    
    void init(unsigned int numChannels, unsigned int numFrames, unsigned int frameTime);
    

    unsigned int TotalTime() const { return numFrames * frameTime; }
    bool OK(unsigned int frame, unsigned int channel) const { return frame < numFrames && channel < numChannels; }
    
    FrameData operator[](unsigned int frame);
    const FrameData operator[](unsigned int frame) const;
    
    unsigned int NumChannels() const { return numChannels;}
    unsigned int NumFrames() const { return numFrames;}
    unsigned int FrameTime() const { return frameTime;}
    bool IsValidData() const { return data != NULL; }

private:
    SequenceData(const SequenceData&);  //make sure we cannot "copy" these
    SequenceData &operator=(const SequenceData& rgb);
    
    unsigned char *invalidData;
    unsigned char *data;
    unsigned int numChannels;
    unsigned int numFrames;
    unsigned int frameTime;
};
#endif
