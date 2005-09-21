$(BUILD_DIR)/$(PUB_DIR_NAME)/%.h : $(PUB_DIR_NAME)/%.xml
	@test -d $(@D) || mkdir -p $(@D)
	$(XSLT) -in $< -out $@ -xsl $(PRIVATE_DIR_NAME)/stf-apps/stfDefn2Header.xsl

$(BUILD_DIR)/$(PVT_DIR_NAME)/%.h : $(PVT_DIR_NAME)/%.xml
	@test -d $(@D) || mkdir -p $(@D)
	$(XSLT) -in $< -out $@ -xsl $(<D)/stfDefn2Header.xsl
