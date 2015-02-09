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
    DataLayer* layer = new DataLayer(name, source, mDataLayers.size());
    mDataLayers.push_back(layer);
}

void DataLayerSet::AddDataLayer( wxString name, wxString source, wxString order)
{
    int value = atoi(order.c_str());
    DataLayer* layer = new DataLayer(name, source, value);
    mDataLayers.push_back(layer);
}

void DataLayerSet::MoveLayerUp( int index )
{
    if( index > 0 )
    {
        mDataLayers[index]->SetOrder(index-1);
        mDataLayers[index-1]->SetOrder(index);
        SortLayers();
    }
}

void DataLayerSet::MoveLayerDown( int index )
{
    if( index <  mDataLayers.size()-1 )
    {
        mDataLayers[index]->SetOrder(index+1);
        mDataLayers[index+1]->SetOrder(index);
        SortLayers();
    }
}

void DataLayerSet::SortLayers()
{
    std::sort(mDataLayers.begin(),mDataLayers.end(),SortDataLayersByOrder);
}

bool DataLayerSet::SortDataLayersByOrder(DataLayer* layer1, DataLayer* layer2)
{
    return layer1->GetOrder() < layer2->GetOrder();
}
