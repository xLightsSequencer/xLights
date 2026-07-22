# KLightMapper (Map-from-Lights camera scan) - Linux desktop wiring.
#
# Pulled into the cbp2make-generated makefile by the top-level Makefile (it
# rewrites the generated `all:` line to `include ../build_scripts/linux/*.mak`),
# so it runs after cbp2make has defined INC_LINUX_* and LIB_LINUX_*. KLightMapper
# is a required, auto-fetched dependency: the Makefile's `klightmapper` target
# downloads libklightmapper.so + the headers before the build, so these flags are
# added unconditionally. Paths are relative to the xLights/ build dir.
INC_LINUX_DEBUG   += -I../include/klightmapper
INC_LINUX_RELEASE += -I../include/klightmapper
# Link the shared lib and bake in an $ORIGIN-relative RUNPATH so the binary finds
# libklightmapper.so both from the dev tree (bin/xLights -> ../lib/linux) and when
# installed (usr/bin/xLights -> ../lib, where the Makefile install target drops
# it). $$ORIGIN survives make expansion as $ORIGIN; the single quotes keep the
# shell from touching it.
LIB_LINUX_DEBUG   += -L../lib/linux -lklightmapper -Wl,-rpath,'$$ORIGIN/../lib/linux' -Wl,-rpath,'$$ORIGIN/../lib'
LIB_LINUX_RELEASE += -L../lib/linux -lklightmapper -Wl,-rpath,'$$ORIGIN/../lib/linux' -Wl,-rpath,'$$ORIGIN/../lib'
