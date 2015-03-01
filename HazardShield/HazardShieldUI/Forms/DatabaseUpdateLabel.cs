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
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HazardShieldUI.Forms
{
    /// <summary>
    /// A collection of labels used to display database version information.
    /// </summary>
    class DatabaseUpdateLabel
    {
        /// <summary>
        /// The name of the database.
        /// </summary>
        public string DatabaseName { get; private set; }

        /// <summary>
        /// A label that shows the database name.
        /// </summary>
        public KryptonLabel LblDatabaseName { get; private set; }

        /// <summary>
        /// A label that shows the installed database version.
        /// </summary>
        public KryptonLabel LblInstalledVersion { get; private set; }

        /// <summary>
        /// A label that shows the status of the database.
        /// </summary>
        public KryptonLabel LblUpdateStatus { get; private set; }

        /// <summary>
        /// Initializes a new instance of DatabaseUpdateLabel.
        /// </summary>
        /// <param name="databaseName">The name of the database.</param>
        /// <param name="version">The version of the installed database.</param>
        public DatabaseUpdateLabel(string databaseName, string version)
        {
            DatabaseName = databaseName;

            LblDatabaseName = new KryptonLabel();

            // Capitalize first character.
            LblDatabaseName.Text = string.Format("{0}{1}", char.ToUpper(databaseName[0]), databaseName.Substring(1));

            LblInstalledVersion = new KryptonLabel();
            LblInstalledVersion.Text = version;

            LblUpdateStatus = new KryptonLabel();
            LblUpdateStatus.Text = string.Empty;
        }
    }
}