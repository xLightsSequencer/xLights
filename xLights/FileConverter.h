#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <vector>

#ifndef FPP
#include <wx/string.h>
#endif
#include "DataLayer.h"
#include "SequenceData.h"
#include "Color.h"

class xLightsFrame; // forward declare to prevent including the world
class ConvertDialog;
class ConvertLogDialog;
class OutputManager;
class wxArrayInt;
class wxArrayString;

class ConvertParameters
{
public:
    enum ReadMode
    {
        READ_MODE_HEADER_ONLY,  // use this mode to read only the header info from the file
        READ_MODE_LOAD_MAIN,    // use this mode to load the file into the main sequencer
        READ_MODE_NORMAL,       // use this mode to read data into provided sequence data object
        READ_MODE_IGNORE_BLACK, // use this mode to read data into provided sequence data object ignoring any black pixels
        READ_MODE_IMPORT        // use this mode to read data into provided sequence data object during import
    };

    const wxString inp_filename;
    const wxString out_filename;
    int sequence_interval;
    SequenceData& seq_data;
    OutputManager* _outputManager = nullptr;
    DataLayer* data_layer = nullptr;
    std::string* media_filename = nullptr;
    bool channels_off_at_end = false;
    bool map_empty_channels = false;
    bool map_no_network_channels = false;
    ReadMode read_mode;
    xLightsFrame* xLightsFrm = nullptr;
    ConvertDialog* convertDialog = nullptr;
    ConvertLogDialog* convertLogDialog = nullptr;

    void SetStatusText(wxString msg);
    void ConversionError(wxString msg);
    void PlayerError(wxString msg);
    void AppendConvertStatus(const wxString &msg, bool flushBuffer = true);

    ConvertParameters( wxString inp_filename_,
                       SequenceData& seq_data_,
                       OutputManager* outputManager_,
                       ReadMode read_mode_,
                       xLightsFrame* xLightsFrm_,
                       ConvertDialog* convertDialog_,
                       ConvertLogDialog* convertLogDialog_,
                       std::string* media_filename_ = nullptr,
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

    
        static bool LoadVixenProfile(ConvertParameters& params, const wxString& ProfileName,
                                     wxArrayInt& VixChannels, wxArrayString& VixChannelNames,
                                     std::vector<xlColor> &VixChannelColors);

    protected:
    private:
};
