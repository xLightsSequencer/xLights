using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Plugin
{
    public partial class StatusForm : Form
    {
        private Plugin _plugin;
        public StatusForm(Plugin plugin)
        {
            _plugin = plugin;
            InitializeComponent();
        }

        private void StatusForm_Load(object sender, EventArgs e)
        {

        }

        public void SetStatus(string status)
        {
            textBox1.SuspendLayout();
            Point scroll = textBox1.AutoScrollOffset;
            textBox1.Text = status;
            textBox1.AutoScrollOffset = scroll;
            textBox1.ResumeLayout();
        }

        private void Timer1_Tick(object sender, EventArgs e)
        {
            string result;
            _plugin.xSchedule_Action("GetPlayingStatus", "", "", out result);
            SetStatus(result.ToString());
        }
    }
}
