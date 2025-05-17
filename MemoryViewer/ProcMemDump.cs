using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace nl
{
    public class ProcMemDump
    {
        private const int PROCESS_ALL_ACCESS = 0x1F0FFF;

        public static void Dump(Process process, string directory)
        {
            IntPtr hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, process.Id);
            SystemInfo systemInfo = SystemInfo.Instance;
            MemoryBasicInformation memInfo;
            
            Console.WriteLine("Process Control Block:");
            Console.WriteLine($"\tPhysical memory usage     : {process.WorkingSet64:X}");
            Console.WriteLine($"\tBase priority             : {process.BasePriority}");
            Console.WriteLine($"\tPriority class            : {process.PriorityClass}");
            Console.WriteLine($"\tUser processor time       : {process.UserProcessorTime}");
            Console.WriteLine($"\tPrivileged processor time : {process.PrivilegedProcessorTime}");
            Console.WriteLine($"\tTotal processor time      : {process.TotalProcessorTime}");
            Console.WriteLine($"\tPaged system memory size  : {process.PagedSystemMemorySize64:X}");
            Console.WriteLine($"\tPaged memory size         : {process.PagedMemorySize64:X}");

            Console.WriteLine("Sytsem Info:");
            Console.WriteLine($"\twProcessorArchitecture : {systemInfo.wProcessorArchitecture}");
            Console.WriteLine($"\twReserved : {systemInfo.wReserved}");
            Console.WriteLine($"\tdwPageSize : {systemInfo.dwPageSize}");
            Console.WriteLine($"\tlpMinimumApplicationAddress : {systemInfo.lpMinimumApplicationAddress}");
            Console.WriteLine($"\tlpMaximumApplicationAddress : {systemInfo.lpMaximumApplicationAddress}");
            Console.WriteLine($"\tdwActiveProcessorMask : {systemInfo.dwActiveProcessorMask}");
            Console.WriteLine($"\tdwNumberOfProcessors : {systemInfo.dwNumberOfProcessors}");
            Console.WriteLine($"\tdwProcessorType : {systemInfo.dwProcessorType}");
            Console.WriteLine($"\tdwAllocationGranularity : {systemInfo.dwAllocationGranularity}");
            Console.WriteLine($"\twProcessorLevel : {systemInfo.wProcessorLevel}");
            Console.WriteLine($"\twProcessorRevision : {systemInfo.wProcessorRevision}");

            IntPtr address = IntPtr.Zero;
            int pageSize = (int)systemInfo.dwPageSize;
            int dumpSizeMax = 0x00200000;
            int dumpedSize = 0;

            while (dumpedSize < dumpSizeMax && MemoryBasicInformation.VirtualQueryEx(hProcess, address, out memInfo, (uint)Marshal.SizeOf<MemoryBasicInformation>()) != 0)
            {
                bool isReadable = (memInfo.state == 0x1000) // MEM_COMMIT
                   && ((memInfo.protect & 0xF0) != 0) // PAGE_READONLY, PAGE_READWRITE, etc.
                   && ((memInfo.protect & 0x100) == 0); // PAGE_GUARD 확인

                if (isReadable)
                {
                    long regionSize = (long)memInfo.regionSize;
                    long baseAddr = (long)memInfo.baseAddress;

                    for (long i = 0; i < regionSize; i += pageSize)
                    {
                        IntPtr readAddr = new IntPtr(baseAddr + i);
                        byte[] buffer = new byte[pageSize];
                        int bytesRead;

                        bool success = ReadProcessMemory(hProcess, readAddr, buffer, pageSize, out bytesRead);

                        if (success && bytesRead == pageSize)
                        {
                            // buffer[]에 데이터 있음

                            dumpedSize += pageSize;

                            Console.Write($"\rRead from {readAddr.ToString("X")} success. ({dumpedSize:X16}/{dumpSizeMax:X16})");
                            Write(directory, buffer, readAddr);
                        }
                        else
                        {
                            Console.WriteLine($"Read failed at {readAddr.ToString("X")}");
                            break;
                        }
                    }
                }

                // 다음 주소로 이동
                address = new IntPtr((long)memInfo.baseAddress + (long)memInfo.regionSize);
            }

            CloseHandle(hProcess);
        }

        private static void Write(string directory, byte[] buffer, long baseAddress)
        {            
            FileStream fs = new FileStream(directory + @$"\{baseAddress:X16}.txt", FileMode.Create, FileAccess.Write);
            StreamWriter wr = new StreamWriter(fs);

            for (int i = 0; i < buffer.Length; ++i)
            {
                if (i % 16 == 0)
                {
                    wr.Write($"\n{baseAddress + i:X16}\t");
                }
                wr.Write($"{buffer[i]:x02} ");
            }

            wr.WriteLine();
            wr.Close();
            fs.Close();
        }

        [DllImport("kernel32.dll")]
        private static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll")]
        private static extern bool CloseHandle(IntPtr hObject);

        [DllImport("kernel32.dll")]
        private static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] buffer, int size, out int bytesRead);
    }
}