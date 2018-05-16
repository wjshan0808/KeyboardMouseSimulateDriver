namespace BrazosTweaker
{
    partial class StatusControl
    {
        /// <summary> 
        /// Erforderliche Designervariable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Verwendete Ressourcen bereinigen.
        /// </summary>
        /// <param name="disposing">True, wenn verwaltete Ressourcen gelöscht werden sollen; andernfalls False.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Vom Komponenten-Designer generierter Code

        /// <summary> 
        /// Erforderliche Methode für die Designerunterstützung. 
        /// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(StatusControl));
            this.refreshButton = new System.Windows.Forms.Button();
            this.Reg64CPU = new System.Windows.Forms.Label();
            this.PCIDevices = new System.Windows.Forms.Label();
            this.PStateReg1 = new System.Windows.Forms.Label();
            this.PStateReg2 = new System.Windows.Forms.Label();
            this.Reg32NB = new System.Windows.Forms.Label();
            this.NbPStateReg1 = new System.Windows.Forms.Label();
            this.ClockReg = new System.Windows.Forms.Label();
            this.BIOSReg = new System.Windows.Forms.Label();
            this.RegLabel64CPU = new System.Windows.Forms.Label();
            this.RegLabel32NB = new System.Windows.Forms.Label();
            this.PCIDevicesLabel = new System.Windows.Forms.Label();
            this.RegLabel3 = new System.Windows.Forms.Label();
            this.RegLabel4 = new System.Windows.Forms.Label();
            this.RegLabel5 = new System.Windows.Forms.Label();
            this.RegLabel12 = new System.Windows.Forms.Label();
            this.RegLabel13 = new System.Windows.Forms.Label();
            this.RegLabel6 = new System.Windows.Forms.Label();
            this.RegLabel7 = new System.Windows.Forms.Label();
            this.RegLabel8 = new System.Windows.Forms.Label();
            this.RegLabel9 = new System.Windows.Forms.Label();
            this.RegLabel10 = new System.Windows.Forms.Label();
            this.RegLabel11 = new System.Windows.Forms.Label();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.tableLayoutPanelStatus = new System.Windows.Forms.TableLayoutPanel();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.tableLayoutPanelStatus.SuspendLayout();
            this.SuspendLayout();
            // 
            // refreshButton
            // 
            this.refreshButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.refreshButton.AutoSize = true;
            this.refreshButton.Image = global::BrazosTweaker.Properties.Resources.refresh;
            this.refreshButton.Location = new System.Drawing.Point(220, 40);
            this.refreshButton.Margin = new System.Windows.Forms.Padding(4);
            this.refreshButton.Name = "refreshButton";
            this.refreshButton.Padding = new System.Windows.Forms.Padding(4, 0, 0, 0);
            this.refreshButton.Size = new System.Drawing.Size(120, 33);
            this.refreshButton.TabIndex = 0;
            this.refreshButton.Text = "Refresh";
            this.refreshButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
            this.toolTip1.SetToolTip(this.refreshButton, "Load the current settings directly from the CPU registers.");
            this.refreshButton.UseVisualStyleBackColor = true;
            // 
            // Reg64CPU
            // 
            this.Reg64CPU.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.Reg64CPU.AutoSize = true;
            this.Reg64CPU.Location = new System.Drawing.Point(4, 0);
            this.Reg64CPU.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.Reg64CPU.Name = "Reg64CPU";
            this.Reg64CPU.Size = new System.Drawing.Size(82, 17);
            this.Reg64CPU.TabIndex = 7;
            this.Reg64CPU.Text = "Freq";
            // 
            // PCIDevices
            // 
            this.PCIDevices.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.PCIDevices.AutoSize = true;
            this.PCIDevices.Location = new System.Drawing.Point(4, 0);
            this.PCIDevices.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.PCIDevices.Name = "PCIDevices";
            this.PCIDevices.Size = new System.Drawing.Size(82, 17);
            this.PCIDevices.TabIndex = 7;
            this.PCIDevices.Text = "Freq";
            // 
            // PStateReg1
            // 
            this.PStateReg1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.PStateReg1.AutoSize = true;
            this.PStateReg1.Location = new System.Drawing.Point(4, 0);
            this.PStateReg1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.PStateReg1.Name = "PStateReg1";
            this.PStateReg1.Size = new System.Drawing.Size(82, 17);
            this.PStateReg1.TabIndex = 7;
            this.PStateReg1.Text = "Freq";
            // 
            // PStateReg2
            // 
            this.PStateReg2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.PStateReg2.AutoSize = true;
            this.PStateReg2.Location = new System.Drawing.Point(4, 0);
            this.PStateReg2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.PStateReg2.Name = "PStateReg2";
            this.PStateReg2.Size = new System.Drawing.Size(82, 17);
            this.PStateReg2.TabIndex = 7;
            this.PStateReg2.Text = "Freq";
            // 
            // Reg32NB
            // 
            this.Reg32NB.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.Reg32NB.AutoSize = true;
            this.Reg32NB.Location = new System.Drawing.Point(4, 0);
            this.Reg32NB.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.Reg32NB.Name = "Reg32NB";
            this.Reg32NB.Size = new System.Drawing.Size(82, 17);
            this.Reg32NB.TabIndex = 7;
            this.Reg32NB.Text = "Freq";
            // 
            // NbPStateReg1
            // 
            this.NbPStateReg1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.NbPStateReg1.AutoSize = true;
            this.NbPStateReg1.Location = new System.Drawing.Point(4, 0);
            this.NbPStateReg1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.NbPStateReg1.Name = "NbPStateReg1";
            this.NbPStateReg1.Size = new System.Drawing.Size(82, 17);
            this.NbPStateReg1.TabIndex = 7;
            this.NbPStateReg1.Text = "Freq";
            // 
            // ClockReg
            // 
            this.ClockReg.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.ClockReg.AutoSize = true;
            this.ClockReg.Location = new System.Drawing.Point(4, 0);
            this.ClockReg.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.ClockReg.Name = "ClockReg";
            this.ClockReg.Size = new System.Drawing.Size(82, 17);
            this.ClockReg.TabIndex = 7;
            this.ClockReg.Text = "Freq";
            // 
            // BIOSReg
            // 
            this.BIOSReg.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.BIOSReg.AutoSize = true;
            this.BIOSReg.Location = new System.Drawing.Point(4, 0);
            this.BIOSReg.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.BIOSReg.Name = "BIOSReg";
            this.BIOSReg.Size = new System.Drawing.Size(82, 17);
            this.BIOSReg.TabIndex = 7;
            this.BIOSReg.Text = "Freq";
            // 
            // RegLabel64CPU
            // 
            this.RegLabel64CPU.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel64CPU.AutoSize = true;
            this.RegLabel64CPU.Location = new System.Drawing.Point(4, 0);
            this.RegLabel64CPU.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel64CPU.Name = "RegLabel64CPU";
            this.RegLabel64CPU.Size = new System.Drawing.Size(82, 17);
            this.RegLabel64CPU.TabIndex = 7;
            this.RegLabel64CPU.Text = "Freq";
            // 
            // RegLabel32NB
            // 
            this.RegLabel32NB.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel32NB.AutoSize = true;
            this.RegLabel32NB.Location = new System.Drawing.Point(4, 0);
            this.RegLabel32NB.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel32NB.Name = "RegLabel32NB";
            this.RegLabel32NB.Size = new System.Drawing.Size(82, 17);
            this.RegLabel32NB.TabIndex = 7;
            this.RegLabel32NB.Text = "Freq";
            // 
            // PCIDevicesLabel
            // 
            this.PCIDevicesLabel.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.PCIDevicesLabel.AutoSize = true;
            this.PCIDevicesLabel.Location = new System.Drawing.Point(4, 0);
            this.PCIDevicesLabel.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.PCIDevicesLabel.Name = "PCIDevicesLabel";
            this.PCIDevicesLabel.Size = new System.Drawing.Size(82, 17);
            this.PCIDevicesLabel.TabIndex = 7;
            this.PCIDevicesLabel.Text = "Freq";
            // 
            // RegLabel3
            // 
            this.RegLabel3.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel3.AutoSize = true;
            this.RegLabel3.Location = new System.Drawing.Point(4, 0);
            this.RegLabel3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel3.Name = "RegLabel3";
            this.RegLabel3.Size = new System.Drawing.Size(82, 17);
            this.RegLabel3.TabIndex = 7;
            this.RegLabel3.Text = "Freq";
            // 
            // RegLabel4
            // 
            this.RegLabel4.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel4.AutoSize = true;
            this.RegLabel4.Location = new System.Drawing.Point(4, 0);
            this.RegLabel4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel4.Name = "RegLabel4";
            this.RegLabel4.Size = new System.Drawing.Size(82, 17);
            this.RegLabel4.TabIndex = 7;
            this.RegLabel4.Text = "Freq";
            // 
            // RegLabel5
            // 
            this.RegLabel5.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel5.AutoSize = true;
            this.RegLabel5.Location = new System.Drawing.Point(4, 0);
            this.RegLabel5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel5.Name = "RegLabel5";
            this.RegLabel5.Size = new System.Drawing.Size(82, 17);
            this.RegLabel5.TabIndex = 7;
            this.RegLabel5.Text = "Freq";
            // 
            // RegLabel12
            // 
            this.RegLabel12.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel12.AutoSize = true;
            this.RegLabel12.Location = new System.Drawing.Point(4, 0);
            this.RegLabel12.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel12.Name = "RegLabel12";
            this.RegLabel12.Size = new System.Drawing.Size(82, 17);
            this.RegLabel12.TabIndex = 7;
            this.RegLabel12.Text = "Freq";
            // 
            // RegLabel13
            // 
            this.RegLabel13.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel13.AutoSize = true;
            this.RegLabel13.Location = new System.Drawing.Point(4, 0);
            this.RegLabel13.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel13.Name = "RegLabel13";
            this.RegLabel13.Size = new System.Drawing.Size(82, 17);
            this.RegLabel13.TabIndex = 7;
            this.RegLabel13.Text = "Freq";
            // 
            // RegLabel6
            // 
            this.RegLabel6.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel6.AutoSize = true;
            this.RegLabel6.Location = new System.Drawing.Point(4, 0);
            this.RegLabel6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel6.Name = "RegLabel6";
            this.RegLabel6.Size = new System.Drawing.Size(82, 17);
            this.RegLabel6.TabIndex = 7;
            this.RegLabel6.Text = "Freq";
            // 
            // RegLabel7
            // 
            this.RegLabel7.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel7.AutoSize = true;
            this.RegLabel7.Location = new System.Drawing.Point(4, 0);
            this.RegLabel7.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel7.Name = "RegLabel7";
            this.RegLabel7.Size = new System.Drawing.Size(82, 17);
            this.RegLabel7.TabIndex = 7;
            this.RegLabel7.Text = "Freq";
            // 
            // RegLabel8
            // 
            this.RegLabel8.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel8.AutoSize = true;
            this.RegLabel8.Location = new System.Drawing.Point(4, 0);
            this.RegLabel8.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel8.Name = "RegLabel8";
            this.RegLabel8.Size = new System.Drawing.Size(82, 17);
            this.RegLabel8.TabIndex = 7;
            this.RegLabel8.Text = "Freq";
            // 
            // RegLabel9
            // 
            this.RegLabel9.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel9.AutoSize = true;
            this.RegLabel9.Location = new System.Drawing.Point(4, 0);
            this.RegLabel9.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel9.Name = "RegLabel9";
            this.RegLabel9.Size = new System.Drawing.Size(82, 17);
            this.RegLabel9.TabIndex = 7;
            this.RegLabel9.Text = "Freq";
            // 
            // RegLabel10
            // 
            this.RegLabel10.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel10.AutoSize = true;
            this.RegLabel10.Location = new System.Drawing.Point(4, 0);
            this.RegLabel10.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel10.Name = "RegLabel10";
            this.RegLabel10.Size = new System.Drawing.Size(82, 17);
            this.RegLabel10.TabIndex = 7;
            this.RegLabel10.Text = "Freq";
            // 
            // RegLabel11
            // 
            this.RegLabel11.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.RegLabel11.AutoSize = true;
            this.RegLabel11.Location = new System.Drawing.Point(4, 0);
            this.RegLabel11.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.RegLabel11.Name = "RegLabel11";
            this.RegLabel11.Size = new System.Drawing.Size(82, 17);
            this.RegLabel11.TabIndex = 7;
            this.RegLabel11.Text = "Freq";

            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 30000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.IsBalloon = true;
            this.toolTip1.ReshowDelay = 500;
            // 
            // tableLayoutPanelStatus
            // 
            this.tableLayoutPanelStatus.AutoSize = true;
            this.tableLayoutPanelStatus.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.tableLayoutPanelStatus.ColumnCount = 3;
            this.tableLayoutPanelStatus.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanelStatus.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanelStatus.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanelStatus.Controls.Add(this.Reg64CPU, 1, 0); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.Reg32NB, 1, 1); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel64CPU, 0, 0); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel32NB, 0, 1); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.PCIDevicesLabel, 0, 2); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.PCIDevices, 1, 2); //Column / Row
            //this.tableLayoutPanelStatus.Controls.Add(this.PStateReg1, 1, 2); //Column / Row
            //this.tableLayoutPanelStatus.Controls.Add(this.PStateReg2, 1, 3); //Column / Row
            //this.tableLayoutPanelStatus.Controls.Add(this.NbPStateReg1, 1, 5); //Column / Row
            //this.tableLayoutPanelStatus.Controls.Add(this.ClockReg, 1, 6); //Column / Row
            //this.tableLayoutPanelStatus.Controls.Add(this.BIOSReg, 1, 7); //Column / Row
            /*this.tableLayoutPanelStatus.Controls.Add(this.RegLabel3, 0, 3); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel4, 0, 4); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel5, 0, 5); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel12, 0, 6); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel13, 0, 7); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel6, 0, 8); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel7, 0, 9); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel8, 0, 10); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel9, 0, 11); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel10, 0, 12); //Column / Row
            this.tableLayoutPanelStatus.Controls.Add(this.RegLabel11, 0, 13); //Column / Row
             * */
            this.tableLayoutPanelStatus.Controls.Add(this.refreshButton, 2, 0);
            this.tableLayoutPanelStatus.Dock = System.Windows.Forms.DockStyle.Top;
            this.tableLayoutPanelStatus.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanelStatus.Margin = new System.Windows.Forms.Padding(0);
            this.tableLayoutPanelStatus.Name = "tableLayoutPanelStatus";
            this.tableLayoutPanelStatus.RowCount = 3;
            this.tableLayoutPanelStatus.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanelStatus.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanelStatus.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanelStatus.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 25F));
            this.tableLayoutPanelStatus.Size = new System.Drawing.Size(344, 77);
            this.tableLayoutPanelStatus.TabIndex = 1;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.flowLayoutPanel1.AutoSize = true;
            this.flowLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.tableLayoutPanelStatus.SetColumnSpan(this.flowLayoutPanel1, 2);
            this.flowLayoutPanel1.Location = new System.Drawing.Point(90, 8);
            this.flowLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(254, 0);
            this.flowLayoutPanel1.TabIndex = 0;
            // 
            // PStateControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tableLayoutPanelStatus);
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "PStateControl";
            this.Size = new System.Drawing.Size(344, 90);
            this.tableLayoutPanelStatus.ResumeLayout(false);
            this.tableLayoutPanelStatus.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        #endregion

        private System.Windows.Forms.Button refreshButton;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelStatus;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.Label Reg64CPU;
        private System.Windows.Forms.Label PCIDevices;
        private System.Windows.Forms.Label PStateReg1;
        private System.Windows.Forms.Label PStateReg2;
        private System.Windows.Forms.Label Reg32NB;
        private System.Windows.Forms.Label NbPStateReg1;
        private System.Windows.Forms.Label ClockReg;
        private System.Windows.Forms.Label BIOSReg;
        private System.Windows.Forms.Label RegLabel64CPU;
        private System.Windows.Forms.Label RegLabel32NB;
        private System.Windows.Forms.Label PCIDevicesLabel;
        private System.Windows.Forms.Label RegLabel3;
        private System.Windows.Forms.Label RegLabel4;
        private System.Windows.Forms.Label RegLabel5;
        private System.Windows.Forms.Label RegLabel12;
        private System.Windows.Forms.Label RegLabel13;
        private System.Windows.Forms.Label RegLabel6;
        private System.Windows.Forms.Label RegLabel7;
        private System.Windows.Forms.Label RegLabel8;
        private System.Windows.Forms.Label RegLabel9;
        private System.Windows.Forms.Label RegLabel10;
        private System.Windows.Forms.Label RegLabel11;
    }
}
