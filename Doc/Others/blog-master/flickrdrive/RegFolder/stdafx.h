// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma warning(disable:4091)
#pragma warning(disable:4838)

#include "ShFrwk.h"

#include "resource.h"

#include "RegFileSystem.h"

#include <string>
#include <list>
#include <vector>
#include <map>

extern CRegShellModule _ShellModule;


// Registry Type:      {C7CAC615-C076-442e-9B34-9BC5880F8F3C}/1 
// Registry ValueType: {C7CAC615-C076-442e-9B34-9BC5880F8F3C}/2
// Registry Value:     {C7CAC615-C076-442e-9B34-9BC5880F8F3C}/3
DEFINE_PROPERTYKEY(PKEY_RegistryType,      0xc7cac615, 0xc076, 0x442e, 0x9b, 0x34, 0x9b, 0xc5, 0x88, 0xf, 0x8f, 0x3c, 1);
DEFINE_PROPERTYKEY(PKEY_RegistryValueType, 0xc7cac615, 0xc076, 0x442e, 0x9b, 0x34, 0x9b, 0xc5, 0x88, 0xf, 0x8f, 0x3c, 2);
DEFINE_PROPERTYKEY(PKEY_RegistryValue,     0xc7cac615, 0xc076, 0x442e, 0x9b, 0x34, 0x9b, 0xc5, 0x88, 0xf, 0x8f, 0x3c, 3);
