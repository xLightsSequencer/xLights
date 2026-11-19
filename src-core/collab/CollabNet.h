#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Network transports for live sessions, and finding sessions to join.
//
// The session logic only needs ICollabTransport (CollabTransport.h); this
// adds what a real network needs around it: listening, connecting with a
// pairing code, and discovery. Where no transport is built (Windows and
// Linux for now) CreateLanTransport / CreateLanBrowser return nullptr and the
// app hides the feature.

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "CollabTransport.h"

namespace Collab {

// Runs `fn` on the thread the session lives on (the main thread in the
// apps). Transports deliver every callback through it. Tests pass their own
// queue and pump it.
using Dispatcher = std::function<void(std::function<void()> fn)>;

// What a host advertises, and what a browser reports.
struct SessionAdvert {
    std::string id;           // the service instance; unique on the network
    std::string sequenceName; // the open sequence's file name, no folder
    std::string hostName;     // the host's display name
    std::string deviceKind;   // "mac", "ipad", ...
    int protocol = kProtocolVersion;
};

enum class NetState { Idle, Listening, Connecting, Connected, Failed, Closed };

// The host's PeerId: its LocalPeer(), and the one peer a client sees (the
// id its onPeer(Joined) carries, to pass to CollabClient::Join). A client's
// LocalPeer() is the id the host gave its connection.
inline constexpr PeerId kLanHostPeer = 1;

class INetTransport : public ICollabTransport {
public:
    // Host: accept participants that know `pairingCode`, and advertise
    // `advert` (its id is filled in by the transport). False with `error` set
    // when listening can't start.
    virtual bool Listen(const std::string& pairingCode, const SessionAdvert& advert, std::string& error) = 0;
    // Client: connect to the session `sessionId` (SessionAdvert::id) with the
    // code the host shows. Asynchronous: success arrives as onPeer(host,
    // Joined), failure as onState(Failed, reason). A wrong code fails and
    // never reaches the host's session.
    virtual void Connect(const std::string& sessionId, const std::string& pairingCode) = 0;
    // Drops every connection and stops advertising. No callbacks after it
    // returns.
    virtual void Close() = 0;
    virtual NetState State() const = 0;

    // Called through the dispatcher, like onMessage / onPeer / onBlob.
    std::function<void(NetState state, const std::string& reason)> onState;
};

class ISessionBrowser {
public:
    virtual ~ISessionBrowser() = default;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    // The full current list, on every change. Through the dispatcher.
    std::function<void(const std::vector<SessionAdvert>& sessions)> onChanged;
    // Browsing failed or was refused (e.g. local network permission denied).
    std::function<void(const std::string& reason)> onError;
};

// nullptr where there is no LAN transport.
std::unique_ptr<INetTransport> CreateLanTransport(Dispatcher dispatch);
std::unique_ptr<ISessionBrowser> CreateLanBrowser(Dispatcher dispatch);
bool LanTransportAvailable();

// A fresh 6-digit pairing code.
std::string NewPairingCode();

} // namespace Collab
