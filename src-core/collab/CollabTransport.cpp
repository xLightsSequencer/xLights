/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabTransport.h"

#include <algorithm>
#include <filesystem>
#include <random>
#include <system_error>

namespace Collab {

void LoopbackTransport::Send(PeerId to, const Message& m) {
    _net.Enqueue(_id, to, EncodeMessage(m));
}

void LoopbackTransport::Broadcast(const Message& m) {
    std::string bytes = EncodeMessage(m);
    for (const auto& p : _net._peers) {
        if (p->_id != _id) {
            _net.Enqueue(_id, p->_id, bytes);
        }
    }
}

void LoopbackTransport::SendBlob(PeerId to, BlobId id, const std::string& path) {
    if (!_net.IsConnected(_id) || !_net.IsConnected(to) || to == _id) {
        return;
    }
    // An unreadable file arrives as a missing one, as a real transport's
    // failed transfer would.
    std::string copy = (std::filesystem::path(_net.BlobFolder()) / ("blob" + std::to_string(++_net._blobCopies))).string();
    std::error_code ec;
    std::filesystem::copy_file(path, copy, std::filesystem::copy_options::overwrite_existing, ec);
    LoopbackNetwork::Frame f;
    f.from = _id;
    f.to = to;
    f.bytes = ec ? std::string() : copy;
    f.blob = true;
    f.blobId = id;
    _net.Push(std::move(f), _net._now + _net.RandomDelay());
}

LoopbackNetwork::LoopbackNetwork(LoopbackOptions opts) :
    _opts(opts), _rng(opts.seed) {
}

LoopbackNetwork::~LoopbackNetwork() {
    if (!_blobFolder.empty()) {
        std::error_code ec;
        std::filesystem::remove_all(_blobFolder, ec);
    }
}

const std::string& LoopbackNetwork::BlobFolder() {
    if (_blobFolder.empty()) {
        std::random_device rd;
        auto dir = std::filesystem::temp_directory_path() / ("xl_collab_blobs_" + std::to_string(rd()) + "_" + std::to_string(rd()));
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        _blobFolder = dir.string();
    }
    return _blobFolder;
}

LoopbackTransport& LoopbackNetwork::AddPeer() {
    PeerId id = (PeerId)_peers.size() + 1;
    _peers.push_back(std::unique_ptr<LoopbackTransport>(new LoopbackTransport(*this, id)));
    _connected[id] = true;
    for (const auto& p : _peers) {
        if (p->_id != id && _connected[p->_id]) {
            EnqueueEvent(p->_id, id, PeerEvent::Joined);
            EnqueueEvent(id, p->_id, PeerEvent::Joined);
        }
    }
    return *_peers.back();
}

void LoopbackNetwork::Disconnect(PeerId peer) {
    if (!IsConnected(peer)) {
        return;
    }
    _connected[peer] = false;
    for (auto it = _queue.begin(); it != _queue.end();) {
        if (it->second.from == peer || it->second.to == peer) {
            if (it->second.blob && !it->second.bytes.empty()) {
                std::error_code ec;
                std::filesystem::remove(it->second.bytes, ec);
            }
            it = _queue.erase(it);
        } else {
            ++it;
        }
    }
    for (const auto& p : _peers) {
        if (p->_id != peer && _connected[p->_id]) {
            EnqueueEvent(p->_id, peer, PeerEvent::Lost);
        }
    }
}

bool LoopbackNetwork::IsConnected(PeerId peer) const {
    auto it = _connected.find(peer);
    return it != _connected.end() && it->second;
}

uint64_t LoopbackNetwork::RandomDelay() {
    if (_opts.maxDelayMs <= _opts.minDelayMs) {
        return _opts.minDelayMs;
    }
    std::uniform_int_distribution<uint64_t> d(_opts.minDelayMs, _opts.maxDelayMs);
    return d(_rng);
}

void LoopbackNetwork::Push(Frame f, uint64_t at) {
    _queue.emplace(std::make_pair(at, _order++), std::move(f));
}

void LoopbackNetwork::Enqueue(PeerId from, PeerId to, std::string bytes) {
    if (!IsConnected(from) || !IsConnected(to) || from == to) {
        return;
    }
    if (tamper) {
        tamper(from, to, bytes);
        if (bytes.empty()) {
            ++_dropped;
            return;
        }
    }
    std::uniform_real_distribution<double> u(0.0, 1.0);
    if (_opts.dropRate > 0 && u(_rng) < _opts.dropRate) {
        ++_dropped;
        return;
    }
    int copies = (_opts.duplicateRate > 0 && u(_rng) < _opts.duplicateRate) ? 2 : 1;
    for (int c = 0; c < copies; ++c) {
        uint64_t at = _now + RandomDelay();
        if (_opts.fifoPerLink) {
            uint64_t& last = _linkLast[{ from, to }];
            at = std::max(at, last);
            last = at;
        }
        Frame f;
        f.from = from;
        f.to = to;
        f.bytes = bytes;
        Push(std::move(f), at);
    }
}

void LoopbackNetwork::EnqueueEvent(PeerId to, PeerId about, PeerEvent e) {
    Frame f;
    f.from = about;
    f.to = to;
    f.event = true;
    f.peerEvent = e;
    uint64_t at = _now;
    if (_opts.fifoPerLink) {
        // An event about a peer comes after everything it already sent.
        uint64_t& last = _linkLast[{ about, to }];
        at = std::max(at, last);
        last = at;
    }
    Push(std::move(f), at);
}

uint64_t LoopbackNetwork::NextDeliveryTime() const {
    return _queue.empty() ? _now : _queue.begin()->first.first;
}

void LoopbackNetwork::Deliver(const Frame& f) {
    struct RemoveBlob {
        const Frame& f;
        ~RemoveBlob() {
            if (f.blob && !f.bytes.empty()) {
                std::error_code ec;
                std::filesystem::remove(f.bytes, ec);
            }
        }
    } removeBlob{ f };
    if (!IsConnected(f.to)) {
        return;
    }
    if (!f.event && !IsConnected(f.from)) {
        return;
    }
    LoopbackTransport* dest = _peers[f.to - 1].get();
    auto run = [&]() {
        if (f.event) {
            if (dest->onPeer) {
                dest->onPeer(f.from, f.peerEvent);
            }
        } else if (f.blob) {
            ++_blobsDelivered;
            if (dest->onBlob) {
                dest->onBlob(f.from, f.blobId, f.bytes);
            }
        } else if (auto m = DecodeMessage(f.bytes)) {
            if (dest->onMessage) {
                dest->onMessage(f.from, *m);
            }
        }
    };
    ++_delivered;
    if (deliveryWrapper) {
        deliveryWrapper(f.to, run);
    } else {
        run();
    }
}

bool LoopbackNetwork::Step() {
    if (_queue.empty()) {
        return false;
    }
    auto it = _queue.begin();
    _now = std::max(_now, it->first.first);
    Frame f = std::move(it->second);
    _queue.erase(it);
    Deliver(f);
    return true;
}

void LoopbackNetwork::RunUntil(uint64_t time) {
    while (!_queue.empty() && _queue.begin()->first.first <= time) {
        Step();
    }
    _now = std::max(_now, time);
}

bool LoopbackNetwork::RunUntilIdle(size_t maxFrames) {
    size_t n = 0;
    while (!_queue.empty()) {
        if (n++ >= maxFrames) {
            return false;
        }
        Step();
    }
    return true;
}

} // namespace Collab
