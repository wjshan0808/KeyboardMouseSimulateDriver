#pragma once 
#ifndef __H_KEYBOARD_MOUSE_SIMULATE_DRIVER_DEFINES_H__
#define __H_KEYBOARD_MOUSE_SIMULATE_DRIVER_DEFINES_H__


#include <Windows.h>
#include <winioctl.h>


typedef UINT(WINAPI *PFN_GetSystemWow64Directory)(LPTSTR szBuffer, UINT nBufferSize);

//
#define CTL_IO_KEYBOARD_CMD     0x64    //¼üÅÌÃüÁî¶Ë¿Ú
#define CTL_IO_KEYBOARD_DATA    0x60    //¼üÅÌÊý¾Ý¶Ë¿Ú

// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.

#define FILE_DEVICE_WINIO 0x00008010

// Macro definition for defining IOCTL and FSCTL function control codes.
// Note that function codes 0-2047 are reserved for Microsoft Corporation,
// and 2048-4095 are reserved for customers.

#define WINIO_IOCTL_INDEX 0x810

// Define our own private IOCTL

#define IOCTL_WINIO_MAPPHYSTOLIN     CTL_CODE(FILE_DEVICE_WINIO,  \
	WINIO_IOCTL_INDEX,      \
	METHOD_BUFFERED,        \
	FILE_ANY_ACCESS)

#define IOCTL_WINIO_UNMAPPHYSADDR    CTL_CODE(FILE_DEVICE_WINIO,  \
	WINIO_IOCTL_INDEX + 1,  \
	METHOD_BUFFERED,        \
	FILE_ANY_ACCESS)

#define IOCTL_WINIO_ENABLEDIRECTIO   CTL_CODE(FILE_DEVICE_WINIO,  \
	WINIO_IOCTL_INDEX + 2,   \
	METHOD_BUFFERED,         \
	FILE_ANY_ACCESS)

#define IOCTL_WINIO_DISABLEDIRECTIO  CTL_CODE(FILE_DEVICE_WINIO,  \
	WINIO_IOCTL_INDEX + 3,   \
	METHOD_BUFFERED,         \
	FILE_ANY_ACCESS)

#define IOCTL_WINIO_READPORT		 CTL_CODE(FILE_DEVICE_WINIO,  \
	WINIO_IOCTL_INDEX + 4,   \
	METHOD_BUFFERED,         \
	FILE_ANY_ACCESS)

#define IOCTL_WINIO_WRITEPORT		 CTL_CODE(FILE_DEVICE_WINIO,  \
	WINIO_IOCTL_INDEX + 5,   \
	METHOD_BUFFERED,         \
	FILE_ANY_ACCESS)


#pragma pack(push, 1)
 
struct Port
{
  USHORT m_nPortAddress;      //Address
  ULONG m_nPortValue;         //Value
  UCHAR m_nPortSize;          //Size
};
 

#pragma pack(pop)

#endif // !__H_KEYBOARD_MOUSE_SIMULATE_DRIVER_DEFINES_H__
