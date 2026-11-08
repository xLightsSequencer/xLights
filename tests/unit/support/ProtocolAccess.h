#pragma once

// Lets the protocol tests reach the packet buffers (_data, _sequenceNum, _serial...) that the output classes
// keep private or protected, so framing can be checked without owning a real network or serial port.
// It uses the standard "explicit instantiation ignores access checks" idiom: PT(obj, name) is an lvalue for the
// member registered with PT_STEAL below (for member functions it is a pointer-to-member to call with (obj.*...)(args)).

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>
#include <pugixml.hpp>

#ifndef _WIN32
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

#include "outputs/ArtNetOutput.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/DDPOutput.h"
#include "outputs/DMXOutput.h"
#include "outputs/E131Output.h"
#include "outputs/GenericSerialOutput.h"
#include "outputs/KinetOutput.h"
#include "outputs/LOROptimisedOutput.h"
#include "outputs/LOROutput.h"
#include "outputs/LorController.h"
#include "outputs/LorControllers.h"
#include "outputs/NullOutput.h"
#include "outputs/OPCOutput.h"
#include "outputs/OpenDMXOutput.h"
#include "outputs/OpenPixelNetOutput.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "outputs/PixelNetOutput.h"
#include "outputs/RenardOutput.h"
#include "outputs/SerialOutput.h"
#include "outputs/TwinklyOutput.h"
#include "outputs/ZCPPOutput.h"
#include "outputs/serial.h"
#include "outputs/xxxEthernetOutput.h"
#include "outputs/xxxSerialOutput.h"


namespace protocols_test {
template <class Tag>
struct Stolen {
    static inline typename Tag::type value{};
};
template <class Tag, typename Tag::type P>
struct Steal {
    static inline const bool done = (Stolen<Tag>::value = P, true);
};
} // namespace protocols_test

#define PT_STEAL(Name, Class, Type, member)                       \
    namespace protocols_test::steal {                             \
    struct Name { using type = Type; };                           \
    }                                                             \
    template struct protocols_test::Steal<protocols_test::steal::Name, &Class::member>;
#define PT(obj, Name) ((obj).*::protocols_test::Stolen<::protocols_test::steal::Name>::value)
#define PT_FN(Name) (::protocols_test::Stolen<::protocols_test::steal::Name>::value)

#ifndef _WIN32
PT_STEAL(serial_fd, SerialPort, int SerialPort::*, _fd)
#endif
PT_STEAL(so_serial, SerialOutput, SerialPort* SerialOutput::*, _serial)
PT_STEAL(so_config, SerialOutput, char (SerialOutput::*)[4], _serialConfig)
PT_STEAL(ren_datalen, RenardOutput, int RenardOutput::*, _datalen)
PT_STEAL(ren_data, RenardOutput, std::vector<uint8_t> RenardOutput::*, _data)
PT_STEAL(out_ok, Output, bool Output::*, _ok)
PT_STEAL(out_changed, Output, bool Output::*, _changed)
PT_STEAL(out_start, Output, int32_t Output::*, _startChannel)
PT_STEAL(out_universe, Output, int Output::*, _universe)
PT_STEAL(out_channels, Output, int32_t Output::*, _channels)
PT_STEAL(out_nullnumber, Output, int Output::*, _nullNumber)
PT_STEAL(out_skipped, Output, int Output::*, _skippedFrames)
PT_STEAL(out_baud, Output, int Output::*, _baudRate)
PT_STEAL(out_proxyout, Output, Output* Output::*, _fppProxyOutput)
PT_STEAL(xs_dev_from, xxxSerialOutput, uint8_t (xxxSerialOutput::*)(uint32_t) const, GetDeviceFromChannel)
PT_STEAL(xs_chan_on, xxxSerialOutput, uint8_t (xxxSerialOutput::*)(uint32_t) const, GetChannelOnDevice)
PT_STEAL(xs_dev_ch, xxxSerialOutput, uint8_t (xxxSerialOutput::*)(uint8_t) const, GetDeviceChannels)
PT_STEAL(lo_inuse, LOROptimisedOutput, bool (LOROptimisedOutput::*)[256], unit_id_in_use)
PT_STEAL(lo_gen, LOROptimisedOutput, void (LOROptimisedOutput::*)(uint8_t[], size_t&, int, int, bool, uint8_t, uint8_t, uint8_t), GenerateCommand)
PT_STEAL(e131_data, E131Output, uint8_t (E131Output::*)[E131_PACKET_LEN], _data)
PT_STEAL(e131_seq, E131Output, uint8_t E131Output::*, _sequenceNum)
PT_STEAL(e131_dg, E131Output, sockets::UDPSocket* E131Output::*, _datagram)
PT_STEAL(e131_remote, E131Output, std::string E131Output::*, _remoteIp)
PT_STEAL(e131_prio, E131Output, uint8_t E131Output::*, _priority)
PT_STEAL(art_data, ArtNetOutput, uint8_t (ArtNetOutput::*)[ARTNET_PACKET_LEN], _data)
PT_STEAL(art_seq, ArtNetOutput, uint8_t ArtNetOutput::*, _sequenceNum)
PT_STEAL(art_dg, ArtNetOutput, sockets::UDPSocket* ArtNetOutput::*, _datagram)
PT_STEAL(art_remote, ArtNetOutput, std::string ArtNetOutput::*, _remoteIp)
PT_STEAL(ddp_data, DDPOutput, uint8_t (DDPOutput::*)[DDP_PACKET_LEN], _data)
PT_STEAL(ddp_seq, DDPOutput, uint8_t DDPOutput::*, _sequenceNum)
PT_STEAL(ddp_dg, DDPOutput, sockets::UDPSocket* DDPOutput::*, _datagram)
PT_STEAL(ddp_full, DDPOutput, uint8_t* DDPOutput::*, _fulldata)
PT_STEAL(ddp_cpp, DDPOutput, int DDPOutput::*, _channelsPerPacket)
PT_STEAL(ddp_keep, DDPOutput, bool DDPOutput::*, _keepChannelNumbers)
PT_STEAL(ddp_remote, DDPOutput, std::string DDPOutput::*, _remoteIp)
PT_STEAL(kin_data, KinetOutput, uint8_t (KinetOutput::*)[KINET_PACKET_LEN], _data)
PT_STEAL(kin_seq, KinetOutput, uint32_t KinetOutput::*, _sequenceNum)
PT_STEAL(kin_dg, KinetOutput, sockets::UDPSocket* KinetOutput::*, _datagram)
PT_STEAL(kin_remote, KinetOutput, std::string KinetOutput::*, _remoteIp)
PT_STEAL(opc_data, OPCOutput, uint8_t* OPCOutput::*, _data)
PT_STEAL(opc_sock, OPCOutput, sockets::TCPSocket* OPCOutput::*, _socket)
PT_STEAL(zc_dg, ZCPPOutput, sockets::UDPSocket* ZCPPOutput::*, _datagram)
PT_STEAL(zc_packet, ZCPPOutput, ZCPP_packet_t ZCPPOutput::*, _packet)
PT_STEAL(zc_seq, ZCPPOutput, uint8_t ZCPPOutput::*, _sequenceNum)
PT_STEAL(zc_remote, ZCPPOutput, std::string ZCPPOutput::*, _remoteIp)
PT_STEAL(zc_serialise, ZCPPOutput, std::string (ZCPPOutput::*)(), SerialiseProtocols)
PT_STEAL(xe_dg, xxxEthernetOutput, sockets::UDPSocket* xxxEthernetOutput::*, _datagram)
PT_STEAL(zc_data, ZCPPOutput, uint8_t* ZCPPOutput::*, _data)
PT_STEAL(zc_used, ZCPPOutput, long ZCPPOutput::*, _usedChannels)
PT_STEAL(zc_model, ZCPPOutput, std::list<ZCPP_packet_t*> ZCPPOutput::*, _modelData)
PT_STEAL(zc_extra, ZCPPOutput, std::list<ZCPP_packet_t*> ZCPPOutput::*, _extraConfig)
PT_STEAL(xe_data, xxxEthernetOutput, uint8_t* xxxEthernetOutput::*, _data)
PT_STEAL(xe_packet, xxxEthernetOutput, uint8_t (xxxEthernetOutput::*)[1208], _packet)

namespace protocols_test {

inline pugi::xml_node ParseNode(pugi::xml_document& doc, const std::string& xml) {
    doc.load_string(xml.c_str());
    return doc.first_child();
}

inline std::string PrintNode(pugi::xml_node n) {
    std::ostringstream ss;
    n.print(ss, "", pugi::format_raw);
    return ss.str();
}

inline std::string SaveToString(Output& o) {
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("root");
    o.Save(root);
    return PrintNode(root.first_child());
}

#ifndef _WIN32
// A unidirectional pipe standing in for a serial line: the output writes to one end and the test reads
// the exact bytes off the other.
struct Pipe {
    int r = -1;
    int w = -1;
    Pipe() {
        int fds[2];
        if (::pipe(fds) == 0) {
            r = fds[0];
            w = fds[1];
            ::fcntl(r, F_SETFL, ::fcntl(r, F_GETFL) | O_NONBLOCK);
            ::fcntl(w, F_SETFL, ::fcntl(w, F_GETFL) | O_NONBLOCK);
        }
    }
    ~Pipe() {
        if (r >= 0) ::close(r);
    }
    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;

    // The SerialPort takes ownership of the write end.
    SerialPort* MakePort() {
        auto* p = new SerialPort();
        PT(*p, serial_fd) = w;
        w = -1;
        return p;
    }

    std::vector<uint8_t> Drain() {
        std::vector<uint8_t> res;
        uint8_t buf[4096];
        for (;;) {
            ssize_t n = ::read(r, buf, sizeof(buf));
            if (n <= 0) break;
            res.insert(res.end(), buf, buf + n);
        }
        return res;
    }

    // Reads until at least `min` bytes arrived or the timeout expires (for the threaded LOR pump).
    std::vector<uint8_t> DrainWait(size_t min, int timeoutMs) {
        std::vector<uint8_t> res;
        auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        while (std::chrono::steady_clock::now() < end) {
            auto d = Drain();
            res.insert(res.end(), d.begin(), d.end());
            if (res.size() >= min) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return res;
    }
};

// Opens `out` against the "NotConnected" port (so no device is touched), then swaps a pipe-backed port in.
template <class T>
bool OpenOnPipe(T& out, Pipe& pipe) {
    out.SetCommPort("NotConnected");
    bool ok = out.Open();
    PT(out, so_serial) = pipe.MakePort();
    return ok;
}
#endif

} // namespace protocols_test
