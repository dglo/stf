<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:stf="http://glacier.lbl.gov/icecube/daq/stf"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output indent="yes" method="xml" omit-xml-declaration="no"/>
    <xsl:variable name="testName" select="stf:test/name"/>
    <xsl:template match="/">
        <stf:setup xmlns:stf="http://glacier.lbl.gov/icecube/daq/stf"
            xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://glacier.lbl.gov/icecube/daq/stf stf.xsd">
            <xsl:element name="{$testName}">
                <xsl:apply-templates select="stf:test/inputParameter"/>
            </xsl:element>
        </stf:setup>
    </xsl:template>
    <xsl:template match="stf:test/inputParameter">
        <xsl:variable name="paraName" select="./name"/>
        <xsl:apply-templates mode="comment" select="./description"/>
        <xsl:element name="{$paraName}">
            <xsl:apply-templates select="./*"/>
        </xsl:element>
    </xsl:template>
    <xsl:template match="stf:test/inputParameter/*">
        <xsl:value-of select="@default"/>
    </xsl:template>
    <xsl:template match="stf:test/inputParameter/description" mode="comment">
        <xsl:comment>
            <xsl:value-of select="."/>
        </xsl:comment>
    </xsl:template>
</xsl:stylesheet>
