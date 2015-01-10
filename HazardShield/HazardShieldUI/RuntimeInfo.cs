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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HazardShieldUI
{
    /// <summary>
    /// Contains paths to common Hazard Shield files and directories.
    /// </summary>
    public static class RuntimeInfo
    {
        /// <summary>
        /// The databases, settings, and log files are stored here.
        /// </summary>
        public static string HzrDataDirectory { get; private set; }

        /// <summary>
        /// Directory to store temporary files.
        /// </summary>
        public static string HzrTempDirectory { get; private set; }

        /// <summary>
        /// The full file path of the settings file.
        /// </summary>
        public static string HzrSettingsFile { get; private set; }

        /// <summary>
        /// The directory Hazard Shield was launched from.
        /// </summary>
        public static string HzrStartupPath { get; private set; }

        /// <summary>
        /// The directory where language files are stored.
        /// </summary>
        public static string HzrLanguagePath { get; private set; }

        static RuntimeInfo()
        {
            HzrDataDirectory = string.Format("{0}\\Hazard Shield", Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData));
            HzrTempDirectory = string.Format("{0}\\Temp", HzrDataDirectory);
            HzrSettingsFile = string.Format("{0}\\settings.ini", HzrDataDirectory);
            HzrStartupPath = Application.StartupPath;
            HzrLanguagePath = string.Format("{0}\\Languages", HzrStartupPath);
        }

    }
}