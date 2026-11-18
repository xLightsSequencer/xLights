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

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <functional>

namespace ip_utils
{
    // Lets an object hand a callback that uses `this` to the async ResolveIP and be
    // destroyed without waiting for the lookup: once Cancel() or Reset() returns, the
    // callbacks wrapped so far never run.  Both wait only for a callback already in progress.
    class ResolveGuard {
    public:
        ResolveGuard() : _state(std::make_shared<State>()) {}
        // a copy belongs to a different object, so it gets its own state
        ResolveGuard(const ResolveGuard&) : _state(std::make_shared<State>()) {}
        ResolveGuard& operator=(const ResolveGuard&) { return *this; }
        ~ResolveGuard() { Cancel(); }

        void Cancel() {
            std::lock_guard<std::mutex> lock(_state->mtx);
            _state->alive = false;
        }

        // Drops the callbacks wrapped so far but keeps guarding new ones.  Returns true if
        // a lookup was still pending, so the owner knows to queue it again.
        bool Reset() {
            bool pending = false;
            {
                std::lock_guard<std::mutex> lock(_state->mtx);
                _state->alive = false;
                pending = _state->pending > 0;
            }
            _state = std::make_shared<State>();
            return pending;
        }

        std::function<void(const std::string&)> Wrap(std::function<void(const std::string&)> func) const {
            {
                std::lock_guard<std::mutex> lock(_state->mtx);
                ++_state->pending;
            }
            return [state = _state, func = std::move(func)](const std::string& r) {
                std::lock_guard<std::mutex> lock(state->mtx);
                --state->pending;
                if (state->alive) {
                    func(r);
                }
            };
        }

    private:
        struct State {
            std::mutex mtx;
            bool alive = true;
            int pending = 0;
        };
        std::shared_ptr<State> _state;
    };

	bool IsIPValid(const std::string& ip);

	bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
    bool IsValidHostname(const std::string& ip);
	bool IsIPv6(const std::string& ip);
	std::string CleanupIP(const std::string& ip);
	std::string ResolveIP(const std::string& ip);

    void ResolveIP(const std::string& ip, std::function<void(const std::string &)> &&func);
    void waitForAllToResolve();
    void shutdownResolvePool();

	std::list<std::string> GetLocalIPs();
    bool IsValidLocalIP(const std::string& ip);
    bool IsInSameSubnet(const std::string& ip1, const std::string& ip2, const std::string& mask = "255.255.255.0");

};
