using System;
using System.Runtime.InteropServices;

namespace nl
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SystemInfo
    {
        public static SystemInfo Instance
        {
            get
            {
                SystemInfo systemInfo;
                GetSystemInfo(out systemInfo);
                return systemInfo;
            }
        }

        public ushort wProcessorArchitecture;
        public ushort wReserved;
        public uint dwPageSize;
        public IntPtr lpMinimumApplicationAddress;
        public IntPtr lpMaximumApplicationAddress;
        public IntPtr dwActiveProcessorMask;
        public uint dwNumberOfProcessors;
        public uint dwProcessorType;
        public uint dwAllocationGranularity;
        public ushort wProcessorLevel;
        public ushort wProcessorRevision;

        [DllImport("kernel32.dll")]
        private static extern void GetSystemInfo(out SystemInfo systemInfo);
    }
}