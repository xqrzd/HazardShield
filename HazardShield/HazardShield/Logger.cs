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

namespace HazardShield
{
    /// <summary>
    /// These values are the same as those in MessageBoxIcon.
    /// </summary>
    public enum LogType
    {
        Error = 16,
        Warning = 48,
        Information = 64
    }

    /// <summary>
    /// Event arguments for logged messages.
    /// </summary>
    public class MessageLoggedEventArgs : EventArgs
    {
        /// <summary>
        /// Type of message.
        /// </summary>
        public LogType MessageType { get; private set; }

        /// <summary>
        /// Logged message.
        /// </summary>
        public string Message { get; private set; }

        /// <summary>
        /// Constructor for MessageLoggedEventArgs.
        /// </summary>
        /// <param name="messageType">Type of message.</param>
        /// <param name="message">Message to log.</param>
        public MessageLoggedEventArgs(LogType messageType, string message)
        {
            MessageType = messageType;
            Message = message;
        }
    }

    /// <summary>
    /// Used for logging messages.
    /// </summary>
    public static class Logger
    {
        /// <summary>
        /// Message event.
        /// </summary>
        public static event EventHandler<MessageLoggedEventArgs> MessageLogged;

        /// <summary>
        /// Logs a message.
        /// </summary>
        /// <param name="type">The log severity.</param>
        /// <param name="format">A composite format string.</param>
        /// <param name="args">An object to format.</param>
        internal static void Log(LogType type, string format, params object[] args)
        {
            if (MessageLogged != null)
                MessageLogged(null, new MessageLoggedEventArgs(type, string.Format(format, args)));
        }
    }
}