#include "DataLayer.h"
#include <algorithm>

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

DataLayer* DataLayerSet::GetDataLayer(int index)
{
    if(index < mDataLayers.size())
    {
        return mDataLayers[index];
    }
    else
    {
        nullptr;
    }
}

void DataLayerSet::RemoveDataLayer(int index)
{
    if(index<mDataLayers.size())
    {
        DataLayer *layer = mDataLayers[index];
        mDataLayers.erase(mDataLayers.begin()+index);
        delete layer;
    }
}

void DataLayerSet::AddDataLayer( wxString name, wxString source)
{
    DataLayer* layer = new DataLayer(name, source);
    mDataLayers.push_back(layer);
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
