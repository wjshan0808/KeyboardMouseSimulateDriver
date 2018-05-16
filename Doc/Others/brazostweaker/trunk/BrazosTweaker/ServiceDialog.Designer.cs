namespace BrazosTweaker
{
	partial class ServiceDialog
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            this.makePermanentCheckBox = new System.Windows.Forms.CheckBox();
            this.pStatesLabel = new System.Windows.Forms.Label();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.enableCustomCnQCheckBox = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.updateButton = new System.Windows.Forms.Button();
            this.applyButton = new System.Windows.Forms.Button();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.balancedProfileControl = new BrazosTweaker.CnQProfileControl();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.highPerformanceProfileControl = new BrazosTweaker.CnQProfileControl();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.powerSaverProfileControl = new BrazosTweaker.CnQProfileControl();
            this.turboCheckBox = new System.Windows.Forms.CheckBox();
            this.button1 = new System.Windows.Forms.Button();
            this.serviceController1 = new System.ServiceProcess.ServiceController();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.flowLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // makePermanentCheckBox
            // 
            this.makePermanentCheckBox.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.makePermanentCheckBox.AutoSize = true;
            this.tableLayoutPanel1.SetColumnSpan(this.makePermanentCheckBox, 3);
            this.makePermanentCheckBox.Location = new System.Drawing.Point(3, 3);
            this.makePermanentCheckBox.Name = "makePermanentCheckBox";
            this.makePermanentCheckBox.Size = new System.Drawing.Size(218, 17);
            this.makePermanentCheckBox.TabIndex = 0;
            this.makePermanentCheckBox.Text = "Make custom P-state settings permanent";
            this.toolTip1.SetToolTip(this.makePermanentCheckBox, "Apply custom P-state settings when the system is started/resumed.");
            this.makePermanentCheckBox.UseVisualStyleBackColor = true;
            // 
            // pStatesLabel
            // 
            this.pStatesLabel.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.pStatesLabel.AutoSize = true;
            this.pStatesLabel.Location = new System.Drawing.Point(27, 26);
            this.pStatesLabel.Margin = new System.Windows.Forms.Padding(3);
            this.pStatesLabel.Name = "pStatesLabel";
            this.pStatesLabel.Size = new System.Drawing.Size(199, 65);
            this.pStatesLabel.TabIndex = 1;
            this.pStatesLabel.Text = "P0: 2|2 @ 1.225V/100MHz/1600MHz\r\nP1: 2.5|2.5 @ 1.05V/100MHz/1280MHz\r\nP2: 4|4 @ 0." +
                "85V/100MHz/800MHz\r\nP3: 6|6 @ 0.85V/100MHz/533MHz\r\nP4: 9.5|9.5 @ 0.8V/100MHz/336M" +
                "Hz\r\n";
            // 
            // toolTip1
            // 
            this.toolTip1.AutoPopDelay = 30000;
            this.toolTip1.InitialDelay = 500;
            this.toolTip1.IsBalloon = true;
            this.toolTip1.ReshowDelay = 500;
            // 
            // enableCustomCnQCheckBox
            // 
            this.enableCustomCnQCheckBox.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.enableCustomCnQCheckBox.AutoSize = true;
            this.tableLayoutPanel1.SetColumnSpan(this.enableCustomCnQCheckBox, 3);
            this.enableCustomCnQCheckBox.Location = new System.Drawing.Point(3, 144);
            this.enableCustomCnQCheckBox.Margin = new System.Windows.Forms.Padding(3, 15, 3, 3);
            this.enableCustomCnQCheckBox.Name = "enableCustomCnQCheckBox";
            this.enableCustomCnQCheckBox.Size = new System.Drawing.Size(157, 17);
            this.enableCustomCnQCheckBox.TabIndex = 4;
            this.enableCustomCnQCheckBox.Text = "Enable custom Cool & Quiet";
            this.toolTip1.SetToolTip(this.enableCustomCnQCheckBox, "Let BrazosTweaker handle P-state transitions.\r\n \r\nCannot be used with hardware Tu" +
                    "rbo.");
            this.enableCustomCnQCheckBox.UseMnemonic = false;
            this.enableCustomCnQCheckBox.UseVisualStyleBackColor = true;
            // 
            // label3
            // 
            this.label3.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.label3.AutoSize = true;
            this.tableLayoutPanel1.SetColumnSpan(this.label3, 2);
            this.label3.ForeColor = System.Drawing.Color.DarkRed;
            this.label3.Location = new System.Drawing.Point(27, 170);
            this.label3.Margin = new System.Windows.Forms.Padding(3, 6, 3, 6);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(357, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "IMPORTANT: Disable Cool & Quiet in the BIOS menu for proper operation!";
            this.toolTip1.SetToolTip(this.label3, "Otherwise the Windows CPU driver will interfere with BrazosTweaker.\r\nYou can alte" +
                    "rnatively set the CPU % range in every Windows power scheme to 100%.");
            this.label3.UseMnemonic = false;
            // 
            // updateButton
            // 
            this.updateButton.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.updateButton.AutoSize = true;
            this.updateButton.Enabled = false;
            this.updateButton.Image = global::BrazosTweaker.Properties.Resources.refresh;
            this.updateButton.Location = new System.Drawing.Point(297, 45);
            this.updateButton.Name = "updateButton";
            this.updateButton.Padding = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.updateButton.Size = new System.Drawing.Size(90, 27);
            this.updateButton.TabIndex = 2;
            this.updateButton.Text = "&Update";
            this.updateButton.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
            this.toolTip1.SetToolTip(this.updateButton, "Use the current settings from the CPU registers.");
            this.updateButton.UseVisualStyleBackColor = true;
            this.updateButton.Click += new System.EventHandler(this.updateButton_Click);
            // 
            // applyButton
            // 
            this.applyButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.applyButton.AutoSize = true;
            this.applyButton.Location = new System.Drawing.Point(307, 392);
            this.applyButton.Margin = new System.Windows.Forms.Padding(3, 12, 3, 3);
            this.applyButton.Name = "applyButton";
            this.applyButton.Size = new System.Drawing.Size(80, 27);
            this.applyButton.TabIndex = 7;
            this.applyButton.Text = "&Apply";
            this.toolTip1.SetToolTip(this.applyButton, "Writes the settings to the registry and (re)starts the service.");
            this.applyButton.UseVisualStyleBackColor = true;
            this.applyButton.Click += new System.EventHandler(this.applyButton_Click);
            // 
            // tabControl1
            // 
            this.tableLayoutPanel1.SetColumnSpan(this.tabControl1, 2);
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Enabled = false;
            this.tabControl1.Location = new System.Drawing.Point(27, 192);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(360, 185);
            this.tabControl1.TabIndex = 6;
            this.toolTip1.SetToolTip(this.tabControl1, "Custom C&Q settings for each power scheme.");
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.balancedProfileControl);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(352, 159);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Balanced";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // balancedProfileControl
            // 
            this.balancedProfileControl.AutoSize = true;
            this.balancedProfileControl.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.balancedProfileControl.Location = new System.Drawing.Point(3, 3);
            this.balancedProfileControl.Margin = new System.Windows.Forms.Padding(4);
            this.balancedProfileControl.Name = "balancedProfileControl";
            this.balancedProfileControl.Profile = BrazosTweaker.CnQProfile.Balanced;
            this.balancedProfileControl.Size = new System.Drawing.Size(293, 150);
            this.balancedProfileControl.TabIndex = 0;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.highPerformanceProfileControl);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(352, 159);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "High performance";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // highPerformanceProfileControl
            // 
            this.highPerformanceProfileControl.AutoSize = true;
            this.highPerformanceProfileControl.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.highPerformanceProfileControl.Location = new System.Drawing.Point(3, 3);
            this.highPerformanceProfileControl.Margin = new System.Windows.Forms.Padding(4);
            this.highPerformanceProfileControl.Name = "highPerformanceProfileControl";
            this.highPerformanceProfileControl.Profile = BrazosTweaker.CnQProfile.HighPerformance;
            this.highPerformanceProfileControl.Size = new System.Drawing.Size(293, 150);
            this.highPerformanceProfileControl.TabIndex = 0;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.powerSaverProfileControl);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(352, 159);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Power saver";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // powerSaverProfileControl
            // 
            this.powerSaverProfileControl.AutoSize = true;
            this.powerSaverProfileControl.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.powerSaverProfileControl.Location = new System.Drawing.Point(3, 3);
            this.powerSaverProfileControl.Margin = new System.Windows.Forms.Padding(4);
            this.powerSaverProfileControl.Name = "powerSaverProfileControl";
            this.powerSaverProfileControl.Profile = BrazosTweaker.CnQProfile.PowerSaver;
            this.powerSaverProfileControl.Size = new System.Drawing.Size(293, 150);
            this.powerSaverProfileControl.TabIndex = 0;
            // 
            // turboCheckBox
            // 
            this.turboCheckBox.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.turboCheckBox.AutoSize = true;
            this.turboCheckBox.Location = new System.Drawing.Point(3, 3);
            this.turboCheckBox.Name = "turboCheckBox";
            this.turboCheckBox.Size = new System.Drawing.Size(137, 17);
            this.turboCheckBox.TabIndex = 0;
            this.turboCheckBox.Text = "Enable hardware Turbo";
            this.toolTip1.SetToolTip(this.turboCheckBox, "Enable the Turbo. By disabling it, P0 can be used as\r\nnormal P-state.");
            this.turboCheckBox.UseVisualStyleBackColor = true;
            // 
            // button1
            // 
            this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.button1.AutoSize = true;
            this.button1.Location = new System.Drawing.Point(193, 392);
            this.button1.Margin = new System.Windows.Forms.Padding(3, 12, 3, 3);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(98, 27);
            this.button1.TabIndex = 8;
            this.button1.Text = "Reset PStates";
            this.toolTip1.SetToolTip(this.button1, "Removes all customized settings (voltages/dividers) from registry to start over w" +
                    "ith default settings.");
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // serviceController1
            // 
            this.serviceController1.ServiceName = "BrazosTweaker";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel1.ColumnCount = 3;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 24F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.Controls.Add(this.makePermanentCheckBox, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.applyButton, 2, 6);
            this.tableLayoutPanel1.Controls.Add(this.tabControl1, 1, 5);
            this.tableLayoutPanel1.Controls.Add(this.updateButton, 2, 1);
            this.tableLayoutPanel1.Controls.Add(this.pStatesLabel, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.label3, 1, 4);
            this.tableLayoutPanel1.Controls.Add(this.enableCustomCnQCheckBox, 0, 3);
            this.tableLayoutPanel1.Controls.Add(this.flowLayoutPanel1, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.button1, 1, 6);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(12, 12);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 7;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.Size = new System.Drawing.Size(390, 422);
            this.tableLayoutPanel1.TabIndex = 0;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.flowLayoutPanel1.AutoSize = true;
            this.tableLayoutPanel1.SetColumnSpan(this.flowLayoutPanel1, 3);
            this.flowLayoutPanel1.Controls.Add(this.turboCheckBox);
            this.flowLayoutPanel1.Location = new System.Drawing.Point(0, 106);
            this.flowLayoutPanel1.Margin = new System.Windows.Forms.Padding(0, 12, 0, 0);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(143, 23);
            this.flowLayoutPanel1.TabIndex = 3;
            // 
            // ServiceDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(414, 446);
            this.Controls.Add(this.tableLayoutPanel1);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(400, 400);
            this.Name = "ServiceDialog";
            this.ShowInTaskbar = false;
            this.Text = "BrazosTweaker service configuration";
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.flowLayoutPanel1.ResumeLayout(false);
            this.flowLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.CheckBox makePermanentCheckBox;
		private System.Windows.Forms.Label pStatesLabel;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.CheckBox enableCustomCnQCheckBox;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Button applyButton;
		private System.ServiceProcess.ServiceController serviceController1;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.TabPage tabPage3;
		private CnQProfileControl balancedProfileControl;
		private CnQProfileControl highPerformanceProfileControl;
		private CnQProfileControl powerSaverProfileControl;
		private System.Windows.Forms.Button updateButton;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.CheckBox turboCheckBox;
        private System.Windows.Forms.Button button1;
	}
}