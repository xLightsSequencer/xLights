
INSTALL_ROOT    =
PREFIX          = /usr/local

MKDIR           = mkdir -p
CHK_DIR_EXISTS  = test -d
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE        = rm -f

SUBDIRS         = xLights

all: subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS): FORCE
	${MAKE} -C $@ -f xLights.cbp.mak

clean: $(addsuffix _clean,$(SUBDIRS))

$(addsuffix _clean,$(SUBDIRS)):
	${MAKE} -C $(subst _clean,,$@) -f xLights.cbp.mak clean

install:
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/${PREFIX}/bin || $(MKDIR) $(INSTALL_ROOT)/${PREFIX}/bin
	-$(INSTALL_PROGRAM) bin/xLights $(INSTALL_ROOT)/${PREFIX}/bin/xLights

uninstall:
	-$(DEL_FILE) $(INSTALL_ROOT)/${PREFIX}/bin/xLights

FORCE:
