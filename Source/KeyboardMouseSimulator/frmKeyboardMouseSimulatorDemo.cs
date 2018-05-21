using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace KeyboardMouseSimulator
{
  public partial class frmKeyboardMouseSimulatorDemo : Form
  {
    private System.Threading.Thread m_trdKeyboardInterval = null;
    private volatile bool m_bChecking = false;
    private volatile bool m_bInitialized = false;
    private SimulateWays m_nSimulateWay = SimulateWays.Unknow;

    public frmKeyboardMouseSimulatorDemo()
    {
      InitializeComponent();
    }


    private void frmGlobalKeyboardSet_Load(object sender, EventArgs e)
    {
      m_bChecking = true;

      System.Threading.ThreadPool.QueueUserWorkItem((ojb) =>
      {
        Position cp = new Position();
        while (m_bChecking)
        {
          ShowCheckout(KeyboardMouseSimulateDriverAPI.Checkout());

          KeyboardMouseSimulateDriverAPI.CursorPosition(ref cp, true);
          ShowCursorPosition(cp.m_nX, cp.m_nY);

          System.Threading.Thread.Sleep(50);
        }
      });
    }

    private void frmGlobalKeyboardSet_FormClosing(object sender, FormClosingEventArgs e)
    {
      m_bChecking = false;

      if (m_trdKeyboardInterval != null)
      {
        m_trdKeyboardInterval.Abort();
        m_trdKeyboardInterval.Join();
      }

      Reset();
    }


    private void btnMouseOperate_Click(object sender, EventArgs e)
    {
      if (SimulateWays.Unknow == m_nSimulateWay)
      {
        ShowInfoBoard("请右键菜单选择模拟方式...");
        return;
      }

      ButtonControl(btnMouseOperate, false, "Mouse\r\nOperating ...");

      if (m_trdKeyboardInterval != null)
      {
        try
        {
          m_trdKeyboardInterval.Abort();
          m_trdKeyboardInterval.Join();
        }
        catch (Exception ex)
        {
          MessageBox.Show(ex.Message);
        }
      }

      Parameters stParameter = new Parameters();
      stParameter.m_nCursorPositionX = (uint)nudCursorPositionX.Value;
      stParameter.m_nCursorPositionY = (uint)nudCursorPositionY.Value;

      if (rdobtnMouseLeft.Checked)
        stParameter.m_nMouseButtons = KeyboardMouseSimulator.MouseButtons.LeftDown;
      else if (rdobtnMouseRight.Checked)
        stParameter.m_nMouseButtons = KeyboardMouseSimulator.MouseButtons.RightDown;
      else
        stParameter.m_nMouseButtons = KeyboardMouseSimulator.MouseButtons.Move;

      m_trdKeyboardInterval = new System.Threading.Thread(MouseOperate);
      m_trdKeyboardInterval.IsBackground = false;
      m_trdKeyboardInterval.Priority = System.Threading.ThreadPriority.AboveNormal;
      m_trdKeyboardInterval.Start(stParameter);
    }

    private void MouseOperate(object obj)
    {
      if (!m_bInitialized)
      {
        int nReturn = KeyboardMouseSimulateDriverAPI.Initialize((int)m_nSimulateWay);
        m_bInitialized = (0 == nReturn);
        ShowInfoBoard("Initialize", nReturn, m_bInitialized);
        System.Threading.Thread.Sleep(1000);
      }

      ShowInfoBoard("Ready...");
      System.Threading.Thread.Sleep(1000);
      ShowInfoBoard("Go ! ! !");
      System.Threading.Thread.Sleep(1000);

      if (m_bInitialized)
      {
        bool bResult = false;
        Parameters stParameter = (Parameters)obj;

        if (KeyboardMouseSimulator.MouseButtons.LeftDown == (KeyboardMouseSimulator.MouseButtons.LeftDown & stParameter.m_nMouseButtons))
        {
          bResult = KeyboardMouseSimulateDriverAPI.MouseDown((uint)KeyboardMouseSimulator.MouseButtons.LeftDown);
          bResult &= KeyboardMouseSimulateDriverAPI.MouseUp((uint)KeyboardMouseSimulator.MouseButtons.LeftUp);

          ShowInfoBoard(KeyboardMouseSimulator.MouseButtons.LeftDown, bResult);
        }
        else if (KeyboardMouseSimulator.MouseButtons.RightDown == (KeyboardMouseSimulator.MouseButtons.RightDown & stParameter.m_nMouseButtons))
        {
          bResult = KeyboardMouseSimulateDriverAPI.MouseDown((uint)KeyboardMouseSimulator.MouseButtons.RightDown);
          bResult &= KeyboardMouseSimulateDriverAPI.MouseUp((uint)KeyboardMouseSimulator.MouseButtons.RightUp);

          ShowInfoBoard(KeyboardMouseSimulator.MouseButtons.RightDown, bResult);
        }
        else //Move Checked
        {
          bResult = KeyboardMouseSimulateDriverAPI.MouseMove(stParameter.m_nCursorPositionX, stParameter.m_nCursorPositionY);

          ShowInfoBoard(KeyboardMouseSimulator.MouseButtons.Move, bResult);
        }
      }

      ButtonControl(btnMouseOperate, true, "Mouse\r\nOperate");
    }


    private void btnSimulate_Click(object sender, EventArgs e)
    {
      if (SimulateWays.Unknow == m_nSimulateWay)
      {
        ShowInfoBoard("请右键菜单选择模拟方式...");
        return;
      }

      ButtonControl(btnSimulate, false, "Wroking ...");

      if (m_trdKeyboardInterval != null)
      {
        try
        {
          m_trdKeyboardInterval.Abort();
          m_trdKeyboardInterval.Join();
        }
        catch (Exception ex)
        {
          MessageBox.Show(ex.Message);
        }
      }

      Parameters stParameter = new Parameters();
      stParameter.m_nPeriod = (int)nudPeriod.Value;
      stParameter.m_nDuration = (int)nudDurationS.Value;
      stParameter.m_nInterval = (int)nudIntervalMS.Value;

      if (rdobtnKeyO.Checked)
        stParameter.m_nKeyCode = (uint)Keys.O;
      else if (rdobtnKeyG.Checked)
        stParameter.m_nKeyCode = (uint)Keys.G;
      //else
      //  stParameter.m_nKeyCode = 0;

      m_trdKeyboardInterval = new System.Threading.Thread(Simulate);
      m_trdKeyboardInterval.IsBackground = false;
      m_trdKeyboardInterval.Priority = System.Threading.ThreadPriority.AboveNormal;
      m_trdKeyboardInterval.Start(stParameter);
    }

    private void Simulate(object obj)
    {
      if (!m_bInitialized)
      {
        int nReturn = KeyboardMouseSimulateDriverAPI.Initialize((int)m_nSimulateWay);
        m_bInitialized = (0 == nReturn);
        ShowInfoBoard("Initialize", nReturn, m_bInitialized);
        System.Threading.Thread.Sleep(1000);
      }

      ShowInfoBoard("Ready...");
      System.Threading.Thread.Sleep(1000);
      ShowInfoBoard("Go ! ! !");
      System.Threading.Thread.Sleep(1000);

      if (m_bInitialized)
      {
        Parameters stParameter = (Parameters)obj;
        for (int i = 0; i < stParameter.m_nPeriod; i++)
        {
          int nTimes = 1;
          DateTime dtStart = DateTime.Now;
          while (stParameter.m_nDuration > (DateTime.Now - dtStart).TotalSeconds)
          {
            ShowInfoBoard(nTimes, KeyboardMouseSimulateDriverAPI.KeyDown(stParameter.m_nKeyCode));
            ShowInfoBoard(nTimes++, KeyboardMouseSimulateDriverAPI.KeyUp(stParameter.m_nKeyCode));

            System.Threading.Thread.Sleep(stParameter.m_nInterval);
          }
        }
      }

      ButtonControl(btnSimulate, true, "Simulate");
    }


    private void tsmiSimulateWayEvent_Click(object sender, EventArgs e)
    {
      ToolStripMenuItem tsmi = sender as ToolStripMenuItem;
      MenuControl(tsmi);
      m_nSimulateWay = SimulateWays.Event;
    }

    private void tsmiSimulateWayWinIO_Click(object sender, EventArgs e)
    {
      ToolStripMenuItem tsmi = sender as ToolStripMenuItem;
      MenuControl(tsmi);
      m_nSimulateWay = SimulateWays.WinIo;
    }

    private void tsmiSimulateWayWinRing0_Click(object sender, EventArgs e)
    {
      ToolStripMenuItem tsmi = sender as ToolStripMenuItem;
      MenuControl(tsmi);
      m_nSimulateWay = SimulateWays.WinRing0;
    }


    private void Reset()
    {
      KeyboardMouseSimulateDriverAPI.Uninitialize();
      Application.DoEvents();

      m_bInitialized = false;
    }


    private void Test_SetCursorPosition()
    {
      Position cp = new Position();
      cp.m_nX = (int)nudCursorPositionX.Value;
      cp.m_nY = (int)nudCursorPositionY.Value;
      KeyboardMouseSimulateDriverAPI.CursorPosition(ref cp, false);
    }


    private void ShowCursorPosition(int nX, int nY)
    {
      if (lblCurrentCursorPositionXY.InvokeRequired)
        lblCurrentCursorPositionXY.Invoke(new Action<int, int>(ShowCursorPosition), nX, nY);
      else
        lblCurrentCursorPositionXY.Text = string.Format("X:{0}\r\n\r\nY:{1}", nX, nY);
    }

    private void ShowCheckout(ulong nCheckout)
    {
      string strCheckout =
        new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Unspecified).AddSeconds(nCheckout).ToString("yyyy-MM-dd HH:mm:ss");
      ShowCheckout(strCheckout);
    }

    private void ShowCheckout(string strCheckout)
    {
      if (lblCheckout.InvokeRequired)
        lblCheckout.Invoke(new Action<string>(ShowCheckout), strCheckout);
      else
        lblCheckout.Text = strCheckout;
    }


    private void ShowInfoBoard(int nTimes, bool bResult)
    {
      ShowInfoBoard(string.Format("Times {0}", nTimes), bResult);
    }

    private void ShowInfoBoard(string strInfo, int nValue, bool bResult)
    {
      string strResult = string.Format("{0} Code {1}", strInfo, nValue);

      ShowInfoBoard(strResult, bResult);
    }

    private void ShowInfoBoard(KeyboardMouseSimulator.MouseButtons nMouseButton, bool bResult)
    {
      string strResult = string.Empty;

      switch (nMouseButton)
      {
        case KeyboardMouseSimulator.MouseButtons.LeftDown:
        case KeyboardMouseSimulator.MouseButtons.LeftUp:
          strResult = string.Format("Simulate Mouse '{0}' Button", "Left");
          break;
        case KeyboardMouseSimulator.MouseButtons.RightDown:
        case KeyboardMouseSimulator.MouseButtons.RightUp:
          strResult = string.Format("Simulate Mouse '{0}' Button", "Right");
          break;
        case KeyboardMouseSimulator.MouseButtons.MiddleDown:
        case KeyboardMouseSimulator.MouseButtons.MiddleUp:
          strResult = string.Format("Simulate Mouse '{0}' Button", "Middle");
          break;
        case KeyboardMouseSimulator.MouseButtons.XDown:
        case KeyboardMouseSimulator.MouseButtons.XUp:
          strResult = string.Format("Simulate Mouse '{0}' Button", "X");
          break;
        case KeyboardMouseSimulator.MouseButtons.Wheel:
          strResult = string.Format("Simulate Mouse '{0}'", "Wheel");
          break;
        case KeyboardMouseSimulator.MouseButtons.Move:
          strResult = string.Format("Simulate Mouse '{0}'", "Move");
          break;
      }

      ShowInfoBoard(strResult, bResult);
    }

    private void ShowInfoBoard(string strInfo)
    {
      if (lblInfoBoard.InvokeRequired)
        lblInfoBoard.Invoke(new Action<string>(ShowInfoBoard), strInfo);
      else
      {
        lblInfoBoard.ForeColor = Color.Black;

        lblInfoBoard.Text = strInfo;
      }
    }

    private void ShowInfoBoard(string strInfo, bool bOption)
    {
      if (lblInfoBoard.InvokeRequired)
        lblInfoBoard.Invoke(new Action<string, bool>(ShowInfoBoard), strInfo, bOption);
      else
      {
        if (bOption)
          lblInfoBoard.ForeColor = Color.Green;
        else
          lblInfoBoard.ForeColor = Color.Red;
        //else
        //  lblInfoBoard.ForeColor = Color.Black;

        lblInfoBoard.Text = (strInfo + " " + (bOption ? "Succeed." : "Failed."));
      }
    }


    private void ButtonControl(Button btnButton, bool bEnabled, string strText)
    {
      if (btnButton.InvokeRequired)
        btnButton.Invoke(new Action<Button, bool, string>(ButtonControl), btnButton, bEnabled, strText);
      else
      {
        btnButton.Text = strText;
        btnButton.Enabled = bEnabled;
      }
    }

    private void MenuControl(ToolStripMenuItem tsmiItem)
    {
      Reset();

      ContextMenuStrip cms = tsmiItem.Owner as ContextMenuStrip;
      if (null == cms)
      {
        MessageBox.Show("ContextMenuStrip as Failed.");
        return;
      }

      //
      foreach (var item in cms.Items)
      {
        if (item is ToolStripMenuItem)
        {
          ToolStripMenuItem tsmi = item as ToolStripMenuItem;
          tsmi.Checked = false;
        }
      }
      //ToolStripMenuItem
      tsmiItem.Checked = true;
    }
  }
}
