
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
ICON_SIZES      = 16x16 32x32 64x64 128x128 256x256
SHARE_FILES     = xlights.linux.properties phoneme_mapping extended_dictionary standard_dictionary user_dictionary xschedule.linux.properties
QMVAMP_FILES	= INSTALL_linux.txt qm-vamp-plugins.n3 README.txt qm-vamp-plugins.cat
PATH            := $(CURDIR)/wxWidgets-3.1.2:$(PATH)

SUBDIRS         = xLights xSchedule xCapture xFade xSchedule/xSMSDaemon

.NOTPARALLEL:

all: wxwidgets31 cbp2make linkliquid makefile subdirs

#############################################################################

subdirs: $(SUBDIRS)

$(SUBDIRS): FORCE
	@${MAKE} -C $@ -f `basename $@`.cbp.mak OBJDIR_LINUX_DEBUG=".objs_debug" linux_release


#############################################################################

linkliquid:
	@printf "Linking libliquid\n"
	@if test ! -e lib/linux/libliquidfun.a; \
		then if test "${DEB_HOST_ARCH}" = "i386"; \
            then ln -s libliquidfun.a.i686 lib/linux/libliquidfun.a; \
            elif test "${DEB_HOST_ARCH}" = "amd64"; \
            then ln -s libliquidfun.a.x86_64 lib/linux/libliquidfun.a; \
            else ln -s libliquidfun.a.`uname -m` lib/linux/libliquidfun.a; \
        fi; \
	fi

wxwidgets31: FORCE
	@printf "Checking wxwidgets\n"
	@if test "`wx-config --version`" != "3.1.2"; \
		then if test ! -d wxWidgets-3.1.2; \
			then echo Downloading wxwidgets; wget --no-verbose -c https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.2/wxWidgets-3.1.2.tar.bz2; \
			tar xfj wxWidgets-3.1.2.tar.bz2; \
		fi; \
		cd wxWidgets-3.1.2; \
		patch -p1 < ../lib/linux/wxwidgets-31.patch; \
		CXXFLAGS="-std=gnu++14" ./configure --enable-cxx11 --enable-std_containers --enable-std_string --enable-std_string_conv_in_wxstring --enable-backtrace --enable-exceptions --enable-mediactrl --enable-graphics_ctx --enable-monolithic --disable-shared --disable-gtktest --disable-sdltest --with-gtk=2 --disable-pcx --disable-iff --without-libtiff; \
		echo Building wxwidgets; \
		${MAKE} -s; \
		echo Completed build of wxwidgets; \
        fi


#############################################################################

debug: $(addsuffix _debug,$(SUBDIRS))

$(addsuffix _debug,$(SUBDIRS)):
	@${MAKE} -C $(subst _debug,,$@) -f $(subst _debug,,`basename $@`).cbp.mak OBJDIR_LINUX_DEBUG=".objs_debug" linux_debug

#############################################################################

clean: $(addsuffix _clean,$(SUBDIRS))

$(addsuffix _clean,$(SUBDIRS)):
	@${MAKE} -C $(subst _clean,,$@) -f $(subst _clean,,`basename $@`).cbp.mak OBJDIR_LINUX_DEBUG=".objs_debug" clean
	

#############################################################################

install:
	@$(CHK_DIR_EXISTS) $(DESTDIR)/${PREFIX}/bin || $(MKDIR) $(DESTDIR)/${PREFIX}/bin
	-$(INSTALL_PROGRAM) -D bin/xLights $(DESTDIR)/${PREFIX}/bin/xLights
	-$(INSTALL_PROGRAM) -D bin/xSchedule $(DESTDIR)/${PREFIX}/bin/xSchedule
	-$(INSTALL_PROGRAM) -D bin/xSMSDaemon $(DESTDIR)/${PREFIX}/bin/xSMSDaemon
	-$(INSTALL_PROGRAM) -D bin/xCapture $(DESTDIR)/${PREFIX}/bin/xCapture
	-$(INSTALL_PROGRAM) -D bin/xFade $(DESTDIR)/${PREFIX}/bin/xFade
	-$(INSTALL_PROGRAM) -D bin/xlights.desktop $(DESTDIR)/${PREFIX}/share/applications/xlights.desktop
	-$(INSTALL_PROGRAM) -D bin/xschedule.desktop $(DESTDIR)/${PREFIX}/share/applications/xschedule.desktop
	-$(INSTALL_PROGRAM) -D bin/xsmsdaemon.desktop $(DESTDIR)/${PREFIX}/share/applications/xsmsdaemon.desktop
	-$(INSTALL_PROGRAM) -D bin/xcapture.desktop $(DESTDIR)/${PREFIX}/share/applications/xcapture.desktop
	-$(INSTALL_PROGRAM) -D bin/xfade.desktop $(DESTDIR)/${PREFIX}/share/applications/xfade.desktop
	$(foreach share, $(SHARE_FILES), install -D -m 644 bin/$(share) $(DESTDIR)/${PREFIX}/share/xLights/$(share) ;)
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/colorcurves
	cp -r colorcurves/* $(DESTDIR)/${PREFIX}/share/xLights/colorcurves
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/controllers
	cp -r controllers/* $(DESTDIR)/${PREFIX}/share/xLights/controllers
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xSchedule/xScheduleWeb
	cp -r bin/xScheduleWeb/* $(DESTDIR)/${PREFIX}/share/xSchedule/xScheduleWeb
	#install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/songs
	#cp -r songs/* $(DESTDIR)/${PREFIX}/share/xLights/songs
	$(foreach size, $(ICON_SIZES), install -D -m 644 xLights/Images.xcassets/AppIcon.appiconset/$(size).png $(DESTDIR)/${PREFIX}/share/icons/hicolor/$(size)/apps/xlights.png ; )
	install -D -m 644 xSchedule/Assets.xcassets/AppIcon.appiconset/xschedule-6.png $(DESTDIR)/${PREFIX}/share/icons/hicolor/16x16/apps/xschedule.png
	install -D -m 644 xSchedule/Assets.xcassets/AppIcon.appiconset/xschedule-9.png $(DESTDIR)/${PREFIX}/share/icons/hicolor/32x32/apps/xschedule.png
	install -D -m 644 xSchedule/Assets.xcassets/AppIcon.appiconset/xschedule-13.png $(DESTDIR)/${PREFIX}/share/icons/hicolor/256x256/apps/xschedule.png
	install -d -m 755 $(DESTDIR)/${PREFIX}/lib/vamp
	$(foreach qmvamp, $(QMVAMP_FILES), install -D -m 644 lib/linux/qm-vamp-plugins-1.7/$(qmvamp) $(DESTDIR)/${PREFIX}/lib/vamp/$(share) ;)
	install -D -m 644 lib/linux/qm-vamp-plugins-1.7/qm-vamp-plugins.so.`uname -m` $(DESTDIR)/${PREFIX}/lib/vamp/qm-vamp-plugins.so

uninstall:
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/bin/xSchedule
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/bin/xLights
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/share/applications/xlights.desktop
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/share/applications/xschedule.desktop
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/share/applications/xcapture.desktop
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/share/applications/xfade.desktop

#############################################################################

cbp2make:
	@if test -n "`cbp2make --version`"; \
		then $(DEL_FILE) xLights/xLights.cbp.mak xSchedule/xSchedule.cbp.mak xCapture/xCapture.cbp.mak xFade/xFade.cbp.mak xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak; \
	fi

makefile: xLights/xLights.cbp.mak xSchedule/xSchedule.cbp.mak xCapture/xCapture.cbp.mak xFade/xFade.cbp.mak xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak

xLights/xLights.cbp.mak: xLights/xLights.cbp
	@cbp2make -in xLights/xLights.cbp -cfg cbp2make.cfg -out xLights/xLights.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	@cp xLights/xLights.cbp.mak xLights/xLights.cbp.mak.orig
	@cat xLights/xLights.cbp.mak.orig \
		| sed \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xLights/xLights.cbp.mak

xSchedule/xSchedule.cbp.mak: xSchedule/xSchedule.cbp
	@cbp2make -in xSchedule/xSchedule.cbp -cfg cbp2make.cfg -out xSchedule/xSchedule.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	@cp xSchedule/xSchedule.cbp.mak xSchedule/xSchedule.cbp.mak.orig
	@cat xSchedule/xSchedule.cbp.mak.orig \
		| sed \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xSchedule/xSchedule.cbp.mak

xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak: xSchedule/xSMSDaemon/xSMSDaemon.cbp
	@cbp2make -in xSchedule/xSMSDaemon/xSMSDaemon.cbp -cfg cbp2make.cfg -out xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	@cp xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak.orig
	@cat xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak.orig \
		| sed \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak

xCapture/xCapture.cbp.mak: xCapture/xCapture.cbp
	@cbp2make -in xCapture/xCapture.cbp -cfg cbp2make.cfg -out xCapture/xCapture.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	@cp xCapture/xCapture.cbp.mak xCapture/xCapture.cbp.mak.orig
	@cat xCapture/xCapture.cbp.mak.orig \
		| sed \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xCapture/xCapture.cbp.mak

xFade/xFade.cbp.mak: xFade/xFade.cbp
	@cbp2make -in xFade/xFade.cbp -cfg cbp2make.cfg -out xFade/xFade.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	@cp xFade/xFade.cbp.mak xFade/xFade.cbp.mak.orig
	@cat xFade/xFade.cbp.mak.orig \
		| sed \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xFade/xFade.cbp.mak

#############################################################################

FORCE:
