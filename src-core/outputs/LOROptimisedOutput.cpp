
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LOROptimisedOutput.h"
#include <cstring>
#include "Controller.h"
#include "../models/OutputModelManager.h"
#include "UtilFunctions.h"

#include "serial.h"

#include <cassert>
#include <cstdlib>

#include <log.h>

#define MINIMUM_MILLIS_AFTER_WRITE_BEFORE_CLOSE 100


// Property grid choices moved to ui/controllerproperties/OutputPropertyAdapters
void LOROptimisedOutput::InitialiseTypes() {
    // Choices initialization moved to LOROptimisedOutputPropertyAdapter
}

#pragma region Private Functions
void LOROptimisedOutput::SaveAttr(pugi::xml_node node) {

    pugi::xml_node cntrl_node = node.append_child("controllers");
    _controllers.Save(cntrl_node);
    SerialOutput::SaveAttr(node);
}

void LOROptimisedOutput::CalcChannels(int& channel_count, int& channels_per_pass, int& controller_channels_to_process, LorController* cntrl) {

    LorController::AddressMode addr_mode = cntrl->GetAddressMode();
    controller_channels_to_process = channel_count;
    channels_per_pass = controller_channels_to_process;
    std::string type = cntrl->GetType();
    int unit_id = cntrl->GetUnitId();
    if ((type == "Pixie2") || (type == "Pixie4") || (type == "Pixie8") || (type == "Pixie16")) {
        std::size_t found = type.find("Pixie");
        if (found != std::string::npos) {
            int outputs_per_card = (int)std::strtol(type.substr(found + 5, type.length() - found - 5).c_str(), nullptr, 10);
            channels_per_pass = channel_count;
            channel_count = outputs_per_card * channels_per_pass;
            controller_channels_to_process = channel_count;
            for (int i = 0; i < outputs_per_card; i++) {
                unit_id_in_use[unit_id + i] = true;
            }
        }
    }
    else {
        if (addr_mode == LorController::AddressMode::LOR_ADDR_MODE_LEGACY) {
            channels_per_pass = 16;
            int num_ids = channel_count / channels_per_pass;
            if ((num_ids * channels_per_pass) < channel_count) {
                ++num_ids;
            }
            for (int i = 0; i < num_ids; ++i) {
                unit_id_in_use[unit_id + i] = true;
            }
        }
        else if (addr_mode == LorController::AddressMode::LOR_ADDR_MODE_SPLIT) {
            channels_per_pass = channel_count / 2;
            unit_id_in_use[unit_id] = true;
            unit_id_in_use[unit_id + 1] = true;
        }
    }
}

void LOROptimisedOutput::CalcTotalChannels() {
    int total_channels = 0;
    int channel_count = 0;
    int controller_channels_to_process = 0;
    int channels_per_pass = 0;
    for (size_t i = 0; i < 255; ++i) {
        unit_id_in_use[i] = false;
    }
    unit_id_in_use[0] = true;  // we don't use id 0
    for (const auto& it : _controllers.GetControllers()) {
        channel_count = it->GetNumChannels();
        int unit_id = it->GetUnitId();
        unit_id_in_use[unit_id] = true;
        CalcChannels(channel_count, channels_per_pass, controller_channels_to_process, it);
        total_channels += channel_count;
    }
    _channels = total_channels;
}

void LOROptimisedOutput::GenerateCommand(uint8_t d[], size_t& idx, int unit_id, int bank, bool value_byte, uint8_t dbyte, uint8_t lsb, uint8_t msb) {

    d[idx++] = 0;        // Leading zero
    d[idx++] = unit_id;  // Unit ID
                         // command byte w/optional value & shift bytes
    if (bank > 0) {
        d[idx] = 0x51;
        if (value_byte) {
            d[idx++] |= 0x02;
            d[idx] = dbyte;
        }
        ++idx;
        d[idx] = bank;
        if (!msb) {
            d[idx] |= 0x40;
        }
        else if (!lsb) {
            d[idx] |= 0x80;
        }
        ++idx;
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
        ++idx;
    }
    if (lsb) {
        d[idx++] = lsb;
    }
    if (msb) {
        d[idx++] = msb;
    }
}
#pragma endregion

#pragma region Constructors and Destructors
LOROptimisedOutput::LOROptimisedOutput(const LOROptimisedOutput& from) :
    LOROutput(from), _controllers(from._controllers)
{
    InitialiseTypes();
    SetupHistory();
}

LOROptimisedOutput::LOROptimisedOutput(pugi::xml_node node) : LOROutput(node), _controllers(node) {
    InitialiseTypes();
    SetupHistory();
    CalcTotalChannels();
}

LOROptimisedOutput::LOROptimisedOutput() : LOROutput() {
    InitialiseTypes();
    SetupHistory();
    GetControllers().GetControllers().push_back(new LorController());
}

void LOROptimisedOutput::SetupHistory() {
    for (size_t i = 0; i < MAX_BANKS; ++i) {
        banks_changed[i] = true;
    }
}
#pragma endregion 

#pragma region Frame Handling

bool LOROptimisedOutput::Open()
{
    //_framesSinceForcedOutput = 0xFF;
    _changed = true;
    return LOROutput::Open();
}

void LOROptimisedOutput::EndFrame(int suppressFrames)
{

    if (!_enabled || _suspend) return;

    if (_changed) {
        SetManyChannels(0, _curData, 0);
        _changed = false;
    }
    LOROutput::EndFrame(suppressFrames);
}
#pragma endregion 

#pragma region Data Setting
void LOROptimisedOutput::SetOneChannel(int32_t channel, unsigned char data) {

    if (!_enabled || _serial == nullptr || !_ok) return;

    if (!_changed) {
        // Don't try to only send changes since this is used for test mode
        // and not all channels are written every frame
        SetupHistory();
        assert(sizeof(_curData) <= sizeof(_lastSent));
        memset(_curData, 0x00, sizeof(_curData));
        memset(_lastSent, 0xFF, sizeof(_curData));
        _changed = true;
    }

    assert((size_t)channel < sizeof(_curData));
    _curData[channel] = data;
}

#define LOR_FORCE_SEND_FRAMES 20
void LOROptimisedOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size) {

    

    if (!_enabled || _serial == nullptr || !_ok) return;

    if (!TxEmpty()) {
        spdlog::debug("    LOROptimisedOutput: SetManyChannels skipped due to transmit buffer stackup");
        return;
    }

    int cur_channel = channel;
    int total_bytes_sent = 0;

    for (const auto& it : _controllers.GetControllers()) {
        int channel_count = it->GetNumChannels();
        int unit_id = it->GetUnitId();

        int controller_channels_to_process = channel_count;
        int channels_per_pass = controller_channels_to_process;
        CalcChannels(channel_count, channels_per_pass, controller_channels_to_process, it);

        while (controller_channels_to_process > 0) {
            size_t idx = 0;  // running index for placing next byte
            uint8_t d[8192];
            std::vector< std::vector<std::pair<uint8_t, uint16_t>> > lorBankData;
            lorBankData.resize((channels_per_pass / 16) + 1);

            bool bank_changed = false;
            bool frame_changed = false;
            bool color_mode[MAX_BANKS];

            //if (_framesSinceForcedOutput > LOR_FORCE_SEND_FRAMES) {
            //    bank_changed = true;
            //    frame_changed = true;
            //}
            //++_framesSinceForcedOutput;

            // gather all the data and compress common values on a per 16 channel bank basis
            int channels_to_process = channels_per_pass;
            int chan_offset = 0;
            int shift_offset = 0;
            color_mode[0] = false;
            while (channels_to_process > 0) {
                bool processed = false;

                assert((size_t)cur_channel < sizeof(_curData));
                if ((data[cur_channel] > 0) && (data[cur_channel] < 0xFF)) {
                    if ((unsigned int)shift_offset < MAX_BANKS) {
                        color_mode[shift_offset] = true;
                    }
                }

                assert((size_t)shift_offset < sizeof(lorBankData));
                for (int i = 0; i < (int)lorBankData[shift_offset].size(); ++i) {
                    if (lorBankData[shift_offset][i].first == data[cur_channel]) {
                        lorBankData[shift_offset][i].second |= (1 << chan_offset);
                        processed = true;
                        break;
                    }
                }

                if (!processed) {
                    // std::pair<uint8_t, uint16_t> first = value, second = bits
                    lorBankData[shift_offset].push_back(std::pair<uint8_t, uint16_t>({ data[cur_channel], 1 << chan_offset }));
                }

                if (data[cur_channel] != _lastSent[cur_channel]) {
                    bank_changed = true;
                    frame_changed = true;
                }
                _lastSent[cur_channel] = data[cur_channel];

                ++chan_offset;
                if (chan_offset == 16 || (channels_to_process == 1)) {
                    chan_offset = 0;
                    banks_changed[shift_offset] = bank_changed;
                    bank_changed = false;
                    ++shift_offset;
                    if ((unsigned int)shift_offset < MAX_BANKS) {
                        color_mode[shift_offset] = false;
                    }
                }
                --channels_to_process;
                ++cur_channel;
            }

            // now build the commands to send out the serial port
            for (int bank = (int)lorBankData.size() - 1; bank >= 0; --bank) {
                assert((size_t)bank < sizeof(banks_changed));
                if (banks_changed[bank]) {
                    int num_bank_records = lorBankData[bank].size();

                    // process channels that are zero
                    for (int i = 0; i < num_bank_records; ++i) {
                        if (lorBankData[bank][i].first == 0) {
                            if (num_bank_records == 1) {  // entire bank is zero
                                d[idx++] = 0;
                                d[idx++] = unit_id;
                                if (bank > 0) {
                                    d[idx++] = 0x51;
                                    d[idx++] = 0xC0 | bank;
                                }
                                else {
                                    d[idx++] = 0x41;
                                }
                            }
                            else if (color_mode[bank]) {
                                // send a value byte command for 0 if in color mode
                                // otherwise its either all off or all on and will be covered
                                // by other commands.
                                uint8_t lsb = lorBankData[bank][i].second & 0xFF;
                                uint8_t msb = lorBankData[bank][i].second >> 8;
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
                                ++idx;
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
                        if (lorBankData[bank][i].first == 0xFF) {
                            uint8_t lsb = lorBankData[bank][i].second & 0xFF;
                            uint8_t msb = lorBankData[bank][i].second >> 8;
                            bool value_byte = lorBankData[bank][i].first != 0xFF;
                            GenerateCommand(d, idx, unit_id, bank, value_byte, _data[lorBankData[bank][i].first], lsb, msb);
                        }
                    }

                    // now send all commands that are values between 0 and FF
                    for (int i = 0; i < num_bank_records; ++i) {
                        if ((lorBankData[bank][i].first != 0) && (lorBankData[bank][i].first != 0xFF)) {
                            uint8_t lsb = lorBankData[bank][i].second & 0xFF;
                            uint8_t msb = lorBankData[bank][i].second >> 8;
                            bool value_byte = lorBankData[bank][i].first != 0xFF;
                            GenerateCommand(d, idx, unit_id, bank, value_byte, _data[lorBankData[bank][i].first], lsb, msb);
                        }
                    }
                }
            }
            if (frame_changed) {
                d[idx++] = 0x0;
                d[idx++] = 0x0;
                //_framesSinceForcedOutput = 0;
            }

            if (_serial != nullptr && frame_changed) {
                _serial->Write((char*)d, idx);
                // After we output we dont want to close too early as that causes crashes
                SetDontDieUntil(GetCurrentTimeMillis() + MINIMUM_MILLIS_AFTER_WRITE_BEFORE_CLOSE);
                total_bytes_sent += idx;
            }

            for (int bank = 0; bank < (int)lorBankData.size(); bank++) {
                lorBankData[bank].clear();
            }
            lorBankData.clear();
            controller_channels_to_process -= channels_per_pass;
            ++unit_id;
        }
    }
    //spdlog::debug("    LOROptimisedOutput: Sent {} bytes", total_bytes_sent);
}

void LOROptimisedOutput::AllOff() {

    //
    //spdlog::debug("    LOROptimisedOutput: AllOff starting");

    if (!_enabled) return;

    int bank = 0;
    for (const auto& it : _controllers.GetControllers()) {
        int unit_id = it->GetUnitId();

        int channel_count = it->GetNumChannels();
        int controller_channels_to_process = channel_count;
        int channels_per_pass = controller_channels_to_process;
        CalcChannels(channel_count, channels_per_pass, controller_channels_to_process, it);

        while (controller_channels_to_process > 0) {
            size_t idx = 0;
            uint8_t d[1024];
            int channels_to_process = channels_per_pass;
            while (channels_to_process > 0) {
                d[idx++] = 0;
                d[idx++] = unit_id;
                if (bank > 0) {
                    d[idx++] = 0x51;
                    d[idx++] = 0xC0 | bank;
                }
                else {
                    d[idx++] = 0x41;
                }
                channels_to_process -= 16;
                ++bank;
            }
            d[idx++] = 0x0;
            d[idx++] = 0x0;

            if (_serial != nullptr) {
                _serial->Write((char*)d, idx);
                // After we output we dont want to close too early as that causes crashes
                SetDontDieUntil(GetCurrentTimeMillis() + MINIMUM_MILLIS_AFTER_WRITE_BEFORE_CLOSE);
            }
            controller_channels_to_process -= channels_per_pass;
            ++unit_id;
        }
    }

    // we need to clear last sent
    memset(_lastSent, 0x00, sizeof(_lastSent));

    SendHeartbeat();
    _lastheartbeat = _timer_msec;

    //spdlog::debug("    LOROptimisedOutput: AllOff finished");
}
#pragma endregion

