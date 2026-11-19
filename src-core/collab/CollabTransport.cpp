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

Message SendFailedSummary(const Message& m) {
    Message s;
    s.type = m.type;
    s.rev = m.rev;
    s.clientSeq = m.clientSeq;
    s.blob = m.blob;
    s.participant = m.participant;
    s.kind = m.kind;
    return s;
}

void LoopbackTransport::Send(PeerId to, const Message& m) {
    std::string bytes = EncodeMessage(m);
    if (bytes.size() > _net._opts.maxMessageBytes) {
        _net.NotifySendFailed(_id, to, m, bytes.size());
        return;
    }
    _net.Enqueue(_id, to, std::move(bytes));
}

void LoopbackTransport::Broadcast(const Message& m) {
    std::string bytes = EncodeMessage(m);
    if (bytes.size() > _net._opts.maxMessageBytes) {
        _net.NotifySendFailed(_id, kNoPeer, m, bytes.size());
        return;
    }
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
    ++_net._blobsSent[_id];
    if (_net.loseBlob && _net.loseBlob(_id, to, id)) {
        // Gone without a word, as some transports lose a transfer.
        return;
    }
    // An unreadable file arrives as a missing one, as a real transport's
    // failed transfer would.
    std::string copy = (std::filesystem::path(_net.BlobFolder()) / ("blob" + std::to_string(++_net._blobCopies))).string();
    std::error_code ec;
    std::filesystem::copy_file(path, copy, std::filesystem::copy_options::overwrite_existing, ec);
    uint64_t size = ec ? 0 : (uint64_t)std::filesystem::file_size(copy, ec);
    uint64_t at = _net._now + _net.RandomDelay();
    uint64_t rate = _net._opts.blobBytesPerMs;
    if (rate != 0) {
        // One blob after another on a link, reporting progress at least
        // every half second of transfer.
        uint64_t& free = _net._blobLinkFree[{ _id, to }];
        at = std::max(at, free);
        uint64_t step = std::max<uint64_t>(1, std::min<uint64_t>(kBlobProgressBytes, rate * 500));
        for (uint64_t got = 0; got < size; got += step) {
            LoopbackNetwork::Frame p;
            p.from = _id;
            p.to = to;
            p.blobId = id;
            p.note = LoopbackNetwork::Frame::Note::Progress;
            p.received = got;
            p.size = size;
            _net.Push(std::move(p), at + got / rate);
        }
        at += (size + rate - 1) / rate;
        free = at;
    }
    LoopbackNetwork::Frame f;
    f.from = _id;
    f.to = to;
    f.bytes = ec ? std::string() : copy;
    f.blob = true;
    f.blobId = id;
    _net.Push(std::move(f), at);
    LoopbackNetwork::Frame d;
    d.from = to;
    d.to = _id;
    d.blobId = id;
    d.note = LoopbackNetwork::Frame::Note::Done;
    d.ok = !ec;
    _net.Push(std::move(d), at);
}

void LoopbackNetwork::NotifySendFailed(PeerId from, PeerId to, const Message& m, size_t bytes) {
    Frame f;
    f.from = to;
    f.to = from;
    f.note = Frame::Note::SendFailed;
    f.failed = SendFailedSummary(m);
    f.bytes = "a message of " + std::to_string(bytes) + " bytes is too large to send";
    Push(std::move(f), _now);
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
        Frame& f = it->second;
        if (f.note == Frame::Note::Done && f.from == peer && f.to != peer) {
            // The receiver went: the sender learns its blob won't arrive.
            f.ok = false;
            ++it;
            continue;
        }
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
    if (!f.event && f.note != Frame::Note::SendFailed && f.note != Frame::Note::Done && !IsConnected(f.from)) {
        return;
    }
    LoopbackTransport* dest = _peers[f.to - 1].get();
    auto run = [&]() {
        if (f.note == Frame::Note::Progress) {
            if (dest->onBlobProgress) {
                dest->onBlobProgress(f.from, f.blobId, f.received, f.size);
            }
        } else if (f.note == Frame::Note::Done) {
            if (dest->onBlobDone) {
                dest->onBlobDone(f.from, f.blobId, f.ok);
            }
        } else if (f.note == Frame::Note::SendFailed) {
            if (dest->onSendFailed) {
                dest->onSendFailed(f.from, f.failed, f.bytes);
            }
        } else if (f.event) {
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
    if (f.note == Frame::Note::None) {
        ++_delivered;
    }
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
