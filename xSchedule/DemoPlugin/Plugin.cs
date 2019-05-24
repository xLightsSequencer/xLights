using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using xScheduleWrapper;
using System.Drawing;

namespace Plugin
{
    public class Plugin
    {
        string _showDir = "";
        string _xScheduleURL = "";
        StatusForm _form;

        public string GetMenuString()
        {
            return "Demo Plugin";
        }
        public string GetWebFolder()
        {
            return "";
        }

        public bool xSchedule_Action(string command, string parameters, string data, out string buffer)
        {
            return xScheduleWrapper.xScheduleWrapper.Do_xSchedule_Action(command, parameters, data, out buffer);
        }

        public bool Load(string showDir)
        {
            _showDir = showDir;
            return true;
        }

        public void Unload()
        {
        }

        public bool HandleWeb(string command, string parameters, string data, string reference, out string response)
        {
            response = "";
            return false;
        }

        public bool Start(string showDir, string xScheduleURL)
        {
            _showDir = showDir;
            _xScheduleURL = xScheduleURL;

            if (_form != null) return true;

            _form = new StatusForm(this);
            _form.Show();
            return true;
        }

        public void Stop()
        {
            if (_form == null) return;

            _form.Close();
            _form = null;
        }

        public void WipeSettings()
        {

        }

        public void NotifyStatus(string status)
        {
        }

        public void ManipulateBuffer(PixelBuffer buffer)
        {
        }
    }
}