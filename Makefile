
INSTALL_ROOT    =
PREFIX          = /usr/local

# Ignore some warnings for now to make compile output cleaner
# until the issues are cleaned up in the code.
IGNORE_WARNINGS = -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-switch -Wno-unused-but-set-variable -Wno-parentheses -Wno-maybe-uninitialized -Wno-return-type -Wno-uninitialized -Wno-unused-value

# Copied from xLights.cbp.mak with added -std option
CFLAGS          = -DwxUSE_UNICODE -std=gnu++11

# Copied from xLights.cbp.mak with slight addition to ignore warnings for now
CFLAGS_RELEASELINUX = $(CFLAGS) -O2 -Wall `wx-config --version=2.9 --cflags` -Winvalid-pch -DWX_PRECOMP -DNDEBUG $(IGNORE_WARNINGS)


MKDIR           = mkdir -p
CHK_DIR_EXISTS  = test -d
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE        = rm -f

SUBDIRS         = xLights

all: makefile subdirs

#############################################################################

subdirs: $(SUBDIRS)

$(SUBDIRS): FORCE
	@${MAKE} -C $@ -f xLights.cbp.mak CFLAGS_RELEASELINUX="$(CFLAGS_RELEASELINUX)" OBJDIR_DEBUGLINUX=".objs_debug" releaselinux


#############################################################################

debug: $(addsuffix _debug,$(SUBDIRS))

$(addsuffix _debug,$(SUBDIRS)):
	@${MAKE} -C $(subst _debug,,$@) -f xLights.cbp.mak OBJDIR_DEBUGLINUX=".objs_debug" debuglinux

#############################################################################

clean: $(addsuffix _clean,$(SUBDIRS))

$(addsuffix _clean,$(SUBDIRS)):
	@${MAKE} -C $(subst _clean,,$@) -f xLights.cbp.mak OBJDIR_DEBUGLINUX=".objs_debug" clean

#############################################################################

install:
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/${PREFIX}/bin || $(MKDIR) $(INSTALL_ROOT)/${PREFIX}/bin
	-$(INSTALL_PROGRAM) bin/xLights $(INSTALL_ROOT)/${PREFIX}/bin/xLights

uninstall:
	-$(DEL_FILE) $(INSTALL_ROOT)/${PREFIX}/bin/xLights

#############################################################################

makefile: xLights/xLights.cbp.mak

xLights/xLights.cbp.mak: xLights/xLights.cbp
	@cbp2make -in xLights/xLights.cbp -cfg cbp2make.cfg -out xLights/xLights.cbp.mak \
			--with-deps --keep-outdir --keep-objdir

#############################################################################

FORCE:
