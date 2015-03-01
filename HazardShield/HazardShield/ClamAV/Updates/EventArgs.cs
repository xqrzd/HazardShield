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

namespace HazardShield.ClamAV.Updates
{
    /// <summary>
    /// Provides data for the DownloadCompleted event.
    /// </summary>
    public class FreshclamDownloadCompletedArgs : EventArgs
    {
        /// <summary>
        /// The name of the database
        /// </summary>
        public string Database { get; private set; }

        /// <summary>
        /// The version of the database. If the database was updated, this will be set to the new version.
        /// </summary>
        public uint Version { get; private set; }

        /// <summary>
        /// True if the database was updated, false if the database is already up to date.
        /// </summary>
        public bool Updated { get; private set; }

        /// <summary>
        /// Initializes a new instance of FreshclamDownloadCompletedArgs.
        /// </summary>
        /// <param name="database">The database name.</param>
        /// <param name="version">The database version.</param>
        /// <param name="updated">Whether the database was updated.</param>
        public FreshclamDownloadCompletedArgs(string database, uint version, bool updated)
        {
            Database = database;
            Version = version;
            Updated = updated;
        }
    }

    /// <summary>
    /// Provides data for the ProgressChanged event.
    /// </summary>
    public class FreshclamProgressChangedArgs : EventArgs
    {
        /// <summary>
        /// The current progress.
        /// </summary>
        public string Progress { get; private set; }

        /// <summary>
        /// Initializes a new instance of FreshclamProgressChangedArgs.
        /// </summary>
        /// <param name="progress">The current progress.</param>
        public FreshclamProgressChangedArgs(string progress)
        {
            Progress = progress;
        }
    }

    /// <summary>
    /// Provides data for the UpdateFinished event.
    /// </summary>
    public class FreshclamUpdateFinishedEventArgs : EventArgs
    {
        /// <summary>
        /// True if any of the specified databases were updated.
        /// </summary>
        public bool ReloadRequired { get; private set; }

        /// <summary>
        /// Initializes a new instance of FreshclamUpdateFinishedEventArgs.
        /// </summary>
        /// <param name="reloadRequired">True if any of the databases were updated.</param>
        public FreshclamUpdateFinishedEventArgs(bool reloadRequired)
        {
            ReloadRequired = reloadRequired;
        }
    }

}