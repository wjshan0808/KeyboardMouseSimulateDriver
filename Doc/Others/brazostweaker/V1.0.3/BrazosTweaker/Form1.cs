using System;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using Microsoft.Win32;

namespace BrazosTweaker
{
	public partial class Form1 : Form
	{
		private class PowerScheme
		{
			public Guid Guid;
			public string Name;

			public override string ToString()
			{
				return Name;
			}
		}


		private static readonly bool _useWindowsPowerSchemes = (Environment.OSVersion.Version.Major >= 6);


		public Form1()
		{
			InitializeComponent();

			notifyIcon.Icon = this.Icon;
			notifyIcon.ContextMenuStrip = new ContextMenuStrip();
			notifyIcon.Visible = true;

			this.Width += p0StateControl.GetDeltaOptimalWidth();
            this.Height = 230;

			p0StateControl.LoadFromHardware(0);
			p1StateControl.LoadFromHardware(1);
			p2StateControl.LoadFromHardware(2);
			nbp0StateControl.LoadFromHardware(3);
            nbp1StateControl.LoadFromHardware(4);
            statusinfo.LoadFromHardware();

			if (!_useWindowsPowerSchemes)
			{
				// use BrazosTweaker's power schemes (via the registry)
				powerSchemesComboBox.Items.Add(new PowerScheme() { Name = "Balanced" });
				powerSchemesComboBox.Items.Add(new PowerScheme() { Name = "High performance" });
				powerSchemesComboBox.Items.Add(new PowerScheme() { Name = "Power saver" });

				var key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(@"Software\BrazosTweaker");

				if (key == null)
					powerSchemesComboBox.SelectedIndex = 0;
				else
				{
					powerSchemesComboBox.SelectedIndex = (int)key.GetValue("PowerScheme", 0);
					key.Close();
				}

				InitializeNotifyIconContextMenu();

				powerSchemesComboBox.SelectedIndexChanged += (s, e) =>
				{
					var k = Microsoft.Win32.Registry.LocalMachine.CreateSubKey(@"Software\BrazosTweaker");
					k.SetValue("PowerScheme", powerSchemesComboBox.SelectedIndex);
					k.Close();
					SynchronizeNotifyIconContextMenu();
				};

				return;
			}

			int guidSize = 16;
			IntPtr guid = Marshal.AllocHGlobal(guidSize);

			// get the GUID of the current power scheme
			IntPtr activeGuidPointer;
			if (PowerGetActiveScheme(IntPtr.Zero, out activeGuidPointer) != 0)
				throw new Exception("PowerGetActiveScheme()");
			Guid activeGuid = (Guid)Marshal.PtrToStructure(activeGuidPointer, typeof(Guid));
			LocalFree(activeGuidPointer);

			// iterate over all power schemes
			for (int i = 0; true; i++)
			{
				if (PowerEnumerate(IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, 0x10, i, guid, ref guidSize) != 0)
					break;

				// get the required buffer size
				int size = 0;
                if (PowerReadFriendlyName(IntPtr.Zero, guid, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, ref size) != 0)
                    break;//throw new Exception("PowerReadFriendlyName()");

				IntPtr stringBuffer = Marshal.AllocHGlobal(size);

				// get the scheme name
				if (PowerReadFriendlyName(IntPtr.Zero, guid, IntPtr.Zero, IntPtr.Zero, stringBuffer, ref size) != 0)
					throw new Exception("PowerReadFriendlyName()");

				var item = new PowerScheme()
				{
					Guid = (Guid)Marshal.PtrToStructure(guid, typeof(Guid)),
					Name = Marshal.PtrToStringUni(stringBuffer)
				};

				Marshal.FreeHGlobal(stringBuffer);

				powerSchemesComboBox.Items.Add(item);

				if (item.Guid == activeGuid)
					powerSchemesComboBox.SelectedIndex = i;
			}

			Marshal.FreeHGlobal(guid);

			InitializeNotifyIconContextMenu();

			powerSchemesComboBox.SelectedIndexChanged += (s, e) =>
			{
				var item = (PowerScheme)powerSchemesComboBox.SelectedItem;
				if (PowerSetActiveScheme(IntPtr.Zero, ref item.Guid) != 0)
					throw new Exception("PowerSetActiveScheme()");
				SynchronizeNotifyIconContextMenu();
			};
		}


		#region Power schemes API (Vista+).

		[DllImport("PowrProf.dll")]
		private static extern uint PowerEnumerate(IntPtr RootPowerKey, IntPtr SchemeGuid,
			IntPtr SubGroupOfPowerSettingsGuid, int AccessFlags, int Index,
			IntPtr Buffer, ref int BufferSize);

		[DllImport("PowrProf.dll")]
		private static extern uint PowerReadFriendlyName(IntPtr RootPowerKey, IntPtr SchemeGuid,
			IntPtr SubGroupOfPowerSettingsGuid, IntPtr PowerSettingGuid,
			IntPtr Buffer, ref int BufferSize);

		[DllImport("PowrProf.dll")]
		private static extern uint PowerGetActiveScheme(IntPtr UserRootPowerKey, out IntPtr SchemeGuid);

		[DllImport("PowrProf.dll")]
		private static extern uint PowerSetActiveScheme(IntPtr UserRootPowerKey, ref Guid SchemeGuid);

		[DllImport("Kernel32.dll")]
		private static extern IntPtr LocalFree(IntPtr hMem);

		#endregion


		/// <summary>
		/// Copies the power schemes combox box items to the tray icon's
		/// context menu.
		/// </summary>
		private void InitializeNotifyIconContextMenu()
		{
			int selectedIndex = powerSchemesComboBox.SelectedIndex;

			for (int i = 0; i < powerSchemesComboBox.Items.Count; i++)
			{
				var item = (PowerScheme)powerSchemesComboBox.Items[i];

				var menuItem = new ToolStripMenuItem(item.Name);
				menuItem.Checked = (i == selectedIndex);
				// a click causes the selection of another scheme in the combo box,
				// thereby switching to the clicked scheme
				int copy = i; // we _must_ not use i in the lambda expression, because it is incremented up to powerSchemesComboBox.Items.Count later on
				              // therefore use a private copy
				menuItem.Click += (s, e) => powerSchemesComboBox.SelectedIndex = copy;

				notifyIcon.ContextMenuStrip.Items.Add(menuItem);
			}

			// separator
			notifyIcon.ContextMenuStrip.Items.Add("-");

			var exitItem = new ToolStripMenuItem("Exit");
			exitItem.Click += (s, e) => this.Close();
			notifyIcon.ContextMenuStrip.Items.Add(exitItem);
		}

		/// <summary>
		/// Marks the tray icon's context menu item which represents the
		/// currently selected item in the power schemes combo box.
		/// </summary>
		private void SynchronizeNotifyIconContextMenu()
		{
			int selectedIndex = powerSchemesComboBox.SelectedIndex;

			for (int i = 0; i < powerSchemesComboBox.Items.Count; i++)
			{
				var item = (ToolStripMenuItem)notifyIcon.ContextMenuStrip.Items[i];
				item.Checked = (i == selectedIndex);
			}
		}


		protected override void WndProc(ref Message m)
		{
			const int WM_SIZE = 5;
			const int SIZE_MINIMIZED = 1;

			// hide the form when it is minimized and stop the timer
			// the form is re-shown by left-clicking the notify icon
			if (m.Msg == WM_SIZE && m.WParam.ToInt32() == SIZE_MINIMIZED)
			{
				timer1.Enabled = false;
				Hide();
			}

			base.WndProc(ref m);
		}


		private void applyButton_Click(object sender, EventArgs e)
		{
			var controls = new PStateControl[5] { p0StateControl, p1StateControl, p2StateControl, nbp0StateControl, nbp1StateControl };
            var statuscontrols = new StatusControl[1] { statusinfo };

			int lastModifiedControl = Array.FindLastIndex(controls, (c) => { return c.IsModified; } );
			if (lastModifiedControl < 0)
				return; // no control is modified

            if (lastModifiedControl > 2)
            {
                lastModifiedControl = 2; //checking CPU P-States only -> skip NB P-States
            }
			for (int i = 1; i <= lastModifiedControl; i++)
			{
				// make sure the neighboring P-state on the left specifies a >= VID
				if (controls[i - 1].Vid < controls[i].Vid)
				{
					MessageBox.Show(string.Format("P{0}'s VID is greater than P{1}'s.", i, i - 1), "Invalid P-state settings", MessageBoxButtons.OK, MessageBoxIcon.Error);
					tabControl1.SelectedIndex = i;
					return;
				}
            }

			timer1.Enabled = false;

			// try to temporarily set the number of boosted (Turbo) P-states to 0
			// this should suspend the restriction of software P-state multis by F3x1F0[MaxSwPstateCpuCof]
			bool turboEnabled = K10Manager.IsTurboEnabled();
			int boostedStates = K10Manager.GetNumBoostedStates();
			if (boostedStates != 0)
				K10Manager.SetTurbo(false);

			for (int i = 0; i < 5; i++)
				controls[i].Save();

			if (turboEnabled)
				K10Manager.SetTurbo(true);

			// refresh the P-states
            for (int i = 0; i < 5; i++)
                controls[i].LoadFromHardware(i);

            statuscontrols[0].LoadFromHardware();

			timer1.Enabled = true;
		}

		private void serviceButton_Click(object sender, EventArgs e)
		{
			using (ServiceDialog dialog = new ServiceDialog(this.Icon))
			{
				dialog.ShowDialog();

				// refresh the P-states
				if (dialog.Applied)
				{
					p0StateControl.LoadFromHardware(0);
					p1StateControl.LoadFromHardware(1);
					p2StateControl.LoadFromHardware(2);
					nbp0StateControl.LoadFromHardware(3);
					nbp1StateControl.LoadFromHardware(4);
                    statusinfo.LoadFromHardware();
				}
			}
		}

		private void timer1_Tick(object sender, EventArgs e)
		{
			// get the current P-state of the first core
			int currentPState = K10Manager.GetCurrentPState(0);
            int currentNbPState = K10Manager.GetNbPState();

			tabControl1.SuspendLayout();
			for (int i = 0; i < 3; i++)
				tabControl1.TabPages[i].Text = "P" + i + (i == currentPState ? "*" : string.Empty);

            for (int i = 3; i < 5; i++)
                tabControl1.TabPages[i].Text = "NB P" + (i - 3) + ((i - 3) == currentNbPState ? "*" : string.Empty);
			
            tabControl1.ResumeLayout();
		}

		private void notifyIcon_MouseClick(object sender, MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left && !this.Visible)
			{
				this.Show();
				this.WindowState = FormWindowState.Normal;

				// refresh the current P-state
				timer1_Tick(timer1, EventArgs.Empty);

				// odd, but necessary
				Refresh();

				// keep refreshing the current P-state
				timer1.Enabled = true;
			}
		}

		private void Form1_FormClosed(object sender, FormClosedEventArgs e)
		{
			Application.Exit();
		}
	}
}
