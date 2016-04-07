#ifndef FILECONVERTER_H
#define FILECONVERTER_H

#ifndef FPP
#include <wx/string.h>
#endif
#include "NetInfo.h"
#include "DataLayer.h"
#include "SequenceData.h"

class xLightsFrame; // forward declare to prevent including the world
class ConvertDialog;

class ConvertParameters
{
public:
    enum ReadMode
    {
        READ_MODE_HEADER_ONLY,  // use this mode to read only the header info from the file
        READ_MODE_LOAD_MAIN,    // use this mode to load the file into the main sequencer
        READ_MODE_NORMAL,       // use this mode to read data into provided sequence data object
        READ_MODE_IGNORE_BLACK  // use this mode to read data into provided sequence data object ignoring any black pixels
    };

    const wxString inp_filename;
    const wxString out_filename;
    int sequence_interval;
    SequenceData& seq_data;
    NetInfoClass& NetInfo;
    DataLayer* data_layer;
    wxString* media_filename;
    bool channels_off_at_end;
    bool map_empty_channels;
    bool map_no_network_channels;
    ReadMode read_mode;
    xLightsFrame* xLightsFrm;
    ConvertDialog* convertDialog;

    void SetStatusText(wxString msg);
    void ConversionError(wxString msg);
    void PlayerError(wxString msg);
    void AppendConvertStatus(const wxString &msg, bool flushBuffer = true);

    ConvertParameters( wxString inp_filename_,
                       SequenceData& seq_data_,
                       NetInfoClass& NetInfo_,
                       ReadMode read_mode_,
                       xLightsFrame* xLightsFrm_,
                       ConvertDialog* convertDialog_,
                       wxString* media_filename_ = nullptr,
                       DataLayer* data_layer_ = nullptr,
                       wxString out_filename_ = wxEmptyString,
                       int sequence_interval_ = 50,
                       bool channels_off_at_end_ = false,
                       bool map_empty_channels_= false,
                       bool map_no_network_channels_ = false);

};

class FileConverter
{
    public:
        FileConverter();

        static void ReadLorFile(ConvertParameters& params);
        static void ReadXlightsFile(ConvertParameters& params);
        static void ReadHLSFile(ConvertParameters& params);
        static void ReadVixFile(ConvertParameters& params);
        static void ReadGlediatorFile(ConvertParameters& params);
        static void ReadConductorFile(ConvertParameters& params);
        static void ReadFalconFile(ConvertParameters& params);
        static void WriteFalconPiFile(ConvertParameters& params);

    protected:
    private:
};

#endif // FILECONVERTER_H
