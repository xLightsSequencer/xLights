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

// Debug-only check that sequence document state (effects and layers) is
// changed through SequenceCommands. Document mutators call
// XL_SEQ_MUTATION("Effect::SetSetting"); outside a SeqMutationGuard::Scope
// that is reported according to the XL_COMMAND_GUARD environment variable:
//
//   assert (default in Debug builds) assert
//   log    log the first mutation seen from each call site
//   count  only count them (tests read ViolationCount())
//   off    nothing
//
// Command application holds a Scope. Code that legitimately builds document
// state directly (loading a sequence, render-time temporaries) holds one too.
// A SequenceEditCapture holds a CaptureScope, which allows only the
// mutations the capture records: effects and layers, elements added or
// removed, element render-disable and fixed timing, sequence faces, and
// media entries added or removed.
// Release builds compile all of this away.

#include <cstddef>

namespace SeqMutationGuard {

enum class Mode { Off, Log, Count, Assert };

#ifndef NDEBUG

class Scope {
public:
    Scope();
    ~Scope();
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
};

class CaptureScope {
public:
    CaptureScope();
    ~CaptureScope();
    CaptureScope(const CaptureScope&) = delete;
    CaptureScope& operator=(const CaptureScope&) = delete;
};

void Check(const char* what, const void* caller);
void SetMode(Mode m);
Mode GetMode();
size_t ViolationCount();
void ResetViolationCount();

#else

class Scope {
public:
    Scope() {}
};

class CaptureScope {
public:
    CaptureScope() {}
};

inline void Check(const char*, const void*) {}
inline void SetMode(Mode) {}
inline Mode GetMode() { return Mode::Off; }
inline size_t ViolationCount() { return 0; }
inline void ResetViolationCount() {}

#endif

} // namespace SeqMutationGuard

#ifndef NDEBUG
#if defined(_MSC_VER)
#include <intrin.h>
#define XL_SEQ_MUTATION(what) SeqMutationGuard::Check(what, _ReturnAddress())
#else
#define XL_SEQ_MUTATION(what) SeqMutationGuard::Check(what, __builtin_return_address(0))
#endif
#else
#define XL_SEQ_MUTATION(what) ((void)0)
#endif
