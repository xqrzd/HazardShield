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
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HazardShieldUI.Forms
{
    /// <summary>
    /// Utility methods for Windows Forms.
    /// </summary>
    static class Utils
    {
        /// <summary>
        /// Sets the DoubleBuffered property on the specified control
        /// </summary>
        /// <param name="control">The control to double buffer</param>
        public static void SetDoubleBuffered(Control control)
        {
            //Taxes: Remote Desktop Connection and painting
            //http://blogs.msdn.com/oldnewthing/archive/2006/01/03/508694.aspx
            if (SystemInformation.TerminalServerSession)
                return;

            PropertyInfo propertyInfo = typeof(Control).GetProperty("DoubleBuffered", BindingFlags.NonPublic | BindingFlags.Instance);
            propertyInfo.SetValue(control, true);
        }

        /// <summary>
        /// Makes a KryptonButton's text bold
        /// </summary>
        /// <param name="button">The button to make bold</param>
        public static void MakeKryptonTextBold(KryptonButton button)
        {
            button.StateCommon.Content.ShortText.Font = new Font(KryptonManager.CurrentGlobalPalette.GetContentLongTextFont(PaletteContentStyle.ButtonStandalone, PaletteState.Normal), FontStyle.Bold);
        }

    }
}