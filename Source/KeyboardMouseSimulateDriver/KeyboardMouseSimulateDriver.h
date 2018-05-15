#pragma once 
#ifndef __H_KEYBOARD_MOUSE_SIMULATE_DRIVER_H__
#define __H_KEYBOARD_MOUSE_SIMULATE_DRIVER_H__

#ifdef KEYBOARD_MOUSE_SIMULATE_DRIVER_EXPORTS
#define KEYBOARD_MOUSE_SIMULATE_DRIVER_API __declspec(dllexport)
#else
#define KEYBOARD_MOUSE_SIMULATE_DRIVER_API __declspec(dllimport)
#endif


extern "C"
{
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API long long _stdcall Checkout();
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API int _stdcall Initialize();

  KEYBOARD_MOUSE_SIMULATE_DRIVER_API short _stdcall KeyStatus(unsigned int nKeyValue);

  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall KeyDown(unsigned int nKeyValue);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall KeyUp(unsigned int nKeyValue);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall EventKeyDown(unsigned int nKeyValue);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall EventKeyUp(unsigned int nKeyValue);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall MouseDown(unsigned int nMouseValue);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API bool _stdcall MouseUp(unsigned int nMouseValue);
  KEYBOARD_MOUSE_SIMULATE_DRIVER_API void _stdcall Uninitialize();
};

#endif // !__H_KEYBOARD_MOUSE_SIMULATE_DRIVER_H__
