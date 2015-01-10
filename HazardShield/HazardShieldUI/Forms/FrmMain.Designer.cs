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
            ((System.ComponentModel.ISupportInitialize)(this.PanelMainTab)).BeginInit();
            this.PanelMainTab.SuspendLayout();
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
            this.PanelMainTab.Size = new System.Drawing.Size(167, 671);
            this.PanelMainTab.TabIndex = 0;
            // 
            // TabBtnMainAbout
            // 
            this.TabBtnMainAbout.AutoSize = true;
            this.TabBtnMainAbout.Location = new System.Drawing.Point(12, 348);
            this.TabBtnMainAbout.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainAbout.Name = "TabBtnMainAbout";
            this.TabBtnMainAbout.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainAbout.TabIndex = 7;
            this.TabBtnMainAbout.Values.Text = "About";
            this.TabBtnMainAbout.SizeChanged += new System.EventHandler(this.TabBtnMainAbout_SizeChanged);
            this.TabBtnMainAbout.Click += new System.EventHandler(this.TabBtnMainAbout_Click);
            // 
            // TabBtnMainReserved
            // 
            this.TabBtnMainReserved.AutoSize = true;
            this.TabBtnMainReserved.Location = new System.Drawing.Point(12, 300);
            this.TabBtnMainReserved.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainReserved.Name = "TabBtnMainReserved";
            this.TabBtnMainReserved.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainReserved.TabIndex = 6;
            this.TabBtnMainReserved.Values.Text = "Reserved";
            this.TabBtnMainReserved.SizeChanged += new System.EventHandler(this.TabBtnMainReserved_SizeChanged);
            this.TabBtnMainReserved.Click += new System.EventHandler(this.TabBtnMainReserved_Click);
            // 
            // TabBtnMainTools
            // 
            this.TabBtnMainTools.AutoSize = true;
            this.TabBtnMainTools.Location = new System.Drawing.Point(12, 252);
            this.TabBtnMainTools.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainTools.Name = "TabBtnMainTools";
            this.TabBtnMainTools.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainTools.TabIndex = 5;
            this.TabBtnMainTools.Values.Text = "Tools";
            this.TabBtnMainTools.SizeChanged += new System.EventHandler(this.TabBtnMainTools_SizeChanged);
            this.TabBtnMainTools.Click += new System.EventHandler(this.TabBtnMainTools_Click);
            // 
            // TabBtnMainQuarantine
            // 
            this.TabBtnMainQuarantine.AutoSize = true;
            this.TabBtnMainQuarantine.Location = new System.Drawing.Point(12, 204);
            this.TabBtnMainQuarantine.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainQuarantine.Name = "TabBtnMainQuarantine";
            this.TabBtnMainQuarantine.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainQuarantine.TabIndex = 4;
            this.TabBtnMainQuarantine.Values.Text = "Quarantine";
            this.TabBtnMainQuarantine.SizeChanged += new System.EventHandler(this.TabBtnMainQuarantine_SizeChanged);
            this.TabBtnMainQuarantine.Click += new System.EventHandler(this.TabBtnMainQuarantine_Click);
            // 
            // TabBtnMainSettings
            // 
            this.TabBtnMainSettings.AutoSize = true;
            this.TabBtnMainSettings.Location = new System.Drawing.Point(12, 156);
            this.TabBtnMainSettings.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainSettings.Name = "TabBtnMainSettings";
            this.TabBtnMainSettings.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainSettings.TabIndex = 3;
            this.TabBtnMainSettings.Values.Text = "Settings";
            this.TabBtnMainSettings.SizeChanged += new System.EventHandler(this.TabBtnMainSettings_SizeChanged);
            this.TabBtnMainSettings.Click += new System.EventHandler(this.TabBtnMainSettings_Click);
            // 
            // TabBtnMainProtection
            // 
            this.TabBtnMainProtection.AutoSize = true;
            this.TabBtnMainProtection.Location = new System.Drawing.Point(12, 108);
            this.TabBtnMainProtection.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainProtection.Name = "TabBtnMainProtection";
            this.TabBtnMainProtection.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainProtection.TabIndex = 2;
            this.TabBtnMainProtection.Values.Text = "Protection";
            this.TabBtnMainProtection.SizeChanged += new System.EventHandler(this.TabBtnMainProtection_SizeChanged);
            this.TabBtnMainProtection.Click += new System.EventHandler(this.TabBtnMainProtection_Click);
            // 
            // TabBtnMainUpdates
            // 
            this.TabBtnMainUpdates.AutoSize = true;
            this.TabBtnMainUpdates.Location = new System.Drawing.Point(12, 60);
            this.TabBtnMainUpdates.MinimumSize = new System.Drawing.Size(140, 0);
            this.TabBtnMainUpdates.Name = "TabBtnMainUpdates";
            this.TabBtnMainUpdates.Size = new System.Drawing.Size(140, 40);
            this.TabBtnMainUpdates.TabIndex = 1;
            this.TabBtnMainUpdates.Values.Text = "Updates";
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
            this.TabBtnMainScanner.Values.Text = "Scanner";
            this.TabBtnMainScanner.SizeChanged += new System.EventHandler(this.TabBtnMainScanner_SizeChanged);
            this.TabBtnMainScanner.Click += new System.EventHandler(this.TabBtnMainScanner_Click);
            // 
            // FrmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1121, 671);
            this.Controls.Add(this.PanelMainTab);
            this.Name = "FrmMain";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Hazard Shield 3 Beta";
            ((System.ComponentModel.ISupportInitialize)(this.PanelMainTab)).EndInit();
            this.PanelMainTab.ResumeLayout(false);
            this.PanelMainTab.PerformLayout();
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



    }
}