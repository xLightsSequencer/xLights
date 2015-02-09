#ifndef DATALAYER_H
#define DATALAYER_H

#include <wx/string.h>
#include <vector>

class DataLayer
{
    public:
        DataLayer(wxString name, wxString source, int order)
        : mName(name), mSource(source), mOrder(order) {};

        virtual ~DataLayer() {};

        wxString GetName() { return mName; }
        void SetName(wxString name) { mName = name; }

        wxString GetSource() { return mSource; }
        void SetSource(wxString source) { mSource = source; }

        int GetOrder() { return mOrder; }
        void SetOrder(int order) { mOrder = order; }

    private:
        wxString mName;
        wxString mSource;
        int mOrder;
};

class DataLayerSet
{
    public:
        DataLayerSet();
        virtual ~DataLayerSet();

        DataLayer* GetDataLayer(int index);
        void RemoveDataLayer(int index);

        int GetNumLayers() { return mDataLayers.size(); }

        void AddDataLayer( wxString name, wxString source);
        void AddDataLayer( wxString name, wxString source, wxString order);
        void MoveLayerUp( int index );
        void MoveLayerDown( int index );

        void SortLayers();
        static bool SortDataLayersByOrder(DataLayer* layer1,DataLayer* layer2);

    private:
        std::vector<DataLayer*> mDataLayers;
};

#endif // DATALAYER_H
