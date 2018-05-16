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
            this.refreshButton = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.tableLayoutPanelStatus = new System.Windows.Forms.TableLayoutPanel();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.Reg64CPU = new System.Windows.Forms.Label();
            this.RegLabel64CPU = new System.Windows.Forms.Label();
            this.ResetButton = new System.Windows.Forms.Button();
            this.Reg32NB = new System.Windows.Forms.Label();
            this.RegLabel32NB = new System.Windows.Forms.Label();
            this.PCIDevicesLabel = new System.Windows.Forms.Label();
            this.PCIDevices = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // refreshButton
            // 
            this.refreshButton.AutoSize = true;
            this.refreshButton.Image = global::BrazosTweaker.Properties.Resources.refresh;
            this.refreshButton.Location = new System.Drawing.Point(13, 14);
            this.refreshButton.Name = "refreshButton";
            this.refreshButton.Padding = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.refreshButton.Size = new System.Drawing.Size(90, 27);
            this.refreshButton.TabIndex = 0;
            this.refreshButton.Text = "Refresh";
            this.refreshButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
            this.toolTip1.SetToolTip(this.refreshButton, "Load the current settings directly from the CPU registers.");
            this.refreshButton.UseVisualStyleBackColor = true;
            // 
            // tableLayoutPanelStatus
            // 
            this.tableLayoutPanelStatus.AutoSize = true;
            this.tableLayoutPanelStatus.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.tableLayoutPanelStatus.ColumnCount = 3;
            this.tableLayoutPanelStatus.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanelStatus.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanelStatus.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanelStatus.Dock = System.Windows.Forms.DockStyle.Top;
            this.tableLayoutPanelStatus.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanelStatus.Margin = new System.Windows.Forms.Padding(0);
            this.tableLayoutPanelStatus.Name = "tableLayoutPanelStatus";
            this.tableLayoutPanelStatus.RowCount = 3;
            this.tableLayoutPanelStatus.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanelStatus.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanelStatus.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanelStatus.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tableLayoutPanelStatus.Size = new System.Drawing.Size(426, 0);
            this.tableLayoutPanelStatus.TabIndex = 1;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.flowLayoutPanel1.AutoSize = true;
            this.flowLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.flowLayoutPanel1.Location = new System.Drawing.Point(90, 8);
            this.flowLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(254, 0);
            this.flowLayoutPanel1.TabIndex = 0;
            // 
            // Reg64CPU
            // 
            this.Reg64CPU.AutoSize = true;
            this.Reg64CPU.Location = new System.Drawing.Point(303, 14);
            this.Reg64CPU.Name = "Reg64CPU";
            this.Reg64CPU.Size = new System.Drawing.Size(61, 13);
            this.Reg64CPU.TabIndex = 2;
            this.Reg64CPU.Text = "Reg64CPU";
            // 
            // RegLabel64CPU
            // 
            this.RegLabel64CPU.AutoSize = true;
            this.RegLabel64CPU.Location = new System.Drawing.Point(128, 14);
            this.RegLabel64CPU.Name = "RegLabel64CPU";
            this.RegLabel64CPU.Size = new System.Drawing.Size(87, 13);
            this.RegLabel64CPU.TabIndex = 4;
            this.RegLabel64CPU.Text = "RegLabel64CPU";
            // 
            // ResetButton
            // 
            this.ResetButton.Location = new System.Drawing.Point(13, 47);
            this.ResetButton.Name = "ResetButton";
            this.ResetButton.Size = new System.Drawing.Size(90, 23);
            this.ResetButton.TabIndex = 5;
            this.ResetButton.Text = "Reset PStates";
            this.toolTip1.SetToolTip(this.ResetButton, "Deletes all currently stored settings from the registry! You need to restart in o" +
                    "rder to get back to stock settings.");
            this.ResetButton.UseVisualStyleBackColor = true;
            this.ResetButton.Click += new System.EventHandler(this.ResetButton_Click);
            // 
            // Reg32NB
            // 
            this.Reg32NB.AutoSize = true;
            this.Reg32NB.Location = new System.Drawing.Point(303, 147);
            this.Reg32NB.Name = "Reg32NB";
            this.Reg32NB.Size = new System.Drawing.Size(54, 13);
            this.Reg32NB.TabIndex = 6;
            this.Reg32NB.Text = "Reg32NB";
            // 
            // RegLabel32NB
            // 
            this.RegLabel32NB.AutoSize = true;
            this.RegLabel32NB.Location = new System.Drawing.Point(129, 147);
            this.RegLabel32NB.Name = "RegLabel32NB";
            this.RegLabel32NB.Size = new System.Drawing.Size(80, 13);
            this.RegLabel32NB.TabIndex = 7;
            this.RegLabel32NB.Text = "RegLabel32NB";
            // 
            // PCIDevicesLabel
            // 
            this.PCIDevicesLabel.AutoSize = true;
            this.PCIDevicesLabel.Location = new System.Drawing.Point(129, 217);
            this.PCIDevicesLabel.Name = "PCIDevicesLabel";
            this.PCIDevicesLabel.Size = new System.Drawing.Size(89, 13);
            this.PCIDevicesLabel.TabIndex = 8;
            this.PCIDevicesLabel.Text = "PCIDevicesLabel";
            // 
            // PCIDevices
            // 
            this.PCIDevices.AutoSize = true;
            this.PCIDevices.Location = new System.Drawing.Point(303, 217);
            this.PCIDevices.Name = "PCIDevices";
            this.PCIDevices.Size = new System.Drawing.Size(63, 13);
            this.PCIDevices.TabIndex = 9;
            this.PCIDevices.Text = "PCIDevices";
            // 
            // StatusControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.PCIDevices);
            this.Controls.Add(this.PCIDevicesLabel);
            this.Controls.Add(this.RegLabel32NB);
            this.Controls.Add(this.Reg32NB);
            this.Controls.Add(this.ResetButton);
            this.Controls.Add(this.RegLabel64CPU);
            this.Controls.Add(this.Reg64CPU);
            this.Controls.Add(this.refreshButton);
            this.Controls.Add(this.tableLayoutPanelStatus);
            this.Name = "StatusControl";
            this.Size = new System.Drawing.Size(426, 269);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button refreshButton;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelStatus;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.Label Reg64CPU;
        private System.Windows.Forms.Label RegLabel64CPU;
        private System.Windows.Forms.Button ResetButton;
        private System.Windows.Forms.Label Reg32NB;
        private System.Windows.Forms.Label RegLabel32NB;
        private System.Windows.Forms.Label PCIDevicesLabel;
        private System.Windows.Forms.Label PCIDevices;
    }
}
