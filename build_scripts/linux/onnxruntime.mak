# ONNX Runtime (HTDemucs stem separation) - Linux desktop wiring.
#
# Pulled into the cbp2make-generated makefile by the top-level Makefile (it
# rewrites the generated `all:` line to `include ../build_scripts/linux/*.mak`),
# so it runs after cbp2make has defined INC_LINUX_* and LIB_LINUX_*. The
# Makefile's `onnxruntime` target stages libonnxruntime.so + headers before
# the build. Paths are relative to the xLights/ build dir.
#
# The wiring is conditional on the staged library actually existing (evaluated
# every time the generated makefile is parsed, i.e. after the fetch has run in
# the normal `make` flow). When ci_scripts/fetch_onnxruntime_linux.sh cannot
# stage it — XLIGHTS_ONNXRUNTIME_OPTIONAL=1 plus an unavailable upstream
# release, or a tree that never ran the fetch — the flags are dropped and the
# build proceeds with stem separation disabled: StemSeparator.cpp compiles to
# its no-backend fallback ("no inference backend compiled") and Waveform.cpp
# simply does not offer the stem menu entries. The -DHAVE_ORT define lives
# here rather than in xLights.cbp precisely so it appears and disappears with
# the library; the .cbp is Unix-only so nothing else consumes it.

ifneq ($(wildcard ../lib/linux/libonnxruntime.so),)

INC_LINUX_DEBUG   += -DHAVE_ORT -I../include/onnxruntime
INC_LINUX_RELEASE += -DHAVE_ORT -I../include/onnxruntime

# Link the shared lib and bake in an $ORIGIN-relative RUNPATH so the binary finds
# libonnxruntime.so both from the dev tree (bin/xLights -> ../lib/linux) and when
# installed (usr/bin/xLights -> ../lib, where the Makefile install target drops
# it). $$ORIGIN survives make expansion as $ORIGIN; the single quotes keep the
# shell from touching it.
LIB_LINUX_DEBUG   += -L../lib/linux -lonnxruntime -Wl,-rpath,'$$ORIGIN/../lib/linux' -Wl,-rpath,'$$ORIGIN/../lib'
LIB_LINUX_RELEASE += -L../lib/linux -lonnxruntime -Wl,-rpath,'$$ORIGIN/../lib/linux' -Wl,-rpath,'$$ORIGIN/../lib'

else

$(warning onnxruntime: lib/linux/libonnxruntime.so not staged — building without ONNX Runtime, HTDemucs stem separation DISABLED (see ci_scripts/fetch_onnxruntime_linux.sh))

endif
