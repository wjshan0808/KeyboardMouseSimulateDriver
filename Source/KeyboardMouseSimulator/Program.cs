using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace KeyboardMouseSimulator
{
  static class Program
  {
    /// <summary>
    /// 应用程序的主入口点。
    /// </summary>
    [STAThread]
    static void Main()
    {
      AppDomain.CurrentDomain.DomainUnload += CurrentDomain_DomainUnload;
      Application.ThreadException += Application_ThreadException;
      Application.EnableVisualStyles();
      Application.SetCompatibleTextRenderingDefault(false);
      using (frmKeyboardMouseSimulatorDemo frm = new frmKeyboardMouseSimulatorDemo())
        Application.Run(frm);
    }

    static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
    {
      MessageBox.Show(e.Exception.Message);
    }

    static void CurrentDomain_DomainUnload(object sender, EventArgs e)
    {
      //try
      //{
      //    if (System.IO.File.Exists(FileName))
      //    {
      //        System.IO.File.Delete(FileName);
      //    }
      //}
      //catch (Exception ex)
      //{
      //    //MessageBox.Show("资源清理失败.\n" + ex.Message, "系统提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
      //}
    }
  }
}
