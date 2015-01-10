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
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HazardShieldUI.Forms
{
    public partial class FrmMain : Form
    {
        public FrmMain()
        {
            InitializeComponent();

            //this.DoubleBuffered = true;
            TabBtnMainScanner.Checked = true;
            TabControlMain.HideTabs = true;
            TabControlScanner.HideTabs = true;
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

            button.Checked = true;
            TabControlMain.SelectedTab = tab;

            ResumeLayout();
        }

        void ChangeMainTabSize(KryptonCheckButton button)
        {
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

        private void TabBtnMainScanner_SizeChanged(object sender, EventArgs e)
        {
            ChangeMainTabSize((KryptonCheckButton)sender);
        }

        private void TabBtnMainUpdates_SizeChanged(object sender, EventArgs e)
        {
            ChangeMainTabSize((KryptonCheckButton)sender);
        }

        private void TabBtnMainProtection_SizeChanged(object sender, EventArgs e)
        {
            ChangeMainTabSize((KryptonCheckButton)sender);
        }

        private void TabBtnMainSettings_SizeChanged(object sender, EventArgs e)
        {
            ChangeMainTabSize((KryptonCheckButton)sender);
        }

        private void TabBtnMainQuarantine_SizeChanged(object sender, EventArgs e)
        {
            ChangeMainTabSize((KryptonCheckButton)sender);
        }

        private void TabBtnMainTools_SizeChanged(object sender, EventArgs e)
        {
            ChangeMainTabSize((KryptonCheckButton)sender);
        }

        private void TabBtnMainReserved_SizeChanged(object sender, EventArgs e)
        {
            ChangeMainTabSize((KryptonCheckButton)sender);
        }

        private void TabBtnMainAbout_SizeChanged(object sender, EventArgs e)
        {
            ChangeMainTabSize((KryptonCheckButton)sender);
        }

    }
}