/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DataLayer.h"
#include "UtilFunctions.h"
#include <algorithm>
#include <wx/msgdlg.h>

DataLayer::DataLayer(wxString name, wxString source, wxString data_source)
: mName(name),
  mSource(source),
  mDataSource(data_source),
  num_channels(0),
  num_frames(0),
  channel_offset(0)
{
    if (!source.StartsWith("<")) {
        mSource = FixFile("", source);
    }
    if (!data_source.StartsWith("<")) {
        mDataSource = FixFile("", data_source);
    }
}

DataLayerSet::DataLayerSet()
{

}

DataLayerSet::~DataLayerSet()
{
    for (int i = 0; i < mDataLayers.size(); ++i)
    {
        delete mDataLayers[i];
    }
}

DataLayer* DataLayerSet::GetDataLayer(size_t index)
{
    if (index < mDataLayers.size()) {
        return mDataLayers[index];
    }
    else {
        return nullptr;
    }
}

void DataLayerSet::RemoveDataLayer(int index)
{
    if (index < mDataLayers.size()) {
        DataLayer* layer = mDataLayers[index];
        mDataLayers.erase(mDataLayers.begin() + index);
        delete layer;
    }
}

DataLayer* DataLayerSet::AddDataLayer(wxString name, wxString source, wxString data_source)
{
    DataLayer* layer = new DataLayer(name, source, data_source);
    mDataLayers.push_back(layer);
    return layer;
}

void DataLayerSet::MoveLayerUp(int index)
{
    if (index > 0) {
        DataLayer* tmp = mDataLayers[index - 1];
        mDataLayers[index - 1] = mDataLayers[index];
        mDataLayers[index] = tmp;
    }
}

void DataLayerSet::MoveLayerDown(int index)
{
    if (index < mDataLayers.size() - 1) {
        DataLayer* tmp = mDataLayers[index + 1];
        mDataLayers[index + 1] = mDataLayers[index];
        mDataLayers[index] = tmp;
    }
}
