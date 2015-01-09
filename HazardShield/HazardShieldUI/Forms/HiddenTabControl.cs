// Author: http://homepage.ntlworld.com/mdaudi100/alternate/tabcontrols.html

using System.ComponentModel;
using System.Drawing;

namespace HazardShieldUI.Forms
{
    [ToolboxBitmap(typeof(System.Windows.Forms.TabControl))]
    public class HiddenTabControl : System.Windows.Forms.TabControl
    {
        private bool m_HideTabs = false;

        [DefaultValue(false)]
        [RefreshProperties(RefreshProperties.All)]
        public bool HideTabs
        {
            get { return m_HideTabs; }
            set
            {
                if (m_HideTabs == value)
                    return;

                m_HideTabs = value;

                this.UpdateStyles();
            }
        }

        public override System.Drawing.Rectangle DisplayRectangle
        {
            get
            {
                if (this.HideTabs)
                    return new Rectangle(0, 0, Width, Height);
                else
                    return base.DisplayRectangle;
            }
        }

    }
}