namespace HazardShieldUI.Forms
{
    partial class FrmMain
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
            this.PanelMainTab = new ComponentFactory.Krypton.Toolkit.KryptonPanel();
            this.TabBtnMainAbout = new ComponentFactory.Krypton.Toolkit.KryptonCheckButton();
            this.TabBtnMainReserved = new ComponentFactory.Krypton.Toolkit.KryptonCheckButton();
            this.TabBtnMainTools = new ComponentFactory.Krypton.Toolkit.KryptonCheckButton();
            this.TabBtnMainQuarantine = new ComponentFactory.Krypton.Toolkit.KryptonCheckButton();
            this.TabBtnMainSettings = new ComponentFactory.Krypton.Toolkit.KryptonCheckButton();
            this.TabBtnMainProtection = new ComponentFactory.Krypton.Toolkit.KryptonCheckButton();
            this.TabBtnMainUpdates = new ComponentFactory.Krypton.Toolkit.KryptonCheckButton();
            this.TabBtnMainScanner = new ComponentFactory.Krypton.Toolkit.KryptonCheckButton();
            this.TabControlMain = new HazardShieldUI.Forms.HiddenTabControl();
            this.TabMainScanner = new System.Windows.Forms.TabPage();
            this.TabControlScanner = new HazardShieldUI.Forms.HiddenTabControl();
            this.TabScannerMain = new System.Windows.Forms.TabPage();
            this.TabScannerCustom = new System.Windows.Forms.TabPage();
            this.TabScannerProgress = new System.Windows.Forms.TabPage();
            this.TabScannerResults = new System.Windows.Forms.TabPage();
            this.HeaderScanner = new ComponentFactory.Krypton.Toolkit.KryptonHeader();
            this.TabMainUpdates = new System.Windows.Forms.TabPage();
            this.HeaderUpdates = new ComponentFactory.Krypton.Toolkit.KryptonHeader();
            this.TabMainProtection = new System.Windows.Forms.TabPage();
            this.HeaderProtection = new ComponentFactory.Krypton.Toolkit.KryptonHeader();
            this.TabMainSettings = new System.Windows.Forms.TabPage();
            this.HeaderSettings = new ComponentFactory.Krypton.Toolkit.KryptonHeader();
            this.TabMainQuarantine = new System.Windows.Forms.TabPage();
            this.HeaderQuarantine = new ComponentFactory.Krypton.Toolkit.KryptonHeader();
            this.TabMainTools = new System.Windows.Forms.TabPage();
            this.HeaderTools = new ComponentFactory.Krypton.Toolkit.KryptonHeader();
            this.TabMainReserved = new System.Windows.Forms.TabPage();
            this.HeaderReserved = new ComponentFactory.Krypton.Toolkit.KryptonHeader();
            this.TabMainAbout = new System.Windows.Forms.TabPage();
            this.HeaderAbout = new ComponentFactory.Krypton.Toolkit.KryptonHeader();
            ((System.ComponentModel.ISupportInitialize)(this.PanelMainTab)).BeginInit();
            this.PanelMainTab.SuspendLayout();
            this.TabControlMain.SuspendLayout();
            this.TabMainScanner.SuspendLayout();
            this.TabControlScanner.SuspendLayout();
            this.TabMainUpdates.SuspendLayout();
            this.TabMainProtection.SuspendLayout();
            this.TabMainSettings.SuspendLayout();
            this.TabMainQuarantine.SuspendLayout();
            this.TabMainTools.SuspendLayout();
            this.TabMainReserved.SuspendLayout();
            this.TabMainAbout.SuspendLayout();
            this.SuspendLayout();
            // 
            // PanelMainTab
            // 
            this.PanelMainTab.AutoSize = true;
            this.PanelMainTab.Controls.Add(this.TabBtnMainAbout);
            this.PanelMainTab.Controls.Add(this.TabBtnMainReserved);
            this.PanelMainTab.Controls.Add(this.TabBtnMainTools);
            this.PanelMainTab.Controls.Add(this.TabBtnMainQuarantine);
            this.PanelMainTab.Controls.Add(this.TabBtnMainSettings);
            this.PanelMainTab.Controls.Add(this.TabBtnMainProtection);
            this.PanelMainTab.Controls.Add(this.TabBtnMainUpdates);
            this.PanelMainTab.Controls.Add(this.TabBtnMainScanner);
            this.PanelMainTab.Dock = System.Windows.Forms.DockStyle.Left;
            this.PanelMainTab.Location = new System.Drawing.Point(0, 0);
            this.PanelMainTab.MinimumSize = new System.Drawing.Size(140, 0);
            this.PanelMainTab.Name = "PanelMainTab";
            this.PanelMainTab.Padding = new System.Windows.Forms.Padding(0, 0, 12, 0);
            this.PanelMainTab.PanelBackStyle = ComponentFactory.Krypton.Toolkit.PaletteBackStyle.PanelAlternate;
            this.PanelMainTab.Size = new System.Drawing.Size(167, 416);
            this.PanelMainTab.TabIndex = 0;
            // 
            // TabBtnMainAbout
            // 
            this.TabBtnMainAbout.AutoSize = true;
            this.TabBtnMainAbout.Location = new System.Drawing.Point(12, 362);
            this.TabBtnMainAbout.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainAbout.Name = "TabBtnMainAbout";
            this.TabBtnMainAbout.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainAbout.TabIndex = 7;
            this.TabBtnMainAbout.SizeChanged += new System.EventHandler(this.TabBtnMainAbout_SizeChanged);
            this.TabBtnMainAbout.Click += new System.EventHandler(this.TabBtnMainAbout_Click);
            // 
            // TabBtnMainReserved
            // 
            this.TabBtnMainReserved.AutoSize = true;
            this.TabBtnMainReserved.Location = new System.Drawing.Point(12, 312);
            this.TabBtnMainReserved.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainReserved.Name = "TabBtnMainReserved";
            this.TabBtnMainReserved.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainReserved.TabIndex = 6;
            this.TabBtnMainReserved.SizeChanged += new System.EventHandler(this.TabBtnMainReserved_SizeChanged);
            this.TabBtnMainReserved.Click += new System.EventHandler(this.TabBtnMainReserved_Click);
            // 
            // TabBtnMainTools
            // 
            this.TabBtnMainTools.AutoSize = true;
            this.TabBtnMainTools.Location = new System.Drawing.Point(12, 262);
            this.TabBtnMainTools.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainTools.Name = "TabBtnMainTools";
            this.TabBtnMainTools.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainTools.TabIndex = 5;
            this.TabBtnMainTools.SizeChanged += new System.EventHandler(this.TabBtnMainTools_SizeChanged);
            this.TabBtnMainTools.Click += new System.EventHandler(this.TabBtnMainTools_Click);
            // 
            // TabBtnMainQuarantine
            // 
            this.TabBtnMainQuarantine.AutoSize = true;
            this.TabBtnMainQuarantine.Location = new System.Drawing.Point(12, 212);
            this.TabBtnMainQuarantine.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainQuarantine.Name = "TabBtnMainQuarantine";
            this.TabBtnMainQuarantine.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainQuarantine.TabIndex = 4;
            this.TabBtnMainQuarantine.SizeChanged += new System.EventHandler(this.TabBtnMainQuarantine_SizeChanged);
            this.TabBtnMainQuarantine.Click += new System.EventHandler(this.TabBtnMainQuarantine_Click);
            // 
            // TabBtnMainSettings
            // 
            this.TabBtnMainSettings.AutoSize = true;
            this.TabBtnMainSettings.Location = new System.Drawing.Point(12, 162);
            this.TabBtnMainSettings.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainSettings.Name = "TabBtnMainSettings";
            this.TabBtnMainSettings.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainSettings.TabIndex = 3;
            this.TabBtnMainSettings.SizeChanged += new System.EventHandler(this.TabBtnMainSettings_SizeChanged);
            this.TabBtnMainSettings.Click += new System.EventHandler(this.TabBtnMainSettings_Click);
            // 
            // TabBtnMainProtection
            // 
            this.TabBtnMainProtection.AutoSize = true;
            this.TabBtnMainProtection.Location = new System.Drawing.Point(12, 112);
            this.TabBtnMainProtection.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainProtection.Name = "TabBtnMainProtection";
            this.TabBtnMainProtection.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainProtection.TabIndex = 2;
            this.TabBtnMainProtection.SizeChanged += new System.EventHandler(this.TabBtnMainProtection_SizeChanged);
            this.TabBtnMainProtection.Click += new System.EventHandler(this.TabBtnMainProtection_Click);
            // 
            // TabBtnMainUpdates
            // 
            this.TabBtnMainUpdates.AutoSize = true;
            this.TabBtnMainUpdates.Location = new System.Drawing.Point(12, 62);
            this.TabBtnMainUpdates.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainUpdates.Name = "TabBtnMainUpdates";
            this.TabBtnMainUpdates.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainUpdates.TabIndex = 1;
            this.TabBtnMainUpdates.SizeChanged += new System.EventHandler(this.TabBtnMainUpdates_SizeChanged);
            this.TabBtnMainUpdates.Click += new System.EventHandler(this.TabBtnMainUpdates_Click);
            // 
            // TabBtnMainScanner
            // 
            this.TabBtnMainScanner.AutoSize = true;
            this.TabBtnMainScanner.Location = new System.Drawing.Point(12, 12);
            this.TabBtnMainScanner.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainScanner.Name = "TabBtnMainScanner";
            this.TabBtnMainScanner.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainScanner.TabIndex = 0;
            this.TabBtnMainScanner.SizeChanged += new System.EventHandler(this.TabBtnMainScanner_SizeChanged);
            this.TabBtnMainScanner.Click += new System.EventHandler(this.TabBtnMainScanner_Click);
            // 
            // TabControlMain
            // 
            this.TabControlMain.Alignment = System.Windows.Forms.TabAlignment.Bottom;
            this.TabControlMain.Controls.Add(this.TabMainScanner);
            this.TabControlMain.Controls.Add(this.TabMainUpdates);
            this.TabControlMain.Controls.Add(this.TabMainProtection);
            this.TabControlMain.Controls.Add(this.TabMainSettings);
            this.TabControlMain.Controls.Add(this.TabMainQuarantine);
            this.TabControlMain.Controls.Add(this.TabMainTools);
            this.TabControlMain.Controls.Add(this.TabMainReserved);
            this.TabControlMain.Controls.Add(this.TabMainAbout);
            this.TabControlMain.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TabControlMain.Location = new System.Drawing.Point(167, 0);
            this.TabControlMain.Name = "TabControlMain";
            this.TabControlMain.SelectedIndex = 0;
            this.TabControlMain.Size = new System.Drawing.Size(667, 416);
            this.TabControlMain.TabIndex = 1;
            // 
            // TabMainScanner
            // 
            this.TabMainScanner.Controls.Add(this.TabControlScanner);
            this.TabMainScanner.Controls.Add(this.HeaderScanner);
            this.TabMainScanner.Location = new System.Drawing.Point(4, 4);
            this.TabMainScanner.Name = "TabMainScanner";
            this.TabMainScanner.Size = new System.Drawing.Size(659, 390);
            this.TabMainScanner.TabIndex = 0;
            this.TabMainScanner.UseVisualStyleBackColor = true;
            // 
            // TabControlScanner
            // 
            this.TabControlScanner.Alignment = System.Windows.Forms.TabAlignment.Bottom;
            this.TabControlScanner.Controls.Add(this.TabScannerMain);
            this.TabControlScanner.Controls.Add(this.TabScannerCustom);
            this.TabControlScanner.Controls.Add(this.TabScannerProgress);
            this.TabControlScanner.Controls.Add(this.TabScannerResults);
            this.TabControlScanner.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TabControlScanner.Location = new System.Drawing.Point(0, 52);
            this.TabControlScanner.Name = "TabControlScanner";
            this.TabControlScanner.SelectedIndex = 0;
            this.TabControlScanner.Size = new System.Drawing.Size(659, 338);
            this.TabControlScanner.TabIndex = 1;
            // 
            // TabScannerMain
            // 
            this.TabScannerMain.Location = new System.Drawing.Point(4, 4);
            this.TabScannerMain.Name = "TabScannerMain";
            this.TabScannerMain.Size = new System.Drawing.Size(651, 312);
            this.TabScannerMain.TabIndex = 0;
            this.TabScannerMain.UseVisualStyleBackColor = true;
            // 
            // TabScannerCustom
            // 
            this.TabScannerCustom.Location = new System.Drawing.Point(4, 4);
            this.TabScannerCustom.Name = "TabScannerCustom";
            this.TabScannerCustom.Size = new System.Drawing.Size(651, 312);
            this.TabScannerCustom.TabIndex = 1;
            this.TabScannerCustom.UseVisualStyleBackColor = true;
            // 
            // TabScannerProgress
            // 
            this.TabScannerProgress.Location = new System.Drawing.Point(4, 4);
            this.TabScannerProgress.Name = "TabScannerProgress";
            this.TabScannerProgress.Size = new System.Drawing.Size(651, 312);
            this.TabScannerProgress.TabIndex = 2;
            this.TabScannerProgress.UseVisualStyleBackColor = true;
            // 
            // TabScannerResults
            // 
            this.TabScannerResults.Location = new System.Drawing.Point(4, 4);
            this.TabScannerResults.Name = "TabScannerResults";
            this.TabScannerResults.Size = new System.Drawing.Size(651, 312);
            this.TabScannerResults.TabIndex = 3;
            this.TabScannerResults.UseVisualStyleBackColor = true;
            // 
            // HeaderScanner
            // 
            this.HeaderScanner.Dock = System.Windows.Forms.DockStyle.Top;
            this.HeaderScanner.Location = new System.Drawing.Point(0, 0);
            this.HeaderScanner.Name = "HeaderScanner";
            this.HeaderScanner.Size = new System.Drawing.Size(659, 52);
            this.HeaderScanner.TabIndex = 0;
            this.HeaderScanner.Values.Image = global::HazardShieldUI.Properties.Resources.scanner;
            // 
            // TabMainUpdates
            // 
            this.TabMainUpdates.Controls.Add(this.HeaderUpdates);
            this.TabMainUpdates.Location = new System.Drawing.Point(4, 4);
            this.TabMainUpdates.Name = "TabMainUpdates";
            this.TabMainUpdates.Size = new System.Drawing.Size(659, 390);
            this.TabMainUpdates.TabIndex = 1;
            this.TabMainUpdates.UseVisualStyleBackColor = true;
            // 
            // HeaderUpdates
            // 
            this.HeaderUpdates.Dock = System.Windows.Forms.DockStyle.Top;
            this.HeaderUpdates.Location = new System.Drawing.Point(0, 0);
            this.HeaderUpdates.Name = "HeaderUpdates";
            this.HeaderUpdates.Size = new System.Drawing.Size(659, 52);
            this.HeaderUpdates.TabIndex = 0;
            this.HeaderUpdates.Values.Image = global::HazardShieldUI.Properties.Resources.updates;
            // 
            // TabMainProtection
            // 
            this.TabMainProtection.Controls.Add(this.HeaderProtection);
            this.TabMainProtection.Location = new System.Drawing.Point(4, 4);
            this.TabMainProtection.Name = "TabMainProtection";
            this.TabMainProtection.Size = new System.Drawing.Size(659, 390);
            this.TabMainProtection.TabIndex = 2;
            this.TabMainProtection.UseVisualStyleBackColor = true;
            // 
            // HeaderProtection
            // 
            this.HeaderProtection.Dock = System.Windows.Forms.DockStyle.Top;
            this.HeaderProtection.Location = new System.Drawing.Point(0, 0);
            this.HeaderProtection.Name = "HeaderProtection";
            this.HeaderProtection.Size = new System.Drawing.Size(659, 52);
            this.HeaderProtection.TabIndex = 0;
            this.HeaderProtection.Values.Image = global::HazardShieldUI.Properties.Resources.realtime;
            // 
            // TabMainSettings
            // 
            this.TabMainSettings.Controls.Add(this.HeaderSettings);
            this.TabMainSettings.Location = new System.Drawing.Point(4, 4);
            this.TabMainSettings.Name = "TabMainSettings";
            this.TabMainSettings.Size = new System.Drawing.Size(659, 390);
            this.TabMainSettings.TabIndex = 3;
            this.TabMainSettings.UseVisualStyleBackColor = true;
            // 
            // HeaderSettings
            // 
            this.HeaderSettings.Dock = System.Windows.Forms.DockStyle.Top;
            this.HeaderSettings.Location = new System.Drawing.Point(0, 0);
            this.HeaderSettings.Name = "HeaderSettings";
            this.HeaderSettings.Size = new System.Drawing.Size(659, 52);
            this.HeaderSettings.TabIndex = 0;
            this.HeaderSettings.Values.Image = global::HazardShieldUI.Properties.Resources.settings;
            // 
            // TabMainQuarantine
            // 
            this.TabMainQuarantine.Controls.Add(this.HeaderQuarantine);
            this.TabMainQuarantine.Location = new System.Drawing.Point(4, 4);
            this.TabMainQuarantine.Name = "TabMainQuarantine";
            this.TabMainQuarantine.Size = new System.Drawing.Size(659, 390);
            this.TabMainQuarantine.TabIndex = 4;
            this.TabMainQuarantine.UseVisualStyleBackColor = true;
            // 
            // HeaderQuarantine
            // 
            this.HeaderQuarantine.Dock = System.Windows.Forms.DockStyle.Top;
            this.HeaderQuarantine.Location = new System.Drawing.Point(0, 0);
            this.HeaderQuarantine.Name = "HeaderQuarantine";
            this.HeaderQuarantine.Size = new System.Drawing.Size(659, 52);
            this.HeaderQuarantine.TabIndex = 0;
            this.HeaderQuarantine.Values.Image = global::HazardShieldUI.Properties.Resources.quarantine;
            // 
            // TabMainTools
            // 
            this.TabMainTools.Controls.Add(this.HeaderTools);
            this.TabMainTools.Location = new System.Drawing.Point(4, 4);
            this.TabMainTools.Name = "TabMainTools";
            this.TabMainTools.Size = new System.Drawing.Size(659, 390);
            this.TabMainTools.TabIndex = 5;
            this.TabMainTools.UseVisualStyleBackColor = true;
            // 
            // HeaderTools
            // 
            this.HeaderTools.Dock = System.Windows.Forms.DockStyle.Top;
            this.HeaderTools.Location = new System.Drawing.Point(0, 0);
            this.HeaderTools.Name = "HeaderTools";
            this.HeaderTools.Size = new System.Drawing.Size(659, 52);
            this.HeaderTools.TabIndex = 0;
            this.HeaderTools.Values.Image = global::HazardShieldUI.Properties.Resources.tools;
            // 
            // TabMainReserved
            // 
            this.TabMainReserved.Controls.Add(this.HeaderReserved);
            this.TabMainReserved.Location = new System.Drawing.Point(4, 4);
            this.TabMainReserved.Name = "TabMainReserved";
            this.TabMainReserved.Size = new System.Drawing.Size(659, 390);
            this.TabMainReserved.TabIndex = 6;
            this.TabMainReserved.UseVisualStyleBackColor = true;
            // 
            // HeaderReserved
            // 
            this.HeaderReserved.Dock = System.Windows.Forms.DockStyle.Top;
            this.HeaderReserved.Location = new System.Drawing.Point(0, 0);
            this.HeaderReserved.Name = "HeaderReserved";
            this.HeaderReserved.Size = new System.Drawing.Size(659, 31);
            this.HeaderReserved.TabIndex = 0;
            this.HeaderReserved.Values.Image = null;
            // 
            // TabMainAbout
            // 
            this.TabMainAbout.Controls.Add(this.HeaderAbout);
            this.TabMainAbout.Location = new System.Drawing.Point(4, 4);
            this.TabMainAbout.Name = "TabMainAbout";
            this.TabMainAbout.Size = new System.Drawing.Size(659, 390);
            this.TabMainAbout.TabIndex = 7;
            this.TabMainAbout.UseVisualStyleBackColor = true;
            // 
            // HeaderAbout
            // 
            this.HeaderAbout.Dock = System.Windows.Forms.DockStyle.Top;
            this.HeaderAbout.Location = new System.Drawing.Point(0, 0);
            this.HeaderAbout.Name = "HeaderAbout";
            this.HeaderAbout.Size = new System.Drawing.Size(659, 52);
            this.HeaderAbout.TabIndex = 0;
            this.HeaderAbout.Values.Image = global::HazardShieldUI.Properties.Resources.logo_48;
            // 
            // FrmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(834, 416);
            this.Controls.Add(this.TabControlMain);
            this.Controls.Add(this.PanelMainTab);
            this.Name = "FrmMain";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Hazard Shield 3 Beta";
            ((System.ComponentModel.ISupportInitialize)(this.PanelMainTab)).EndInit();
            this.PanelMainTab.ResumeLayout(false);
            this.PanelMainTab.PerformLayout();
            this.TabControlMain.ResumeLayout(false);
            this.TabMainScanner.ResumeLayout(false);
            this.TabMainScanner.PerformLayout();
            this.TabControlScanner.ResumeLayout(false);
            this.TabMainUpdates.ResumeLayout(false);
            this.TabMainUpdates.PerformLayout();
            this.TabMainProtection.ResumeLayout(false);
            this.TabMainProtection.PerformLayout();
            this.TabMainSettings.ResumeLayout(false);
            this.TabMainSettings.PerformLayout();
            this.TabMainQuarantine.ResumeLayout(false);
            this.TabMainQuarantine.PerformLayout();
            this.TabMainTools.ResumeLayout(false);
            this.TabMainTools.PerformLayout();
            this.TabMainReserved.ResumeLayout(false);
            this.TabMainReserved.PerformLayout();
            this.TabMainAbout.ResumeLayout(false);
            this.TabMainAbout.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ComponentFactory.Krypton.Toolkit.KryptonPanel PanelMainTab;
        private ComponentFactory.Krypton.Toolkit.KryptonCheckButton TabBtnMainAbout;
        private ComponentFactory.Krypton.Toolkit.KryptonCheckButton TabBtnMainReserved;
        private ComponentFactory.Krypton.Toolkit.KryptonCheckButton TabBtnMainTools;
        private ComponentFactory.Krypton.Toolkit.KryptonCheckButton TabBtnMainQuarantine;
        private ComponentFactory.Krypton.Toolkit.KryptonCheckButton TabBtnMainSettings;
        private ComponentFactory.Krypton.Toolkit.KryptonCheckButton TabBtnMainProtection;
        private ComponentFactory.Krypton.Toolkit.KryptonCheckButton TabBtnMainUpdates;
        private ComponentFactory.Krypton.Toolkit.KryptonCheckButton TabBtnMainScanner;
        private HiddenTabControl TabControlMain;
        private System.Windows.Forms.TabPage TabMainScanner;
        private System.Windows.Forms.TabPage TabMainUpdates;
        private System.Windows.Forms.TabPage TabMainProtection;
        private System.Windows.Forms.TabPage TabMainSettings;
        private System.Windows.Forms.TabPage TabMainQuarantine;
        private System.Windows.Forms.TabPage TabMainTools;
        private System.Windows.Forms.TabPage TabMainReserved;
        private System.Windows.Forms.TabPage TabMainAbout;
        private ComponentFactory.Krypton.Toolkit.KryptonHeader HeaderScanner;
        private HiddenTabControl TabControlScanner;
        private System.Windows.Forms.TabPage TabScannerMain;
        private System.Windows.Forms.TabPage TabScannerCustom;
        private System.Windows.Forms.TabPage TabScannerProgress;
        private System.Windows.Forms.TabPage TabScannerResults;
        private ComponentFactory.Krypton.Toolkit.KryptonHeader HeaderUpdates;
        private ComponentFactory.Krypton.Toolkit.KryptonHeader HeaderProtection;
        private ComponentFactory.Krypton.Toolkit.KryptonHeader HeaderSettings;
        private ComponentFactory.Krypton.Toolkit.KryptonHeader HeaderQuarantine;
        private ComponentFactory.Krypton.Toolkit.KryptonHeader HeaderTools;
        private ComponentFactory.Krypton.Toolkit.KryptonHeader HeaderReserved;
        private ComponentFactory.Krypton.Toolkit.KryptonHeader HeaderAbout;



    }
}