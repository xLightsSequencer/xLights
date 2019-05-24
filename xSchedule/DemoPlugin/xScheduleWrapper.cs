// *******************************************
//
// To build an xSchedule plugin do NOT touch this
// code.  All changes should be made in Plugin.cs
//
// *******************************************

using System;
using System.Runtime.InteropServices;
using System.Drawing;

// requires package UnmanagedExports - version 1.6
// https://github.com/3F/DllExport
// https://github.com/3F/DllExport/wiki/Quick-start

namespace xScheduleWrapper
{
    public class PixelBuffer
    {
        IntPtr _buffer { get; }
        int _bufferSize;

        public PixelBuffer(IntPtr buffer, int bufferSize)
        {
            _buffer = buffer;
            _bufferSize = bufferSize;
        }

        int GetSize() { return _bufferSize; }
        int GetPixels() { return _bufferSize / 3; }

        public byte this[int index]
        {
            get
            {
                if (index >= _bufferSize) return 0;
                return Marshal.ReadByte(_buffer + index);
            }
            set
            {
                if (index >= _bufferSize) return; // out of bounds
                Marshal.WriteByte(_buffer + index, value);
            }
        }

        public Color GetPixel(int index)
        {
            if (3 * index + 2 >= _bufferSize) return Color.Black;
            return Color.FromArgb(this[index * 3],
                this[index * 3 + 1], 
                this[index * 3] + 2);
        }

        public void SetPixel(int index, Color c)
        {
            if (3 * index + 2 >= _bufferSize) return;
            this[3 * index] = c.R;
            this[3 * index+1] = c.G;
            this[3 * index+2] = c.B;
        }
    }

    public static class xScheduleWrapper
    {
        static IntPtr _action;
        static Plugin.Plugin _plugin = new Plugin.Plugin();

        static void SetString(IntPtr buffer, int bufferSize, string value)
        {
            if (bufferSize <= value.Length)
            {
                Marshal.WriteByte(buffer, 0x00);
            }
            else
            {
                int i = 0;
                foreach(char c in value)
                {
                    Marshal.WriteByte(buffer + i, (byte)c);
                    i++;
                }
                Marshal.WriteByte(buffer + i, 0x00);
            }
        }

        static void SetWString(IntPtr buffer, int bufferSize, string value)
        {
            if (bufferSize <= value.Length / 2)
            {
                Marshal.WriteInt16(buffer, 0x00);
            }
            else
            {
                int i = 0;
                foreach (char c in value)
                {
                    Marshal.WriteInt16(buffer + i, (byte)c);
                    i += 2;
                }
                Marshal.WriteInt16(buffer + i, 0x00);
            }
        }

        //delegate void xSchedule_Action_Delegate(string command, string parameters, string data, out StringBuilder buffer, int bufferSize);
        public delegate bool xSchedule_Action_Delegate(
            [MarshalAs(UnmanagedType.LPStr)]string command,
            [MarshalAs(UnmanagedType.LPWStr)]string parameters,
            [MarshalAs(UnmanagedType.LPStr)]string data,
            IntPtr buffer, 
            int bufferSize);
        public static bool Do_xSchedule_Action(string command, string parameters, string data, out string buffer)
        {
            xSchedule_Action_Delegate p_xSchedule_Action = (xSchedule_Action_Delegate)Marshal.GetDelegateForFunctionPointer(_action, typeof(xSchedule_Action_Delegate));

            int bufferSize = 4096;
            IntPtr lpBuffer = Marshal.AllocHGlobal(bufferSize);

            bool res = p_xSchedule_Action(command, parameters, data, lpBuffer, bufferSize);

            buffer = Marshal.PtrToStringAnsi(lpBuffer);

            // Free the buffer.
            Marshal.FreeHGlobal(lpBuffer);
            lpBuffer = IntPtr.Zero;

            return res;
        }

        [DllExport("xSchedule_Load", CallingConvention = CallingConvention.StdCall)]
        public static bool xSchedule_Load([MarshalAs(UnmanagedType.LPStr)]string showDir)
        {
            return _plugin.Load(showDir);
        }

        [DllExport("xSchedule_Unload", CallingConvention = CallingConvention.StdCall)]
        public static void xSchedule_Unload()
        {
            _plugin.Unload();
        }

        [DllExport("xSchedule_GetVirtualWebFolder", CallingConvention = CallingConvention.StdCall)]
        public static void xSchedule_GetVirtualWebFolder(IntPtr buffer, int bufferSize)
        {
            SetString(buffer, bufferSize, _plugin.GetWebFolder());
        }

        [DllExport("xSchedule_GetMenuLabel", CallingConvention = CallingConvention.StdCall)]
        public static void xSchedule_GetMenuLabel(IntPtr buffer, int bufferSize)
        {
            SetString(buffer, bufferSize, _plugin.GetMenuString());
        }

        [DllExport("xSchedule_HandleWeb", CallingConvention = CallingConvention.StdCall)]
        public static bool xSchedule_HandleWeb(
            [MarshalAs(UnmanagedType.LPStr)]string command, 
            [MarshalAs(UnmanagedType.LPWStr)]string parameters, 
            [MarshalAs(UnmanagedType.LPWStr)]string data, 
            [MarshalAs(UnmanagedType.LPWStr)]string reference, 
            IntPtr response, int responseSize)
        {
            string resp;
            bool res = _plugin.HandleWeb(command, parameters, data, reference, out resp);
            SetWString(response, responseSize, resp);
            return res;
        }

        [DllExport("xSchedule_Start", CallingConvention = CallingConvention.StdCall)]
        public static bool xSchedule_Start([MarshalAs(UnmanagedType.LPStr)]string showDir, [MarshalAs(UnmanagedType.LPStr)]string xScheduleURL, IntPtr action)
        {
            _action = action;

            return _plugin.Start(showDir, xScheduleURL);
        }

        [DllExport("xSchedule_Stop", CallingConvention = CallingConvention.StdCall)]
        public static void xSchedule_Stop()
        {
            _plugin.Stop();
        }

        [DllExport("xSchedule_WipeSettings", CallingConvention = CallingConvention.StdCall)]
        public static void xSchedule_WipeSettings()
        {
            _plugin.WipeSettings();
        }

        [DllExport("xSchedule_NotifyStatus", CallingConvention = CallingConvention.StdCall)]
        public static void xSchedule_NotifyStatus([MarshalAs(UnmanagedType.LPStr)]string status)
        {
            _plugin.NotifyStatus(status);
        }

        [DllExport("xSchedule_ManipulateBuffer", CallingConvention = CallingConvention.StdCall)]
        public static void xSchedule_ManipulateBuffer(IntPtr buffer, int bufferSize)
        {
            _plugin.ManipulateBuffer(new PixelBuffer(buffer, bufferSize));
        }
    }
}