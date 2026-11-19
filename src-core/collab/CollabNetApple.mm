/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef __APPLE__

#include "CollabFiles.h"
#include "CollabNet.h"
#include "CollabNetWire.h"

#import <Foundation/Foundation.h>
#import <Network/Network.h>
#include <CommonCrypto/CommonCryptoError.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonKeyDerivation.h>
#include <Security/CipherSuite.h>
#include <Security/SecProtocolOptions.h>
#include <dns_sd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <map>
#include <memory>
#include <system_error>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace Collab {

namespace {

constexpr size_t kMaxInFlight = 128u << 10;
constexpr unsigned kPbkdfRounds = 100000;
constexpr int64_t kConnectTimeoutMs = 15000;
constexpr int64_t kHandshakeTimeoutMs = 15000;
constexpr int64_t kCloseGraceMs = 2000;
// Failed pairing handshakes the host tolerates before it stops accepting
// connections for a while; each further round doubles the pause.
constexpr int kPairingFailuresAllowed = 5;
constexpr int64_t kPairingBackoffFirstMs = 5000;
constexpr int64_t kPairingBackoffMaxMs = 60000;

dispatch_data_t DataOf(const std::string& s) {
    return dispatch_data_create(s.data(), s.size(), nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
}

std::string ErrorText(nw_error_t err, bool beforeJoin) {
    if (err == nil) {
        return beforeJoin ? "The host closed the connection." : "The connection was closed.";
    }
    nw_error_domain_t domain = nw_error_get_error_domain(err);
    int code = nw_error_get_error_code(err);
    if (domain == nw_error_domain_tls) {
        return beforeJoin ? "The pairing code was not accepted." : "The secure connection failed (" + std::to_string(code) + ").";
    }
    if (domain == nw_error_domain_dns) {
        if (code == kDNSServiceErr_PolicyDenied) {
            return "Local network access is turned off for xLights in Settings.";
        }
        if (code == kDNSServiceErr_NoSuchRecord || code == kDNSServiceErr_NoSuchName) {
            return "The session is no longer on the network.";
        }
        return "The session could not be found (" + std::to_string(code) + ").";
    }
    if (domain == nw_error_domain_posix) {
        if (code == ECONNREFUSED) {
            return "The host is not accepting connections.";
        }
        if (code == ETIMEDOUT) {
            return "The host did not answer.";
        }
        return std::string(strerror(code));
    }
    return "Network error " + std::to_string(code) + ".";
}

// TXT values are limited to 255 bytes; cut on a UTF-8 boundary.
std::string TxtValue(const std::string& s) {
    constexpr size_t kMax = 200;
    if (s.size() <= kMax) {
        return s;
    }
    size_t n = kMax;
    while (n > 0 && ((unsigned char)s[n] & 0xC0) == 0x80) {
        --n;
    }
    return s.substr(0, n);
}

std::string MakeFolder() {
    NSString* tmp = NSTemporaryDirectory();
    fs::path base = tmp != nil ? fs::path([tmp fileSystemRepresentation]) : fs::temp_directory_path();
    fs::path dir = base / ("xlcollab-" + Wire::HexOf(Wire::NewSalt()));
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir.string();
}

nw_parameters_t MakeParams(const std::string& key) {
    std::string k = key;
    return nw_parameters_create_secure_tcp(
        ^(nw_protocol_options_t tls) {
          sec_protocol_options_t sec = nw_tls_copy_sec_protocol_options(tls);
          sec_protocol_options_add_pre_shared_key(sec, DataOf(k), DataOf(Wire::kPskIdentity));
          sec_protocol_options_append_tls_ciphersuite(sec, (tls_ciphersuite_t)TLS_PSK_WITH_AES_128_GCM_SHA256);
          // The PSK suites are TLS 1.2 only.
          sec_protocol_options_set_min_tls_protocol_version(sec, tls_protocol_version_TLSv12);
          sec_protocol_options_set_max_tls_protocol_version(sec, tls_protocol_version_TLSv12);
        },
        ^(nw_protocol_options_t tcp) {
          nw_tcp_options_set_no_delay(tcp, true);
          nw_tcp_options_set_enable_keepalive(tcp, true);
          nw_tcp_options_set_keepalive_idle_time(tcp, 10);
          nw_tcp_options_set_keepalive_interval(tcp, 5);
          nw_tcp_options_set_keepalive_count(tcp, 3);
          nw_tcp_options_set_connection_timeout(tcp, 10);
        });
}

struct OutBlob {
    BlobId id = 0;
    std::string path; // our own copy, deleted once sent or dropped
    uint64_t size = 0;
    uint64_t sent = 0;
    FILE* f = nullptr;
    bool started = false;
};

struct Conn {
    nw_connection_t c = nil;
    PeerId id = kNoPeer;
    bool ready = false;  // TLS is up
    bool joined = false; // the session has been told (onPeer Joined)
    bool gone = false;
    Wire::FrameReader reader;
    std::deque<std::string> control;
    std::deque<OutBlob> blobs;
    size_t inflight = 0;

    Files::IncomingBlob in;

    void DropFiles() {
        std::error_code ec;
        for (auto& b : blobs) {
            if (b.f != nullptr) {
                fclose(b.f);
            }
            fs::remove(b.path, ec);
        }
        blobs.clear();
        in.Abort();
    }
};

class LanTransport;

class TransportImpl : public std::enable_shared_from_this<TransportImpl> {
public:
    TransportImpl(Dispatcher d, INetTransport* owner) :
        _dispatch(std::move(d)), _owner(owner), _alive(std::make_shared<std::atomic<bool>>(true)) {
        _q = dispatch_queue_create("org.xlights.collab.net", DISPATCH_QUEUE_SERIAL);
        _folder = MakeFolder();
    }
    ~TransportImpl() {
        std::error_code ec;
        fs::remove_all(_folder, ec);
    }

    // ---- called on the session thread ----

    bool Listen(const std::string& code, const SessionAdvert& advert, std::string& error) {
        if (_state.load() != NetState::Idle) {
            error = "The transport is already in use.";
            return false;
        }
        _salt = Wire::NewSalt();
        std::string key = LanTest::DeriveKey(code, _salt);
        _serviceName = Wire::ServiceNameFor(_salt);
        nw_listener_t listener = nw_listener_create(MakeParams(key));
        if (listener == nil) {
            error = "Could not open a network listener.";
            return false;
        }
        nw_advertise_descriptor_t ad = nw_advertise_descriptor_create_bonjour_service(_serviceName.c_str(), Wire::kServiceType, nullptr);
        nw_txt_record_t txt = nw_txt_record_create_dictionary();
        auto put = [&](const char* k, const std::string& v) {
            std::string tv = TxtValue(v);
            nw_txt_record_set_key(txt, k, (const uint8_t*)tv.data(), tv.size());
        };
        put("seq", advert.sequenceName);
        put("host", advert.hostName);
        put("kind", advert.deviceKind);
        put("proto", std::to_string(advert.protocol));
        put("salt", Wire::HexOf(_salt));
        nw_advertise_descriptor_set_txt_record_object(ad, txt);
        nw_listener_set_advertise_descriptor(listener, ad);
        nw_listener_set_queue(listener, _q);

        std::weak_ptr<TransportImpl> w = weak_from_this();
        dispatch_semaphore_t started = dispatch_semaphore_create(0);
        auto outcome = std::make_shared<std::string>();
        auto settled = std::make_shared<bool>(false);
        nw_listener_set_state_changed_handler(listener, ^(nw_listener_state_t st, nw_error_t err) {
          auto self = w.lock();
          if (!self || self->_closed) {
              return;
          }
          if (st == nw_listener_state_ready) {
              if (!*settled) {
                  *settled = true;
                  dispatch_semaphore_signal(started);
              }
              self->SetState(NetState::Listening, "");
          } else if (st == nw_listener_state_failed || (st == nw_listener_state_waiting && err != nil)) {
              std::string why = "Could not listen for participants: " + ErrorText(err, false);
              if (!*settled) {
                  *settled = true;
                  *outcome = why;
                  dispatch_semaphore_signal(started);
              }
              self->SetState(NetState::Failed, why);
          }
        });
        nw_listener_set_new_connection_handler(listener, ^(nw_connection_t c) {
          auto self = w.lock();
          if (!self || self->_closed) {
              nw_connection_cancel(c);
              return;
          }
          self->OnIncoming(c);
        });
        nw_listener_set_advertised_endpoint_changed_handler(listener, ^(nw_endpoint_t ep, bool added) {
          auto self = w.lock();
          if (self && added) {
              const char* name = nw_endpoint_get_bonjour_service_name(ep);
              if (name != nullptr) {
                  self->_serviceName = name;
              }
          }
        });
        _isHost = true;
        _localId = kLanHostPeer;
        _listener = listener;
        _state = NetState::Listening;
        nw_listener_start(listener);
        // The listener comes up (or fails) almost at once; waiting lets
        // Listen report a failure directly. The handler runs on our queue,
        // never on this thread.
        if (dispatch_semaphore_wait(started, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) != 0) {
            error = "The network listener did not start.";
        } else {
            error = *outcome;
        }
        if (!error.empty()) {
            dispatch_sync(_q, ^{
              nw_listener_cancel(_listener);
              _listener = nil;
            });
            _isHost = false;
            _localId = kNoPeer;
            _state = NetState::Failed;
            return false;
        }
        return true;
    }

    void Connect(const std::string& sessionId, const std::string& code) {
        if (_state.load() != NetState::Idle) {
            return;
        }
        std::string salt;
        nw_endpoint_t ep = nil;
        if (sessionId.rfind("tcp:", 0) == 0) {
            // tcp:<address>:<port>:<salt hex>
            size_t a = sessionId.find(':', 4);
            size_t b = a == std::string::npos ? a : sessionId.find(':', a + 1);
            if (b != std::string::npos && Wire::SaltFromHex(sessionId.substr(b + 1), salt)) {
                std::string host = sessionId.substr(4, a - 4);
                std::string port = sessionId.substr(a + 1, b - a - 1);
                ep = nw_endpoint_create_host(host.c_str(), port.c_str());
            }
        } else if (Wire::SaltFromServiceName(sessionId, salt)) {
            ep = nw_endpoint_create_bonjour_service(sessionId.c_str(), Wire::kServiceType, "local.");
        }
        _isHost = false;
        if (ep == nil) {
            SetState(NetState::Failed, "This is not a session xLights can join.");
            return;
        }
        SetState(NetState::Connecting, "");
        std::weak_ptr<TransportImpl> w = weak_from_this();
        std::string c = code;
        // Key stretching takes a moment; keep it off the session thread.
        dispatch_async(_q, ^{
          auto self = w.lock();
          if (!self || self->_closed) {
              return;
          }
          nw_connection_t conn = nw_connection_create(ep, MakeParams(LanTest::DeriveKey(c, salt)));
          auto cn = std::make_shared<Conn>();
          cn->c = conn;
          cn->id = kLanHostPeer;
          self->_conns[kLanHostPeer] = cn;
          self->StartConn(cn);
          std::weak_ptr<Conn> wc = cn;
          dispatch_after(dispatch_time(DISPATCH_TIME_NOW, kConnectTimeoutMs * NSEC_PER_MSEC), self->_q, ^{
            auto s2 = w.lock();
            auto c2 = wc.lock();
            if (s2 && c2 && !c2->joined && !c2->gone && !s2->_closed) {
                s2->Gone(c2, "The host did not answer.", false);
            }
          });
        });
    }

    void Close() {
        _alive->store(false);
        if (_state.load() == NetState::Closed) {
            return;
        }
        _state = NetState::Closed;
        dispatch_sync(_q, ^{
          CloseOnQueue();
        });
    }

    NetState State() const { return _state.load(); }
    PeerId LocalPeer() const { return _localId.load(); }

    void Send(PeerId to, const Message& m) {
        if (_state.load() == NetState::Closed) {
            return;
        }
        std::string payload = EncodeMessage(m);
        if (payload.size() > Wire::kMaxMessageBytes) {
            TooLarge(to, m, payload.size());
            return;
        }
        std::string frame = Wire::EncodeFrame(Wire::FrameType::Message, payload);
        std::weak_ptr<TransportImpl> w = weak_from_this();
        dispatch_async(_q, ^{
          if (auto self = w.lock()) {
              self->Enqueue(to, frame);
          }
        });
    }

    void Broadcast(const Message& m) {
        if (_state.load() == NetState::Closed) {
            return;
        }
        std::string payload = EncodeMessage(m);
        if (payload.size() > Wire::kMaxMessageBytes) {
            TooLarge(kNoPeer, m, payload.size());
            return;
        }
        std::string frame = Wire::EncodeFrame(Wire::FrameType::Message, payload);
        std::weak_ptr<TransportImpl> w = weak_from_this();
        dispatch_async(_q, ^{
          auto self = w.lock();
          if (!self) {
              return;
          }
          std::vector<PeerId> ids;
          for (auto& [id, cn] : self->_conns) {
              if (cn->joined) {
                  ids.push_back(id);
              }
          }
          for (PeerId id : ids) {
              self->Enqueue(id, frame);
          }
        });
    }

    void SendBlob(PeerId to, BlobId id, const std::string& path) {
        if (_state.load() == NetState::Closed) {
            return;
        }
        FILE* src = Files::OpenForCopy(path);
        if (src == nullptr) {
            spdlog::warn("collab: could not open {} to send it", path);
            PostBlobDone(to, id, false);
            return;
        }
        std::string copy = (fs::path(_folder) / ("out-" + std::to_string(++_outCopies))).string();
        std::weak_ptr<TransportImpl> w = weak_from_this();
        Files::FileCopier::Job job;
        job.src = src;
        job.dest = copy;
        job.done = [w, to, id, copy](bool ok, uint64_t size) {
            auto self = w.lock();
            std::error_code ec;
            if (!self) {
                fs::remove(copy, ec);
                return;
            }
            if (!ok || size > Wire::kMaxBlobBytes) {
                spdlog::warn("collab: could not copy a file to send it");
                fs::remove(copy, ec);
                self->PostBlobDone(to, id, false);
                return;
            }
            dispatch_async(self->_q, ^{
              auto self2 = w.lock();
              std::error_code e2;
              if (!self2) {
                  fs::remove(copy, e2);
                  return;
              }
              auto it = self2->_conns.find(to);
              if (it == self2->_conns.end() || !it->second->joined || it->second->gone || self2->_closed) {
                  fs::remove(copy, e2);
                  self2->PostBlobDone(to, id, false);
                  return;
              }
              OutBlob b;
              b.id = id;
              b.path = copy;
              b.size = size;
              it->second->blobs.push_back(b);
              self2->Pump(it->second);
            });
        };
        _copier->Add(std::move(job));
    }

    std::string DirectSessionId(const std::string& address) {
        __block std::string id;
        dispatch_sync(_q, ^{
          if (_listener != nil) {
              id = "tcp:" + address + ":" + std::to_string(nw_listener_get_port(_listener)) + ":" + Wire::HexOf(_salt);
          }
        });
        return id;
    }

    void SetBackoff(const LanTest::PairingBackoff& b) {
        dispatch_sync(_q, ^{
          _backoff = b;
        });
    }

    LanTest::PairingStatus Pairing() {
        __block LanTest::PairingStatus st;
        dispatch_sync(_q, ^{
          st.failedHandshakes = _failedHandshakes;
          st.refusedConnections = _refused;
          st.windowMs = _windowMs;
          st.refusing = std::chrono::steady_clock::now() < _refuseUntil;
        });
        return st;
    }

    void SendRaw(PeerId peer, const std::string& bytes) {
        std::weak_ptr<TransportImpl> w = weak_from_this();
        dispatch_async(_q, ^{
          if (auto self = w.lock()) {
              self->Enqueue(peer, bytes);
          }
        });
    }

    const std::string& Folder() const { return _folder; }

private:
    // ---- everything below runs on _q ----

    void Post(std::function<void(INetTransport&)> fn) {
        auto alive = _alive;
        INetTransport* owner = _owner;
        _dispatch([alive, owner, fn]() {
            if (alive->load()) {
                fn(*owner);
            }
        });
    }

    void PostBlobDone(PeerId to, BlobId id, bool sent) {
        Post([to, id, sent](INetTransport& t) {
            if (t.onBlobDone) {
                t.onBlobDone(to, id, sent);
            }
        });
    }

    void PostBlobProgress(PeerId from, BlobId id, uint64_t received, uint64_t size) {
        Post([from, id, received, size](INetTransport& t) {
            if (t.onBlobProgress) {
                t.onBlobProgress(from, id, received, size);
            }
        });
    }

    void TooLarge(PeerId to, const Message& m, size_t bytes) {
        std::string why = "a message of " + std::to_string(bytes) + " bytes is too large to send";
        spdlog::warn("collab: {}", why);
        Message summary = SendFailedSummary(m);
        Post([to, summary, why](INetTransport& t) {
            if (t.onSendFailed) {
                t.onSendFailed(to, summary, why);
            }
        });
    }

    void SetState(NetState s, const std::string& reason) {
        if (_state.load() == NetState::Closed) {
            return;
        }
        _state = s;
        Post([s, reason](INetTransport& t) {
            if (t.onState) {
                t.onState(s, reason);
            }
        });
    }

    void OnIncoming(nw_connection_t c) {
        if (std::chrono::steady_clock::now() < _refuseUntil) {
            ++_refused;
            nw_connection_cancel(c);
            return;
        }
        auto cn = std::make_shared<Conn>();
        cn->c = c;
        cn->id = _nextId++;
        _conns[cn->id] = cn;
        StartConn(cn);
        std::weak_ptr<TransportImpl> w = weak_from_this();
        std::weak_ptr<Conn> wc = cn;
        // Nobody who can't finish the handshake gets to hold a connection.
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, kHandshakeTimeoutMs * NSEC_PER_MSEC), _q, ^{
          auto self = w.lock();
          auto c2 = wc.lock();
          if (self && c2 && !c2->ready && !c2->gone) {
              self->Gone(c2, "", false);
          }
        });
    }

    void StartConn(const std::shared_ptr<Conn>& cn) {
        std::weak_ptr<TransportImpl> w = weak_from_this();
        std::weak_ptr<Conn> wc = cn;
        nw_connection_set_queue(cn->c, _q);
        nw_connection_set_state_changed_handler(cn->c, ^(nw_connection_state_t st, nw_error_t err) {
          auto self = w.lock();
          auto c = wc.lock();
          if (!self || !c || c->gone || self->_closed) {
              return;
          }
          switch (st) {
          case nw_connection_state_ready:
              self->OnReady(c);
              break;
          case nw_connection_state_waiting:
              // Not reachable right now (refused, no route, TLS refused).
              // Retrying in the background would hide a wrong code.
              if (!c->ready) {
                  self->Gone(c, ErrorText(err, true), false);
              }
              break;
          case nw_connection_state_failed:
              self->Gone(c, ErrorText(err, !c->joined), false);
              break;
          case nw_connection_state_cancelled:
              self->Gone(c, ErrorText(nil, !c->joined), false);
              break;
          default:
              break;
          }
        });
        nw_connection_start(cn->c);
    }

    void OnReady(const std::shared_ptr<Conn>& cn) {
        cn->ready = true;
        if (_isHost) {
            cn->control.push_front(Wire::WelcomeFrame(cn->id));
            cn->joined = true;
            PeerId id = cn->id;
            Post([id](INetTransport& t) {
                if (t.onPeer) {
                    t.onPeer(id, PeerEvent::Joined);
                }
            });
        }
        Receive(cn);
        Pump(cn);
    }

    void Receive(const std::shared_ptr<Conn>& cn) {
        std::weak_ptr<TransportImpl> w = weak_from_this();
        std::weak_ptr<Conn> wc = cn;
        nw_connection_receive(cn->c, 1, 256u << 10, ^(dispatch_data_t content, nw_content_context_t, bool complete, nw_error_t err) {
          auto self = w.lock();
          auto c = wc.lock();
          if (!self || !c || c->gone || self->_closed) {
              return;
          }
          if (content != nil) {
              dispatch_data_apply(content, ^bool(dispatch_data_t, size_t, const void* buf, size_t size) {
                c->reader.Feed(buf, size);
                return true;
              });
              if (!self->ProcessFrames(c)) {
                  spdlog::warn("collab: dropping peer {} after a malformed frame", c->id);
                  self->Gone(c, "The other side sent something that isn't part of a session.", false);
                  return;
              }
              if (c->gone) {
                  return;
              }
          }
          if (err != nil) {
              self->Gone(c, ErrorText(err, !c->joined), false);
              return;
          }
          if (complete) {
              self->Gone(c, ErrorText(nil, !c->joined), true);
              return;
          }
          self->Receive(c);
        });
    }

    bool ProcessFrames(const std::shared_ptr<Conn>& cn) {
        Wire::Frame f;
        while (true) {
            auto r = cn->reader.Next(f);
            if (r == Wire::FrameReader::Result::More) {
                return true;
            }
            if (r == Wire::FrameReader::Result::Bad) {
                return false;
            }
            if (!HandleFrame(cn, f)) {
                return false;
            }
        }
    }

    bool HandleFrame(const std::shared_ptr<Conn>& cn, const Wire::Frame& f) {
        PeerId from = cn->id;
        if (f.type == Wire::FrameType::Welcome) {
            uint32_t assigned = 0;
            if (_isHost || cn->joined || !Wire::ReadU32(f.payload, 0, assigned) || assigned == kNoPeer || assigned == kLanHostPeer) {
                return false;
            }
            _localId = assigned;
            cn->joined = true;
            SetState(NetState::Connected, "");
            Post([](INetTransport& t) {
                if (t.onPeer) {
                    t.onPeer(kLanHostPeer, PeerEvent::Joined);
                }
            });
            return true;
        }
        if (!cn->joined) {
            return false;
        }
        switch (f.type) {
        case Wire::FrameType::Message: {
            auto m = DecodeMessage(f.payload);
            if (!m) {
                return false;
            }
            Message msg = std::move(*m);
            Post([from, msg](INetTransport& t) {
                if (t.onMessage) {
                    t.onMessage(from, msg);
                }
            });
            return true;
        }
        case Wire::FrameType::BlobStart: {
            uint64_t id = 0, size = 0;
            Wire::ReadU64(f.payload, 0, id);
            Wire::ReadU64(f.payload, 8, size);
            std::string path = (fs::path(_folder) / ("in-" + std::to_string(++_inFiles))).string();
            if (cn->in.Start(path, id, size, Wire::kMaxBlobBytes) != Files::IncomingBlob::Result::Ok) {
                return false;
            }
            PostBlobProgress(from, id, 0, size);
            return true;
        }
        case Wire::FrameType::BlobData: {
            uint64_t id = 0;
            Wire::ReadU64(f.payload, 0, id);
            uint64_t before = cn->in.Received();
            if (cn->in.Data(id, std::string_view(f.payload).substr(8)) != Files::IncomingBlob::Result::Ok) {
                return false;
            }
            if (before / kBlobProgressBytes != cn->in.Received() / kBlobProgressBytes) {
                PostBlobProgress(from, id, cn->in.Received(), cn->in.Size());
            }
            return true;
        }
        case Wire::FrameType::BlobEnd: {
            uint64_t id = 0;
            Wire::ReadU64(f.payload, 0, id);
            std::string path, why;
            switch (cn->in.End(id, path, why)) {
            case Files::IncomingBlob::Result::Received:
                break;
            case Files::IncomingBlob::Result::Failed:
                // Not the peer's fault: the connection stays, and the session
                // asks for the file again once it sees the transfer stall.
                spdlog::warn("collab: dropped a file from peer {}: {}", from, why);
                return true;
            default:
                return false;
            }
            Post([from, id, path](INetTransport& t) {
                if (t.onBlob) {
                    t.onBlob(from, id, path);
                }
                std::error_code ec;
                fs::remove(path, ec);
            });
            return true;
        }
        default:
            return false;
        }
    }

    void Enqueue(PeerId to, const std::string& frame) {
        auto it = _conns.find(to);
        if (it == _conns.end() || _closed) {
            return;
        }
        auto cn = it->second;
        if (!cn->joined || cn->gone) {
            return;
        }
        cn->control.push_back(frame);
        Pump(cn);
    }

    // Messages first; blob data only while little is waiting in the
    // connection, so a message never queues behind a whole snapshot.
    void Pump(const std::shared_ptr<Conn>& cn) {
        if (!cn->ready || cn->gone || _closed) {
            return;
        }
        while (cn->inflight < kMaxInFlight) {
            std::string frame;
            if (!cn->control.empty()) {
                frame = std::move(cn->control.front());
                cn->control.pop_front();
            } else if (!cn->blobs.empty()) {
                OutBlob& b = cn->blobs.front();
                if (!b.started) {
                    b.f = fopen(b.path.c_str(), "rb");
                    if (b.f == nullptr) {
                        std::error_code ec;
                        fs::remove(b.path, ec);
                        PostBlobDone(cn->id, b.id, false);
                        cn->blobs.pop_front();
                        continue;
                    }
                    b.started = true;
                    frame = Wire::BlobStartFrame(b.id, b.size);
                } else if (b.sent < b.size) {
                    std::string chunk(std::min<uint64_t>(Wire::kBlobChunkBytes, b.size - b.sent), '\0');
                    size_t n = fread(chunk.data(), 1, chunk.size(), b.f);
                    if (n == 0) {
                        // Our own copy shrank or can't be read: the receiver
                        // will see a short blob and drop us; better to say so.
                        Gone(cn, "A file being sent could not be read.", false);
                        return;
                    }
                    chunk.resize(n);
                    b.sent += n;
                    frame = Wire::BlobDataFrame(b.id, chunk);
                } else {
                    frame = Wire::BlobEndFrame(b.id);
                    fclose(b.f);
                    std::error_code ec;
                    fs::remove(b.path, ec);
                    PostBlobDone(cn->id, b.id, true);
                    cn->blobs.pop_front();
                }
            } else {
                break;
            }
            Write(cn, frame);
        }
    }

    void Write(const std::shared_ptr<Conn>& cn, const std::string& frame) {
        size_t n = frame.size();
        cn->inflight += n;
        std::weak_ptr<TransportImpl> w = weak_from_this();
        std::weak_ptr<Conn> wc = cn;
        nw_connection_send(cn->c, DataOf(frame), NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, false, ^(nw_error_t err) {
          auto c = wc.lock();
          if (!c) {
              return;
          }
          c->inflight -= std::min(c->inflight, n);
          auto self = w.lock();
          if (self && err == nil) {
              self->Pump(c);
          }
        });
    }

    // The connection is finished: tell the session if it knew about it.
    void Gone(const std::shared_ptr<Conn>& cn, const std::string& reason, bool graceful) {
        if (cn->gone) {
            return;
        }
        cn->gone = true;
        // Blobs still queued for it never go: the session must not wait for them.
        for (const auto& b : cn->blobs) {
            PostBlobDone(cn->id, b.id, false);
        }
        cn->DropFiles();
        _conns.erase(cn->id);
        nw_connection_cancel(cn->c);
        if (_closed) {
            return;
        }
        if (_isHost && !cn->ready) {
            FailedHandshake();
        }
        PeerId id = cn->id;
        PeerEvent e = graceful ? PeerEvent::Left : PeerEvent::Lost;
        if (cn->joined) {
            Post([id, e](INetTransport& t) {
                if (t.onPeer) {
                    t.onPeer(id, e);
                }
            });
        }
        if (!_isHost) {
            std::string why = reason;
            if (cn->joined) {
                why = graceful ? "The host ended the session." : "The connection to the host was lost.";
            }
            SetState(NetState::Failed, why);
        }
    }

    // Guessing the code online is the cheapest attack on a 6-digit code, so
    // every source shares one budget: a good handshake from one participant
    // doesn't lift the pause another caused.
    void FailedHandshake() {
        ++_failedHandshakes;
        if (++_failuresThisRound < _backoff.failures) {
            return;
        }
        _failuresThisRound = 0;
        _windowMs = _windowMs == 0 ? _backoff.firstMs : std::min(_windowMs * 2, _backoff.maxMs);
        _refuseUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(_windowMs);
        spdlog::warn("collab: {} failed pairing attempts; refusing new connections for {} ms", _failedHandshakes, _windowMs);
    }

    void CloseOnQueue() {
        _closed = true;
        if (_listener != nil) {
            nw_listener_cancel(_listener);
            _listener = nil;
        }
        for (auto& [id, cn] : _conns) {
            cn->gone = true;
            cn->DropFiles();
            nw_connection_t c = cn->c;
            if (!cn->ready) {
                nw_connection_cancel(c);
                continue;
            }
            // Whatever the session already sent (its Bye) still goes out;
            // then the connection closes cleanly, or is cut if it can't.
            for (auto& frame : cn->control) {
                nw_connection_send(c, DataOf(frame), NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, false, ^(nw_error_t) {
                });
            }
            cn->control.clear();
            nw_connection_send(c, nil, NW_CONNECTION_FINAL_MESSAGE_CONTEXT, true, ^(nw_error_t) {
              nw_connection_cancel(c);
            });
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, kCloseGraceMs * NSEC_PER_MSEC), _q, ^{
              nw_connection_force_cancel(c);
            });
        }
        _conns.clear();
        std::error_code ec;
        for (auto it = fs::directory_iterator(_folder, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) {
            fs::remove_all(it->path(), ec);
        }
    }

    Dispatcher _dispatch;
    INetTransport* _owner;
    std::shared_ptr<std::atomic<bool>> _alive;
    dispatch_queue_t _q;
    std::atomic<NetState> _state{ NetState::Idle };
    std::atomic<PeerId> _localId{ kNoPeer };
    bool _isHost = false;
    bool _closed = false;
    nw_listener_t _listener = nil;
    std::string _salt;
    std::string _serviceName;
    std::map<PeerId, std::shared_ptr<Conn>> _conns;
    PeerId _nextId = kLanHostPeer + 1;
    std::string _folder;
    std::atomic<uint64_t> _outCopies{ 0 };
    std::shared_ptr<Files::FileCopier> _copier = std::make_shared<Files::FileCopier>();
    LanTest::PairingBackoff _backoff{ kPairingFailuresAllowed, kPairingBackoffFirstMs, kPairingBackoffMaxMs };
    int _failuresThisRound = 0;
    int _failedHandshakes = 0;
    int _refused = 0;
    int64_t _windowMs = 0;
    std::chrono::steady_clock::time_point _refuseUntil{};
    uint64_t _inFiles = 0;
};

class LanTransport : public INetTransport {
public:
    explicit LanTransport(Dispatcher d) : _impl(std::make_shared<TransportImpl>(std::move(d), this)) {}
    ~LanTransport() override { _impl->Close(); }

    PeerId LocalPeer() const override { return _impl->LocalPeer(); }
    void Send(PeerId to, const Message& m) override { _impl->Send(to, m); }
    void Broadcast(const Message& m) override { _impl->Broadcast(m); }
    void SendBlob(PeerId to, BlobId id, const std::string& path) override { _impl->SendBlob(to, id, path); }
    bool Listen(const std::string& code, const SessionAdvert& advert, std::string& error) override { return _impl->Listen(code, advert, error); }
    void Connect(const std::string& sessionId, const std::string& code) override { _impl->Connect(sessionId, code); }
    void Close() override { _impl->Close(); }
    NetState State() const override { return _impl->State(); }

    TransportImpl& Impl() { return *_impl; }

private:
    std::shared_ptr<TransportImpl> _impl;
};

// ---------------------------------------------------------------------------
// Browsing
// ---------------------------------------------------------------------------

class BrowserImpl : public std::enable_shared_from_this<BrowserImpl> {
public:
    BrowserImpl(Dispatcher d, ISessionBrowser* owner) : _dispatch(std::move(d)), _owner(owner) {
        _q = dispatch_queue_create("org.xlights.collab.browse", DISPATCH_QUEUE_SERIAL);
    }

    void Start() {
        Stop();
        _alive = std::make_shared<std::atomic<bool>>(true);
        auto alive = _alive;
        nw_browse_descriptor_t desc = nw_browse_descriptor_create_bonjour_service(Wire::kServiceType, "local.");
        nw_browse_descriptor_set_include_txt_record(desc, true);
        nw_browser_t b = nw_browser_create(desc, nullptr);
        nw_browser_set_queue(b, _q);
        std::weak_ptr<BrowserImpl> w = weak_from_this();
        nw_browser_set_state_changed_handler(b, ^(nw_browser_state_t st, nw_error_t err) {
          auto self = w.lock();
          if (!self || !alive->load()) {
              return;
          }
          if (st == nw_browser_state_failed || (st == nw_browser_state_waiting && err != nil)) {
              std::string why = ErrorText(err, false);
              self->Post(alive, [why](ISessionBrowser& o) {
                  if (o.onError) {
                      o.onError(why);
                  }
              });
          }
        });
        nw_browser_set_browse_results_changed_handler(b, ^(nw_browse_result_t oldResult, nw_browse_result_t newResult, bool batchComplete) {
          auto self = w.lock();
          if (!self || !alive->load()) {
              return;
          }
          nw_browse_result_change_t changes = nw_browse_result_get_changes(oldResult, newResult);
          if ((changes & nw_browse_result_change_result_removed) != 0 || newResult == nil) {
              if (oldResult != nil) {
                  self->_found.erase(NameOf(oldResult));
              }
          } else {
              SessionAdvert a;
              if (ParseResult(newResult, a)) {
                  self->_found[a.id] = a;
              }
          }
          if (batchComplete) {
              std::vector<SessionAdvert> list;
              for (auto& [id, a] : self->_found) {
                  list.push_back(a);
              }
              self->Post(alive, [list](ISessionBrowser& o) {
                  if (o.onChanged) {
                      o.onChanged(list);
                  }
              });
          }
        });
        _browser = b;
        nw_browser_start(b);
    }

    void Stop() {
        if (_alive) {
            _alive->store(false);
        }
        if (_browser != nil) {
            nw_browser_t b = _browser;
            _browser = nil;
            dispatch_sync(_q, ^{
              nw_browser_cancel(b);
              _found.clear();
            });
        }
    }

private:
    static std::string NameOf(nw_browse_result_t r) {
        nw_endpoint_t ep = nw_browse_result_copy_endpoint(r);
        const char* name = ep != nil ? nw_endpoint_get_bonjour_service_name(ep) : nullptr;
        return name != nullptr ? name : "";
    }

    static bool ParseResult(nw_browse_result_t r, SessionAdvert& a) {
        a.id = NameOf(r);
        std::string salt;
        if (!Wire::SaltFromServiceName(a.id, salt)) {
            return false;
        }
        nw_txt_record_t txt = nw_browse_result_copy_txt_record_object(r);
        if (txt == nil) {
            return true;
        }
        auto get = [&](const char* key) {
            __block std::string v;
            nw_txt_record_access_key(txt, key, ^bool(const char*, nw_txt_record_find_key_t found, const uint8_t* value, size_t len) {
              if (found == nw_txt_record_find_key_non_empty_value && value != nullptr) {
                  v.assign((const char*)value, len);
              }
              return true;
            });
            return v;
        };
        a.sequenceName = get("seq");
        a.hostName = get("host");
        a.deviceKind = get("kind");
        std::string proto = get("proto");
        a.protocol = proto.empty() ? 0 : (int)std::strtol(proto.c_str(), nullptr, 10);
        return true;
    }

    void Post(const std::shared_ptr<std::atomic<bool>>& alive, std::function<void(ISessionBrowser&)> fn) {
        ISessionBrowser* owner = _owner;
        _dispatch([alive, owner, fn]() {
            if (alive->load()) {
                fn(*owner);
            }
        });
    }

    Dispatcher _dispatch;
    ISessionBrowser* _owner;
    dispatch_queue_t _q;
    nw_browser_t _browser = nil;
    std::shared_ptr<std::atomic<bool>> _alive;
    std::map<std::string, SessionAdvert> _found;
};

class LanBrowser : public ISessionBrowser {
public:
    explicit LanBrowser(Dispatcher d) : _impl(std::make_shared<BrowserImpl>(std::move(d), this)) {}
    ~LanBrowser() override { _impl->Stop(); }
    void Start() override { _impl->Start(); }
    void Stop() override { _impl->Stop(); }

private:
    std::shared_ptr<BrowserImpl> _impl;
};

} // namespace

std::unique_ptr<INetTransport> CreateLanTransport(Dispatcher dispatch) {
    return std::make_unique<LanTransport>(std::move(dispatch));
}

std::unique_ptr<ISessionBrowser> CreateLanBrowser(Dispatcher dispatch) {
    return std::make_unique<LanBrowser>(std::move(dispatch));
}

bool LanTransportAvailable() {
    return true;
}

std::string SecretDigest(const std::string& secret) {
    unsigned char out[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(secret.data(), (CC_LONG)secret.size(), out);
    return Wire::HexOf(std::string_view((const char*)out, sizeof(out)));
}

namespace LanTest {

std::string DeriveKey(const std::string& code, const std::string& salt) {
    std::string s = std::string("xlights-collab-v1:") + salt;
    uint8_t out[32];
    if (CCKeyDerivationPBKDF(kCCPBKDF2, code.data(), code.size(), (const uint8_t*)s.data(), s.size(), kCCPRFHmacAlgSHA256, kPbkdfRounds, out,
                             sizeof(out)) != kCCSuccess) {
        return {};
    }
    return std::string((const char*)out, sizeof(out));
}

std::string DirectSessionId(INetTransport& host, const std::string& address) {
    auto* t = dynamic_cast<LanTransport*>(&host);
    return t != nullptr ? t->Impl().DirectSessionId(address) : "";
}

void SetPairingBackoff(INetTransport& host, const PairingBackoff& b) {
    if (auto* lt = dynamic_cast<LanTransport*>(&host)) {
        lt->Impl().SetBackoff(b);
    }
}

PairingStatus Pairing(INetTransport& host) {
    auto* lt = dynamic_cast<LanTransport*>(&host);
    return lt != nullptr ? lt->Impl().Pairing() : PairingStatus{};
}

void SendRaw(INetTransport& t, PeerId peer, const std::string& bytes) {
    if (auto* lt = dynamic_cast<LanTransport*>(&t)) {
        lt->Impl().SendRaw(peer, bytes);
    }
}

std::string BlobFolder(INetTransport& t) {
    auto* lt = dynamic_cast<LanTransport*>(&t);
    return lt != nullptr ? lt->Impl().Folder() : "";
}

} // namespace LanTest

} // namespace Collab

#endif // __APPLE__
