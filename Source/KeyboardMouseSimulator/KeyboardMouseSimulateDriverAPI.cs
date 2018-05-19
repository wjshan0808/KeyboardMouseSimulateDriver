using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace KeyboardMouseSimulator
{
  /// <summary>
  /// 
  /// </summary>
  /// <returns></returns>
  public delegate ulong CheckoutDeletage();

  /// <summary>
  /// 鼠标按键
  /// </summary>
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

  /// <summary>
  /// 参数
  /// </summary>
  public struct Parameters
  {
    public int m_nPeriod;
    public int m_nDuration;
    public int m_nInterval;
    public uint m_nKeyCode;

    public uint m_nCursorPositionX;
    public uint m_nCursorPositionY;
    public MouseButtons m_nMouseButtons;
  }

  /// <summary>
  /// 模拟方式
  /// </summary>
  public enum SimulateWays
  {
    Unknow = 0x00,
    WinRing0 = 0x01,
    WinIo = 0x02,
    Event = 0x03
  }

  /// <summary>
  /// 位置坐标
  /// </summary>
  [StructLayout(LayoutKind.Sequential)]
  public struct Position
  {
    public int m_nX;
    public int m_nY;

    //[MarshalAs(UnmanagedType.LPWStr)]
    //public string Name;
  }

  /// <summary>
  /// APIs
  /// </summary>
  public class KeyboardMouseSimulateDriverAPI
  {
    private const string DriverFileName = "KeyboardMouseSimulateDriver.dll";


    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static ulong Checkout();

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static short KeyStatus(uint nKey);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static bool CursorPosition(ref Position stPosition, bool bGetOrSet);


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

    [DllImport(DriverFileName, EntryPoint = "MouseMove", CallingConvention = CallingConvention.StdCall)]
    public extern static bool MouseMove(uint nX, uint nY);

    [DllImport(DriverFileName, CallingConvention = CallingConvention.StdCall)]
    public extern static void Uninitialize();


    [DllImport("kernel32.dll")]
    public extern static IntPtr LoadLibrary(string strFileName);

    [DllImport("kernel32.dll", SetLastError = true)]
    public extern static bool FreeLibrary(IntPtr nModule);

    [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true, ExactSpelling = false)]
    public extern static IntPtr GetProcAddress(IntPtr nModule, [MarshalAs(UnmanagedType.LPStr)] string strProcName);

    public Delegate GetDelegate(IntPtr nMoudle, string strProcName, Type procType)
    {
      IntPtr ptr = GetProcAddress(nMoudle, strProcName);
      if (IntPtr.Zero != ptr)
        return Marshal.GetDelegateForFunctionPointer(ptr, procType);
      return null;
    }
  }
}
