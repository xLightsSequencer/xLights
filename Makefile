PREFIX          = /usr

# When not installing to a custom location append that location
# to PKG_CONFIG_PATH so that pkgconfig can find the libraries installed.
# The "if" is to add : only when there's an existing PKG_CONFIG_PATH.
export PKG_CONFIG_PATH := $(if $(PKG_CONFIG_PATH),$(PKG_CONFIG_PATH):$(PREFIX)/lib/pkgconfig/,$(PREFIX)/lib/pkgconfig)

# Make sure wx-config will be found.
export PATH := $(PREFIX)/bin:$(PATH)

# Ignore some warnings for now to make compile output cleaner
# until the issues are cleaned up in the code.
#IGNORE_WARNINGS = -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-switch -Wno-unused-but-set-variable -Wno-parentheses -Wno-return-type -Wno-uninitialized -Wno-unused-value -Wno-sequence-point -Wno-comment -Wno-unused-function
IGNORE_WARNINGS = -Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unknown-pragmas

MKDIR           = mkdir -p
CHK_DIR_EXISTS  = test -d
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE        = rm -f
ICON_SIZES      = 16x16 32x32 64x64 128x128 256x256
SHARE_FILES     = xlights.linux.properties phoneme_mapping extended_dictionary standard_dictionary user_dictionary xschedule.linux.properties xcapture.linux.properties  xfade.linux.properties xscanner.linux.properties xscanner.linux.properties xsmsdaemon.linux.properties remotefalcon.linux.properties
QMVAMP_FILES	= INSTALL_linux.txt qm-vamp-plugins.n3 README.txt qm-vamp-plugins.cat
# run with `SUDO= make` when installing to a location that doesn't require root
SUDO		= `which sudo`

SUBDIRS         = xLights xSchedule xCapture xFade xScanner xSchedule/xSMSDaemon xSchedule/RemoteFalcon

WXWIDGETS_TAG=xlights_2023.02

.NOTPARALLEL:

all: wxwidgets31 log4cpp cbp2make linkliquid libxlsxwriter makefile subdirs

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

libxlsxwriter: FORCE
	@printf "Linking libxlsxwriter\n"
	@if test ! -e dependencies/libxlsxwriter/lib/libxlsxwriter.a; \
		then cd dependencies/libxlsxwriter; \
		${MAKE} -s; \
	fi

log4cpp: FORCE
	@printf "Checking log4cpp\n"
	@if test "`log4cpp-config --version`" != "1.1"; \
		then if test ! -d log4cpp; \
			then echo Downloading log4cpp; wget --no-verbose -c https://nchc.dl.sourceforge.net/project/log4cpp/log4cpp-1.1.x%20%28new%29/log4cpp-1.1/log4cpp-1.1.3.tar.gz; \
			tar xfz log4cpp-1.1.3.tar.gz ;\
		fi; \
		cd log4cpp; \
		./configure --prefix=$(PREFIX); \
		echo Building log4cpp; \
		${MAKE} -s; \
		echo Installing log4cpp; \
		$(SUDO) ${MAKE} install DESTDIR=$(DESTDIR); \
		echo Completed build/install of log4cpp; \
		fi

wxwidgets31: FORCE
	@printf "Checking wxwidgets\n"
	@if test "`wx-config --version`" != "3.3.0"; \
		then if test ! -d wxWidgets-$(WXWIDGETS_TAG); \
			then echo Downloading wxwidgets; git clone --depth=1 --shallow-submodules  --recurse-submodules -b $(WXWIDGETS_TAG) https://github.com/xLightsSequencer/wxWidgets wxWidgets-$(WXWIDGETS_TAG); \
		fi; \
		cd wxWidgets-$(WXWIDGETS_TAG); \
		./configure --enable-cxx11 --with-cxx=17 --enable-std_containers --enable-std_string_conv_in_wxstring --enable-backtrace --enable-exceptions --enable-mediactrl --enable-graphics_ctx --enable-monolithic --disable-gtktest --disable-sdltest --with-gtk=3 --disable-glcanvasegl --disable-pcx --disable-iff --without-libtiff --prefix=$(PREFIX); \
		echo Building wxwidgets; \
		${MAKE} -j 4 -s; \
		echo Installing wxwidgets; \
		$(SUDO) ${MAKE} install DESTDIR=$(DESTDIR); \
		echo Completed build/install of wxwidgets; \
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
	-$(INSTALL_PROGRAM) -D bin/xSMSDaemon.so $(DESTDIR)/${PREFIX}/bin/xSMSDaemon.so
	-$(INSTALL_PROGRAM) -D bin/RemoteFalcon.so $(DESTDIR)/${PREFIX}/bin/RemoteFalcon.so
	-$(INSTALL_PROGRAM) -D bin/xCapture $(DESTDIR)/${PREFIX}/bin/xCapture
	-$(INSTALL_PROGRAM) -D bin/xFade $(DESTDIR)/${PREFIX}/bin/xFade
	-$(INSTALL_PROGRAM) -D bin/xScanner $(DESTDIR)/${PREFIX}/bin/xScanner
	-$(INSTALL_PROGRAM) -D bin/xlights.desktop $(DESTDIR)/${PREFIX}/share/applications/xlights.desktop
	-$(INSTALL_PROGRAM) -D bin/xschedule.desktop $(DESTDIR)/${PREFIX}/share/applications/xschedule.desktop
	-$(INSTALL_PROGRAM) -D bin/xsmsdaemon.desktop $(DESTDIR)/${PREFIX}/share/applications/xsmsdaemon.desktop
	-$(INSTALL_PROGRAM) -D bin/xcapture.desktop $(DESTDIR)/${PREFIX}/share/applications/xcapture.desktop
	-$(INSTALL_PROGRAM) -D bin/xfade.desktop $(DESTDIR)/${PREFIX}/share/applications/xfade.desktop
	-$(INSTALL_PROGRAM) -D bin/xscanner.desktop $(DESTDIR)/${PREFIX}/share/applications/xscanner.desktop
	$(foreach share, $(SHARE_FILES), install -D -m 644 bin/$(share) $(DESTDIR)/${PREFIX}/share/xLights/$(share) ;)
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/colorcurves
	cp -r colorcurves/* $(DESTDIR)/${PREFIX}/share/xLights/colorcurves
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/controllers
	cp -r controllers/* $(DESTDIR)/${PREFIX}/share/xLights/controllers
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/meshobjects
	cp -r meshobjects/* $(DESTDIR)/${PREFIX}/share/xLights/meshobjects
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/valuecurves
	cp -r valuecurves/* $(DESTDIR)/${PREFIX}/share/xLights/valuecurves
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xSchedule/xScheduleWeb
	cp -r bin/xScheduleWeb/* $(DESTDIR)/${PREFIX}/share/xSchedule/xScheduleWeb
	install -d -m 755 $(DESTDIR)/${PREFIX}/share/xScanner
	cp xScanner/MacLookup.txt $(DESTDIR)/${PREFIX}/share/xScanner/MacLookup.txt
	#install -d -m 755 $(DESTDIR)/${PREFIX}/share/xLights/songs
	#cp -r songs/* $(DESTDIR)/${PREFIX}/share/xLights/songs
	$(foreach size, $(ICON_SIZES), install -D -m 644 images/xLightsIcons/$(size).png $(DESTDIR)/${PREFIX}/share/icons/hicolor/$(size)/apps/xlights.png ; )
	install -D -m 644 images/xLightsIcons/16x16.png $(DESTDIR)/${PREFIX}/share/icons/hicolor/16x16/apps/xschedule.png
	install -D -m 644 images/xLightsIcons/32x32.png $(DESTDIR)/${PREFIX}/share/icons/hicolor/32x32/apps/xschedule.png
	install -D -m 644 images/xLightsIcons/256x256.png $(DESTDIR)/${PREFIX}/share/icons/hicolor/256x256/apps/xschedule.png
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
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/share/applications/xscanner.desktop
	-$(DEL_FILE) $(DESTDIR)/${PREFIX}/share/applications/xsmsdaemon.desktop

#############################################################################

cbp2make:
	@if test -n "`cbp2make --version`"; \
		then $(DEL_FILE) xLights/xLights.cbp.mak xSchedule/xSchedule.cbp.mak xCapture/xCapture.cbp.mak xFade/xFade.cbp.mak xScanner/xScanner.cbp.mak xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak xSchedule/RemoteFalcon/RemoteFalcon.cbp.mak; \
	fi

makefile: xLights/xLights.cbp.mak xSchedule/xSchedule.cbp.mak xCapture/xCapture.cbp.mak xFade/xFade.cbp.mak xScanner/xScanner.cbp.mak xSchedule/xSMSDaemon/xSMSDaemon.cbp.mak xSchedule/RemoteFalcon/RemoteFalcon.cbp.mak

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

xSchedule/RemoteFalcon/RemoteFalcon.cbp.mak: xSchedule/RemoteFalcon/RemoteFalcon.cbp
	@cbp2make -in xSchedule/RemoteFalcon/RemoteFalcon.cbp -cfg cbp2make.cfg -out xSchedule/RemoteFalcon/RemoteFalcon.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	@cp xSchedule/RemoteFalcon/RemoteFalcon.cbp.mak xSchedule/RemoteFalcon/RemoteFalcon.cbp.mak.orig
	@cat xSchedule/RemoteFalcon/RemoteFalcon.cbp.mak.orig \
		| sed \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xSchedule/RemoteFalcon/RemoteFalcon.cbp.mak

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

xScanner/xScanner.cbp.mak: xScanner/xScanner.cbp
	@cbp2make -in xScanner/xScanner.cbp -cfg cbp2make.cfg -out xScanner/xScanner.cbp.mak \
			--with-deps --keep-outdir --keep-objdir
	@cp xScanner/xScanner.cbp.mak xScanner/xScanner.cbp.mak.orig
	@cat xScanner/xScanner.cbp.mak.orig \
		| sed \
			-e "s/CFLAGS_LINUX_RELEASE = \(.*\)/CFLAGS_LINUX_RELEASE = \1 $(IGNORE_WARNINGS)/" \
			-e "s/OBJDIR_LINUX_DEBUG = \(.*\)/OBJDIR_LINUX_DEBUG = .objs_debug/" \
		> xScanner/xScanner.cbp.mak

#############################################################################

FORCE:
