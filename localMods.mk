# Add auto-generated include files to list of includes
XML_HDR_SEARCH := $(patsubst %,%/*.xml, $(PUB_DIRS) $(PVT_DIRS))
XML_HDRS_SRCS := $(wildcard $(XML_HDR_SEARCH))
XML_HDRS := $(patsubst %.xml,$(BUILD_DIR)/%.$(C_INC_SUFFIX), $(XML_HDRS_SRCS))
XML_HDR_TARGETS :=$(subst $(PLATFORM_PUB_ROOT)/$(PLATFORM),$(PUB_ROOT), $(subst $(PVT_ROOT)/$(PLATFORM),$(PVT_ROOT), $(XML_HDRS)))

DEFAULT_XML_TARGETS := $(patsubst %.xml,$(COMMON_RSRC_DIR)/%Default.xml, $(subst private/stf-apps/,, $(XML_HDRS_SRCS)))

BUILT_FILES += $(XML_HDR_TARGETS) $(BUILD_DIR)/$(PVT_DIR_NAME)/stf-apps/stfDictionary.c $(DEFAULT_XML_TARGETS)

vpath %.xml $(PLATFORM_PUB_ROOT) $(PUB_ROOT) $(PLATFORM_PVT_ROOT) $(PVT_ROOT)
vpath %.xsd $(PLATFORM_PUB_ROOT) $(PUB_ROOT) $(PLATFORM_PVT_ROOT) $(PVT_ROOT)

-include ../dom-loader/$(PLATFORM_PUB_ROOT)/loader/loaderMods.mk
