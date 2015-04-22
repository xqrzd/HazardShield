/*
*  Copyright (C) 2015 Orbitech
*
*  Authors: xqrzd
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*  MA 02110-1301, USA.
*/

using ComponentFactory.Krypton.Toolkit;
using HazardShield.ClamAV.Updates;
using HazardShield.Scanning;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HazardShieldUI.Forms
{
    public partial class FrmMain : Form
    {
        Language CurrentLanguage;
        GuiProtector UiProtector;
        List<DatabaseUpdateLabel> DatabaseUpdateLabels;
        Freshclam FreshclamWrapper;
        Scanner _scanner;

        public FrmMain()
        {
            UiProtector = new GuiProtector();
            UiProtector.StartProtection();

            InitializeComponent();

            ApplyStyle(PaletteModeManager.Office2010Blue);

            Icon = Properties.Resources.icon;
            RadScannerSmart.Checked = true;
            TabBtnMainScanner.Checked = true;
            TabControlMain.HideTabs = true;
            TabControlScanner.HideTabs = true;

            Utils.MakeKryptonTextBold(BtnScannerScan);
            Utils.MakeKryptonTextBold(BtnUpdateCheck);
            Utils.MakeKryptonTextBold(BtnUpdateDownload);

            CurrentLanguage = new Language();
            CurrentLanguage.LoadLanguage(RuntimeInfo.HzrLanguagePath + "\\English.txt");
            SetControlsText(this);

            SetUpdateText();

            FreshclamWrapper = new Freshclam(RuntimeInfo.HzrDataDirectory);
            FreshclamWrapper.ProgressChanged += FreshclamProgressChanged;
            FreshclamWrapper.DownloadCompleted += FreshclamDownloadCompleted;
            FreshclamWrapper.UpdateFinished += FreshclamUpdateFinished;

            AddClamDatabaseUpdateLabels();

            _scanner = new Scanner();
        }

        void ApplyStyle(PaletteModeManager style)
        {
            KryptonManager kryptonManager = new KryptonManager();
            kryptonManager.GlobalPaletteMode = style;

            Color c = KryptonManager.CurrentGlobalPalette.GetBackColor1(PaletteBackStyle.PanelClient, PaletteState.Normal);

            TabScannerMain.BackColor = c;
            TabScannerCustom.BackColor = c;
            TabScannerProgress.BackColor = c;
            TabScannerResults.BackColor = c;

            TabMainUpdates.BackColor = c;
        }

        void SetUpdateText()
        {
            LblUpdateProgramVer.Text = Application.ProductVersion;
            LblUpdateDatabaseVer.Text = "0";
            LblUpdateClamAVVer.Text = Scanner.ClamAVVersion;
        }

        void SetControlsText(Control control, ToolTip toolTip = null)
        {
            foreach (Control c in control.Controls)
            {
                SetControlsText(c, toolTip);

                string name = c.Name;

                if (!string.IsNullOrEmpty(name))
                {
                    // KryptonHeader has 2 values, heading and description.
                    if (c is KryptonHeader)
                    {
                        KryptonHeader header = (KryptonHeader)c;

                        header.Values.Heading = CurrentLanguage.GetText(name);
                        header.Values.Description = CurrentLanguage.GetText(string.Format("{0}_Desc", name));
                    }
                    else
                    {
                        c.Text = CurrentLanguage.GetText(name);

                        if (toolTip != null)
                        {
                            string toolTipText = CurrentLanguage.GetText(string.Format("{0}_TT", name), true);
                            if (toolTipText != string.Empty)
                                toolTip.SetToolTip(c, toolTipText);
                        }
                    }
                }
            }
        }

        void ChangeMainTab(KryptonCheckButton button, TabPage tab)
        {
            SuspendLayout();

            TabBtnMainScanner.Checked = false;
            TabBtnMainUpdates.Checked = false;
            TabBtnMainProtection.Checked = false;
            TabBtnMainSettings.Checked = false;
            TabBtnMainQuarantine.Checked = false;
            TabBtnMainTools.Checked = false;
            TabBtnMainReserved.Checked = false;
            TabBtnMainAbout.Checked = false;

            TabControlMain.SelectedTab = tab;
            button.Checked = true;
            button.Focus();

            ResumeLayout();
        }

        void ChangeMainTabSize(KryptonCheckButton button)
        {
            // TODO: disable callback
            TabBtnMainScanner.Size = button.Size;
            TabBtnMainUpdates.Size = button.Size;
            TabBtnMainProtection.Size = button.Size;
            TabBtnMainSettings.Size = button.Size;
            TabBtnMainQuarantine.Size = button.Size;
            TabBtnMainTools.Size = button.Size;
            TabBtnMainReserved.Size = button.Size;
            TabBtnMainAbout.Size = button.Size;
        }

        private void TabBtnMainScanner_Click(object sender, EventArgs e)
        {
            ChangeMainTab((KryptonCheckButton)sender, TabMainScanner);
        }

        private void TabBtnMainUpdates_Click(object sender, EventArgs e)
        {
            ChangeMainTab((KryptonCheckButton)sender, TabMainUpdates);
        }

        private void TabBtnMainProtection_Click(object sender, EventArgs e)
        {
            ChangeMainTab((KryptonCheckButton)sender, TabMainProtection);
        }

        private void TabBtnMainSettings_Click(object sender, EventArgs e)
        {
            ChangeMainTab((KryptonCheckButton)sender, TabMainSettings);
        }

        private void TabBtnMainQuarantine_Click(object sender, EventArgs e)
        {
            ChangeMainTab((KryptonCheckButton)sender, TabMainQuarantine);
        }

        private void TabBtnMainTools_Click(object sender, EventArgs e)
        {
            ChangeMainTab((KryptonCheckButton)sender, TabMainTools);
        }

        private void TabBtnMainReserved_Click(object sender, EventArgs e)
        {
            ChangeMainTab((KryptonCheckButton)sender, TabMainReserved);
        }

        private void TabBtnMainAbout_Click(object sender, EventArgs e)
        {
            ChangeMainTab((KryptonCheckButton)sender, TabMainAbout);
        }

        private void BtnUpdatesCheck_SizeChanged(object sender, EventArgs e)
        {
            BtnUpdateDownload.Location = new Point(BtnUpdateCheck.Location.X + BtnUpdateCheck.Size.Width + 8, BtnUpdateCheck.Location.Y);
        }

        private void FrmMain_FormClosed(object sender, FormClosedEventArgs e)
        {
            UiProtector.DisableProtection();
        }

        #region Updates

        void AddClamDatabaseUpdateLabels()
        {
            DatabaseUpdateLabels = new List<DatabaseUpdateLabel>();

            Point location = new Point();
            location.Y = LblUpdateClamAV.Location.Y + 22;

            foreach (ClamDatabase database in FreshclamWrapper.Databases)
            {
                DatabaseUpdateLabel updateControl = new DatabaseUpdateLabel(database.Name, database.Header.Version.ToString());

                location.X = LblUpdateClamAV.Location.X;
                updateControl.LblDatabaseName.Location = location;

                location.X = LblUpdateClamAVVer.Location.X;
                updateControl.LblInstalledVersion.Location = location;

                location.X = LblUpdateClamAVStatus.Location.X;
                updateControl.LblUpdateStatus.Location = location;

                TabMainUpdates.Controls.Add(updateControl.LblDatabaseName);
                TabMainUpdates.Controls.Add(updateControl.LblInstalledVersion);
                TabMainUpdates.Controls.Add(updateControl.LblUpdateStatus);

                location.Y += 22; // Labels are 22 pixels vertically between each other
                DatabaseUpdateLabels.Add(updateControl);
            }

            SetControlsForUpdate();
        }

        void SetControlsForUpdate()
        {
            BtnUpdateDownload.Visible = false;

            LblUpdateProgramStatus.Values.Image = null;
            LblUpdateDatabaseStatus.Values.Image = null;
            LblUpdateClamAVStatus.Values.Image = null;

            LblUpdateProgramStatus.Text = string.Empty;
            LblUpdateDatabaseStatus.Text = string.Empty;
            LblUpdateClamAVStatus.Text = string.Empty;
            LblUpdateStatus.Text = string.Empty;

            foreach (DatabaseUpdateLabel updateControl in DatabaseUpdateLabels)
            {
                updateControl.LblUpdateStatus.Text = string.Empty;
                updateControl.LblUpdateStatus.Values.Image = null;
            }
        }

        void ResetUpdateControls()
        {
            BtnUpdateCheck.Enabled = true;
            //BtnCancelUpdate.Visible = false;
        }

        void UpdateUIVersionText()
        {
            // Don't need to update program text, since Hazard Shield / ClamAV cannot be updated without restarting the application
            //LblInstalledDatabaseVersion.Text = "";

            var databaseLabelPairs = FreshclamWrapper.Databases.Join(
                DatabaseUpdateLabels,
                db => db.Name,
                lbl => lbl.DatabaseName,
                (db, lbl) => new { ClamDatabase = db, UpdateLabel = lbl });

            foreach (var databaseLabelPair in databaseLabelPairs)
            {
                databaseLabelPair.UpdateLabel.LblInstalledVersion.Text = databaseLabelPair.ClamDatabase.Header.Version.ToString();
            }
        }

        void FreshclamProgressChanged(object sender, FreshclamProgressChangedArgs e)
        {
            this.Invoke((MethodInvoker)delegate
            {
                LblUpdateStatus.Text = e.Progress;
            });
        }

        void FreshclamDownloadCompleted(object sender, FreshclamDownloadCompletedArgs e)
        {
            this.Invoke((MethodInvoker)delegate
            {
                DatabaseUpdateLabel updateControl = DatabaseUpdateLabels.First(label => label.DatabaseName == e.Database);

                KryptonLabel statusLabel = updateControl.LblUpdateStatus;

                statusLabel.Values.Image = Properties.Resources.image_check;

                if (e.Updated)
                    statusLabel.Text = string.Format(CurrentLanguage.GetText("LblDatabaseUpdated"), updateControl.LblInstalledVersion.Text);
                else
                    statusLabel.Text = CurrentLanguage.GetText("LblDatabaseUpToDate");
            });
        }

        void FreshclamUpdateFinished(object sender, FreshclamUpdateFinishedEventArgs e)
        {
            this.Invoke((MethodInvoker)delegate
            {
                if (e.ReloadRequired)
                {
                    UpdateUIVersionText();

                    //// TODO: send signal to HzrService to reload ClamAV DB
                    //if (ScannerInstance != null)
                    //{
                    //    ScannerInstance.Dispose();
                    //    ScannerInstance = null;
                    //}
                }

                LblUpdateStatus.Text = CurrentLanguage.GetText("LblUpdateComplete");
                ResetUpdateControls();
            });
        }

        #endregion

        private void BtnUpdateCheck_Click(object sender, EventArgs e)
        {
            BtnUpdateCheck.Enabled = false;
            //BtnCancelUpdate.Visible = true;
            TabBtnMainUpdates.Focus();

            SetControlsForUpdate();

            Thread thread = new Thread(FreshclamWrapper.UpdateDatabases);
            thread.IsBackground = true;
            thread.Start();


            {
                // Remove this once updater is finished.
                LblUpdateProgramStatus.Text = " Not supported yet";
                LblUpdateProgramStatus.Values.Image = Properties.Resources.image_info;

                LblUpdateDatabaseStatus.Text = " Not supported yet";
                LblUpdateDatabaseStatus.Values.Image = Properties.Resources.image_info;

                LblUpdateClamAVStatus.Text = " Not supported yet";
                LblUpdateClamAVStatus.Values.Image = Properties.Resources.image_info;
            }
        }

        private void BtnScannerScan_Click(object sender, EventArgs e)
        {
            TabControlScanner.SelectedTab = TabScannerProgress;
        }

        private void BtnScannerPauseResume_SizeChanged(object sender, EventArgs e)
        {
            BtnScannerAbort.Location = new Point(BtnScannerPauseResume.Location.X + BtnScannerPauseResume.Size.Width + 8, BtnScannerPauseResume.Location.Y);
        }

        private void BtnScannerPauseResume_Click(object sender, EventArgs e)
        {
        }

    }
}