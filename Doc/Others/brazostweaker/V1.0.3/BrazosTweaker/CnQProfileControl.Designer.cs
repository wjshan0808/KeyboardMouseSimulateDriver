namespace BrazosTweaker
{
	partial class CnQProfileControl
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.label5 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.numSamplesDownNumericUpDown = new System.Windows.Forms.NumericUpDown();
			this.numSamplesUpNumericUpDown = new System.Windows.Forms.NumericUpDown();
			this.thresholdUpNumericUpDown = new System.Windows.Forms.NumericUpDown();
			this.samplingIntervalNumericUpDown = new System.Windows.Forms.NumericUpDown();
			this.label2 = new System.Windows.Forms.Label();
			this.thresholdDownNumericUpDown = new System.Windows.Forms.NumericUpDown();
			this.label6 = new System.Windows.Forms.Label();
			this.aggressiveUpCheckBox = new System.Windows.Forms.CheckBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label8 = new System.Windows.Forms.Label();
			this.maxPStateNumericUpDown = new System.Windows.Forms.NumericUpDown();
			this.label3 = new System.Windows.Forms.Label();
			this.minPStateNumericUpDown = new System.Windows.Forms.NumericUpDown();
			this.gangedRadioButton = new System.Windows.Forms.RadioButton();
			this.ungangedRadioButton = new System.Windows.Forms.RadioButton();
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			this.pictureBox1 = new System.Windows.Forms.PictureBox();
			this.pictureBox2 = new System.Windows.Forms.PictureBox();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.panel1 = new System.Windows.Forms.Panel();
			((System.ComponentModel.ISupportInitialize)(this.numSamplesDownNumericUpDown)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.numSamplesUpNumericUpDown)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.thresholdUpNumericUpDown)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.samplingIntervalNumericUpDown)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.thresholdDownNumericUpDown)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.maxPStateNumericUpDown)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.minPStateNumericUpDown)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
			this.tableLayoutPanel1.SuspendLayout();
			this.panel1.SuspendLayout();
			this.SuspendLayout();
			// 
			// label5
			// 
			this.label5.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(3, 107);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(110, 13);
			this.label5.TabIndex = 11;
			this.label5.Text = "Consecutive samples:";
			// 
			// Reg64CPU
			// 
			this.label4.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(3, 81);
			this.label4.Name = "Reg64CPU";
			this.label4.Size = new System.Drawing.Size(85, 13);
			this.label4.TabIndex = 8;
			this.label4.Text = "Load thresholds:";
			// 
			// numSamplesDownNumericUpDown
			// 
			this.numSamplesDownNumericUpDown.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.numSamplesDownNumericUpDown.Location = new System.Drawing.Point(240, 104);
			this.numSamplesDownNumericUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.numSamplesDownNumericUpDown.Name = "numSamplesDownNumericUpDown";
			this.numSamplesDownNumericUpDown.Size = new System.Drawing.Size(50, 20);
			this.numSamplesDownNumericUpDown.TabIndex = 13;
			this.toolTip1.SetToolTip(this.numSamplesDownNumericUpDown, "Number of consecutive low-load samples causing a\r\ntransition to a higher (lower p" +
					"erformance) P-state.");
			this.numSamplesDownNumericUpDown.Value = new decimal(new int[] {
            8,
            0,
            0,
            0});
			// 
			// numSamplesUpNumericUpDown
			// 
			this.numSamplesUpNumericUpDown.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.numSamplesUpNumericUpDown.Location = new System.Drawing.Point(154, 104);
			this.numSamplesUpNumericUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.numSamplesUpNumericUpDown.Name = "numSamplesUpNumericUpDown";
			this.numSamplesUpNumericUpDown.Size = new System.Drawing.Size(50, 20);
			this.numSamplesUpNumericUpDown.TabIndex = 12;
			this.toolTip1.SetToolTip(this.numSamplesUpNumericUpDown, "Number of consecutive high-load samples causing a\r\ntransition to a lower (higher " +
					"performance) P-state.");
			this.numSamplesUpNumericUpDown.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
			// 
			// thresholdUpNumericUpDown
			// 
			this.thresholdUpNumericUpDown.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.thresholdUpNumericUpDown.Location = new System.Drawing.Point(154, 78);
			this.thresholdUpNumericUpDown.Maximum = new decimal(new int[] {
            99,
            0,
            0,
            0});
			this.thresholdUpNumericUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.thresholdUpNumericUpDown.Name = "thresholdUpNumericUpDown";
			this.thresholdUpNumericUpDown.Size = new System.Drawing.Size(50, 20);
			this.thresholdUpNumericUpDown.TabIndex = 9;
			this.toolTip1.SetToolTip(this.thresholdUpNumericUpDown, "Minimum load threshold for a high-load sample, in percent.\r\nIn ganged mode, this " +
					"applies to the average core load.");
			this.thresholdUpNumericUpDown.Value = new decimal(new int[] {
            25,
            0,
            0,
            0});
			// 
			// samplingIntervalNumericUpDown
			// 
			this.samplingIntervalNumericUpDown.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.samplingIntervalNumericUpDown.Increment = new decimal(new int[] {
            5,
            0,
            0,
            0});
			this.samplingIntervalNumericUpDown.Location = new System.Drawing.Point(154, 26);
			this.samplingIntervalNumericUpDown.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
			this.samplingIntervalNumericUpDown.Minimum = new decimal(new int[] {
            10,
            0,
            0,
            0});
			this.samplingIntervalNumericUpDown.Name = "samplingIntervalNumericUpDown";
			this.samplingIntervalNumericUpDown.Size = new System.Drawing.Size(50, 20);
			this.samplingIntervalNumericUpDown.TabIndex = 3;
			this.toolTip1.SetToolTip(this.samplingIntervalNumericUpDown, "Interval between CPU load measurements, in milliseconds.\r\nA load sample is the av" +
					"erage user load during an interval.");
			this.samplingIntervalNumericUpDown.Value = new decimal(new int[] {
            50,
            0,
            0,
            0});
			// 
			// VIDlabel
			// 
			this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(3, 29);
			this.label2.Name = "VIDlabel";
			this.label2.Size = new System.Drawing.Size(115, 13);
			this.label2.TabIndex = 2;
			this.label2.Text = "Load sampling interval:";
			// 
			// thresholdDownNumericUpDown
			// 
			this.thresholdDownNumericUpDown.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.thresholdDownNumericUpDown.Location = new System.Drawing.Point(240, 78);
			this.thresholdDownNumericUpDown.Maximum = new decimal(new int[] {
            99,
            0,
            0,
            0});
			this.thresholdDownNumericUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.thresholdDownNumericUpDown.Name = "thresholdDownNumericUpDown";
			this.thresholdDownNumericUpDown.Size = new System.Drawing.Size(50, 20);
			this.thresholdDownNumericUpDown.TabIndex = 10;
			this.toolTip1.SetToolTip(this.thresholdDownNumericUpDown, "Maximum load threshold for a low-load sample, in percent.\r\nIn ganged mode, this a" +
					"pplies to the average core load.");
			this.thresholdDownNumericUpDown.Value = new decimal(new int[] {
            20,
            0,
            0,
            0});
			// 
			// label6
			// 
			this.label6.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(3, 132);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(77, 13);
			this.label6.TabIndex = 14;
			this.label6.Text = "Aggressive up:";
			// 
			// aggressiveUpCheckBox
			// 
			this.aggressiveUpCheckBox.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.aggressiveUpCheckBox.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.aggressiveUpCheckBox, 3);
			this.aggressiveUpCheckBox.Location = new System.Drawing.Point(154, 130);
			this.aggressiveUpCheckBox.Name = "aggressiveUpCheckBox";
			this.aggressiveUpCheckBox.Size = new System.Drawing.Size(65, 17);
			this.aggressiveUpCheckBox.TabIndex = 15;
			this.aggressiveUpCheckBox.Text = "Enabled";
			this.toolTip1.SetToolTip(this.aggressiveUpCheckBox, "Descend directly to the lowest (highest performance) P-state.");
			this.aggressiveUpCheckBox.UseVisualStyleBackColor = true;
			// 
			// Cofstate
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 5);
			this.label1.Name = "Cofstate";
			this.label1.Size = new System.Drawing.Size(37, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "Mode:";
			// 
			// label8
			// 
			this.label8.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(210, 55);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(24, 13);
			this.label8.TabIndex = 6;
			this.label8.Text = "-";
			this.label8.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// maxPStateNumericUpDown
			// 
			this.maxPStateNumericUpDown.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.maxPStateNumericUpDown.Location = new System.Drawing.Point(240, 52);
			this.maxPStateNumericUpDown.Maximum = new decimal(new int[] {
            4,
            0,
            0,
            0});
			this.maxPStateNumericUpDown.Name = "maxPStateNumericUpDown";
			this.maxPStateNumericUpDown.Size = new System.Drawing.Size(50, 20);
			this.maxPStateNumericUpDown.TabIndex = 7;
			this.toolTip1.SetToolTip(this.maxPStateNumericUpDown, "Maximum allowed P-state (lowest performance).\r\nIf your BIOS initializes the CPU w" +
					"ith fewer P-states, you need to\r\npermanently customize the additional P-states t" +
					"o unlock them.");
			this.maxPStateNumericUpDown.Value = new decimal(new int[] {
            4,
            0,
            0,
            0});
			// 
			// FSBlabel
			// 
			this.label3.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(3, 55);
			this.label3.Name = "FSBlabel";
			this.label3.Size = new System.Drawing.Size(81, 13);
			this.label3.TabIndex = 4;
			this.label3.Text = "P-state bounds:";
			// 
			// minPStateNumericUpDown
			// 
			this.minPStateNumericUpDown.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.minPStateNumericUpDown.Location = new System.Drawing.Point(154, 52);
			this.minPStateNumericUpDown.Maximum = new decimal(new int[] {
            4,
            0,
            0,
            0});
			this.minPStateNumericUpDown.Name = "minPStateNumericUpDown";
			this.minPStateNumericUpDown.Size = new System.Drawing.Size(50, 20);
			this.minPStateNumericUpDown.TabIndex = 5;
			this.toolTip1.SetToolTip(this.minPStateNumericUpDown, "Minimum allowed P-state (highest performance).\r\nIf your CPU supports Turbo and it" +
					" is enabled, P0 is the\r\nTurbo state which cannot be enforced by software.");
			// 
			// gangedRadioButton
			// 
			this.gangedRadioButton.AutoSize = true;
			this.gangedRadioButton.Checked = true;
			this.gangedRadioButton.Location = new System.Drawing.Point(3, 3);
			this.gangedRadioButton.Name = "gangedRadioButton";
			this.gangedRadioButton.Size = new System.Drawing.Size(63, 17);
			this.gangedRadioButton.TabIndex = 0;
			this.gangedRadioButton.TabStop = true;
			this.gangedRadioButton.Text = "Ganged";
			this.toolTip1.SetToolTip(this.gangedRadioButton, "Keep all cores in the same P-state at a time.");
			this.gangedRadioButton.UseVisualStyleBackColor = true;
			// 
			// ungangedRadioButton
			// 
			this.ungangedRadioButton.AutoSize = true;
			this.ungangedRadioButton.Dock = System.Windows.Forms.DockStyle.Right;
			this.ungangedRadioButton.Location = new System.Drawing.Point(67, 0);
			this.ungangedRadioButton.Name = "ungangedRadioButton";
			this.ungangedRadioButton.Size = new System.Drawing.Size(75, 23);
			this.ungangedRadioButton.TabIndex = 1;
			this.ungangedRadioButton.Text = "Unganged";
			this.toolTip1.SetToolTip(this.ungangedRadioButton, "Manage each core\'s P-state individually.\r\nNot recommended for single-threaded loa" +
					"ds.");
			this.ungangedRadioButton.UseVisualStyleBackColor = true;
			// 
			// toolTip1
			// 
			this.toolTip1.AutoPopDelay = 30000;
			this.toolTip1.InitialDelay = 500;
			this.toolTip1.IsBalloon = true;
			this.toolTip1.ReshowDelay = 500;
			// 
			// pictureBox1
			// 
			this.pictureBox1.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.pictureBox1.Image = global::BrazosTweaker.Properties.Resources.arrow_up_green;
			this.pictureBox1.Location = new System.Drawing.Point(124, 89);
			this.pictureBox1.Name = "pictureBox1";
			this.tableLayoutPanel1.SetRowSpan(this.pictureBox1, 2);
			this.pictureBox1.Size = new System.Drawing.Size(24, 24);
			this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.pictureBox1.TabIndex = 38;
			this.pictureBox1.TabStop = false;
			this.toolTip1.SetToolTip(this.pictureBox1, "Switching to higher performance (lower P-states).");
			// 
			// pictureBox2
			// 
			this.pictureBox2.Anchor = System.Windows.Forms.AnchorStyles.Right;
			this.pictureBox2.Image = global::BrazosTweaker.Properties.Resources.arrow_down_blue;
			this.pictureBox2.Location = new System.Drawing.Point(210, 89);
			this.pictureBox2.Name = "pictureBox2";
			this.tableLayoutPanel1.SetRowSpan(this.pictureBox2, 2);
			this.pictureBox2.Size = new System.Drawing.Size(24, 24);
			this.pictureBox2.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
			this.pictureBox2.TabIndex = 39;
			this.pictureBox2.TabStop = false;
			this.toolTip1.SetToolTip(this.pictureBox2, "Switching to lower performance (higher P-states).");
			// 
			// tableLayoutPanelPstates
			// 
			this.tableLayoutPanel1.AutoSize = true;
			this.tableLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.tableLayoutPanel1.ColumnCount = 5;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 30F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 30F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel1.Controls.Add(this.panel1, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.pictureBox2, 3, 3);
			this.tableLayoutPanel1.Controls.Add(this.pictureBox1, 1, 3);
			this.tableLayoutPanel1.Controls.Add(this.label1, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.label2, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.label3, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.label4, 0, 3);
			this.tableLayoutPanel1.Controls.Add(this.label5, 0, 4);
			this.tableLayoutPanel1.Controls.Add(this.label6, 0, 5);
			this.tableLayoutPanel1.Controls.Add(this.maxPStateNumericUpDown, 4, 2);
			this.tableLayoutPanel1.Controls.Add(this.thresholdDownNumericUpDown, 4, 3);
			this.tableLayoutPanel1.Controls.Add(this.numSamplesDownNumericUpDown, 4, 4);
			this.tableLayoutPanel1.Controls.Add(this.label8, 3, 2);
			this.tableLayoutPanel1.Controls.Add(this.samplingIntervalNumericUpDown, 2, 1);
			this.tableLayoutPanel1.Controls.Add(this.minPStateNumericUpDown, 2, 2);
			this.tableLayoutPanel1.Controls.Add(this.thresholdUpNumericUpDown, 2, 3);
			this.tableLayoutPanel1.Controls.Add(this.numSamplesUpNumericUpDown, 2, 4);
			this.tableLayoutPanel1.Controls.Add(this.aggressiveUpCheckBox, 2, 5);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
			this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(0);
			this.tableLayoutPanel1.Name = "tableLayoutPanelPstates";
			this.tableLayoutPanel1.RowCount = 6;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(293, 150);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// panel1
			// 
			this.panel1.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.panel1, 3);
			this.panel1.Controls.Add(this.gangedRadioButton);
			this.panel1.Controls.Add(this.ungangedRadioButton);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel1.Location = new System.Drawing.Point(151, 0);
			this.panel1.Margin = new System.Windows.Forms.Padding(0);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(142, 23);
			this.panel1.TabIndex = 1;
			// 
			// CnQProfileControl
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.AutoSize = true;
			this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.Controls.Add(this.tableLayoutPanel1);
			this.Name = "CnQProfileControl";
			this.Size = new System.Drawing.Size(293, 150);
			((System.ComponentModel.ISupportInitialize)(this.numSamplesDownNumericUpDown)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.numSamplesUpNumericUpDown)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.thresholdUpNumericUpDown)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.samplingIntervalNumericUpDown)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.thresholdDownNumericUpDown)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.maxPStateNumericUpDown)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.minPStateNumericUpDown)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.NumericUpDown numSamplesDownNumericUpDown;
		private System.Windows.Forms.NumericUpDown numSamplesUpNumericUpDown;
		private System.Windows.Forms.NumericUpDown thresholdUpNumericUpDown;
		private System.Windows.Forms.NumericUpDown samplingIntervalNumericUpDown;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.NumericUpDown thresholdDownNumericUpDown;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.CheckBox aggressiveUpCheckBox;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.NumericUpDown maxPStateNumericUpDown;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.NumericUpDown minPStateNumericUpDown;
		private System.Windows.Forms.RadioButton gangedRadioButton;
		private System.Windows.Forms.RadioButton ungangedRadioButton;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.PictureBox pictureBox2;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Panel panel1;
	}
}
