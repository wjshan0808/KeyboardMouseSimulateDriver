#pragma once 
#ifndef __H_KEYBOARD_MOUSE_SIMULATE_DRIVER_H__
#define __H_KEYBOARD_MOUSE_SIMULATE_DRIVER_H__

#ifdef KEYBOARD_MOUSE_SIMULATE_DRIVER_EXPORTS
#define KEYBOARD_MOUSE_SIMULATE_DRIVER_API __declspec(dllexport)
#else
#define KEYBOARD_MOUSE_SIMULATE_DRIVER_API __declspec(dllimport)
#endif

//#pragma pack(push, 1)
//
//struct Position
//{
//  int m_nX;
//  int m_nY;
//};
//
//#pragma pack(pop)

extern "C"
{
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API long long _stdcall Checkout();
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API short _stdcall KeyStatus(unsigned int nKey);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall CursorPosition(POINT &stPosition, bool bGetOrSet);
  
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API int _stdcall Initialize(int nDriverType);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall MouseMove(unsigned int nX, unsigned int nY);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall MouseDown(unsigned int nButtons);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall MouseUp(unsigned int nButtons);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall KeyDown(unsigned int nKey);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall KeyUp(unsigned int nKey);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API void _stdcall Uninitialize();
};

#endif // !__H_KEYBOARD_MOUSE_SIMULATE_DRIVER_H__
