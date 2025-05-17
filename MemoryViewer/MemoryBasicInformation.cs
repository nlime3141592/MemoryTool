using System;
using System.Runtime.InteropServices;

namespace nl
{
    [StructLayout(LayoutKind.Sequential)]
    public struct MemoryBasicInformation
    {
        public IntPtr baseAddress;
        public IntPtr allocationBase;
        public uint allocationProtect;
        public IntPtr regionSize;
        public uint state;
        public uint protect;
        public uint type;

        [DllImport("kernel32.dll")]
        public static extern int VirtualQueryEx(IntPtr hProcess, IntPtr lpAddress, out MemoryBasicInformation lpBuffer, uint dwLength);
    }
}