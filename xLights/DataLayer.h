#ifndef DATALAYER_H
#define DATALAYER_H

#include <wx/string.h>
#include <vector>

class DataLayer
{
    public:
        DataLayer(wxString name, wxString source)
        : mName(name), mSource(source) {};

        virtual ~DataLayer() {};

        wxString GetName() { return mName; }
        void SetName(wxString name) { mName = name; }

        wxString GetSource() { return mSource; }
        void SetSource(wxString source) { mSource = source; }

    private:
        wxString mName;
        wxString mSource;
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
        void MoveLayerUp( int index );
        void MoveLayerDown( int index );

    private:
        std::vector<DataLayer*> mDataLayers;
};

#endif // DATALAYER_H
