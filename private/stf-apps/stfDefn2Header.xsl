<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:stf="http://glacier.lbl.gov/icecube/daq/stf"
  xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="text"/>
  <xsl:variable name="nl">
<xsl:text>
</xsl:text>
  </xsl:variable>
  <xsl:template match="/">
    <xsl:apply-templates select="stf:test"/>
  </xsl:template>
  <xsl:template match="stf:test">
  <xsl:variable name="testName" select="name"/>
#include "hal/DOM_MB_fpga.h"

static STF_PARAM <xsl:copy-of select="$testName"/>_params[] = {
    <xsl:apply-templates mode="PARAM" select="inputParameter"/>
    <xsl:apply-templates mode="PARAM" select="outputParameter"/>
   { .name = "" }
};

#include "stf-apps/<xsl:copy-of select="$testName"/>Signatures.h"

static BOOLEAN <xsl:copy-of select="$testName"/>EntryLocal(STF_DESCRIPTOR *d) {
   return <xsl:copy-of select="$testName"/>Entry(d,
<xsl:apply-templates mode="EntryLocal" select="inputParameter"/>
<xsl:apply-templates mode="EntryLocal" select="outputParameter"/>
}

static STF_DESCRIPTOR <xsl:copy-of select="$testName"/>_descriptor = {
  .name = "<xsl:copy-of select="$testName"/>",
  .desc = "<xsl:value-of select="description"/>",
  .majorVersion = <xsl:value-of select="version/@major"/>,
  .minorVersion = <xsl:value-of select="version/@minor"/>,
  .testRunnable = 0,
  .fpgaDependencies = <xsl:choose>
      <xsl:when test='((0=count(fpgaDependencies)) or (0=count(fpgaDependencies/*)))'>0,</xsl:when>
      <xsl:otherwise><xsl:apply-templates select="fpgaDependencies/*"/></xsl:otherwise>
  </xsl:choose>
  .nParams = <xsl:value-of select="count(inputParameter) + count(outputParameter)"/>,
  .params = <xsl:copy-of select="$testName"/>_params,
  .initPt = <xsl:copy-of select="$testName"/>Init,
  .entryPt = <xsl:copy-of select="$testName"/>EntryLocal,
  .isInit = 0
};
<xsl:copy-of select="$nl"/>
  </xsl:template>
  <xsl:template match="stf:test/*/*" mode="PARAM">
   {
     .name = "<xsl:copy-of select="../name"/>",
     .class = "<xsl:apply-templates mode="class" select="."/>",
     .type = "<xsl:value-of select="local-name()"/>",
     .maxValue = <xsl:apply-templates mode="max" select="."/>,
     .minValue = <xsl:apply-templates mode="min" select="."/>,
     .defValue = <xsl:apply-templates mode="default" select="."/>,
     .arraySize = "<xsl:apply-templates mode="arraySize" select="."/>",
     .value = {
       .<xsl:apply-templates mode="value" select="."/>Value = <xsl:apply-templates mode="initialValue" select="."/>
     },
     .arrayLength = <xsl:apply-templates mode="arraySize" select="."/>
   },
  </xsl:template>
  <xsl:template match="stf:test/*" mode="PARAM">
    <xsl:apply-templates mode="PARAM" select="boolean|string|unsignedInt|unsignedLong|unsignedIntArray"/>
  </xsl:template>
  <xsl:template match="stf:test/*/*" mode="max">NULL</xsl:template>
  <xsl:template match="stf:test/inputParameter/unsignedInt|stf:test/inputParameter/unsignedLong" mode="max">"<xsl:value-of select="@maxValue"/>"</xsl:template>
  <xsl:template match="stf:test/*/*" mode="min">NULL</xsl:template>
  <xsl:template match="stf:test/inputParameter/unsignedInt|stf:test/inputParameter/unsignedLong" mode="min">"<xsl:value-of select="@minValue"/>"</xsl:template>
  <xsl:template match="stf:test/outputParameter/*" mode="default">NULL</xsl:template>
  <xsl:template match="stf:test/inputParameter/boolean|stf:test/inputParameter/string|stf:test/inputParameter/unsignedInt|stf:test/inputParameter/unsignedLong|unsignedIntArray" mode="default">"<xsl:value-of select="@default"/>"</xsl:template>
  <xsl:template match="stf:test/*/*" mode="arraySize">1</xsl:template>
  <xsl:template match="stf:test/outputParameter/unsignedIntArray" mode="arraySize"><xsl:value-of select="@length"/></xsl:template>
  <xsl:template match="stf:test/inputParameter/*" mode="class">input</xsl:template>
  <xsl:template match="stf:test/outputParameter/*" mode="class">output</xsl:template>
  <xsl:template match="stf:test/*/boolean" mode="value">bool</xsl:template>
  <xsl:template match="stf:test/*/string" mode="value">char</xsl:template>
  <xsl:template match="stf:test/*/unsignedInt" mode="value">int</xsl:template>
  <xsl:template match="stf:test/*/unsignedLong" mode="value">long</xsl:template>
  <xsl:template match="stf:test/*/unsignedIntArray" mode="value">intArray</xsl:template>
  <xsl:template match="stf:test/*/*" mode="initialValue">0</xsl:template>
 <xsl:template match="stf:test/outputParameter/unsignedIntArray" mode="initialValue">NULL</xsl:template>
  <xsl:template match="stf:test/*/string" mode="initialValue">""</xsl:template>
  <xsl:template match="stf:test/inputParameter/*" mode="initialValue"><xsl:value-of select="@default"/></xsl:template>
  <xsl:template match="stf:test/inputParameter/boolean[@default='false']|stf:test/inputParameter/boolean[@default='False']" mode="initialValue">0</xsl:template>
  <xsl:template match="stf:test/inputParameter/boolean[@default='true']|stf:test/inputParameter/boolean[@default='True']" mode="initialValue">1</xsl:template>
  <xsl:template match="stf:test/inputParameter/string" mode="initialValue">"<xsl:value-of select="@default"/>"</xsl:template>
  <xsl:template match="stf:test/*/*" mode="Entry">
    <xsl:text>                   </xsl:text>
    <xsl:apply-templates mode="signature" select="."/>
    <xsl:apply-templates mode="entryModifier" select="."/>
    <xsl:text> </xsl:text>
    <xsl:copy-of select="../name"/>
  </xsl:template>
  <xsl:template match="stf:test/*" mode="Entry">
    <xsl:apply-templates mode="Entry" select="boolean|string|unsignedInt|unsignedLong|unsignedIntArray"/>
    <xsl:choose>
      <xsl:when test='((0=count(../outputParameter))or("outputParameter"=local-name()))and(last()=position())'>);</xsl:when>
      <xsl:otherwise>,</xsl:otherwise>
    </xsl:choose>
    <xsl:copy-of select="$nl"/>
  </xsl:template>
  <xsl:template match="stf:test/fpgaDependencies/*">DOM_HAL_FPGA_COMP_<xsl:copy-of select="local-name()"/>
    <xsl:choose>
      <xsl:when test='(last()=position())'>,</xsl:when>
      <xsl:otherwise> |<xsl:copy-of select="$nl"/><xsl:text>        </xsl:text></xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="stf:test/*/boolean" mode="signature">BOOLEAN</xsl:template>
  <xsl:template match="stf:test/inputParameter/string" mode="signature">const char*</xsl:template>
  <xsl:template match="stf:test/outputParameter/string" mode="signature">char*</xsl:template>
  <xsl:template match="stf:test/*/unsignedInt" mode="signature">unsigned int</xsl:template>
  <xsl:template match="stf:test/*/unsignedLong" mode="signature">unsigned long</xsl:template>
  <xsl:template match="stf:test/outputParameter/unsignedIntArray" mode="signature">unsigned int</xsl:template>
  <xsl:template match="stf:test/outputParameter/*" mode="entryModifier">* </xsl:template>
  <xsl:template match="stf:test/*/*" mode="EntryLocal">
    <xsl:text>                   </xsl:text>
    <xsl:apply-templates mode="entryLocalModifier" select="."/>
    <xsl:text>getParamByName(d, "</xsl:text>
    <xsl:copy-of select="../name"/><xsl:text>")->value.</xsl:text>
    <xsl:apply-templates mode="value" select="."/>
    <xsl:text>Value</xsl:text>
  </xsl:template>
  <xsl:template match="stf:test/*" mode="EntryLocal">
    <xsl:apply-templates mode="EntryLocal" select="boolean|string|unsignedInt|unsignedLong|unsignedIntArray"/>
    <xsl:choose>
      <xsl:when test='((0=count(../outputParameter))or("outputParameter"=local-name()))and(last()=position())'>);</xsl:when>
      <xsl:otherwise>,</xsl:otherwise>
    </xsl:choose>
    <xsl:copy-of select="$nl"/>
  </xsl:template>
  <xsl:template match="stf:test/outputParameter/*" mode="entryLocalModifier">&amp;</xsl:template>
  <xsl:template match="stf:test/outputParameter/unsignedIntArray" mode="entryLocalModifier"></xsl:template>
</xsl:stylesheet>
