
DESTDIR    =
PREFIX          = /usr/local

# Ignore some warnings for now to make compile output cleaner
# until the issues are cleaned up in the code.
#IGNORE_WARNINGS = -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-switch -Wno-unused-but-set-variable -Wno-parentheses -Wno-return-type -Wno-uninitialized -Wno-unused-value -Wno-sequence-point -Wno-comment -Wno-unused-function
IGNORE_WARNINGS = -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unknown-pragmas

MKDIR           = mkdir -p
CHK_DIR_EXISTS  = test -d
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE        = rm -f
ICON_SIZES      = 16x16 32x32 64x64 256x256
SHARE_FILES     = xlights.linux.properties phoneme_mapping extended_dictionary standard_dictionary user_dictionary
PATH            := $(CURDIR)/wxWidgets-3.1.0:$(PATH)

SUBDIRS         = xLights

.NOTPARALLEL:

all: wxwidgets31 makefile subdirs

#############################################################################

subdirs: $(SUBDIRS)

$(SUBDIRS): FORCE
	@${MAKE} -C $@ -f xLights.cbp.mak OBJDIR_LINUX_DEBUG=".objs_debug" linux_release


#############################################################################

wxwidgets31: FORCE
	if test "`wx-config --release`" != "3.1"; \
		then if test ! -d wxWidgets-3.1.0; \
			then wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.0/wxWidgets-3.1.0.tar.bz2; \
			tar xvfj wxWidgets-3.1.0.tar.bz2; \
		fi; \
		cd wxWidgets-3.1.0;\
			CXXFLAGS="-std=gnu++14" ./configure --enable-mediactrl --enable-graphics_ctx --enable-monolithic --disable-shared --disable-gtktest --disable-sdltest; \
			${MAKE} ;\
	fi

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
	@$(CHK_DIR_EXISTS) $(DESTDIR)/${PREFIX}/bin || $(MKDIR) $(DESTDIR)/${PREFIX}/bin
	-$(INSTALL_PROGRAM) -D bin/xLights $(DESTDIR)/${PREFIX}/bin/xLights
	-$(INSTALL_PROGRAM) -D bin/xlights.desktop $(DESTDIR)/${PREFIX}/share/applications/xlights.desktop
	$(foreach share, $(SHARE_FILES), install -D -m 644 bin/$(share) $(DESTDIR)/${PREFIX}/share/xLights/$(share) ;)
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/piano
	cp -r piano/* $(DESTDIR)/${PREFIX}/share/xLights/piano
	#install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/songs
	#cp -r songs/* $(DESTDIR)/${PREFIX}/share/xLights/songs
	$(foreach size, $(ICON_SIZES), install -D -m 644 xLights/Images.xcassets/AppIcon.appiconset/$(size).png $(DESTDIR)/${PREFIX}/share/icons/hicolor/$(size)/apps/xlights.png ; )

uninstall:
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/bin/xLights
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/share/applications/xlights.desktop

#############################################################################

makefile: xLights/xLights.cbp.mak

xLights/xLights.cbp.mak: xLights/xLights.cbp
	@cbp2make -in xLights/xLights.cbp -cfg cbp2make.cfg -out xLights/xLights.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	cp xLights/xLights.cbp.mak xLights/xLights.cbp.mak.orig
	cat xLights/xLights.cbp.mak.orig \
		| sed \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xLights/xLights.cbp.mak

#############################################################################

FORCE:
