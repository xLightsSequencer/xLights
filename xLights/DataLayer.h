#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>
#include "SequenceData.h"

class DataLayer
{
    public:
        DataLayer(const std::string& name, const std::string& source, const std::string& data_source);

        virtual ~DataLayer() {};

        const std::string& GetName() const { return mName; }
        void SetName(const std::string& name) { mName = name; }

        const std::string& GetSource() const { return mSource; }
        void SetSource(const std::string& source) { mSource = source; }

        const std::string& GetDataSource() const { return mDataSource; }
        void SetDataSource(const std::string& data_source) { mDataSource = data_source; }

        int GetNumChannels() const { return num_channels; }
        void SetNumChannels(int val) { num_channels = val; }

        int GetChannelOffset() const { return channel_offset; }
        void SetChannelOffset(int val) { channel_offset = val; }

        int GetNumFrames() const { return num_frames; }
        void SetNumFrames(int val) { num_frames = val; }

        int GetLORConvertParams() const { return lor_convert_params; }
        void SetLORConvertParams(int val) { lor_convert_params = val; }

        SequenceData& GetSequenceData() { return sequence_data; }

    private:
        std::string mName;
        std::string mSource;
        std::string mDataSource;
        int num_channels;
        int num_frames;
        int channel_offset;
        int lor_convert_params;
        SequenceData sequence_data;
};

class DataLayerSet
{
    public:
        DataLayerSet();
        virtual ~DataLayerSet();

        DataLayer* GetDataLayer(size_t index);
        void RemoveDataLayer(int index);

        int GetNumLayers() const { return mDataLayers.size(); }

        DataLayer* AddDataLayer(const std::string& name, const std::string& source, const std::string& data_source);
        void MoveLayerUp( int index );
        void MoveLayerDown( int index );

    private:
        std::vector<DataLayer*> mDataLayers;
};

