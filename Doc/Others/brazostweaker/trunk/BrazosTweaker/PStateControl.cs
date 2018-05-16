using System;
using System.Windows.Forms;

namespace BrazosTweaker
{
	/// <summary>
	/// Represents a set of P-state settings for all cores of a CPU.
	/// </summary>
	public sealed partial class PStateControl : UserControl
	{
		private static readonly int _numCores = System.Environment.ProcessorCount;

		// cached:
		private static double _maxCOF = -1;
        private static double _minVid, _maxVid;
        private static int _maxPstate = -1;
        
		private int _index = -1; // 0-4
		private PState _pState;

		private int _optimalWidth;
		private bool _modified;


		/// <summary>
		/// Gets or sets the associated hardware P-state index (0-4).
		/// </summary>
		public int PStateIndex
		{
			get { return _index; }
			set
			{
				if (value < 0 || value > 5)
					throw new ArgumentOutOfRangeException("PStateIndex");

				_index = value;
			}
		}

		/// <summary>
		/// Gets a value indicating whether the control has been modified by the user
		/// since the last load/save operation.
		/// </summary>
		public bool IsModified
		{
			get { return _modified; }
		}

		/// <summary>
		/// Gets the currently selected CPU/NB VID.
		/// </summary>
		public double Vid
		{
			get { return (double)VidNumericUpDown.Value; }
		}

		/// <summary>
		/// Gets the currently selected CLK.
		/// </summary>
		public double CLK
		{
			get { return (double)CLKNumericUpDown.Value; }
		}


		/// <summary>
		/// Constructor.
		/// </summary>
		public PStateControl()
		{
			InitializeComponent();

            
			// check if the CPU's maximum multi is limited (non Black Edition CPUs)
			if (_maxCOF < 0)
			{
				// in DesignMode, Program.Ols is null
				if (Program.Ols == null)
				{
					_maxCOF = 31.5;
					_minVid = 0.0125;
					_maxVid = 1.55;
                    _maxPstate = -1;
				}
				else
				{
					_maxCOF = K10Manager.MaxCOF();
                    _maxPstate = K10Manager.GetHighestPState();

                    //double _curDiv = K10Manager.CurrCOF();
                    //uint MainDivEn = K10Manager.MainCofEn();
					K10Manager.GetVidLimits(out _minVid, out _maxVid);
				}
			}

			VidNumericUpDown.Minimum = (decimal)_minVid;
            CLKNumericUpDown.Minimum = 0;
			VidNumericUpDown.Maximum = (decimal)_maxVid;
            CLKNumericUpDown.Maximum = 200;

			// add as many NumericUpDown controls as there are CPU cores for the multis
			for (int i = 0; i < _numCores; i++)
			{
				var control = new NumericUpDown()
				{
					AutoSize = true,
					DecimalPlaces = 2,
                    Increment = (decimal)0.25,
                    Maximum = (decimal)_maxCOF,
					Minimum = 1,
                    TabIndex = i,
					TextAlign = HorizontalAlignment.Center,
					Value = 4,
				};
                toolTip1.SetToolTip(control, "Divider for core " + (i + 1) + ".\r\nReference clock (default: 100 MHz) times " + (_maxCOF + 16) + " divided by the chosen value yields the core speed.");

				control.ValueChanged += (s, e) => _modified = true;

				if (i == 0)
				{
					control.ValueChanged += (s, e) =>
					{
						for (int j = 1; j < _numCores; j++)
						{
							var otherControl = (NumericUpDown)flowLayoutPanel1.Controls[j];
							otherControl.Value = control.Value;
						}
					};
				}

				flowLayoutPanel1.Controls.Add(control);
			}

			VidNumericUpDown.ValueChanged += (s, e) => _modified = true;
		    CLKNumericUpDown.ValueChanged += (s, e) => _modified = true;

			// set the tab order
			VidNumericUpDown.TabIndex = 3 + _numCores;
			CLKNumericUpDown.TabIndex = VidNumericUpDown.TabIndex + 1;
			refreshButton.TabIndex = CLKNumericUpDown.TabIndex + 1;

			// compute the optimal width, based on the number of cores
			_optimalWidth = Cofstate.Width + Cofstate.Margin.Horizontal + flowLayoutPanel1.Controls.Count *
				(flowLayoutPanel1.Controls[0].Width + flowLayoutPanel1.Controls[0].Margin.Horizontal) + 70;

			refreshButton.Click += (s, e) => LoadFromHardware(_index);
		}

		/// <summary>
		/// Returns the delta for the optimal width, based upon the number of cores.
		/// </summary>
		public int GetDeltaOptimalWidth()
		{
			return (_optimalWidth - this.Width);
		}


		/// <summary>
		/// Loads the P-state settings from each core's MSR.
		/// </summary>
		public void LoadFromHardware(int pstatetab)
		{
			if (_index < 0)
				throw new InvalidOperationException("The PStateIndex property needs to be initialized first.");

            if (pstatetab < 3) //hardware loads for CPU
            {
                if (_index <= _maxPstate) //skip, in case just 2 CPU PStates are initialized 
                {
                    _pState = PState.Load(_index);

                    double maxCpuVid = 0;
                    for (int i = 0; i < _pState.Msrs.Length; i++)
                    {
                        var msr = _pState.Msrs[i];

                        var control = (NumericUpDown)flowLayoutPanel1.Controls[i];
                        control.Value = (decimal)msr.Divider;

                        maxCpuVid = Math.Max(maxCpuVid, msr.Vid);
                    }

                    VidNumericUpDown.Value = Math.Min(VidNumericUpDown.Maximum, (decimal)maxCpuVid);
                    //int check = K10Manager.SetBIOSBusSpeed(80); 
                    CLKNumericUpDown.Value = (decimal)K10Manager.GetBIOSBusSpeed();
                    pllfreq.Text = "P" + _index + " Freq (CPU): " + (int)_pState.Msrs[0].PLL + "MHz";
                    Cofstate.Text = "Mult = " + (K10Manager.CurrCOF() + 16) + " divided by ->";
                    Form1.freq[pstatetab] = (int)_pState.Msrs[0].PLL;
                }
                else
                {
                    VidNumericUpDown.Value = (decimal)0.4;
                    CLKNumericUpDown.Value = 100;
                }
            }
            else if (pstatetab == 3) 
            {
                //hardware loads for NB P0
                _pState = PState.Load(_index);
                var control = (NumericUpDown)flowLayoutPanel1.Controls[0];
                control.Value = (decimal)K10Manager.GetNbDivPState0(); 
                VidNumericUpDown.Value = (decimal)(1.55 - 0.0125 * K10Manager.GetNbVidPState0());
                CLKNumericUpDown.Value = (decimal)K10Manager.GetBIOSBusSpeed();
                pllfreq.Text = "P" + (_index - 3)  + " Freq (GPU): " + (int)_pState.Msrs[0].PLL + "MHz";
                Cofstate.Text = "Mult = " + (K10Manager.CurrCOF() + 16) + " divided by ->";
                Form1.freq[pstatetab] = (int)_pState.Msrs[0].PLL;
            }
            else if (pstatetab == 4)
            {
                //hardware loads for NB P0
                _pState = PState.Load(_index);
                var control = (NumericUpDown)flowLayoutPanel1.Controls[0];
                control.Value = (decimal)K10Manager.GetNbDivPState1();
                VidNumericUpDown.Value = (decimal)(1.55 - 0.0125 * K10Manager.GetNbVidPState1());
                CLKNumericUpDown.Value = (decimal)K10Manager.GetBIOSBusSpeed();
                pllfreq.Text = "P" + (_index - 3) + " Freq (GPU): " + (int)_pState.Msrs[0].PLL + "MHz";
                Cofstate.Text = "Mult = " + (K10Manager.CurrCOF() + 16) + " divided by ->";
                Form1.freq[pstatetab] = (int)_pState.Msrs[0].PLL;
            }
            else if (pstatetab == 5) //settings for displaying registers
            {
                VidNumericUpDown.Value = 1;
                CLKNumericUpDown.Value = 100;
            }
            _modified = false;
		}

		/// <summary>
		/// Saves the current P-state settings to each core's MSR.
		/// </summary>
		public void Save()
		{
			if (!_modified)
				return;

			if (_pState == null)
				throw new InvalidOperationException("Load a P-state first for safe initialization.");

			for (int i = 0; i < _numCores; i++)
			{
				var control = (NumericUpDown)flowLayoutPanel1.Controls[i];

				_pState.Msrs[i].Divider = (double)control.Value;
				_pState.Msrs[i].Vid = (double)VidNumericUpDown.Value;
				_pState.Msrs[i].CLK = (double)CLKNumericUpDown.Value;
			}

			_pState.Save(_index);

			_modified = false;
		}
	}
}
