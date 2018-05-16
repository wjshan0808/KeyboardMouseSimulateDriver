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
  public partial class frmKeyboardMouseSimulator : Form
  {
    private System.Threading.Thread m_trdKeyboardInterval = null;
    private volatile bool m_bChecking = false;
    private volatile bool m_bInitialized = false;
    private KeyboardWays m_nKeyboardWay = KeyboardWays.WinIo;

    public frmKeyboardMouseSimulator()
    {
      InitializeComponent();
    }


    private void frmGlobalKeyboardSet_FormClosing(object sender, FormClosingEventArgs e)
    {
      m_bInitialized = false;
      m_bChecking = false;
      if (m_trdKeyboardInterval != null)
      {
        m_trdKeyboardInterval.Abort();
        m_trdKeyboardInterval.Join();
      }
      KeyboardMouseSimulateDriver.Uninitialize();
      Application.DoEvents();
    }

    private void frmGlobalKeyboardSet_Load(object sender, EventArgs e)
    {
      m_bChecking = true;
      System.Threading.ThreadPool.QueueUserWorkItem((ojb) =>
      {
        while (m_bChecking)
        {
          ShowCheckout(KeyboardMouseSimulateDriver.Checkout());

          System.Threading.Thread.Sleep(999);
        }
      });
    }

    private void btnSimulate_Click(object sender, EventArgs e)
    {
      UIControl(false);

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
        stParameter.m_nKeyboardValue = (uint)Keys.O;
      else if (rdobtnKeyG.Checked)
        stParameter.m_nKeyboardValue = (uint)Keys.G;
      else
        stParameter.m_nKeyboardValue = 0;

      if (rdobtnMouseLeft.Checked)
        stParameter.m_nMouseValue = 0x09;
      else if (rdobtnMouseRight.Checked)
        stParameter.m_nMouseValue = 0x02;// 0x0A;//
      else
        stParameter.m_nMouseValue = 0x0C;
      
      m_trdKeyboardInterval = new System.Threading.Thread(Simulate);
      m_trdKeyboardInterval.IsBackground = false;
      m_trdKeyboardInterval.Priority = System.Threading.ThreadPriority.AboveNormal;
      m_trdKeyboardInterval.Start(stParameter);
    }

    private void Simulate(object obj)
    {
      if (m_bInitialized == false)
      {
        int nReturn = KeyboardMouseSimulateDriver.Initialize();
        m_bInitialized = nReturn == 0;
        ShowInfoBoard("Initialize", nReturn, m_bInitialized);
      }

      ShowInfoBoard("Ready...");
      System.Threading.Thread.Sleep(2000);
      ShowInfoBoard("Go !");
      System.Threading.Thread.Sleep(1000);

      if (m_bInitialized)
      {
        Parameters stParameter = (Parameters)obj;
        if (0 != stParameter.m_nKeyboardValue)
        {
          for (int i = 0; i < stParameter.m_nPeriod; i++)
          {
            int nTimes = 1;
            DateTime dtStart = DateTime.Now;
            while (stParameter.m_nDuration > (DateTime.Now - dtStart).TotalSeconds)
            {
              switch (m_nKeyboardWay)
              {
                case KeyboardWays.Event:
                  ShowInfoBoard(nTimes, KeyboardMouseSimulateDriver.EventKeyDown(stParameter.m_nKeyboardValue));
                  ShowInfoBoard(nTimes++, KeyboardMouseSimulateDriver.EventKeyUp(stParameter.m_nKeyboardValue));
                  break;
                case KeyboardWays.WinIo:
                  ShowInfoBoard(nTimes, KeyboardMouseSimulateDriver.KeyDown(stParameter.m_nKeyboardValue));
                  ShowInfoBoard(nTimes++, KeyboardMouseSimulateDriver.KeyUp(stParameter.m_nKeyboardValue));
                  break;
              }
              System.Threading.Thread.Sleep(stParameter.m_nInterval);
            }
          }
        }
        if (0 != stParameter.m_nMouseValue)
        {
          ShowInfoBoard(-((int)stParameter.m_nMouseValue), KeyboardMouseSimulateDriver.MouseDown(stParameter.m_nMouseValue));
          ShowInfoBoard(-((int)stParameter.m_nMouseValue), KeyboardMouseSimulateDriver.MouseUp(stParameter.m_nMouseValue));
        }
      }
      UIControl(true);
    }

    private void UIControl(bool bEnabled)
    {
      if (btnSimulate.InvokeRequired)
        btnSimulate.Invoke(new Action<bool>(UIControl), bEnabled);
      else
      {
        if (bEnabled)
          btnSimulate.Text = "Simulate";
        else
          btnSimulate.Text = "Wroking ...";
        btnSimulate.Enabled = bEnabled;
      }
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

    private void ShowInfoBoard(string strInfo, int nReturn, bool bResult)
    {
      string strResult = string.Empty;
      if (bResult)
        strResult = string.Format("{0} {1} .", strInfo, "Succeed");
      else
        strResult = string.Format("{0} {1} ErrorCode<{2}>.", strInfo, "Failed", nReturn);

      ShowInfoBoard(strResult);
    }
    private void ShowInfoBoard(int nValue, bool bResult)
    {
      string strResult = string.Empty;
      if (0 < nValue)
        strResult = string.Format("Times {0} {1}.", nValue, bResult ? "Succeed" : "Failed");
      else
      {
        if (-1 == nValue)
          strResult = string.Format("Click Mouse {0} Button {1}.", "Left", bResult ? "Succeed" : "Failed");
        else if (-2 == nValue)
          strResult = string.Format("Click Mouse {0} Button {1}.", "Right", bResult ? "Succeed" : "Failed");
        else if (-4 == nValue)
          strResult = string.Format("Click Mouse {0} Button {1}.", "Middle", bResult ? "Succeed" : "Failed");
        else if (-5 == nValue)
          strResult = string.Format("Click Mouse {0} Button {1}.", "X1", bResult ? "Succeed" : "Failed");
        else if (-6 == nValue)
          strResult = string.Format("Click Mouse {0} Button {1}.", "X2", bResult ? "Succeed" : "Failed");
      }
      ShowInfoBoard(strResult);
    }

    private void ShowInfoBoard(string strInfo)
    {
      if (lblInfoBoard.InvokeRequired)
        lblInfoBoard.Invoke(new Action<string>(ShowInfoBoard), strInfo);
      else
      {
        if (strInfo.Contains("Succeed"))
          lblInfoBoard.ForeColor = Color.Green;
        else if (strInfo.Contains("Failed"))
          lblInfoBoard.ForeColor = Color.Red;
        else
          lblInfoBoard.ForeColor = Color.Black;

        lblInfoBoard.Text = strInfo;
      }
    }

    private void tsmiEventKeyboard_Click(object sender, EventArgs e)
    {
      ToolStripMenuItem tsmi = sender as ToolStripMenuItem;
      KeyboardControl(tsmi);
      m_nKeyboardWay = KeyboardWays.Event;
    }

    private void tsmiWinIoKeyboard_Click(object sender, EventArgs e)
    {
      ToolStripMenuItem tsmi = sender as ToolStripMenuItem;
      KeyboardControl(tsmi);
      m_nKeyboardWay = KeyboardWays.WinIo;
    }

    private void KeyboardControl(ToolStripMenuItem tsmiItem)
    {
      foreach (var item in this.cmsGlobalMenu.Items)
      {
        if (item is ToolStripMenuItem)
        {
          ToolStripMenuItem tsmi = item as ToolStripMenuItem;
          tsmi.Checked = false;
        }
      }

      tsmiItem.Checked = true;
    }
  }
}
