namespace KeyboardMouseSimulator
{
    partial class frmKeyboardMouseSimulatorDemo
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
      this.cmsSimulateWayMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
      this.tsmiSimulateWayEvent = new System.Windows.Forms.ToolStripMenuItem();
      this.tsmiSimulateWayWinIO = new System.Windows.Forms.ToolStripMenuItem();
      this.tsmiSimulateWayWinRing0 = new System.Windows.Forms.ToolStripMenuItem();
      this.lblInterval = new System.Windows.Forms.Label();
      this.lblInfoBoard = new System.Windows.Forms.Label();
      this.lblPeriod = new System.Windows.Forms.Label();
      this.nudPeriod = new System.Windows.Forms.NumericUpDown();
      this.lblDuration = new System.Windows.Forms.Label();
      this.nudDurationS = new System.Windows.Forms.NumericUpDown();
      this.gbMouse = new System.Windows.Forms.GroupBox();
      this.btnMouseOperate = new System.Windows.Forms.Button();
      this.nudCursorPositionY = new System.Windows.Forms.NumericUpDown();
      this.nudCursorPositionX = new System.Windows.Forms.NumericUpDown();
      this.lblNewCursorPositionY = new System.Windows.Forms.Label();
      this.lblNewCursorPositionX = new System.Windows.Forms.Label();
      this.lblCurrentCursorPositionXY = new System.Windows.Forms.Label();
      this.rdobtnMouseRight = new System.Windows.Forms.RadioButton();
      this.rdobtnMouseLeft = new System.Windows.Forms.RadioButton();
      this.rdobtnMouseMove = new System.Windows.Forms.RadioButton();
      ((System.ComponentModel.ISupportInitialize)(this.nudIntervalMS)).BeginInit();
      this.gbKeyboard.SuspendLayout();
      this.cmsSimulateWayMenu.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.nudPeriod)).BeginInit();
      ((System.ComponentModel.ISupportInitialize)(this.nudDurationS)).BeginInit();
      this.gbMouse.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.nudCursorPositionY)).BeginInit();
      ((System.ComponentModel.ISupportInitialize)(this.nudCursorPositionX)).BeginInit();
      this.SuspendLayout();
      // 
      // btnSimulate
      // 
      this.btnSimulate.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.btnSimulate.Location = new System.Drawing.Point(421, 244);
      this.btnSimulate.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.btnSimulate.Name = "btnSimulate";
      this.btnSimulate.Size = new System.Drawing.Size(140, 41);
      this.btnSimulate.TabIndex = 0;
      this.btnSimulate.Text = "Simulate";
      this.btnSimulate.UseVisualStyleBackColor = true;
      this.btnSimulate.Click += new System.EventHandler(this.btnSimulate_Click);
      // 
      // rdobtnKeyO
      // 
      this.rdobtnKeyO.AutoSize = true;
      this.rdobtnKeyO.Checked = true;
      this.rdobtnKeyO.Location = new System.Drawing.Point(24, 34);
      this.rdobtnKeyO.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.rdobtnKeyO.Name = "rdobtnKeyO";
      this.rdobtnKeyO.Size = new System.Drawing.Size(84, 19);
      this.rdobtnKeyO.TabIndex = 2;
      this.rdobtnKeyO.TabStop = true;
      this.rdobtnKeyO.Text = "Key [O]";
      this.rdobtnKeyO.UseVisualStyleBackColor = true;
      // 
      // rdobtnKeyG
      // 
      this.rdobtnKeyG.AutoSize = true;
      this.rdobtnKeyG.Location = new System.Drawing.Point(284, 34);
      this.rdobtnKeyG.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.rdobtnKeyG.Name = "rdobtnKeyG";
      this.rdobtnKeyG.Size = new System.Drawing.Size(84, 19);
      this.rdobtnKeyG.TabIndex = 3;
      this.rdobtnKeyG.Text = "Key [G]";
      this.rdobtnKeyG.UseVisualStyleBackColor = true;
      // 
      // lblCheckout
      // 
      this.lblCheckout.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.lblCheckout.Location = new System.Drawing.Point(12, 8);
      this.lblCheckout.Name = "lblCheckout";
      this.lblCheckout.Size = new System.Drawing.Size(549, 20);
      this.lblCheckout.TabIndex = 4;
      this.lblCheckout.Text = "Checkout";
      this.lblCheckout.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
      // 
      // nudIntervalMS
      // 
      this.nudIntervalMS.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.nudIntervalMS.Location = new System.Drawing.Point(421, 195);
      this.nudIntervalMS.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.nudIntervalMS.Maximum = new decimal(new int[] {
            600,
            0,
            0,
            0});
      this.nudIntervalMS.Name = "nudIntervalMS";
      this.nudIntervalMS.ReadOnly = true;
      this.nudIntervalMS.Size = new System.Drawing.Size(140, 25);
      this.nudIntervalMS.TabIndex = 3;
      this.nudIntervalMS.Value = new decimal(new int[] {
            5,
            0,
            0,
            0});
      // 
      // gbKeyboard
      // 
      this.gbKeyboard.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.gbKeyboard.ContextMenuStrip = this.cmsSimulateWayMenu;
      this.gbKeyboard.Controls.Add(this.rdobtnKeyG);
      this.gbKeyboard.Controls.Add(this.rdobtnKeyO);
      this.gbKeyboard.Location = new System.Drawing.Point(12, 55);
      this.gbKeyboard.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.gbKeyboard.Name = "gbKeyboard";
      this.gbKeyboard.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.gbKeyboard.Size = new System.Drawing.Size(393, 75);
      this.gbKeyboard.TabIndex = 6;
      this.gbKeyboard.TabStop = false;
      this.gbKeyboard.Text = "eg. Keyboard";
      // 
      // cmsSimulateWayMenu
      // 
      this.cmsSimulateWayMenu.ImageScalingSize = new System.Drawing.Size(20, 20);
      this.cmsSimulateWayMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmiSimulateWayEvent,
            this.tsmiSimulateWayWinIO,
            this.tsmiSimulateWayWinRing0});
      this.cmsSimulateWayMenu.Name = "cmsGlobalMenu";
      this.cmsSimulateWayMenu.Size = new System.Drawing.Size(149, 76);
      // 
      // tsmiSimulateWayEvent
      // 
      this.tsmiSimulateWayEvent.CheckOnClick = true;
      this.tsmiSimulateWayEvent.Name = "tsmiSimulateWayEvent";
      this.tsmiSimulateWayEvent.Size = new System.Drawing.Size(148, 24);
      this.tsmiSimulateWayEvent.Text = "Event";
      this.tsmiSimulateWayEvent.Click += new System.EventHandler(this.tsmiSimulateWayEvent_Click);
      // 
      // tsmiSimulateWayWinIO
      // 
      this.tsmiSimulateWayWinIO.CheckOnClick = true;
      this.tsmiSimulateWayWinIO.Name = "tsmiSimulateWayWinIO";
      this.tsmiSimulateWayWinIO.Size = new System.Drawing.Size(148, 24);
      this.tsmiSimulateWayWinIO.Text = "WinIO";
      this.tsmiSimulateWayWinIO.Click += new System.EventHandler(this.tsmiSimulateWayWinIO_Click);
      // 
      // tsmiSimulateWayWinRing0
      // 
      this.tsmiSimulateWayWinRing0.CheckOnClick = true;
      this.tsmiSimulateWayWinRing0.Name = "tsmiSimulateWayWinRing0";
      this.tsmiSimulateWayWinRing0.Size = new System.Drawing.Size(148, 24);
      this.tsmiSimulateWayWinRing0.Text = "WinRing0";
      this.tsmiSimulateWayWinRing0.Click += new System.EventHandler(this.tsmiSimulateWayWinRing0_Click);
      // 
      // lblInterval
      // 
      this.lblInterval.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.lblInterval.AutoSize = true;
      this.lblInterval.Location = new System.Drawing.Point(419, 176);
      this.lblInterval.Name = "lblInterval";
      this.lblInterval.Size = new System.Drawing.Size(103, 15);
      this.lblInterval.TabIndex = 7;
      this.lblInterval.Text = "Interval(ms)";
      // 
      // lblInfoBoard
      // 
      this.lblInfoBoard.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.lblInfoBoard.ForeColor = System.Drawing.Color.Red;
      this.lblInfoBoard.Location = new System.Drawing.Point(12, 32);
      this.lblInfoBoard.Name = "lblInfoBoard";
      this.lblInfoBoard.Size = new System.Drawing.Size(549, 20);
      this.lblInfoBoard.TabIndex = 999;
      this.lblInfoBoard.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
      // 
      // lblPeriod
      // 
      this.lblPeriod.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.lblPeriod.AutoSize = true;
      this.lblPeriod.Location = new System.Drawing.Point(419, 58);
      this.lblPeriod.Name = "lblPeriod";
      this.lblPeriod.Size = new System.Drawing.Size(55, 15);
      this.lblPeriod.TabIndex = 10;
      this.lblPeriod.Text = "Period";
      // 
      // nudPeriod
      // 
      this.nudPeriod.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.nudPeriod.Location = new System.Drawing.Point(421, 78);
      this.nudPeriod.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
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
      this.nudPeriod.Size = new System.Drawing.Size(140, 25);
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
      this.lblDuration.Location = new System.Drawing.Point(419, 118);
      this.lblDuration.Name = "lblDuration";
      this.lblDuration.Size = new System.Drawing.Size(95, 15);
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
      this.nudDurationS.Location = new System.Drawing.Point(421, 136);
      this.nudDurationS.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.nudDurationS.Maximum = new decimal(new int[] {
            600,
            0,
            0,
            0});
      this.nudDurationS.Name = "nudDurationS";
      this.nudDurationS.ReadOnly = true;
      this.nudDurationS.Size = new System.Drawing.Size(140, 25);
      this.nudDurationS.TabIndex = 11;
      this.nudDurationS.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
      // 
      // gbMouse
      // 
      this.gbMouse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
      this.gbMouse.ContextMenuStrip = this.cmsSimulateWayMenu;
      this.gbMouse.Controls.Add(this.rdobtnMouseMove);
      this.gbMouse.Controls.Add(this.btnMouseOperate);
      this.gbMouse.Controls.Add(this.nudCursorPositionY);
      this.gbMouse.Controls.Add(this.nudCursorPositionX);
      this.gbMouse.Controls.Add(this.lblNewCursorPositionY);
      this.gbMouse.Controls.Add(this.lblNewCursorPositionX);
      this.gbMouse.Controls.Add(this.lblCurrentCursorPositionXY);
      this.gbMouse.Controls.Add(this.rdobtnMouseRight);
      this.gbMouse.Controls.Add(this.rdobtnMouseLeft);
      this.gbMouse.Location = new System.Drawing.Point(12, 135);
      this.gbMouse.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.gbMouse.Name = "gbMouse";
      this.gbMouse.Padding = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.gbMouse.Size = new System.Drawing.Size(393, 150);
      this.gbMouse.TabIndex = 13;
      this.gbMouse.TabStop = false;
      this.gbMouse.Text = "eg. Mouse";
      // 
      // btnMouseOperate
      // 
      this.btnMouseOperate.Location = new System.Drawing.Point(257, 75);
      this.btnMouseOperate.Margin = new System.Windows.Forms.Padding(4);
      this.btnMouseOperate.Name = "btnMouseOperate";
      this.btnMouseOperate.Size = new System.Drawing.Size(122, 58);
      this.btnMouseOperate.TabIndex = 10;
      this.btnMouseOperate.Text = "Mouse\r\nOperate";
      this.btnMouseOperate.UseVisualStyleBackColor = true;
      this.btnMouseOperate.Click += new System.EventHandler(this.btnMouseOperate_Click);
      // 
      // nudCursorPositionY
      // 
      this.nudCursorPositionY.Location = new System.Drawing.Point(48, 106);
      this.nudCursorPositionY.Margin = new System.Windows.Forms.Padding(4);
      this.nudCursorPositionY.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
      this.nudCursorPositionY.Name = "nudCursorPositionY";
      this.nudCursorPositionY.Size = new System.Drawing.Size(95, 25);
      this.nudCursorPositionY.TabIndex = 8;
      // 
      // nudCursorPositionX
      // 
      this.nudCursorPositionX.Location = new System.Drawing.Point(48, 75);
      this.nudCursorPositionX.Margin = new System.Windows.Forms.Padding(4);
      this.nudCursorPositionX.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
      this.nudCursorPositionX.Name = "nudCursorPositionX";
      this.nudCursorPositionX.Size = new System.Drawing.Size(95, 25);
      this.nudCursorPositionX.TabIndex = 7;
      // 
      // lblNewCursorPositionY
      // 
      this.lblNewCursorPositionY.AutoSize = true;
      this.lblNewCursorPositionY.Location = new System.Drawing.Point(21, 109);
      this.lblNewCursorPositionY.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
      this.lblNewCursorPositionY.Name = "lblNewCursorPositionY";
      this.lblNewCursorPositionY.Size = new System.Drawing.Size(23, 15);
      this.lblNewCursorPositionY.TabIndex = 6;
      this.lblNewCursorPositionY.Text = "Y:";
      // 
      // lblNewCursorPositionX
      // 
      this.lblNewCursorPositionX.AutoSize = true;
      this.lblNewCursorPositionX.Location = new System.Drawing.Point(21, 78);
      this.lblNewCursorPositionX.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
      this.lblNewCursorPositionX.Name = "lblNewCursorPositionX";
      this.lblNewCursorPositionX.Size = new System.Drawing.Size(23, 15);
      this.lblNewCursorPositionX.TabIndex = 5;
      this.lblNewCursorPositionX.Text = "X:";
      // 
      // lblCurrentCursorPositionXY
      // 
      this.lblCurrentCursorPositionXY.Location = new System.Drawing.Point(151, 75);
      this.lblCurrentCursorPositionXY.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
      this.lblCurrentCursorPositionXY.Name = "lblCurrentCursorPositionXY";
      this.lblCurrentCursorPositionXY.Padding = new System.Windows.Forms.Padding(0, 6, 0, 0);
      this.lblCurrentCursorPositionXY.Size = new System.Drawing.Size(98, 58);
      this.lblCurrentCursorPositionXY.TabIndex = 4;
      this.lblCurrentCursorPositionXY.Text = "X:\r\n\r\nY:";
      // 
      // rdobtnMouseRight
      // 
      this.rdobtnMouseRight.AutoSize = true;
      this.rdobtnMouseRight.Location = new System.Drawing.Point(284, 34);
      this.rdobtnMouseRight.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.rdobtnMouseRight.Name = "rdobtnMouseRight";
      this.rdobtnMouseRight.Size = new System.Drawing.Size(68, 19);
      this.rdobtnMouseRight.TabIndex = 3;
      this.rdobtnMouseRight.Text = "Right";
      this.rdobtnMouseRight.UseVisualStyleBackColor = true;
      // 
      // rdobtnMouseLeft
      // 
      this.rdobtnMouseLeft.AutoSize = true;
      this.rdobtnMouseLeft.Location = new System.Drawing.Point(24, 34);
      this.rdobtnMouseLeft.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.rdobtnMouseLeft.Name = "rdobtnMouseLeft";
      this.rdobtnMouseLeft.Size = new System.Drawing.Size(60, 19);
      this.rdobtnMouseLeft.TabIndex = 2;
      this.rdobtnMouseLeft.Text = "Left";
      this.rdobtnMouseLeft.UseVisualStyleBackColor = true;
      // 
      // rdobtnMouseMove
      // 
      this.rdobtnMouseMove.AutoSize = true;
      this.rdobtnMouseMove.Checked = true;
      this.rdobtnMouseMove.Location = new System.Drawing.Point(154, 34);
      this.rdobtnMouseMove.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.rdobtnMouseMove.Name = "rdobtnMouseMove";
      this.rdobtnMouseMove.Size = new System.Drawing.Size(60, 19);
      this.rdobtnMouseMove.TabIndex = 11;
      this.rdobtnMouseMove.TabStop = true;
      this.rdobtnMouseMove.Text = "Move";
      this.rdobtnMouseMove.UseVisualStyleBackColor = true;
      // 
      // frmKeyboardMouseSimulatorDemo
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(573, 296);
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
      this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
      this.Name = "frmKeyboardMouseSimulatorDemo";
      this.ShowIcon = false;
      this.Text = "KeyboardMouseSimulatorDemo By wjshan0808";
      this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.frmGlobalKeyboardSet_FormClosing);
      this.Load += new System.EventHandler(this.frmGlobalKeyboardSet_Load);
      ((System.ComponentModel.ISupportInitialize)(this.nudIntervalMS)).EndInit();
      this.gbKeyboard.ResumeLayout(false);
      this.gbKeyboard.PerformLayout();
      this.cmsSimulateWayMenu.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)(this.nudPeriod)).EndInit();
      ((System.ComponentModel.ISupportInitialize)(this.nudDurationS)).EndInit();
      this.gbMouse.ResumeLayout(false);
      this.gbMouse.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)(this.nudCursorPositionY)).EndInit();
      ((System.ComponentModel.ISupportInitialize)(this.nudCursorPositionX)).EndInit();
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
        private System.Windows.Forms.Label lblCurrentCursorPositionXY;
        private System.Windows.Forms.Label lblNewCursorPositionY;
        private System.Windows.Forms.Label lblNewCursorPositionX;
        private System.Windows.Forms.NumericUpDown nudCursorPositionY;
        private System.Windows.Forms.NumericUpDown nudCursorPositionX;
        private System.Windows.Forms.Button btnMouseOperate;
        private System.Windows.Forms.ContextMenuStrip cmsSimulateWayMenu;
        private System.Windows.Forms.ToolStripMenuItem tsmiSimulateWayEvent;
        private System.Windows.Forms.ToolStripMenuItem tsmiSimulateWayWinIO;
        private System.Windows.Forms.ToolStripMenuItem tsmiSimulateWayWinRing0;
        private System.Windows.Forms.RadioButton rdobtnMouseMove;
    }
}

