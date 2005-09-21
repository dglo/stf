# Add auto-generated include files to list of includes
#   Build list be translating XML file names into include file names.
XML_HDRS_SEARCH := $(patsubst %,%/*.xml, $(PUB_DIRS) $(PVT_DIRS))
XML_HDRS := $(wildcard $(XML_HDRS_SEARCH))
BUILT_HDRS := $(patsubst %.xml, $(BUILD_DIR)/%.$(C_INC_SUFFIX), $(XML_HDRS))

INCS := $(INCS) $(BUILT_HDRS)