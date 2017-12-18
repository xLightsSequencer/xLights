#include "DataLayer.h"
#include <algorithm>

DataLayer::DataLayer(wxString name, wxString source, wxString data_source)
: mName(name),
  mSource(source),
  mDataSource(data_source),
  num_channels(0),
  num_frames(0),
  channel_offset(0)
{
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
    if(index < mDataLayers.size())
    {
        return mDataLayers[index];
    }
    else
    {
        return nullptr;
    }
}
#include <wx/msgdlg.h>
void DataLayerSet::RemoveDataLayer(int index)
{
    if(index<mDataLayers.size())
    {
        DataLayer *layer = mDataLayers[index];
        mDataLayers.erase(mDataLayers.begin()+index);
        delete layer;
    }
}

DataLayer* DataLayerSet::AddDataLayer( wxString name, wxString source, wxString data_source)
{
    DataLayer* layer = new DataLayer(name, source, data_source);
    mDataLayers.push_back(layer);
    return layer;
}

void DataLayerSet::MoveLayerUp( int index )
{
    if( index > 0 )
    {
        DataLayer* tmp = mDataLayers[index-1];
        mDataLayers[index-1] = mDataLayers[index];
        mDataLayers[index] = tmp;
    }
}

void DataLayerSet::MoveLayerDown( int index )
{
    if( index <  mDataLayers.size()-1 )
    {
        DataLayer* tmp = mDataLayers[index+1];
        mDataLayers[index+1] = mDataLayers[index];
        mDataLayers[index] = tmp;
    }
}
