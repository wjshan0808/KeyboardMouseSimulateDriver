namespace KeyboardMouseSimulator
{
    partial class frmKeyboardMouseSimulator
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
      this.components = new System.ComponentModel.Container();
      this.btnSimulate = new System.Windows.Forms.Button();
      this.rdobtnKeyO = new System.Windows.Forms.RadioButton();
      this.rdobtnKeyG = new System.Windows.Forms.RadioButton();
      this.lblCheckout = new System.Windows.Forms.Label();
      this.nudIntervalMS = new System.Windows.Forms.NumericUpDown();
      this.gbKeyboard = new System.Windows.Forms.GroupBox();
      this.cmsGlobalMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
      this.tsmiWinIO = new System.Windows.Forms.ToolStripMenuItem();
      this.tsmiWinRing0 = new System.Windows.Forms.ToolStripMenuItem();
      this.lblInterval = new System.Windows.Forms.Label();
      this.lblInfoBoard = new System.Windows.Forms.Label();
      this.lblPeriod = new System.Windows.Forms.Label();
      this.nudPeriod = new System.Windows.Forms.NumericUpDown();
      this.lblDuration = new System.Windows.Forms.Label();
      this.nudDurationS = new System.Windows.Forms.NumericUpDown();
      this.gbMouse = new System.Windows.Forms.GroupBox();
      this.rdobtnMouseRight = new System.Windows.Forms.RadioButton();
      this.rdobtnMouseLeft = new System.Windows.Forms.RadioButton();
      ((System.ComponentModel.ISupportInitialize)(this.nudIntervalMS)).BeginInit();
      this.gbKeyboard.SuspendLayout();
      this.cmsGlobalMenu.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.nudPeriod)).BeginInit();
      ((System.ComponentModel.ISupportInitialize)(this.nudDurationS)).BeginInit();
      this.gbMouse.SuspendLayout();
      this.SuspendLayout();
      // 
      // btnSimulate
      // 
      this.btnSimulate.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.btnSimulate.Location = new System.Drawing.Point(316, 145);
      this.btnSimulate.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.btnSimulate.Name = "btnSimulate";
      this.btnSimulate.Size = new System.Drawing.Size(105, 24);
      this.btnSimulate.TabIndex = 0;
      this.btnSimulate.Text = "Simulate";
      this.btnSimulate.UseVisualStyleBackColor = true;
      this.btnSimulate.Click += new System.EventHandler(this.btnSimulate_Click);
      // 
      // rdobtnKeyO
      // 
      this.rdobtnKeyO.AutoSize = true;
      this.rdobtnKeyO.Checked = true;
      this.rdobtnKeyO.Location = new System.Drawing.Point(18, 27);
      this.rdobtnKeyO.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.rdobtnKeyO.Name = "rdobtnKeyO";
      this.rdobtnKeyO.Size = new System.Drawing.Size(65, 16);
      this.rdobtnKeyO.TabIndex = 2;
      this.rdobtnKeyO.TabStop = true;
      this.rdobtnKeyO.Text = "Key [O]";
      this.rdobtnKeyO.UseVisualStyleBackColor = true;
      // 
      // rdobtnKeyG
      // 
      this.rdobtnKeyG.AutoSize = true;
      this.rdobtnKeyG.Location = new System.Drawing.Point(151, 27);
      this.rdobtnKeyG.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.rdobtnKeyG.Name = "rdobtnKeyG";
      this.rdobtnKeyG.Size = new System.Drawing.Size(65, 16);
      this.rdobtnKeyG.TabIndex = 3;
      this.rdobtnKeyG.Text = "Key [G]";
      this.rdobtnKeyG.UseVisualStyleBackColor = true;
      // 
      // lblCheckout
      // 
      this.lblCheckout.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
      this.lblCheckout.Location = new System.Drawing.Point(9, 6);
      this.lblCheckout.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
      this.lblCheckout.Name = "lblCheckout";
      this.lblCheckout.Size = new System.Drawing.Size(295, 16);
      this.lblCheckout.TabIndex = 4;
      this.lblCheckout.Text = "Checkout";
      this.lblCheckout.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
      // 
      // nudIntervalMS
      // 
      this.nudIntervalMS.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.nudIntervalMS.Location = new System.Drawing.Point(316, 109);
      this.nudIntervalMS.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.nudIntervalMS.Maximum = new decimal(new int[] {
            600,
            0,
            0,
            0});
      this.nudIntervalMS.Name = "nudIntervalMS";
      this.nudIntervalMS.ReadOnly = true;
      this.nudIntervalMS.Size = new System.Drawing.Size(105, 21);
      this.nudIntervalMS.TabIndex = 3;
      this.nudIntervalMS.Value = new decimal(new int[] {
            5,
            0,
            0,
            0});
      // 
      // gbKeyboard
      // 
      this.gbKeyboard.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.gbKeyboard.ContextMenuStrip = this.cmsGlobalMenu;
      this.gbKeyboard.Controls.Add(this.rdobtnKeyG);
      this.gbKeyboard.Controls.Add(this.rdobtnKeyO);
      this.gbKeyboard.Location = new System.Drawing.Point(9, 109);
      this.gbKeyboard.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.gbKeyboard.Name = "gbKeyboard";
      this.gbKeyboard.Padding = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.gbKeyboard.Size = new System.Drawing.Size(295, 60);
      this.gbKeyboard.TabIndex = 6;
      this.gbKeyboard.TabStop = false;
      this.gbKeyboard.Text = "eg. Keyboard";
      // 
      // cmsGlobalMenu
      // 
      this.cmsGlobalMenu.ImageScalingSize = new System.Drawing.Size(20, 20);
      this.cmsGlobalMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmiWinIO,
            this.tsmiWinRing0});
      this.cmsGlobalMenu.Name = "cmsGlobalMenu";
      this.cmsGlobalMenu.Size = new System.Drawing.Size(153, 70);
      // 
      // tsmiWinIO
      // 
      this.tsmiWinIO.Checked = true;
      this.tsmiWinIO.CheckOnClick = true;
      this.tsmiWinIO.CheckState = System.Windows.Forms.CheckState.Checked;
      this.tsmiWinIO.Name = "tsmiWinIO";
      this.tsmiWinIO.Size = new System.Drawing.Size(152, 22);
      this.tsmiWinIO.Text = "WinIO";
      this.tsmiWinIO.Click += new System.EventHandler(this.tsmiWinIO_Click);
      // 
      // tsmiWinRing0
      // 
      this.tsmiWinRing0.CheckOnClick = true;
      this.tsmiWinRing0.Name = "tsmiWinRing0";
      this.tsmiWinRing0.Size = new System.Drawing.Size(152, 22);
      this.tsmiWinRing0.Text = "WinRing0";
      this.tsmiWinRing0.Click += new System.EventHandler(this.tsmiWinRing0_Click);
      // 
      // lblInterval
      // 
      this.lblInterval.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.lblInterval.AutoSize = true;
      this.lblInterval.Location = new System.Drawing.Point(314, 94);
      this.lblInterval.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
      this.lblInterval.Name = "lblInterval";
      this.lblInterval.Size = new System.Drawing.Size(77, 12);
      this.lblInterval.TabIndex = 7;
      this.lblInterval.Text = "Interval(ms)";
      // 
      // lblInfoBoard
      // 
      this.lblInfoBoard.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
      this.lblInfoBoard.ForeColor = System.Drawing.Color.Red;
      this.lblInfoBoard.Location = new System.Drawing.Point(9, 26);
      this.lblInfoBoard.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
      this.lblInfoBoard.Name = "lblInfoBoard";
      this.lblInfoBoard.Size = new System.Drawing.Size(295, 16);
      this.lblInfoBoard.TabIndex = 999;
      this.lblInfoBoard.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
      // 
      // lblPeriod
      // 
      this.lblPeriod.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.lblPeriod.AutoSize = true;
      this.lblPeriod.Location = new System.Drawing.Point(314, 6);
      this.lblPeriod.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
      this.lblPeriod.Name = "lblPeriod";
      this.lblPeriod.Size = new System.Drawing.Size(41, 12);
      this.lblPeriod.TabIndex = 10;
      this.lblPeriod.Text = "Period";
      // 
      // nudPeriod
      // 
      this.nudPeriod.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.nudPeriod.Location = new System.Drawing.Point(316, 22);
      this.nudPeriod.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.nudPeriod.Maximum = new decimal(new int[] {
            60,
            0,
            0,
            0});
      this.nudPeriod.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
      this.nudPeriod.Name = "nudPeriod";
      this.nudPeriod.ReadOnly = true;
      this.nudPeriod.Size = new System.Drawing.Size(105, 21);
      this.nudPeriod.TabIndex = 9;
      this.nudPeriod.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
      // 
      // lblDuration
      // 
      this.lblDuration.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.lblDuration.AutoSize = true;
      this.lblDuration.Location = new System.Drawing.Point(314, 51);
      this.lblDuration.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
      this.lblDuration.Name = "lblDuration";
      this.lblDuration.Size = new System.Drawing.Size(71, 12);
      this.lblDuration.TabIndex = 12;
      this.lblDuration.Text = "Duration(s)";
      // 
      // nudDurationS
      // 
      this.nudDurationS.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.nudDurationS.Increment = new decimal(new int[] {
            10,
            0,
            0,
            0});
      this.nudDurationS.Location = new System.Drawing.Point(316, 66);
      this.nudDurationS.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.nudDurationS.Maximum = new decimal(new int[] {
            600,
            0,
            0,
            0});
      this.nudDurationS.Name = "nudDurationS";
      this.nudDurationS.ReadOnly = true;
      this.nudDurationS.Size = new System.Drawing.Size(105, 21);
      this.nudDurationS.TabIndex = 11;
      this.nudDurationS.Value = new decimal(new int[] {
            20,
            0,
            0,
            0});
      // 
      // gbMouse
      // 
      this.gbMouse.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.gbMouse.Controls.Add(this.rdobtnMouseRight);
      this.gbMouse.Controls.Add(this.rdobtnMouseLeft);
      this.gbMouse.Location = new System.Drawing.Point(9, 45);
      this.gbMouse.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.gbMouse.Name = "gbMouse";
      this.gbMouse.Padding = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.gbMouse.Size = new System.Drawing.Size(295, 60);
      this.gbMouse.TabIndex = 13;
      this.gbMouse.TabStop = false;
      this.gbMouse.Text = "eg. Mouse";
      // 
      // rdobtnMouseRight
      // 
      this.rdobtnMouseRight.AutoSize = true;
      this.rdobtnMouseRight.Location = new System.Drawing.Point(151, 27);
      this.rdobtnMouseRight.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.rdobtnMouseRight.Name = "rdobtnMouseRight";
      this.rdobtnMouseRight.Size = new System.Drawing.Size(101, 16);
      this.rdobtnMouseRight.TabIndex = 3;
      this.rdobtnMouseRight.Text = "Mouse [Right]";
      this.rdobtnMouseRight.UseVisualStyleBackColor = true;
      // 
      // rdobtnMouseLeft
      // 
      this.rdobtnMouseLeft.AutoSize = true;
      this.rdobtnMouseLeft.Location = new System.Drawing.Point(18, 27);
      this.rdobtnMouseLeft.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.rdobtnMouseLeft.Name = "rdobtnMouseLeft";
      this.rdobtnMouseLeft.Size = new System.Drawing.Size(95, 16);
      this.rdobtnMouseLeft.TabIndex = 2;
      this.rdobtnMouseLeft.Text = "Mouse [Left]";
      this.rdobtnMouseLeft.UseVisualStyleBackColor = true;
      // 
      // frmKeyboardMouseSimulator
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(430, 178);
      this.Controls.Add(this.gbMouse);
      this.Controls.Add(this.lblDuration);
      this.Controls.Add(this.nudDurationS);
      this.Controls.Add(this.lblPeriod);
      this.Controls.Add(this.lblInterval);
      this.Controls.Add(this.nudPeriod);
      this.Controls.Add(this.nudIntervalMS);
      this.Controls.Add(this.lblInfoBoard);
      this.Controls.Add(this.gbKeyboard);
      this.Controls.Add(this.lblCheckout);
      this.Controls.Add(this.btnSimulate);
      this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
      this.Name = "frmKeyboardMouseSimulator";
      this.ShowIcon = false;
      this.Text = "KeyboardMouseSimulator By wjshan0808";
      this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.frmGlobalKeyboardSet_FormClosing);
      this.Load += new System.EventHandler(this.frmGlobalKeyboardSet_Load);
      ((System.ComponentModel.ISupportInitialize)(this.nudIntervalMS)).EndInit();
      this.gbKeyboard.ResumeLayout(false);
      this.gbKeyboard.PerformLayout();
      this.cmsGlobalMenu.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)(this.nudPeriod)).EndInit();
      ((System.ComponentModel.ISupportInitialize)(this.nudDurationS)).EndInit();
      this.gbMouse.ResumeLayout(false);
      this.gbMouse.PerformLayout();
      this.ResumeLayout(false);
      this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnSimulate;
        private System.Windows.Forms.RadioButton rdobtnKeyO;
        private System.Windows.Forms.RadioButton rdobtnKeyG;
        private System.Windows.Forms.Label lblCheckout;
        private System.Windows.Forms.NumericUpDown nudIntervalMS;
        private System.Windows.Forms.GroupBox gbKeyboard;
        private System.Windows.Forms.Label lblInterval;
        private System.Windows.Forms.Label lblInfoBoard;
        private System.Windows.Forms.Label lblPeriod;
        private System.Windows.Forms.NumericUpDown nudPeriod;
        private System.Windows.Forms.Label lblDuration;
        private System.Windows.Forms.NumericUpDown nudDurationS;
        private System.Windows.Forms.GroupBox gbMouse;
        private System.Windows.Forms.RadioButton rdobtnMouseRight;
        private System.Windows.Forms.RadioButton rdobtnMouseLeft;
        private System.Windows.Forms.ContextMenuStrip cmsGlobalMenu;
        private System.Windows.Forms.ToolStripMenuItem tsmiWinIO;
        private System.Windows.Forms.ToolStripMenuItem tsmiWinRing0;
    }
}

