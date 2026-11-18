#include <catch2/catch_test_case_info.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "render/SequenceMutationGuard.h"

#include <optional>

// Debug builds assert when sequence document state changes outside a
// command (SequenceMutationGuard.h). Most test cases build their sequences
// directly, the way a sequence load does, and test the low-level mutators
// themselves, so each runs inside a SeqMutationGuard::Scope. Test cases
// tagged [seq] - the command, edit-site and capture tests - run under the
// guard: whatever they set up directly holds an explicit Scope, and the
// production paths they drive must not trip it.

namespace {
class SeqGuardListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testCaseStarting(Catch::TestCaseInfo const& info) override {
        bool guarded = false;
        for (const auto& tag : info.tags) {
            if (tag.original == Catch::StringRef("seq")) {
                guarded = true;
            }
        }
        if (!guarded) {
            _scope.emplace();
        }
    }

    void testCaseEnded(Catch::TestCaseStats const&) override {
        _scope.reset();
    }

private:
    std::optional<SeqMutationGuard::Scope> _scope;
};
} // namespace

CATCH_REGISTER_LISTENER(SeqGuardListener)

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>

TEST_CASE("The mutation guard asserts by default in Debug builds", "[seq][guard]") {
#if !defined(NDEBUG) && (defined(DEBUG) || defined(_DEBUG))
    if (std::getenv("XL_COMMAND_GUARD") == nullptr) {
        CHECK(SeqMutationGuard::GetMode() == SeqMutationGuard::Mode::Assert);
    }
#endif
    SUCCEED();
}
