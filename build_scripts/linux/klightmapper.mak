# KLightMapper (Map-from-Lights camera scan) - Linux desktop wiring.
#
# Pulled into the cbp2make-generated makefile by the top-level Makefile (it
# rewrites the generated `all:` line to `include ../build_scripts/linux/*.mak`),
# so it runs after cbp2make has defined INC_LINUX_* and LIB_LINUX_*. KLightMapper
# is a required, auto-fetched dependency: the Makefile's `klightmapper` target
# downloads libklightmapper.so + the headers before the build, so these flags are
# added unconditionally. Paths are relative to the xLights/ build dir.
#
# There used to be a MISSING_LIBAV probe here that disabled the whole feature
# when `ldd` could not resolve libklightmapper.so's libavformat — which happened
# on every distro whose FFmpeg major differed from the one the prebuilt .so was
# built against (Ubuntu 24.04 / FFmpeg 6). That probe is gone because the cause
# is: libklightmapper.so no longer links FFmpeg at all. Its codec tail ships as
# a set of small libklightmapper_av<major>.so companions staged beside it, and
# it loads the one matching the host at runtime. Nothing here can fail to
# resolve any more, so the link is unconditional.
#
# A host with no FFmpeg runtime installed still cannot scan, but that is now a
# runtime condition with an actionable message (klm_scan_backend_status) rather
# than a silently missing menu item decided at build time.

INC_LINUX_DEBUG   += -I../include/klightmapper
INC_LINUX_RELEASE += -I../include/klightmapper

# Link the shared lib and bake in an $ORIGIN-relative RUNPATH so the binary finds
# libklightmapper.so both from the dev tree (bin/xLights -> ../lib/linux) and when
# installed (usr/bin/xLights -> ../lib, where the Makefile install target drops
# it). $$ORIGIN survives make expansion as $ORIGIN; the single quotes keep the
# shell from touching it.
LIB_LINUX_DEBUG   += -L../lib/linux -lklightmapper -Wl,-rpath,'$$ORIGIN/../lib/linux' -Wl,-rpath,'$$ORIGIN/../lib'
LIB_LINUX_RELEASE += -L../lib/linux -lklightmapper -Wl,-rpath,'$$ORIGIN/../lib/linux' -Wl,-rpath,'$$ORIGIN/../lib'
