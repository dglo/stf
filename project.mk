C_BIN_NAMES := menu stfserv echomode
C_EXCLUDE_NAMES := gendir genqry mk-name stftcp stfclient stf-win32
USES_PROJECTS := iceboot dom-loader hal
USES_TOOLS := libxml expat z m

ifeq ("epxa10","$(strip $(PLATFORM))")
  USES_TOOLS := $(filter-out libxml, $(USES_TOOLS))
else
  USES_PROJECTS := $(filter-out dom-loader, $(USES_PROJECTS))
endif
