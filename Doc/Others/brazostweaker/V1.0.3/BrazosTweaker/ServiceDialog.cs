using System;
using System.Windows.Forms;
using System.ServiceProcess;

namespace BrazosTweaker
{
	public partial class ServiceDialog : Form
	{
		private PState[] _pStates = new PState[5];


		/// <summary>
		/// Gets a value indicating whether the service has been (re)started.
		/// </summary>
		public bool Applied { get; private set; }


		public ServiceDialog(System.Drawing.Icon icon)
		{
			InitializeComponent();

			this.Icon = icon;

			makePermanentCheckBox.CheckedChanged += (s, e) => updateButton.Enabled = makePermanentCheckBox.Checked;
			turboCheckBox.CheckedChanged += (s, e) =>
			{
				turboCoresNumericUpDown.Enabled = (turboCheckBox.Checked && !K10Manager.IsTurboLocked());
				if (turboCheckBox.Checked)
				{
					enableCustomCnQCheckBox.Checked = false;
					enableCustomCnQCheckBox.Enabled = false;
				}
				else
					enableCustomCnQCheckBox.Enabled = true;
			};
			enableCustomCnQCheckBox.CheckedChanged += (s, e) => tabControl1.Enabled = enableCustomCnQCheckBox.Checked;

			// select the current P-state settings as default settings
			updateButton_Click(updateButton, EventArgs.Empty);

			turboCheckBox.Enabled = K10Manager.IsTurboSupported();
			turboCheckBox.Checked = (K10Manager.IsTurboEnabled());
			turboCoresNumericUpDown.Enabled = !K10Manager.IsTurboLocked();
			turboCoresNumericUpDown.Value = K10Manager.GetNumTurboCores();
			turboCoresNumericUpDown.Maximum = K10Manager.GetNumCores() - 1;

			balancedProfileControl.LoadFromRegistry();
			highPerformanceProfileControl.LoadFromRegistry();
			powerSaverProfileControl.LoadFromRegistry();

			var key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(@"Software\BrazosTweaker");
			if (key == null)
				return;

			for (int i = 0; i < 5; i++)
			{
				string text = (string)key.GetValue("P" + i);
				_pStates[i] = PState.Decode(text,i);
			}

			RefreshPStatesLabel();

			makePermanentCheckBox.Checked = ((int)key.GetValue("EnableCustomPStates", 0) != 0);

			int turboCores = (int)key.GetValue("TurboCores", (int)turboCoresNumericUpDown.Value);
			turboCheckBox.Checked = (turboCores > 0);
			turboCoresNumericUpDown.Value = turboCores;

			enableCustomCnQCheckBox.Checked = ((int)key.GetValue("EnableCustomCnQ", 0) != 0);

			key.Close();
		}


		private void RefreshPStatesLabel()
		{
			var sb = new System.Text.StringBuilder();

			for (int i = 0; i < 5; i++)
			{
				if (_pStates[i] == null)
					continue;

				if (sb.Length > 0)
					sb.AppendLine();

				sb.AppendFormat("P{0}: {1}", i, _pStates[i].ToString());
			}

			pStatesLabel.Text = sb.ToString();
		}


		private void updateButton_Click(object sender, EventArgs e)
		{
			for (int i = 0; i < 5; i++)
			{
				_pStates[i] = PState.Load(i);

				// disable the current P-state and all following ones in case the
				// first core's CPU VID is > than the previous P-state's
				if ((i > 0 && _pStates[i].Msrs[0].Vid > _pStates[i - 1].Msrs[0].Vid) && (i < 3)) //ignore Vids from NB in comprison
				{
					for (int j = i; j < 5; j++)
						_pStates[j] = null;

					break;
				}
			}

			RefreshPStatesLabel();
		}

		private void applyButton_Click(object sender, EventArgs e)
		{
			var key = Microsoft.Win32.Registry.LocalMachine.CreateSubKey(@"Software\BrazosTweaker");

			key.SetValue("EnableCustomPStates", (makePermanentCheckBox.Checked ? 1 : 0));

			if (makePermanentCheckBox.Checked)
			{
				for (int i = 0; i < 5; i++)
				{
					string valueName = "P" + i;

					if (_pStates[i] != null)
						key.SetValue(valueName, _pStates[i].Encode(i));
					else
						key.DeleteValue(valueName, false);
				}
			}

			key.SetValue("TurboCores", (turboCheckBox.Checked ? (int)turboCoresNumericUpDown.Value : 0));

			key.SetValue("EnableCustomCnQ", (enableCustomCnQCheckBox.Checked ? 1 : 0));
			key.Close();

			if (enableCustomCnQCheckBox.Checked)
			{
				balancedProfileControl.Save();
				highPerformanceProfileControl.Save();
				powerSaverProfileControl.Save();
			}

			try
			{
				serviceController1.Refresh();
				var status = serviceController1.Status;
				if (status != ServiceControllerStatus.Stopped && status != ServiceControllerStatus.StopPending)
					serviceController1.Stop();

				Cursor = Cursors.WaitCursor;
				serviceController1.WaitForStatus(ServiceControllerStatus.Stopped);
				serviceController1.Start();
				Cursor = Cursors.Default;

				Applied = true;
			}
			catch (Exception exception)
			{
				Cursor = Cursors.Default;

				MessageBox.Show("The service could not be (re)started:\n\n" + exception.Message,
					"BrazosTweaker", MessageBoxButtons.OK, MessageBoxIcon.Warning);
			}
		}
	}
}
