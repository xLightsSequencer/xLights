#include "LOROptimisedOutput.h"
#include "LOROptimisedDialog.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>

#pragma region Constructors and Destructors
LOROptimisedOutput::LOROptimisedOutput(SerialOutput* output)
: LOROutput(output)

{
    SetupHistory();
}

LOROptimisedOutput::LOROptimisedOutput(wxXmlNode* node)
: LOROutput(node), _controllers(node)
{
    SetupHistory();
    CalcTotalChannels();
}

LOROptimisedOutput::LOROptimisedOutput() : LOROutput()
{
    SetupHistory();
}

LOROptimisedOutput::~LOROptimisedOutput()
{
}

void LOROptimisedOutput::SetupHistory()
{
    for( size_t i=0; i < 32; ++i ) {
        banks_changed[i] = true;
    }
}
#pragma endregion Constructors and Destructors

#pragma region Save
void LOROptimisedOutput::Save(wxXmlNode* node)
{
    wxXmlNode* cntrl_node = new wxXmlNode(wxXML_ELEMENT_NODE, "controllers");
    node->AddChild(cntrl_node);
    _controllers.Save(cntrl_node);
    SerialOutput::Save(node);
}
#pragma endregion Save

#pragma region Data Setting
void LOROptimisedOutput::SetManyChannels(long channel, unsigned char data[], long size)
{

    if (!_enabled || _serial == nullptr || !_ok) return;

    size_t idx = 0;  // running index for placing next byte
    int cur_channel = channel;
    wxByte d[4096];
    int controller_offset = 0;

    for (auto it = _controllers.GetControllers()->begin(); it != _controllers.GetControllers()->end(); ++it)
    {
        int channel_count = (*it)->GetNumChannels();
        int unit_id = (*it)->GetUnitId();
        LorController::AddressMode addr_mode = (*it)->GetAddressMode();

        int controller_channels_to_process = channel_count;
        int channels_per_pass = controller_channels_to_process;
        if( addr_mode == LorController::LOR_ADDR_MODE_LEGACY ) {
            channels_per_pass = 16;
        }
        else if( addr_mode == LorController::LOR_ADDR_MODE_SPLIT ) {
            channels_per_pass = channel_count/2;
        }

        while( controller_channels_to_process > 0 ) {
            std::vector< std::vector<LORDataPair> > lorBankData;
            lorBankData.resize((channels_per_pass/16)+1);

            bool bank_changed = false;
            bool frame_changed = false;
            bool color_mode[32];

            // gather all the data and compress common values on a per 16 channel bank basis
            int channels_to_process = channels_per_pass;
            int chan_offset = 0;
            int shift_offset = 0;
            color_mode[0] = false;
            while (channels_to_process > 0) {
                bool processed = false;

                if ((data[cur_channel] > 0) && (data[cur_channel] < 0xFF)) {
                    color_mode[shift_offset] = true;
                }

                for (int i = 0; i < lorBankData[shift_offset].size(); ++i) {
                    if( lorBankData[shift_offset][i].value == data[cur_channel] ) {
                        lorBankData[shift_offset][i].bits |= (1 << chan_offset);
                        processed = true;
                        break;
                    }
                }

                if (!processed) {
                    lorBankData[shift_offset].push_back(LORDataPair(data[cur_channel], 1 << chan_offset));
                }

                if (data[cur_channel] != _lastSent[cur_channel]) {
                    bank_changed = true;
                    frame_changed = true;
                }
                _lastSent[cur_channel] = data[cur_channel];

                chan_offset++;
                if (chan_offset == 16 || (channels_to_process == 1)) {
                    chan_offset = 0;
                    banks_changed[shift_offset] = bank_changed;
                    bank_changed = false;
                    shift_offset++;
                    color_mode[shift_offset] = false;
                }
                channels_to_process--;
                cur_channel++;
            }

            // now build the commands to send out the serial port
            for (int bank = lorBankData.size() - 1; bank >= 0; --bank) {
                if (banks_changed[bank]) {
                    int num_bank_records = lorBankData[bank].size();

                    // process channels that are zero
                    for( int i = 0; i < num_bank_records; ++i ) {
                        if( lorBankData[bank][i].value == 0 ) {
                            if( num_bank_records == 1 ) {  // entire bank is zero
                                d[idx++] = 0;
                                d[idx++] = unit_id;
                                if( bank > 0) {
                                    d[idx++] = 0x51;
                                    d[idx++] = 0xC0 | bank;
                                }
                                else {
                                    d[idx++] = 0x41;
                                }
                            } else if( color_mode[bank] ) {
                                // send a value byte command for 0 if in color mode
                                // otherwise its either all off or all on and will be covered
                                // by other commands.
                                wxByte lsb = lorBankData[bank][i].bits & 0xFF;
                                wxByte msb = lorBankData[bank][i].bits >> 8;
                                d[idx++] = 0;
                                d[idx++] = unit_id;
                                d[idx++] = 0x53;
                                d[idx++] = _data[0];
                                d[idx] = bank;
                                if (!msb) {
                                    d[idx] |= 0x40;
                                }
                                else if (!lsb) {
                                    d[idx] |= 0x80;
                                }
                                idx++;
                                if (lsb) {
                                    d[idx++] = lsb;
                                }
                                if (msb) {
                                    d[idx++] = msb;
                                }
                            }
                        }
                    }

                    // send all the channels with 0xFF first
                    for (int i = 0; i < num_bank_records; ++i) {
                        if (lorBankData[bank][i].value == 0xFF) {
                            wxByte lsb = lorBankData[bank][i].bits & 0xFF;
                            wxByte msb = lorBankData[bank][i].bits >> 8;
                            bool value_byte = lorBankData[bank][i].value != 0xFF;
                            GenerateCommand(d, idx, unit_id, bank, value_byte, _data[lorBankData[bank][i].value], lsb, msb);
                        }
                    }

                    // now send all commands that are values between 0 and FF
                    for (int i = 0; i < num_bank_records; ++i) {
                        if ((lorBankData[bank][i].value != 0) && (lorBankData[bank][i].value != 0xFF)) {
                            wxByte lsb = lorBankData[bank][i].bits & 0xFF;
                            wxByte msb = lorBankData[bank][i].bits >> 8;
                            bool value_byte = lorBankData[bank][i].value != 0xFF;
                            GenerateCommand(d, idx, unit_id, bank, value_byte, _data[lorBankData[bank][i].value], lsb, msb);
                        }
                    }
                }
            }
            if( frame_changed ) {
                d[idx++] = 0x0;
                d[idx++] = 0x0;
            }

            if (_serial != nullptr && frame_changed)
            {
                _serial->Write((char *)(&d[controller_offset]), idx);
            }
            controller_offset = idx;

            for (int bank = 0; bank < lorBankData.size(); bank++) {
                lorBankData[bank].clear();
            }
            lorBankData.clear();
            controller_channels_to_process -= channels_per_pass;
            unit_id++;
        }
    }


}

void LOROptimisedOutput::GenerateCommand(wxByte d[], size_t& idx, int unit_id, int bank, bool value_byte, wxByte dbyte, wxByte lsb, wxByte msb)
{
    d[idx++] = 0;        // Leading zero
    d[idx++] = unit_id;  // Unit ID
                         // command byte w/optional value & shift bytes
    if (bank > 0) {
        d[idx] = 0x51;
        if (value_byte) {
            d[idx++] |= 0x02;
            d[idx] = dbyte;
        }
        idx++;
        d[idx] = bank;
        if (!msb) {
            d[idx] |= 0x40;
        }
        else if (!lsb) {
            d[idx] |= 0x80;
        }
        idx++;
    }
    else {
        if (!msb) {
            d[idx] = 0x31;
        }
        else if (!lsb) {
            d[idx] = 0x21;
        }
        else {
            d[idx] = 0x11;
        }
        if (value_byte) {
            d[idx++] |= 0x02;
            d[idx] = dbyte;
        }
        idx++;
    }
    if (lsb) {
        d[idx++] = lsb;
    }
    if (msb) {
        d[idx++] = msb;
    }
}

void LOROptimisedOutput::AllOff()
{
    int channels_to_process = _channels;
    int bank = 0;
    wxByte d[4096];
    size_t idx = 0;
    int controller_offset = 0;

    for (auto it = _controllers.GetControllers()->begin(); it != _controllers.GetControllers()->end(); ++it)
    {
        LorController::AddressMode addr_mode = (*it)->GetAddressMode();
        int unit_id = (*it)->GetUnitId();

        int channel_count = (*it)->GetNumChannels();
        int controller_channels_to_process = channel_count;
        int channels_per_pass = controller_channels_to_process;
        if( addr_mode == LorController::LOR_ADDR_MODE_LEGACY ) {
            channels_per_pass = 16;
        }
        else if( addr_mode == LorController::LOR_ADDR_MODE_SPLIT ) {
            channels_per_pass = channel_count/2;
        }

        while( controller_channels_to_process > 0 ) {
            int channels_to_process = channels_per_pass;
            while (channels_to_process > 0) {
                d[idx++] = 0;
                d[idx++] = unit_id;
                if( bank > 0) {
                    d[idx++] = 0x51;
                    d[idx++] = 0xC0 | bank;
                }
                else {
                    d[idx++] = 0x41;
                }
                channels_to_process -= 16;
                bank++;
            }
            d[idx++] = 0x0;
            d[idx++] = 0x0;

            if (_serial != nullptr)
            {
                _serial->Write((char *)(&d[controller_offset]), idx);
            }
            controller_offset = idx;
            controller_channels_to_process -= channels_per_pass;
            unit_id++;
        }
    }

    SendHeartbeat();
    _lastheartbeat = _timer_msec;
    wxMilliSleep(50);
}
#pragma endregion Data Setting

#pragma region Getters and Setters
std::string LOROptimisedOutput::GetSetupHelp() const
{
    return "LOR controllers attached to any LOR dongle.\nMax of 8 channels at 9600 baud.\nMax of 48 channels at 57600 baud.\nMax of 96 channels at 115200 baud.\nRun your controllers in DMX mode for higher throughput.";
}
#pragma endregion Getters and Setters

#pragma region UI
#ifndef EXCLUDENETWORKUI
// This is a bit funky as we will need to create a serial output then mutate it into the correct output type
Output* LOROptimisedOutput::Configure(wxWindow* parent, OutputManager* outputManager)
{
    LOROptimisedOutput* result = this;

    LorOptimisedDialog dlg(parent, &result, &_controllers, outputManager);

    int res = dlg.ShowModal();

    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    CalcTotalChannels();

    return result;
}
#endif
#pragma endregion UI

void LOROptimisedOutput::CalcTotalChannels()
{
    int channel_count = 0;
    for (auto it = _controllers.GetControllers()->begin(); it != _controllers.GetControllers()->end(); ++it)
    {
        channel_count += (*it)->GetNumChannels();
    }
    _channels = channel_count;
}
