using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace KeyboardMouseSimulator
{

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
    
  public struct Position
  {
    long nX;
    long nY;
  }

  public struct Parameters
  {
    public int m_nPeriod;
    public int m_nDuration;
    public int m_nInterval;
    public uint m_nMouseValue;
    public uint m_nKeyboardValue;
  }

  public enum KeyboardWays
  {
    WinRing0 = 1,
    WinIo = 2,
  }

  public class KeyboardMouseSimulateDriver
  {
    private const string DriverFileName = "KeyboardMouseSimulateDriver.dll";


    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static ulong Checkout();

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static short KeyStatus(uint nKey);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static void CursorPosition(ref Position stPosition, bool bGetOrSet);



    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static void EventKeyDown(uint nKey);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static void EventKeyUp(uint nKey);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static void EventMouseButton(ulong nButtons);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static void EventMouseMove(ulong nX, ulong nY);



    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static int Initialize(int nDriverType);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool KeyDown(uint nKey);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool KeyUp(uint nKey);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool MouseDown(uint nButtons);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool MouseUp(uint nButtons);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool MouseMove(ulong nX, ulong nY);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static void Uninitialize();
  }
}
