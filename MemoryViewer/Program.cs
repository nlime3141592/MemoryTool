using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace nl
{
    internal class Program
    {
        private const int PROCESS_ALL_ACCESS = 0x1F0FFF;

        [DllImport("kernel32.dll")]
        static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll")]
        static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] buffer, int size, out int bytesRead);

        [DllImport("kernel32.dll")]
        static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] buffer, int size, out int bytesWritten);

        [DllImport("kernel32.dll")]
        static extern bool CloseHandle(IntPtr hObject);

        private static void Main(string[] args)
        {
            Process[] processes = Process.GetProcessesByName("CalculatorApp");

            if (processes.Length == 0)
            {
                Console.WriteLine("No Process.");
                return;
            }

            string directory = @"C:\Programming\CSharp\MemoryViewer\pages";
            ProcMemDump.Dump(processes[0], directory);

            // write values
            // byte[] newValue = new byte[] { 0x90, 0x90 };
            // int bytesWritten;
            // WriteProcessMemory(hProcess, targetAddress, newValue, newValue.Length, out bytesWritten);

            // close handle
            // CloseHandle(hProcess);
        }
    }
}