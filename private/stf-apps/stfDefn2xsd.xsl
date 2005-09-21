<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:stf="http://glacier.lbl.gov/icecube/daq/stf"
  xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="xml" omit-xml-declaration="no"/>
  <xsl:variable name="testName" select="stf:test/name"/>
  <xsl:template match="/">
    <xs:schema targetNamespace="http://glacier.lbl.gov/icecube/daq/stf"
      xmlns="http://glacier.lbl.gov/icecube/daq/stf" xmlns:xs="http://www.w3.org/2001/XMLSchema">
      <xs:complexType>
        <xsl:attribute name="name">
          <xsl:copy-of select="$testName"/>Version</xsl:attribute>
        <xs:annotation>
          <xs:documentation>
            <xsl:value-of select="stf:test/description"/>
          </xs:documentation>
        </xs:annotation>
        <xs:attribute name="major" use="required">
          <xs:simpleType>
            <xs:restriction base="xs:nonNegativeInteger">
              <xs:enumeration>
                <xsl:attribute name="value">
                  <xsl:value-of select="stf:test/version/@major"/>
                </xsl:attribute>
              </xs:enumeration>
            </xs:restriction>
          </xs:simpleType>
        </xs:attribute>
        <xs:attribute name="minor" use="required">
          <xs:simpleType>
            <xs:restriction base="xs:nonNegativeInteger">
              <xs:enumeration>
                <xsl:attribute name="value">
                  <xsl:value-of select="stf:test/version/@minor"/>
                </xsl:attribute>
              </xs:enumeration>
            </xs:restriction>
          </xs:simpleType>
        </xs:attribute>
      </xs:complexType>
      <xsl:apply-templates mode="restrictions" select="stf:test/inputParameter"/>
      <xs:complexType>
        <xsl:attribute name="name">
          <xsl:copy-of select="$testName"/>Setup</xsl:attribute>
        <xs:sequence>
          <xs:element minOccurs="0" name="version">
            <xsl:attribute name="type">
              <xsl:copy-of select="$testName"/>Version</xsl:attribute>
          </xs:element>
          <xs:element minOccurs="0" name="parameters">
            <xs:complexType>
              <xs:all>
                <xsl:apply-templates mode="setup" select="stf:test/inputParameter"/>
              </xs:all>
            </xs:complexType>
          </xs:element>
        </xs:sequence>
      </xs:complexType>
      <xs:complexType>
        <xsl:attribute name="name">
          <xsl:copy-of select="$testName"/>Result</xsl:attribute>
        <xs:sequence>
          <xs:element name="version">
            <xsl:attribute name="type">
              <xsl:copy-of select="$testName"/>Version</xsl:attribute>
          </xs:element>
          <xs:element name="parameters">
            <xs:complexType>
              <xs:all>
                <xsl:apply-templates mode="result" select="stf:test/inputParameter|stf:test/outputParameter"/>
              </xs:all>
            </xs:complexType>
          </xs:element>
        </xs:sequence>
      </xs:complexType>
    </xs:schema>
  </xsl:template>
  <xsl:template match="stf:test/inputParameter" mode="restrictions">
    <xsl:apply-templates mode="restrictions" select="unsignedInt"/>
    <xsl:apply-templates mode="restrictions" select="unsignedLong"/>
  </xsl:template>
  <xsl:template match="stf:test/inputParameter/boolean" mode="restrictions"/>
  <xsl:template match="stf:test/inputParameter/unsignedInt" mode="restrictions">
    <xs:simpleType>
      <xsl:attribute name="name">
        <xsl:copy-of select="$testName"/>
        <xsl:copy-of select="../name"/>
      </xsl:attribute>
      <xs:restriction base="xs:unsignedInt">
        <xs:minInclusive>
          <xsl:attribute name="value">
            <xsl:value-of select="@minValue"/>
          </xsl:attribute>
        </xs:minInclusive>
        <xs:maxInclusive>
          <xsl:attribute name="value">
            <xsl:value-of select="@maxValue"/>
          </xsl:attribute>
        </xs:maxInclusive>
      </xs:restriction>
    </xs:simpleType>
  </xsl:template>
  <xsl:template match="stf:test/inputParameter/unsignedLong" mode="restrictions">
    <xs:simpleType>
      <xsl:attribute name="name">
        <xsl:copy-of select="$testName"/>
        <xsl:copy-of select="../name"/>
      </xsl:attribute>
      <xs:restriction base="xs:unsignedLong">
        <xs:minInclusive>
          <xsl:attribute name="value">
            <xsl:value-of select="@minValue"/>
          </xsl:attribute>
        </xs:minInclusive>
        <xs:maxInclusive>
          <xsl:attribute name="value">
            <xsl:value-of select="@maxValue"/>
          </xsl:attribute>
        </xs:maxInclusive>
      </xs:restriction>
    </xs:simpleType>
  </xsl:template>
  <xsl:template match="stf:test/*/boolean" mode="paraType">xs:boolean</xsl:template>
  <xsl:template match="stf:test/*/string" mode="paraType">xs:string</xsl:template>
  <xsl:template match="stf:test/*/unsignedInt" mode="paraType">xs:unsignedInt</xsl:template>
  <xsl:template match="stf:test/*/unsignedLong" mode="paraType">xs:unsignedLong</xsl:template>
  <xsl:template
    match="stf:test/inputParameter/unsignedInt|stf:test/inputParameter/unsignedLong" mode="restrictedParaType">
    <xsl:copy-of select="$testName"/>
    <xsl:copy-of select="../name"/>
  </xsl:template>
  <xsl:template match="stf:test/inputParameter" mode="setup">
    <xs:element>
      <xsl:apply-templates mode="default" select="boolean|string|unsignedInt|unsignedLong"/>
      <xsl:attribute name="minOccurs"> 0 </xsl:attribute>
      <xsl:attribute name="name">
        <xsl:copy-of select="name"/>
      </xsl:attribute>
      <xsl:attribute name="type">
        <xsl:apply-templates mode="paraType" select="boolean|string"/>
        <xsl:apply-templates mode="restrictedParaType" select="unsignedInt|unsignedLong"/>
      </xsl:attribute>
    </xs:element>
  </xsl:template>
  <xsl:template match="stf:test/inputParameter/*" mode="default">
    <xsl:attribute name="default">
      <xsl:value-of select="@default"/>
    </xsl:attribute>
  </xsl:template>
  <xsl:template match="stf:test/*" mode="result">
    <xsl:apply-templates mode="result" select="boolean|string|unsignedInt|unsignedLong"/>
  </xsl:template>
  <xsl:template match="stf:test/*/*" mode="result">
    <xs:element>
      <xsl:attribute name="name">
        <xsl:copy-of select="../name"/>
      </xsl:attribute>
      <xsl:attribute name="type">
        <xsl:apply-templates mode="paraType" select="."/>
      </xsl:attribute>
    </xs:element>
  </xsl:template>
</xsl:stylesheet>
