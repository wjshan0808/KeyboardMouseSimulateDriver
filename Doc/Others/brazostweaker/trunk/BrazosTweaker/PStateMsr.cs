using System;

namespace BrazosTweaker
{
	/// <summary>
	/// Represents the interesting MSR settings of a CPU core's P-state.
	/// </summary>
	public struct PStateMsr
	{
		/// <summary>
		/// Core multiplicator (4, 4.5, 5, ..., 31.5).
		/// </summary>
		public double Divider { get; set; }

		/// <summary>
		/// Core voltage ID (0.0125 ... 1.55V).
		/// </summary>
		public double Vid { get; set; }

		/// <summary>
		/// Bus speed (0 ... 200MHz).
		/// </summary>
		public double CLK { get; set; }

        /// <summary>
        /// Core / GPU frequency.
        /// </summary>
        public double PLL { get; set; }

        /// <summary>
		/// Loads a core's P-state.
		/// </summary>
		public static PStateMsr Load(int pStateIndex, int coreIndex)
		{
			if (pStateIndex < 0 || pStateIndex > 4)
				throw new ArgumentOutOfRangeException("pStateIndex");

            uint lower = 0;
            //branch here for CPU- and NB-Pstates 
            if (pStateIndex < 3)
            {
                lower = (uint)(Program.Ols.ReadMsr(0xC0010064u + (uint)pStateIndex, coreIndex) & 0xFFFFFFFFu);
            }
            else if (pStateIndex == 3)
            {
                // value of interest: F3xDC NbPstate0Register
                lower = Program.Ols.ReadPciConfig(0xC3, 0xDC);
            }
            else if (pStateIndex == 4)
            {
                // value of interest: F6x90 NbPstate1Register
                lower = Program.Ols.ReadPciConfig(0xC6, 0x90);
            }  
			return Decode(lower,pStateIndex);
		}

        public static PStateMsr Decode(uint value, int pstate)
		{
            //uint maxDiv = (uint)K10Manager.MaxCOF();
            uint maxDiv = (uint)K10Manager.CurrCOF();
            uint clk = (uint)K10Manager.GetBIOSBusSpeed();
            bool turbo = K10Manager.IsTurboSupported();
    
            if (pstate < 3)
            {
                if (pstate <= K10Manager.GetHighestPState())
                {
                    uint cpuDidLSD = (value >> 0) & 0x0F;
                    uint cpuDidMSD = (value >> 4) & 0x1F;
                    uint cpuVid = (value >> 9) & 0x7F;

                    double Div = cpuDidMSD + (cpuDidLSD * 0.25) + 1;
                    double DivPLL = cpuDidMSD + (cpuDidLSD * 0.25) + 1;
                    if (maxDiv == 16 && Div < 2) //E-350 seems to restrict PLL frequencies higher than 1.6GHz
                    {
                        DivPLL = 2;
                    }
                    else if (maxDiv == 24 && Div < 4 && !turbo) //C-50 seems to restrict PLL frequencies higher than 1.0GHz
                    {
                        DivPLL = 4;
                    }
                    else if (maxDiv == 24 && Div < 3 && turbo) //C-60 (with turbo seems to restrict PLL frequencies higher than 1.33GHz
                    {
                        DivPLL = 3;
                    }
                    
                    var msr = new PStateMsr()
                    {
                        Divider = Div,
                        Vid = 1.55 - 0.0125 * cpuVid,
                        CLK = clk,
                        PLL = (16 + maxDiv) / DivPLL * clk
                    };
                    return msr;
                }
                else
                {
                    var msr = new PStateMsr()
                    {
                        Divider = 10,
                        Vid = 0.4,
                        CLK = 100,
                        PLL = 1000
                    };
                    return msr;
                }
            }
            else if (pstate == 3)
            {
                uint nclk = ((value >> 20) & 0x7F);
                uint nbVid = ((value >> 12) & 0x7F);
                double nclkdiv = 1;
                //NCLK Div 2-16 ind 0.25 steps / Div 16-32 in 0.5 steps / Div 32-63 in 1.0 steps
                if (nclk >= 8 && nclk <= 63) nclkdiv = nclk * 0.25;
                else if (nclk >= 64 && nclk <= 95) nclkdiv = (nclk - 64) * 0.5 - 16;
                else if (nclk >= 96 && nclk <= 127) nclkdiv = nclk - 64;
                else nclkdiv = 1;
                var msr = new PStateMsr()
                {
                    Divider = nclkdiv,
                    Vid = 1.55 - 0.0125 * nbVid,
                    CLK = clk,
                    PLL = (16 + maxDiv) / nclkdiv * clk
                };
                return msr;
            }
            else if (pstate == 4)
            {
                uint nclk = ((value >> 0) & 0x7F);
                uint nbVid = ((value >> 8) & 0x7F);
                double nclkdiv = 1;
                //NCLK Div 2-16 ind 0.25 steps / Div 16-32 in 0.5 steps / Div 32-63 in 1.0 steps
                if (nclk >= 8 && nclk <= 63) nclkdiv = nclk * 0.25;
                else if (nclk >= 64 && nclk <= 95) nclkdiv = (nclk - 64) * 0.5 - 16;
                else if (nclk >= 96 && nclk <= 127) nclkdiv = nclk - 64;
                else nclkdiv = 1;
                var msr = new PStateMsr()
                {
                    Divider = nclkdiv,
                    Vid = 1.55 - 0.0125 * nbVid,
                    CLK = clk,
                    PLL = (16 + maxDiv) / nclkdiv * clk
                };
                return msr;
            }
            else
            {
                var msr = new PStateMsr()
                {
                    Divider = 10,
                    Vid = 0.4,
                    CLK = 100,
                    PLL = 1000
                };
                return msr;
            }
		}

		/// <summary>
		/// Encodes the settings into the 32 lower bits of a MSR.
		/// </summary>
		public uint Encode(int pstate)
		{
            if (pstate < 3)
            {
                if (Divider < 1 || Divider > 31.5) throw new ArgumentOutOfRangeException("Divider");
                if (Vid <= 0 || Vid > 1.55) throw new ArgumentOutOfRangeException("Vid");
                if (CLK <= 0 || CLK > 200) throw new ArgumentOutOfRangeException("CLK");

                uint cpuDidMSD, cpuDidLSD;
                cpuDidMSD = (uint)Math.Abs(Divider - 1);

                double temp1 = (double)cpuDidMSD;
                double temp2 = Divider - 1 - temp1;
                cpuDidLSD = (uint)Math.Abs(temp2 / 0.25);

                uint cpuVid = (uint)Math.Round((1.55 - Vid) / 0.0125);
                return (cpuVid << 9) | (cpuDidMSD << 4) | cpuDidLSD;
            }
            else if (pstate == 3)
            {
                //K10Manager.SetBIOSBusSpeed((uint)CLK);
                uint nbVid = (uint)Math.Round((1.55 - Vid) / 0.0125);
                //Divider
                //NCLK Div 2-16 ind 0.25 steps / Div 16-32 in 0.5 steps / Div 32-63 in 1.0 steps
                uint nclk = (uint)Math.Round(Divider * 4);

                return (nclk << 20) | (nbVid << 12);
            }
            else if (pstate == 4)
            {
                //K10Manager.SetBIOSBusSpeed((uint)CLK);
                uint nbVid = (uint)Math.Round((1.55 - Vid) / 0.0125);
                //Divider
                //NCLK Div 2-16 ind 0.25 steps / Div 16-32 in 0.5 steps / Div 32-63 in 1.0 steps
                uint nclk = (uint)Math.Round(Divider * 4);

                return (nbVid << 8) | (nclk << 0);
            }
            else
            {
                return 0;
            }
		}
	}
}
