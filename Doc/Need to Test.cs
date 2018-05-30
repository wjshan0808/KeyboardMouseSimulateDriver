using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ConsoleTEST
{
  class Hanoi
  {
    public const string NameDLL = "WinRing0x64.dll";

    public const int KBC_KEY_CMD = 0x64;
    public const int KBC_KEY_DATA = 0x60;

    [DllImport("user32.dll")]
    public static extern int MapVirtualKey(uint Ucode, uint uMapType);

    [DllImport(NameDLL)]
    public static extern bool InitializeOls();
    [DllImport(NameDLL)]
    public static extern void DeinitializeOls();
    [DllImport(NameDLL)]
    public static extern uint GetDllStatus();


    [DllImport(NameDLL)]
    public static extern void WriteIoPortWord(ushort nport, ushort nvalue);

    [DllImport(NameDLL)]
    public static extern bool WriteIoPortWordEx(ushort nport, ushort nvalue);

    [DllImport(NameDLL)]
    public static extern bool ReadIoPortWordEx(ushort nport, ref ushort nvalue);


    [DllImport(NameDLL)]
    public static extern byte ReadIoPortByte(ushort nport, byte nvalue);
    [DllImport(NameDLL)]
    public static extern bool ReadIoPortByteEx(ushort nport, ref byte nvalue);


    [DllImport(NameDLL)]
    public static extern void WriteIoPortByte(ushort nport, byte nvalue);



    [DllImport(NameDLL)]
    public static extern bool GetPortVal(IntPtr wPortAddr, out int pdwPortVal, byte bSize);
    [DllImport(NameDLL)]
    public static extern bool SetPortVal(uint wPortAddr, IntPtr dwPortVal, byte bSize);
    [DllImport(NameDLL)]
    public static extern byte MapPhysToLin(byte pbPhysAddr, uint dwPhysSize, IntPtr PhysicalMemoryHandle);
    [DllImport(NameDLL)]
    public static extern bool UnmapPhysicalMemory(IntPtr PhysicalMemoryHandle, byte pbLinAddr);
    [DllImport(NameDLL)]
    public static extern bool GetPhysLong(IntPtr pbPhysAddr, byte pdwPhysVal);
    [DllImport(NameDLL)]
    public static extern bool SetPhysLong(IntPtr pbPhysAddr, byte dwPhysVal);


    public static void KBCWait4IBE()
    {
      byte dwVal = 0;
      do
      {
        bool flag = ReadIoPortByteEx(0x64, ref dwVal);
      }
      while ((dwVal & 0x2) > 0);
    }

    public static void KeyDown(uint ch)
    {
      int btScancode = MapVirtualKey(ch, 0);
      KBCWait4IBE();
      WriteIoPortByte(0x64, 0xd2);
      KBCWait4IBE();
      WriteIoPortByte(0x60, (byte)btScancode);
    }

    public static void KeyUp(uint ch)
    {
      int btScancode = MapVirtualKey(ch, 0);
      KBCWait4IBE();
      WriteIoPortByte(0x64, 0xd2);
      KBCWait4IBE();
      WriteIoPortByte(0x60, (byte)(btScancode | 0x80));
    }
    public static void OnKey(uint vk_in)
    {
      System.Threading.Thread.Sleep(5);
      //KBCWait4IBE();  
      WriteIoPortByte(0X64, 0xD2);
      System.Threading.Thread.Sleep(5);
      //KBCWait4IBE();  
      WriteIoPortByte(0X60, 0xE2);
      System.Threading.Thread.Sleep(5);
      //KBCWait4IBE();  
      WriteIoPortByte(0X64, 0xD2);
      System.Threading.Thread.Sleep(5);
      //KBCWait4IBE(); 
      WriteIoPortByte(0X60, (byte)MapVirtualKey(vk_in, 0));
      System.Threading.Thread.Sleep(5);
      //KBCWait4IBE();  
      WriteIoPortByte(0X64, 0xD2);
      System.Threading.Thread.Sleep(5);
      //KBCWait4IBE();  
      WriteIoPortByte(0X60, 0xE0);
      System.Threading.Thread.Sleep(5);
      //KBCWait4IBE();  
      WriteIoPortByte(0X64, 0xD2);
      System.Threading.Thread.Sleep(5);
      //KBCWait4IBE(); 
      WriteIoPortByte(0X60, (byte)(MapVirtualKey(vk_in, 0) | 0x80));
    }

    public static void OnMouse()
    {
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X64, 0xD3);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X60, 0x09);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X64, 0xD3);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X60, 0x00);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X64, 0xD3);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X60, 0x00);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X64, 0xD3);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X60, 0x00);
      //松开 
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X64, 0xD3);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X60, 0x08);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X64, 0xD3);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X60, 0x00);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X64, 0xD3);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X60, 0x00);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X64, 0xD3);
      System.Threading.Thread.Sleep(5);
      WriteIoPortByte(0X60, 0x00);

    }

    public static void MyMouseDown(int vKeyCoad)
    {
      //int btScancode = 0;
      //btScancode = MapVirtualKey((byte)vKeyCoad, 0);
      //KBCWait4IBE(); // 'wait for buffer gets empty  
      //SetPortVal(KBC_KEY_CMD, (IntPtr)0xD3, 1);// 'send write command  
      //KBCWait4IBE();
      //SetPortVal(KBC_KEY_DATA, (IntPtr)(btScancode | 0x80), 1);// 'write in io  


      SetPortVal(0x64, (IntPtr)0xd3, 1);//鼠标的0xd2对应键盘的0xd3<br>
      SetPortVal(0x60, (IntPtr)0x0A, 1);//0x09表示LBUTTONDOWN<br>
      SetPortVal(0x60, (IntPtr)0x00, 1);//表示x方向移动是0，移动的距离与诸多因素有关<br>
      SetPortVal(0x60, (IntPtr)0x00, 1);//y方向<br>
      SetPortVal(0x60, (IntPtr)0x00, 1);//z方向，应该是鼠标滚轮的滚动数目，对于无滚轮的鼠标没有这一项。</p> 
    }

    public static void MyMouseUp(int vKeyCoad)
    {
      //int btScancode = 0;
      //btScancode = MapVirtualKey((byte)vKeyCoad, 0);
      //KBCWait4IBE(); // 'wait for buffer gets empty  
      //SetPortVal(KBC_KEY_CMD, (IntPtr)0xD3, 1); //'send write command  
      //KBCWait4IBE();
      //SetPortVal(KBC_KEY_DATA, (IntPtr)(btScancode | 0x80), 1);// 'write in io  

      SetPortVal(0x64, (IntPtr)0xd3, 1);//鼠标的0xd2对应键盘的0xd3<br>
      SetPortVal(0x60, (IntPtr)0x08, 1);//0x09表示LBUTTONDOWN<br>
      SetPortVal(0x60, (IntPtr)0x00, 1);//表示x方向移动是0，移动的距离与诸多因素有关<br>
      SetPortVal(0x60, (IntPtr)0x00, 1);//y方向<br>  

    }
    public static void Move(long x, long y, long z)
    {
      KBCWait4IBE();
      SetPortVal(0x64, (IntPtr)0xd3, 1);
      KBCWait4IBE();
      Thread.Sleep(5);
      SetPortVal(0x60, (IntPtr)0x28, 1);
      KBCWait4IBE();
      Thread.Sleep(5);
      SetPortVal(0x64, (IntPtr)0xd3, 1);
      KBCWait4IBE();
      Thread.Sleep(5);
      SetPortVal(0x60, (IntPtr)x, 1);
      KBCWait4IBE();
      Thread.Sleep(5);
      SetPortVal(0x64, (IntPtr)0xd3, 1);
      KBCWait4IBE();
      Thread.Sleep(5);
      SetPortVal(0x60, (IntPtr)y, 1);
      KBCWait4IBE();
      Thread.Sleep(5);
      SetPortVal(0x64, (IntPtr)0xd3, 1);
      KBCWait4IBE();
      Thread.Sleep(5);
      SetPortVal(0x60, (IntPtr)z, 1);
    }

    public enum Key
    {
      VK_LBUTTON = 1,
      VK_RBUTTON = 2,
      VK_CANCEL = 3,
      VK_MBUTTON = 4,
      VK_BACK = 8,
      VK_TAB = 9,
      VK_CLEAR = 12,
      VK_RETURN = 13,
      VK_SHIFT = 16,
      VK_CONTROL = 17,
      VK_MENU = 18,
      VK_PAUSE = 19,
      VK_CAPITAL = 20,
      VK_ESCAPE = 27,
      VK_SPACE = 32,
      VK_PRIOR = 33,
      VK_NEXT = 34,
      VK_END = 35,
      VK_HOME = 36,
      VK_LEFT = 37,
      VK_UP = 38,
      VK_RIGHT = 39,
      VK_DOWN = 40,
      VK_SELECT = 41,
      VK_PRINT = 42,
      VK_EXECUTE = 43,
      VK_SNAPSHOT = 44,
      VK_INSERT = 45,
      VK_DELETE = 46,
      VK_HELP = 47,
      VK_NUM0 = 48, //0  
      VK_NUM1 = 49, //1  
      VK_NUM2 = 50, //2  
      VK_NUM3 = 51, //3  
      VK_NUM4 = 52, //4  
      VK_NUM5 = 53, //5  
      VK_NUM6 = 54, //6  
      VK_NUM7 = 55, //7  
      VK_NUM8 = 56, //8  
      VK_NUM9 = 57, //9  
      VK_A = 65, //A  
      VK_B = 66, //B  
      VK_C = 67, //C  
      VK_D = 68, //D  
      VK_E = 69, //E  
      VK_F = 70, //F  
      VK_G = 71, //G  
      VK_H = 72, //H  
      VK_I = 73, //I  
      VK_J = 74, //J  
      VK_K = 75, //K  
      VK_L = 76, //L  
      VK_M = 77, //M  
      VK_N = 78, //N  
      VK_O = 79, //O  
      VK_P = 80, //P  
      VK_Q = 81, //Q  
      VK_R = 82, //R  
      VK_S = 83, //S  
      VK_T = 84, //T  
      VK_U = 85, //U  
      VK_V = 86, //V  
      VK_W = 87, //W  
      VK_X = 88, //X  
      VK_Y = 89, //Y  
      VK_Z = 90, //Z  
      VK_NUMPAD0 = 96, //0  
      VK_NUMPAD1 = 97, //1  
      VK_NUMPAD2 = 98, //2  
      VK_NUMPAD3 = 99, //3  
      VK_NUMPAD4 = 100, //4  
      VK_NUMPAD5 = 101, //5  
      VK_NUMPAD6 = 102, //6  
      VK_NUMPAD7 = 103, //7  
      VK_NUMPAD8 = 104, //8  
      VK_NUMPAD9 = 105, //9  
      VK_NULTIPLY = 106,
      VK_ADD = 107,
      VK_SEPARATOR = 108,
      VK_SUBTRACT = 109,
      VK_DECIMAL = 110,
      VK_DIVIDE = 111,
      VK_F1 = 112,
      VK_F2 = 113,
      VK_F3 = 114,
      VK_F4 = 115,
      VK_F5 = 116,
      VK_F6 = 117,
      VK_F7 = 118,
      VK_F8 = 119,
      VK_F9 = 120,
      VK_F10 = 121,
      VK_F11 = 122,
      VK_F12 = 123,
      VK_NUMLOCK = 144,
      VK_SCROLL = 145,
      middleup = 0x0040,
      xdown = 0x0080,
      xup = 0x0100,
      wheel = 0x0800,
      virtualdesk = 0x4000,
      absolute = 0x8000
    }
  }
  class Program
  {
    static void Main(string[] args)
    {
      System.Threading.Thread.Sleep(3000);
      try
      {

        Console.WriteLine("InitializeOls : " + Hanoi.InitializeOls());
        Console.WriteLine("GetDllStatus : " + Hanoi.GetDllStatus());


        Hanoi.OnKey(66);
        System.Threading.Thread.Sleep(3000);
        Hanoi.OnMouse();

        //Console.WriteLine("MykeyDown : ");
        //Hanoi.KeyDown(65);
        //Console.WriteLine("MykeyUp : ");
        //Hanoi.KeyUp(65);


        Console.WriteLine("DeinitializeOls : ");
        Hanoi.DeinitializeOls();

        Console.WriteLine("Done...");
      }
      catch (Exception ex)
      {

        Console.WriteLine("Error : " + ex.Message);
      }

      //System.Threading.Thread.Sleep(3000);
      //Hanoi.MyMouseDown(0x0008);
      //Hanoi.MyMouseUp(0x0008);

      //Console.WriteLine("  OK ");
      //System.Threading.Thread.Sleep(2000);
      //Console.WriteLine("  Next... ");
      //Hanoi.Move(100, 200, 0);

      Console.ReadKey();
    }
  }
}
