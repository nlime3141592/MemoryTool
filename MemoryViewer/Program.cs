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

        private static int ReadInt32(IntPtr hProcess, long address)
        {
            byte[] buffer = new byte[4];
            int readLength;

            ReadProcessMemory(hProcess, (IntPtr)address, buffer, buffer.Length, out readLength);

            if (buffer.Length == readLength)
                return BitConverter.ToInt32(buffer);
            else
                return 0;
        }

        private static bool WriteInt32(IntPtr hProcess, long address, int value)
        {
            byte[] buffer = BitConverter.GetBytes(value);
            int writtenLength;

            WriteProcessMemory(hProcess, (IntPtr)address, buffer, buffer.Length, out writtenLength);

            return buffer.Length == writtenLength;
        }

        private static void Main(string[] args)
        {
            Process[] processes = Process.GetProcessesByName("TowerDefenceAlpha");

            if (processes.Length == 0)
            {
                Console.WriteLine("No Process.");
                return;
            }

            // open handle
            IntPtr hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processes[0].Id);
            long targetAddress = 0x204EE374DC4;

            // read (1)
            int rdValue = ReadInt32(hProcess, targetAddress);
            Console.WriteLine($"Read Value == {rdValue}");

            // write (1)
            int wrValue = 150000;
            WriteInt32(hProcess, targetAddress, wrValue);

            // read (2)
            rdValue = ReadInt32((IntPtr)hProcess, targetAddress);
            Console.WriteLine($"Read Value == {rdValue}");

            // close handle
            CloseHandle(hProcess);
        }
    }
}