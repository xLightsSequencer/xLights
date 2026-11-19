/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// PlatformGroupSessions() on Apple platforms: SharePlay, through the bridge
// over the Swift GroupActivities code in macOS/src-apple-core/collab.

#include "CollabGroup.h"

#include "collab/AppleSharePlayBridge.h"

namespace Collab {
namespace {

class AppleGroupChannel : public IGroupChannel {
public:
    explicit AppleGroupChannel(std::unique_ptr<AppleSharePlay::Channel> channel) :
        _channel(std::move(channel)) {
        // The bridge's channel stops calling back when it is destroyed with
        // this, so `this` is valid in each. Each copies the std::function
        // first: the handler may destroy this channel.
        _channel->onMessage = [this](const std::string& from, const std::string& bytes) {
            auto fn = onMessage;
            if (fn) {
                fn(from, bytes);
            }
        };
        _channel->onMembers = [this](const std::vector<std::string>& ids) {
            auto fn = onMembers;
            if (fn) {
                std::vector<GroupMember> members;
                members.reserve(ids.size());
                for (const auto& id : ids) {
                    // SharePlay does not reveal names; the session's own
                    // hello carries them.
                    members.push_back(GroupMember{ id, std::string() });
                }
                fn(members);
            }
        };
        _channel->onAttachment = [this](const std::string& from, const std::string& key, const std::string& path) {
            auto fn = onAttachment;
            if (fn) {
                fn(from, key, path);
            }
        };
        _channel->onEnded = [this](const std::string& reason) {
            auto fn = onEnded;
            if (fn) {
                fn(reason);
            }
        };
        _channel->onAttachFailed = [this](const std::string& key) {
            auto fn = onAttachFailed;
            if (fn) {
                fn(key);
            }
        };
        _channel->wantsAttachment = [this](const std::string& key) {
            auto fn = wantsAttachment;
            return !fn || fn(key);
        };
    }

    std::string LocalId() const override { return _channel->LocalId(); }
    bool IsInitiator() const override { return _channel->IsInitiator(); }
    size_t MaxMessageBytes() const override { return AppleSharePlay::MaxMessageBytes(); }
    void Send(const std::string& to, const std::string& bytes) override { _channel->Send(to, bytes); }
    uint64_t MaxAttachmentBytes() const override { return AppleSharePlay::MaxAttachmentBytes(); }
    bool Attach(const std::string& key, const std::string& path) override { return _channel->Attach(key, path); }
    void Detach(const std::string& key) override { _channel->Detach(key); }
    void Leave(bool endForEveryone) override { _channel->Leave(endForEveryone); }

private:
    std::unique_ptr<AppleSharePlay::Channel> _channel;
};

AppleSharePlay::Ready Adapt(std::function<void(std::unique_ptr<IGroupChannel>, const std::string&)> ready) {
    return [ready = std::move(ready)](std::unique_ptr<AppleSharePlay::Channel> channel, const std::string& error) {
        if (channel) {
            ready(std::make_unique<AppleGroupChannel>(std::move(channel)), std::string());
        } else {
            ready(nullptr, error);
        }
    };
}

class AppleGroupSessions : public IGroupSessionSource {
public:
    AppleGroupSessions() {
        AppleSharePlay::Observe([this](const AppleSharePlay::Offer& o) {
            _offered = o.id.empty() ? SessionAdvert() : ToAdvert(o);
            auto fn = onSessionOffered;
            if (fn) {
                fn(_offered);
            }
        });
    }

    SessionAdvert OfferedSession() const override { return _offered; }

    bool Available() const override { return AppleSharePlay::Available(); }
    bool InGroup() const override { return AppleSharePlay::InGroup(); }

    void Start(const SessionAdvert& advert, std::function<void(std::unique_ptr<IGroupChannel>, const std::string&)> ready) override {
        AppleSharePlay::Offer o;
        o.id = advert.id;
        o.sequenceName = advert.sequenceName;
        o.hostName = advert.hostName;
        o.deviceKind = advert.deviceKind;
        o.protocol = advert.protocol;
        AppleSharePlay::Start(o, Adapt(std::move(ready)));
    }

    void JoinOffered(const std::string& advertId, std::function<void(std::unique_ptr<IGroupChannel>, const std::string&)> ready) override {
        if (advertId == _offered.id) {
            _offered = SessionAdvert();
        }
        AppleSharePlay::JoinOffered(advertId, Adapt(std::move(ready)));
    }

private:
    static SessionAdvert ToAdvert(const AppleSharePlay::Offer& o) {
        SessionAdvert a;
        a.id = o.id;
        a.sequenceName = o.sequenceName;
        a.hostName = o.hostName;
        a.deviceKind = o.deviceKind;
        if (o.protocol > 0) {
            a.protocol = o.protocol;
        }
        a.group = true;
        return a;
    }

    SessionAdvert _offered;
};

} // namespace

IGroupSessionSource& PlatformGroupSessions() {
    static AppleGroupSessions sessions;
    return sessions;
}

} // namespace Collab
