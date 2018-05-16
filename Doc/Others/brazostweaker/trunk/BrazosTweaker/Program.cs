using System;
using System.Windows.Forms;

namespace BrazosTweaker
{
	public static class Program
	{
		/// <summary>
		/// Global instance of the WinRing0 wrapper.
		/// </summary>
		public static OpenLibSys.Ols Ols { get; private set; }


		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			using (Ols = new OpenLibSys.Ols())
			{
                try
				{
					if (Ols.GetStatus() != (uint)OpenLibSys.Ols.Status.NO_ERROR)
					{
						throw new Exception("WinRing0 could not be initialized.\nMake sure to use the right release for your system: " +
							(IntPtr.Size == 8 ? "x64" : "x86"));
					}

                    var form = new Form1();
					form.CreateControl();

					var args = Environment.GetCommandLineArgs();
					if (args.Length == 2 && args[1].ToLower() == "-minimize")
					{
						const int WM_SIZE = 5;
						const int SIZE_MINIMIZED = 1;

						PostMessage(form.Handle, WM_SIZE, new IntPtr(SIZE_MINIMIZED), IntPtr.Zero);
					}
					else
						form.Show();

					Application.Run();
				}
				catch (Exception e)
				{
					MessageBox.Show(e.Message + "\n\n" + e.StackTrace, "BrazosTweaker exception", MessageBoxButtons.OK, MessageBoxIcon.Error);
					Application.Exit();
				}
			}
		}


		[System.Runtime.InteropServices.DllImport("User32.dll")]
		private static extern int PostMessage(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam);
	}
}
