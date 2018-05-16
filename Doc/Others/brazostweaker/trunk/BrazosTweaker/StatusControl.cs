using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Threading;

namespace BrazosTweaker
{
    /// <summary>
    /// Represents a set all interesting registers to show.
    /// </summary>
    public sealed partial class StatusControl : UserControl
    {
        private int _optimalWidth;
        private bool _modified;

        private int _index = 0; // 0

        //private readonly SMBIOS smbios;
        
        SMBIOS smbios = new SMBIOS();

        // Registers of the embedded controller
        byte EC_DATAPORT = 0x62;	// EC data io-port - port for the EC register value
        byte EC_CTRLPORT = 0x66;	// EC control io-port - port for the EC register index

        // Embedded controller status register bits
        byte EC_STAT_OBF = 0x01;    // Output buffer full 
        byte EC_STAT_IBF = 0x02;    // Input buffer full 
        
        // Embedded controller commands
        // (write to EC_CTRLPORT to initiate read/write operation)
        byte EC_CTRLPORT_READ = 0x80;	
        
        byte TP_ECOFFSET_FANSPEED = 0x94; // 16 bit word, lo/hi byte

        byte[] SENSORADDR = { 0xA8, 0xA9, 0xAA };
                            //Core   Temp1 Temp2

        public string GetVendor()
        {
            string vendor = "Unknown"; 
            if (smbios.Board != null)
            {
                vendor = smbios.BIOS.Vendor.ToString();
            }
            return vendor;
        }

        public string GetMobo()
        {
            string mobo = "Unknown";
            if (smbios.Board != null)
            {
                mobo = smbios.Board.ProductName.ToString();
            }
            return mobo;
        }

        public string GetReport()
        {
            StringBuilder output = new StringBuilder();
            if (smbios.Board != null)
            {
                output.AppendLine(smbios.BIOS.Vendor.ToString());
                output.AppendLine(smbios.BIOS.Version.ToString());
                output.AppendLine(smbios.Board.Manufacturer.ToString());
                output.AppendLine(smbios.Board.ProductName.ToString());
            }
            return output.ToString();
            //return test;
        }

        /// <summary>
        /// Gets or sets the associated hardware P-state index (0-4).
        /// </summary>
        public int StatusIndex
        {
            get { return _index; }
            set
            {
                if (value < 0 || value > 5)
                    throw new ArgumentOutOfRangeException("StatusIndex");

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

        public StatusControl()
        {
            InitializeComponent();

            _optimalWidth = 270;
            _modified = true;
            refreshButton.TabIndex = 0;
            refreshButton.Click += (s, e) => LoadFromHardware();
        }

        /// <summary>
        /// Returns the delta for the optimal width, based upon the number of cores.
        /// </summary>
        public int GetDeltaOptimalWidth()
        {
            return (_optimalWidth - this.Width);
        }

        public string GetECreadings()
        {
            return ECReadings();
        }
        /// <summary>
        /// Loads the P-state settings from each core's MSR.
        /// </summary>
        public void LoadFromHardware()
        {
            RegLabel64CPU.Text = "Bit numbering\nCOFVID 0071\nP-State0 0064\nP-State1 0065\nP-State2 0066\nP-State3 0067\nP-State4 0068\nP-State5 0069\nP-State6 006A\nP-State7 006B";
            Reg64CPU.Text = "63     59     55     51     47     43     39     35     31     27     23     19     15     11     7       3    0\n"
                + COFVidString() + "\n" + CPUPstate0() + "\n" + CPUPstate1() + "\n" + CPUPstate2() + "\n" + CPUPstate3() + "\n" + CPUPstate4() + "\n" + CPUPstate5() + "\n" + CPUPstate6() + "\n" + CPUPstate7();
            RegLabel32NB.Text = "Bit numbering\nNB P-State0 D18F3xDC\nNB P-State1 D18F6x90\nClockTiming D18F3xD4\nBIOSClock D0F0xE4_x0130_80F1";
            Reg32NB.Text = "31     27     23     19     15     11     7       3    0\n" + NBPstate0() + "\n" + NBPstate1() + "\n" + ClockTiming() + "\n" + BIOSClock();
            PCIDevicesLabel.Text = "D18F3x15C\nD0 00\nD1F0 90\nSMBus A0\nD18 C0\nMSRC001_0061 P-State\nBIOS vendor\nBIOS version\nMoBo vendor\nMoBo name";
            PCIDevices.Text = VoltageControl() + "\n" + DebugOutput() + "\n" + MaxPstate() + "\n" + GetReport();
            //PCIDevices.Text = IOInterface();
            /*PStateReg1.Text = "";
            PStateReg2.Text = "";
            NbPStateReg1.Text = "";
            ClockReg.Text = "";
            BIOSReg.Text = "";
            //RegLabel64CPU.Text = "";
            //RegLabel32NB.Text = "";
            //PCIDevicesLabel.Text = "IOOutput";*/
            _modified = false;
        }

        private string Convert32IntToHex(uint Value)
        {
            string[] tmp = new string[8];
            string conv = "";
            for (int i = 0; i < 8; i++)
            {
                uint tmpvalue = ((Value >> (i * 4)) & 0xF);
                if (tmpvalue < 10) tmp[i] = tmpvalue.ToString();
                else if (tmpvalue == 10) tmp[i] = "A";
                else if (tmpvalue == 11) tmp[i] = "B";
                else if (tmpvalue == 12) tmp[i] = "C";
                else if (tmpvalue == 13) tmp[i] = "D";
                else if (tmpvalue == 14) tmp[i] = "E";
                else if (tmpvalue == 15) tmp[i] = "F";
                else tmp[i] = "X";
            }
                
            for (int i = 0; i < 8; i++)
            {
                conv += tmp[7 - i];
                if ((i + 1) % 4 == 0) conv += " ";
            }
            conv += "h"; 
            return conv;
        }

        private bool WaitPortStatus(int bits, int onoff = 0, int timeout = 1000)
        {
            bool result = false;
            ushort port = EC_CTRLPORT;
            int tick = 10;
            
            for (int time = 0; time < timeout; time += tick)
            {
                byte data = Program.Ols.ReadIoPortByte(port);
                int flagstate = (data & bits);
                if (flagstate == onoff)
                {
                    result = true;
                    break;
                }
                Thread.Sleep(tick);
            }
            return result;
        }

        private uint ReadWord(ushort regPort, ushort valPort, byte regIndex)
        {
            Program.Ols.WriteIoPortByte(regPort, regIndex);
            uint value = (uint)Program.Ols.ReadIoPortByte(valPort) << 8;
            regIndex++;
            Program.Ols.WriteIoPortByte(regPort, regIndex);
            value |= Program.Ols.ReadIoPortByte(valPort);

            return value;
        }

        private void WriteByte(ushort regPort, ushort valPort, byte regIndex, byte value)
        {
            Program.Ols.WriteIoPortByte(regPort, regIndex);
            Program.Ols.WriteIoPortByte(valPort, value);
        }


        public byte ReadIOByte(byte offset)
        {
            byte data = 0;
            // wait for IBF and OBF to clear
            bool ready = WaitPortStatus(EC_STAT_IBF | EC_STAT_OBF, 0);
            if (ready)
            {
                // tell 'em we want to "READ"
                Program.Ols.WriteIoPortByte(EC_CTRLPORT, EC_CTRLPORT_READ);
                // wait for IBF to clear (command byte removed from EC's input queue)
                ready = WaitPortStatus(EC_STAT_IBF, 0);
                if (ready)
                {
                    // tell 'em where we want to read from
                    Program.Ols.WriteIoPortByte(EC_DATAPORT, offset);
                    ready = WaitPortStatus(EC_STAT_IBF, 0);
                    if (ready)
                    {
                        // read result (EC byte at offset)
                        data = Program.Ols.ReadIoPortByte(EC_DATAPORT);
                    }
                }
            }
            return data;
        }
			
		public string IOInterface()
        {
            string text = "";
            byte loFan = ReadIOByte(TP_ECOFFSET_FANSPEED);
            byte hiFan = ReadIOByte((byte)(TP_ECOFFSET_FANSPEED + 1));
            //ushort fanSpeed = (ushort)((hiFan << 8) | (loFan & 0xFF));
            //if (fanSpeed != 0) { fanSpeed = (ushort)(1350000 / (fanSpeed * 2)); }
            text += "Fan1: " + loFan + " Fan2: " + hiFan; 
            for (int i = 0; i < 3; i++)
            { // temp sensors 0x78 - 0x7f
                text += " " + i + ": " + ReadIOByte(SENSORADDR[i]);
            }
            /*for (int i = 0; i < 8; i++)
            {
                text += (done >> (7 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }*/
            return text;
        }

        public string ECReadings()
        {
            if (GetVendor().Equals("LENOVO") && GetMobo().Equals("Inagua"))
            {
                string text = "";
                //temps 0xA8, 0xA9, 0xAA
                text += " APU: " + ReadIOByte(SENSORADDR[0]) + "°C ";
                //text += "MB1: " + ReadIOByte(SENSORADDR[1]) + "°C ";
                text += "MoBo: " + ReadIOByte(SENSORADDR[2]) + "°C ";

                byte hiFan = ReadIOByte((byte)(TP_ECOFFSET_FANSPEED + 1));
                int fanspeed = 0;
                if (hiFan != 255) { fanspeed = 8500 - (50 * hiFan); }
                text += "Fan: " + fanspeed + " RPM";

                /*for (int i = 0; i < 8; i++)
                {
                    text += (done >> (7 - i) & 0x1).ToString();
                    if ((i + 1) % 4 == 0) text += " ";
                }*/
                return text;
            }
            else
            {
                return "Temp/Fan on " + GetVendor() + "/" + GetMobo() + " not supported yet.";
            }
        }

        public string DebugOutput()
        {
            string text = "";
            uint settings = Program.Ols.ReadPciConfig(0x00, 0x00);
            text += Convert32IntToHex(settings) + "\n"; 
            settings = Program.Ols.ReadPciConfig(0x90, 0x00);
            text += Convert32IntToHex(settings) + "\n";
            settings = Program.Ols.ReadPciConfig(0xA0, 0x00);
            text += Convert32IntToHex(settings) + "\n";
            settings = Program.Ols.ReadPciConfig(0xC0, 0x00);
            text += Convert32IntToHex(settings);
            return text;
        }

        public string VoltageControl()
        {
            string text = "";
            uint settings = Program.Ols.ReadPciConfig(0xC3, 0x15C);
            double vidl0 = 1.55 - ((settings >> 0 & 0x7F) * 0.0125);
            double vidl1 = 1.55 - ((settings >> 8 & 0x7F) * 0.0125);
            double vidl2 = 1.55 - ((settings >> 16 & 0x7F) * 0.0125);
            double vidl3 = 1.55 - ((settings >> 24 & 0x7F) * 0.0125);

            text += "VidL0 " + vidl0 + "V VidL1 " + vidl1 + "V VidL2 " + vidl2 + "V VidL3 " + vidl3 + "V";
            return text;
        }

        public string MaxPstate()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC0010061u);
            uint maxP = (uint)(msr >> 4 & 0x7);
            uint minP = (uint)(msr & 0x7);
            text += "MaxPState: " + maxP + " MinPState: " + minP;
            return text;
        }
        
        public string COFVidString()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC0010071u);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }
        
        public string CPUPstate0()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC0010064u);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string CPUPstate1()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC0010065u);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string CPUPstate2()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC0010066u);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string CPUPstate3()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC0010067u);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string CPUPstate4()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC0010068u);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string CPUPstate5()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC0010069u);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string CPUPstate6()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC001006Au);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string CPUPstate7()
        {
            string text = "";
            ulong msr = Program.Ols.ReadMsr(0xC001006Bu);
            for (int i = 0; i < 64; i++)
            {
                text += (msr >> (63 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string NBPstate0()
        {
            string text = "";
            uint settings = Program.Ols.ReadPciConfig(0xC3, 0xDC);
            for (int i = 0; i < 32; i++)
            {
                text += (settings >> (31 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string NBPstate1()
        {
            string text = "";
            uint settings = Program.Ols.ReadPciConfig(0xC6, 0x90);
            for (int i = 0; i < 32; i++)
            {
                text += (settings >> (31 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

        public string ClockTiming()
        {
            string text = "";
            uint settings = Program.Ols.ReadPciConfig(0xC3, 0xD4);
            for (int i = 0; i < 32; i++)
            {
                text += (settings >> (31 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

         public string BIOSClock()
        {
            string text = "";
            Program.Ols.WritePciConfig(0x00, 0xE0, 0x013080F1);
            // value of interest: D0F0xE4_x0130_80F1
            uint settings = Program.Ols.ReadPciConfig(0x00, 0xE4); 
            for (int i = 0; i < 32; i++)
            {
                text += (settings >> (31 - i) & 0x1).ToString();
                if ((i + 1) % 4 == 0) text += " ";
            }
            text += "";
            return text;
        }

         private void ResetButton_Click(object sender, EventArgs e)
         {
             DialogResult result = MessageBox.Show("Do you really want to delete all your customized settings?\n"
                    + "If you want that:\n1. Click OK.\n2. Close the application without hitting \"Apply\"\n3. Restart your system.", "Reset PStates", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning);
             if (result == DialogResult.OK)
             {
                 var key = Microsoft.Win32.Registry.LocalMachine.CreateSubKey(@"Software\BrazosTweaker");
                 if (key == null)
                     return;
                 for (int i = 0; i < 5; i++)
                 {
                     string valueName = "P" + i;
                     key.DeleteValue(valueName, false);
                 }
                 key.SetValue("EnableCustomPStates", 0);
                 key.Close();
             }
         }
    }
}