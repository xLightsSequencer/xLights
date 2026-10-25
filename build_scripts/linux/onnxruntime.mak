# ONNX Runtime (HTDemucs stem separation) - Linux desktop wiring.
#
# Pulled into the cbp2make-generated makefile by the top-level Makefile (it
# rewrites the generated `all:` line to `include ../build_scripts/linux/*.mak`),
# so it runs after cbp2make has defined INC_LINUX_* and LIB_LINUX_*. ONNX
# Runtime is a required, auto-fetched dependency: the Makefile's `onnxruntime`
# target downloads libonnxruntime.so + headers before the build, so these
# flags are added unconditionally. Paths are relative to the xLights/ build dir.

INC_LINUX_DEBUG   += -I../include/onnxruntime
INC_LINUX_RELEASE += -I../include/onnxruntime

# Link the shared lib and bake in an $ORIGIN-relative RUNPATH so the binary finds
# libonnxruntime.so both from the dev tree (bin/xLights -> ../lib/linux) and when
# installed (usr/bin/xLights -> ../lib, where the Makefile install target drops
# it). $$ORIGIN survives make expansion as $ORIGIN; the single quotes keep the
# shell from touching it.
LIB_LINUX_DEBUG   += -L../lib/linux -lonnxruntime -Wl,-rpath,'$$ORIGIN/../lib/linux' -Wl,-rpath,'$$ORIGIN/../lib'
LIB_LINUX_RELEASE += -L../lib/linux -lonnxruntime -Wl,-rpath,'$$ORIGIN/../lib/linux' -Wl,-rpath,'$$ORIGIN/../lib'
