using System;
using System.Windows.Forms;

namespace BrazosTweaker
{
	/// <summary>
	/// Available custom C&Q profiles (one per default Windows power scheme).
	/// </summary>
	public enum CnQProfile
	{
		Balanced = 0,
		HighPerformance = 1,
		PowerSaver = 2
	}


	/// <summary>
	/// GUI for the custom C&Q parameters for a power scheme.
	/// </summary>
	public partial class CnQProfileControl : UserControl
	{
		/// <summary>
		/// Gets or sets the associated power scheme.
		/// </summary>
		public CnQProfile Profile { get; set; }


		public CnQProfileControl()
		{
			InitializeComponent();

			gangedRadioButton.CheckedChanged += (s, e) => ungangedRadioButton.Checked = !gangedRadioButton.Checked;
			ungangedRadioButton.CheckedChanged += (s, e) => gangedRadioButton.Checked = !ungangedRadioButton.Checked;

			minPStateNumericUpDown.ValueChanged += (s, e) => { maxPStateNumericUpDown.Minimum = minPStateNumericUpDown.Value; CheckPStateBounds(); };
			maxPStateNumericUpDown.ValueChanged += (s, e) => { minPStateNumericUpDown.Maximum = maxPStateNumericUpDown.Value; CheckPStateBounds(); };

			thresholdUpNumericUpDown.ValueChanged += (s, e) => thresholdDownNumericUpDown.Maximum = thresholdUpNumericUpDown.Value;
			thresholdDownNumericUpDown.ValueChanged += (s, e) => thresholdUpNumericUpDown.Minimum = thresholdDownNumericUpDown.Value;

			SetDefaultValues();
		}

		/// <summary>
		/// Disables all other controls if a single P-state is allowed, otherwise re-enables them.
		/// </summary>
		void CheckPStateBounds()
		{
			bool locked = (minPStateNumericUpDown.Value == maxPStateNumericUpDown.Value);

			gangedRadioButton.Enabled = !locked;
			ungangedRadioButton.Enabled = !locked;
			samplingIntervalNumericUpDown.Enabled = !locked;
			thresholdUpNumericUpDown.Enabled = !locked;
			thresholdDownNumericUpDown.Enabled = !locked;
			numSamplesUpNumericUpDown.Enabled = !locked;
			numSamplesDownNumericUpDown.Enabled = !locked;
			aggressiveUpCheckBox.Enabled = !locked;
		}

		private void SetDefaultValues()
		{
			gangedRadioButton.Checked = true;

			samplingIntervalNumericUpDown.Value = 50;

			minPStateNumericUpDown.Value = (Profile == CnQProfile.PowerSaver ? 1 : 0);
			maxPStateNumericUpDown.Value = (Profile == CnQProfile.HighPerformance ? 0 : 2);

			thresholdUpNumericUpDown.Value =  100 / System.Environment.ProcessorCount - 1;
			thresholdDownNumericUpDown.Value = (int)(0.8f * (float)thresholdUpNumericUpDown.Value);
			thresholdUpNumericUpDown.Minimum = thresholdDownNumericUpDown.Value;
			thresholdDownNumericUpDown.Maximum = thresholdUpNumericUpDown.Value;

			numSamplesUpNumericUpDown.Value = 2;
			numSamplesDownNumericUpDown.Value = 8;

			aggressiveUpCheckBox.Checked = false;
		}

		private string GetRegistryKeyName()
		{
			string subkey;
			switch (Profile)
			{
				case CnQProfile.Balanced:
					subkey = "Balanced";
					break;
				case CnQProfile.HighPerformance:
					subkey = "High performance";
					break;
				case CnQProfile.PowerSaver:
					subkey = "Power saver";
					break;
				default:
					throw new NotSupportedException("The profile is not supported.");
			}

			return @"Software\BrazosTweaker\" + subkey;
		}


		/// <summary>
		/// Loads the settings from the registry.
		/// </summary>
		public void LoadFromRegistry()
		{
			var key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(GetRegistryKeyName());
			if (key == null)
				return;

			gangedRadioButton.Checked = ((int)key.GetValue("Ganged", gangedRadioButton.Checked ? 1 : 0) != 0);

			samplingIntervalNumericUpDown.Value = (int)key.GetValue("SamplingInterval", (int)samplingIntervalNumericUpDown.Value);

			minPStateNumericUpDown.Value = (int)key.GetValue("MinPState", (int)minPStateNumericUpDown.Value);
			maxPStateNumericUpDown.Value = (int)key.GetValue("MaxPState", (int)maxPStateNumericUpDown.Value);

			thresholdUpNumericUpDown.Value = (int)key.GetValue("ThresholdUp", (int)thresholdUpNumericUpDown.Value);
			thresholdDownNumericUpDown.Value = (int)key.GetValue("ThresholdDown", (int)thresholdDownNumericUpDown.Value);

			numSamplesUpNumericUpDown.Value = (int)key.GetValue("NumSamplesUp", (int)numSamplesUpNumericUpDown.Value);
			numSamplesDownNumericUpDown.Value = (int)key.GetValue("NumSamplesDown", (int)numSamplesDownNumericUpDown.Value);

			aggressiveUpCheckBox.Checked = ((int)key.GetValue("AggressiveUp", aggressiveUpCheckBox.Checked ? 1 : 0) != 0);

			key.Close();
		}

		/// <summary>
		/// Saves the settings to the registry.
		/// </summary>
		public void Save()
		{
			var key = Microsoft.Win32.Registry.LocalMachine.CreateSubKey(GetRegistryKeyName());
			if (key == null)
				return;

			key.SetValue("Ganged", gangedRadioButton.Checked ? 1 : 0);

			key.SetValue("SamplingInterval", (int)samplingIntervalNumericUpDown.Value);

			key.SetValue("MinPState", (int)minPStateNumericUpDown.Value);
			key.SetValue("MaxPState", (int)maxPStateNumericUpDown.Value);

			key.SetValue("ThresholdUp", (int)thresholdUpNumericUpDown.Value);
			key.SetValue("ThresholdDown", (int)thresholdDownNumericUpDown.Value);

			key.SetValue("NumSamplesUp", (int)numSamplesUpNumericUpDown.Value);
			key.SetValue("NumSamplesDown", (int)numSamplesDownNumericUpDown.Value);

			key.SetValue("AggressiveUp", aggressiveUpCheckBox.Checked ? 1 : 0);

			key.Close();
		}
	}
}
