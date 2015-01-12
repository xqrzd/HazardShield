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

using HazardShield.Scanning.ClamAV;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace HazardShield.Updates.ClamAV
{
    /// <summary>
    /// Represents a ClamAV database.
    /// </summary>
    public class ClamDatabase
    {
        /// <summary>
        /// The database name.
        /// </summary>
        public string Name { get; private set; }

        /// <summary>
        /// The database header information.
        /// </summary>
        public ClamDatabaseHeader Header { get; private set; }

        /// <summary>
        /// Initializes a new instance of ClamDatabase.
        /// </summary>
        /// <param name="name">The database name.</param>
        /// <param name="header">The database header information.</param>
        public ClamDatabase(string name, ClamDatabaseHeader header)
        {
            Name = name;
            Header = header;
        }
    }

    /// <summary>
    /// A wrapper for freshclam.
    /// </summary>
    public class ClamUpdater
    {
        /// <summary>
        /// Occurs when freshclam outputs a message.
        /// </summary>
        public event EventHandler<ClamAVProgressChangedArgs> ProgressChanged;

        /// <summary>
        /// Occurs when a database has finished updating.
        /// </summary>
        public event EventHandler<ClamAVDownloadCompletedArgs> DownloadCompleted;

        /// <summary>
        /// Occurs when freshclam is finished.
        /// </summary>
        public event EventHandler<ClamAVUpdateFinishedEventArgs> UpdateFinished;

        /// <summary>
        /// A collection of all ClamAV databases in the specified directory.
        /// </summary>
        public IEnumerable<ClamDatabase> Databases { get; private set; }

        string DatabaseDirectory;
        bool ReloadRequired;

        /// <summary>
        /// Initializes a new instance of ClamUpdater.
        /// </summary>
        /// <param name="databaseDirectory">The database directory. All databases in this directory will be updated.</param>
        public ClamUpdater(string databaseDirectory)
        {
            DatabaseDirectory = databaseDirectory;
            ReloadRequired = false;
            Databases = EnumerateDatabases();
        }

        /// <summary>
        /// Updates all ClamAV databases in the specified directory.
        /// </summary>
        public void UpdateDatabases()
        {
            ReloadRequired = false;

            StringBuilder arguments = new StringBuilder();
            arguments.AppendFormat("--stdout --datadir=\"{0}\"", DatabaseDirectory);
            foreach (ClamDatabase database in Databases)
                arguments.AppendFormat(" --update-db {0}", database.Name);

            Process process = new Process();
            process.StartInfo.FileName = "freshclam.exe";
            process.StartInfo.Arguments = arguments.ToString();

            // Don't use ShellExecute, since it doesn't support reading output.
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.RedirectStandardOutput = true;
            process.OutputDataReceived += Freshclam_OutputDataReceived;
            process.Start();
            process.BeginOutputReadLine();
            process.WaitForExit();

            InvokeUpdateFinished();
        }

        /// <summary>
        /// Retrieves a list of all ClamAV databases in the specified directory.
        /// </summary>
        /// <returns>A list of ClamAV databases</returns>
        IEnumerable<ClamDatabase> EnumerateDatabases()
        {
            DirectoryInfo directoryInfo = new DirectoryInfo(DatabaseDirectory);
            foreach (FileInfo fileInfo in directoryInfo.EnumerateFiles().Where(fInfo => fInfo.Extension == ".cld" || fInfo.Extension == ".cvd"))
            {
                ClamDatabaseHeader header = HazardShield.Scanning.ClamAV.ClamAV.GetDatabaseHeader(fileInfo.FullName);
                if (header.Version != 0)
                    yield return new ClamDatabase(Path.GetFileNameWithoutExtension(fileInfo.Name), header);
            }
        }

        /// <summary>
        /// Processes output from freshclam.
        /// </summary>
        /// <param name="sender">The object that raised the event.</param>
        /// <param name="e">The data received from freshclam.</param>
        void Freshclam_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (string.IsNullOrEmpty(e.Data) || e.Data.Length < 5)
                return;

            Match match = Regex.Match(e.Data, @"(?<database>.*)\.(?<action>.*) \(version: (?<version>\d*)");
            if (match.Success)
            {
                string database = match.Groups["database"].Value;
                uint version = Convert.ToUInt32(match.Groups["version"].Value);
                bool updated = match.Groups["action"].Value.Contains("updated");

                InvokeDownloadCompleted(new ClamAVDownloadCompletedArgs(database, version, updated));
            }
            else
                InvokeProgressChanged(new ClamAVProgressChangedArgs(e.Data));
        }

        /// <summary>
        /// Calls the ProgressChanged event.
        /// </summary>
        /// <param name="eventArgs">Progress changed args.</param>
        void InvokeProgressChanged(ClamAVProgressChangedArgs eventArgs)
        {
            if (ProgressChanged != null)
                ProgressChanged(this, eventArgs);
        }

        /// <summary>
        /// Calls the DownloadCompleted event.
        /// </summary>
        /// <param name="eventArgs">Download completed args.</param>
        void InvokeDownloadCompleted(ClamAVDownloadCompletedArgs eventArgs)
        {
            // If any database is updated, a reload is needed
            if (eventArgs.Updated)
                ReloadRequired = true;

            if (DownloadCompleted != null)
                DownloadCompleted(this, eventArgs);
        }

        /// <summary>
        /// Calls the UpdateFinished event.
        /// </summary>
        void InvokeUpdateFinished()
        {
            ClamAVUpdateFinishedEventArgs eventArgs = new ClamAVUpdateFinishedEventArgs(ReloadRequired);

            if (UpdateFinished != null)
                UpdateFinished(this, eventArgs);
        }

    }
}