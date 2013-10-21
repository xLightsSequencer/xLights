
INSTALL_ROOT    =
PREFIX          = /usr/local

# Ignore some warnings for now to make compile output cleaner
# until the issues are cleaned up in the code.
IGNORE_WARNINGS = -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-switch -Wno-unused-but-set-variable -Wno-parentheses -Wno-return-type -Wno-uninitialized -Wno-unused-value

MKDIR           = mkdir -p
CHK_DIR_EXISTS  = test -d
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE        = rm -f

SUBDIRS         = xLights

all: makefile subdirs

#############################################################################

subdirs: $(SUBDIRS)

$(SUBDIRS): FORCE
	@${MAKE} -C $@ -f xLights.cbp.mak OBJDIR_DEBUGLINUX=".objs_debug" releaselinux


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
	cp xLights/xLights.cbp.mak xLights/xLights.cbp.mak.orig
	cat xLights/xLights.cbp.mak.orig \
		| sed \
			-e "s/^CFLAGS = \(.*\)/CFLAGS = \1 -std=gnu++0x/" \
			-e "s/CFLAGS_RELEASELINUX = \(.*\)/CFLAGS_RELEASELINUX = \1 $(IGNORE_WARNINGS)/" \
			-e "s/LDFLAGS_DEBUGLINUX = \(.*\)/LDFLAGS_DEBUGLINUX = \1 \`pkg-config --libs gstreamer-interfaces-0.10\`/" \
			-e "s/LDFLAGS_RELEASELINUX = \(.*\)/LDFLAGS_RELEASELINUX = \1 \`pkg-config --libs gstreamer-interfaces-0.10\`/" \
			-e "s/OBJDIR_DEBUGLINUX = \(.*\)/OBJDIR_DEBUGLINUX = .objs_debug/" \
		> xLights/xLights.cbp.mak

#############################################################################

FORCE:
