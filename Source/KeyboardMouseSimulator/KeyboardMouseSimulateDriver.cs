using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace KeyboardMouseSimulator
{
  public class KeyboardMouseSimulateDriver
  {
    private const string DriverFileName = "KeyboardMouseSimulateDriver.dll";


    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static ulong Checkout();

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static int Initialize();


    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static short KeyStatus(uint nKeyValue);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool KeyDown(uint nKeyValue);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool KeyUp(uint nMouseValue);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool EventKeyDown(uint nKeyValue);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool EventKeyUp(uint nMouseValue);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool MouseDown(uint nKeyValue);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool MouseUp(uint nMouseValue);


    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static void Uninitialize();
  }

  public enum MouseButtons
  {
    Move = 0x0001,
    LeftDown = 0x0002,
    LeftUp = 0x0004,
    RightDown = 0x0008,
    RightUp = 0x0010,
    MiddleDown = 0x0020,
    MiddleUp = 0x0040,
    XDown = 0x0080,
    XUp = 0x0100,
    Wheel = 0x0800,
    VirtualDesk = 0x4000,
    Absolute = 0x8000
  }
}
