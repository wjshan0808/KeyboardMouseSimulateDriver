<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xml:space="preserve">
    <xsl:output method="xml" indent="yes" />
    <xsl:param name="MODULE" />
    <xsl:template match="/">
        
<schema schemaVersion="1.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://schemas.microsoft.com/windows/2006/propertydescription">
    <propertyDescriptionList publisher="viksoe.dk" product="RegFolder">

        <propertyDescription name="Windows.Registry.Type" formatID="{'{C7CAC615-C076-442E-9B34-9BC5880F8F3C}'}" propID="1">
            <searchInfo inInvertedIndex="false" isColumn="false" />
            <typeInfo type="UInt32" canStackBy="true" groupingRange="Discrete" multipleValues="false" aggregationType="Union" isViewable="true" isQueryable="true" />
            <labelInfo label="@{$MODULE},-4002" />
            <displayInfo defaultColumnWidth="18" arrangementMode="Stacked" displayType="Enumerated">
                <enumeratedList>
                    <enum value="0" text="@{$MODULE},-4003" />
                    <enum value="1" text="@{$MODULE},-4004" />
                </enumeratedList>
                <drawControl control="StaticText" />
                <editControl control="Text" />
            </displayInfo>
        </propertyDescription>

        <propertyDescription name="Windows.Registry.ValueType" formatID="{'{C7CAC615-C076-442E-9B34-9BC5880F8F3C}'}" propID="2">
            <searchInfo inInvertedIndex="false" isColumn="true" isColumnSparse="true" columnIndexType="OnDisk" />
            <typeInfo type="UInt32" canStackBy="true" groupingRange="Discrete" multipleValues="false" aggregationType="Union" isViewable="true" isQueryable="true" />
            <labelInfo label="@{$MODULE},-4000" sortDescription="AToZ" />
            <displayInfo defaultColumnWidth="20" arrangementMode="Stacked" displayType="Enumerated">
                <enumeratedList>
                    <enum value="0" text="???" />
                    <enum value="1" text="REG_SZ" />
                    <enum value="2" text="REG_EXPAND_SZ" />
                    <enum value="3" text="REG_BINARY" />
                    <enum value="4" text="REG_DWORD" />
                    <enum value="5" text="REG_DWORD_BIG_ENDIAN" />
                    <enum value="6" text="REG_LINK" />
                    <enum value="7" text="REG_MULTI_SZ" />
                    <enum value="8" text="REG_RESOURCE_LIST" />
                    <enum value="11" text="REG_QWORD" />
                </enumeratedList>
                <drawControl control="StaticText" />
                <editControl control="Text" />
            </displayInfo>
        </propertyDescription>

        <propertyDescription name="Windows.Registry.Value" formatID="{'{C7CAC615-C076-442E-9B34-9BC5880F8F3C}'}" propID="3">
            <searchInfo inInvertedIndex="false" isColumn="true" />
            <typeInfo type="Any" canStackBy="false" multipleValues="false" isViewable="true" isQueryable="true" />
            <labelInfo label="@{$MODULE},-4001" />
            <displayInfo defaultColumnWidth="40" arrangementMode="Stacked" />
        </propertyDescription>

    </propertyDescriptionList>
</schema>
        
    </xsl:template>
</xsl:stylesheet>
