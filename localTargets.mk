$(BUILD_DIR)/$(PUB_DIR_NAME)/%.h : %.xml
	@test -d $(@D) || mkdir -p $(@D)
	$(XSLT) -in $< -out $(@D)/$(*F)Signatures.h -xsl $(<D)/stfDefn2Signatures.xsl
	$(XSLT) -in $< -out $@ -xsl $(<D)/stfDefn2Header.xsl

$(BUILD_DIR)/$(PVT_DIR_NAME)/%.h : %.xml
	@test -d $(@D) || mkdir -p $(@D)
	$(XSLT) -in $< -out $(@D)/$(*F)Signatures.h -xsl $(<D)/stfDefn2Signatures.xsl
	$(XSLT) -in $< -out $@ -xsl $(<D)/stfDefn2Header.xsl

$(BUILD_DIR)/$(PVT_DIR_NAME)/%Dictionary.c : %.xsd
	@test -d $(@D) || mkdir -p $(@D)
	$(XSLT) -in $< -out $@ -xsl $(<D)/xsd2Dictionary.xsl

-include ../dom-loader/$(PLATFORM_PUB_ROOT)/loader/loaderTargets.mk
