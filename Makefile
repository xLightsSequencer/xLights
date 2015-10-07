
INSTALL_ROOT    =
PREFIX          = /usr/local

# Ignore some warnings for now to make compile output cleaner
# until the issues are cleaned up in the code.
IGNORE_WARNINGS = -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-switch -Wno-unused-but-set-variable -Wno-parentheses -Wno-return-type -Wno-uninitialized -Wno-unused-value -Wno-sequence-point -Wno-comment -Wno-unused-function

MKDIR           = mkdir -p
CHK_DIR_EXISTS  = test -d
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE        = rm -f
ICON_SIZES      = 16x16 32x32 64x64 256x256

SUBDIRS         = xLights

all: makefile subdirs

#############################################################################

subdirs: $(SUBDIRS)

$(SUBDIRS): FORCE
	@${MAKE} -C $@ -f xLights.cbp.mak OBJDIR_LINUX_DEBUG=".objs_debug" linux_release


#############################################################################

debug: $(addsuffix _debug,$(SUBDIRS))

$(addsuffix _debug,$(SUBDIRS)):
	@${MAKE} -C $(subst _debug,,$@) -f xLights.cbp.mak OBJDIR_LINUX_DEBUG=".objs_debug" linux_debug

#############################################################################

clean: $(addsuffix _clean,$(SUBDIRS))

$(addsuffix _clean,$(SUBDIRS)):
	@${MAKE} -C $(subst _clean,,$@) -f xLights.cbp.mak OBJDIR_LINUX_DEBUG=".objs_debug" clean

#############################################################################

install:
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/${PREFIX}/bin || $(MKDIR) $(INSTALL_ROOT)/${PREFIX}/bin
	-$(INSTALL_PROGRAM) bin/xLights $(INSTALL_ROOT)/${PREFIX}/bin/xLights
	-$(INSTALL_PROGRAM) bin/xlights.desktop $(INSTALL_ROOT)/${PREFIX}/share/applications/xlights.desktop
	install -d -m 755 $(INSTALL_ROOT)/${PREFIX}/share/xLights/piano
	cp -r piano/* $(INSTALL_ROOT)/${PREFIX}/share/xLights/piano
	install -d -m 755 $(INSTALL_ROOT)/${PREFIX}/share/xLights/songs
	cp -r songs/* $(INSTALL_ROOT)/${PREFIX}/share/xLights/songs
	$(foreach size, $(ICON_SIZES), install -m 644 xLights/Images.xcassets/AppIcon.appiconset/$(size).png $(INSTALL_ROOT)/${PREFIX}/share/icons/hicolor/$(size)/apps/xlights.png ; )

uninstall:
	-$(DEL_FILE) $(INSTALL_ROOT)/${PREFIX}/bin/xLights
	-$(DEL_FILE) $(INSTALL_ROOT)/${PREFIX}/share/applications/xlights.desktop

#############################################################################

makefile: xLights/xLights.cbp.mak

xLights/xLights.cbp.mak: xLights/xLights.cbp
	@cbp2make -in xLights/xLights.cbp -cfg cbp2make.cfg -out xLights/xLights.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	cp xLights/xLights.cbp.mak xLights/xLights.cbp.mak.orig
	cat xLights/xLights.cbp.mak.orig \
		| sed \
			-e "s/^CFLAGS = \(.*\)/CFLAGS = \1 -std=gnu++11/" \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/LDFLAGS_LINUX_DEBUG = \(.*\)/LDFLAGS_LINUX_DEBUG = \1 \`pkg-config --libs gstreamer-interfaces-0.10\`/" \
			-e "s/LDFLAGS_LINUX_RELEASE = \(.*\)/LDFLAGS_LINUX_RELEASE = \1 \`pkg-config --libs gstreamer-interfaces-0.10\`/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xLights/xLights.cbp.mak

#############################################################################

FORCE:
