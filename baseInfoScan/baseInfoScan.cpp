// baseInfoScan.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "baseInfoScan.h"

#pragma warning(disable:4996)

extern "C"
{
	__declspec(dllexport) struct ProcessInfo * processScan(struct ProcessInfo  * P)
	{
		/*
		Function: scan processInfo, save it with a linknode
		return: struct processInfo *P
		*/
		struct ProcessInfo * processNode;
		struct ProcessInfo * currentProcessInfo;

		int i = 1;

		HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (procSnap == INVALID_HANDLE_VALUE)
		{
			printf("CreateToolhelp32Snapshot failed, %d\n", GetLastError());
		}
		PROCESSENTRY32 procEntry = { 0 };
		procEntry.dwSize = sizeof(PROCESSENTRY32);


		BOOL bRet = Process32First(procSnap, &procEntry);
		processNode = (struct ProcessInfo *)malloc(sizeof(struct ProcessInfo));
		processNode->th32ProcessID = procEntry.th32ProcessID;
		processNode->szExeFile = (TCHAR*)malloc(sizeof(procEntry.szExeFile));
		memcpy(processNode->szExeFile, procEntry.szExeFile, sizeof(procEntry.szExeFile));
		processNode->next = NULL;
		currentProcessInfo = processNode;
		P = processNode;

		while (bRet)
		{
			bRet = Process32Next(procSnap, &procEntry);
			if (bRet)
			{
				i++;
				processNode = (struct ProcessInfo *)malloc(sizeof(struct ProcessInfo));
				processNode->th32ProcessID = procEntry.th32ProcessID;
				processNode->szExeFile = (TCHAR*)malloc(sizeof(procEntry.szExeFile));
				processNode->next = NULL;
				memcpy(processNode->szExeFile, procEntry.szExeFile, sizeof(procEntry.szExeFile));
				currentProcessInfo->next = processNode;
				currentProcessInfo = processNode;
			}
		}

		currentProcessInfo->next = NULL;
		CloseHandle(procSnap);
		return P;
	}


	/*
		GPU
	*/
	void getGPUInfo()
	{};


	/*   
	把eax = 0作为输入参数，可以得到CPU的制造商信息。
	cpuid指令执行以后，会返回一个12字符的制造商信息，
	前四个字符的ASC码按低位到高位放在ebx，中间四个放在edx，最后四个字符放在ecx。
	*/
	void initCpu(DWORD veax, struct HardwareInfo &hardwareInfo)
	{
		DWORD deax;
		DWORD debx;
		DWORD decx;
		DWORD dedx;

		__asm
		{
			mov eax, veax
			cpuid
			mov deax, eax
			mov debx, ebx
			mov decx, ecx
			mov dedx, edx
		}
		hardwareInfo.cpuInfo.reg.dedx = dedx;
		hardwareInfo.cpuInfo.reg.deax = deax;
		hardwareInfo.cpuInfo.reg.debx = debx;
		hardwareInfo.cpuInfo.reg.decx = decx;
	}
	
	/*  
	在我的电脑上点击右键，选择属性，可以在窗口的下面看到一条CPU的信息，
	这就是CPU的商标字符串。CPU的商标字符串也是通过cpuid得到的。
	由于商标的字符串很长(48个字符)，所以不能在一次cpuid指令执行时全部得到，
	所以intel把它分成了3个操作，eax的输入参数分别是0x80000002,0x80000003,0x80000004，
	每次返回的16个字符，按照从低位到高位的顺序依次放在eax, ebx, ecx, edx。
	因此，可以用循环的方式，每次执行完以后保存结果，然后执行下一次cpuid。
	*/
	void getCPUInfo(struct HardwareInfo &hardwareInfo)
	{
		const DWORD id = 0x80000002; // start 0x80000002 end to 0x80000004  
		memset(hardwareInfo.cpuInfo.cpuType, 0, sizeof(hardwareInfo.cpuInfo.cpuType));

		for (DWORD t = 0; t < 3; t++)
		{
			initCpu(id + t, hardwareInfo);

			memcpy(hardwareInfo.cpuInfo.cpuType + 16 * t + 0, &(hardwareInfo.cpuInfo.reg.deax), 4);
			memcpy(hardwareInfo.cpuInfo.cpuType + 16 * t + 4, &(hardwareInfo.cpuInfo.reg.debx), 4);
			memcpy(hardwareInfo.cpuInfo.cpuType + 16 * t + 8, &(hardwareInfo.cpuInfo.reg.decx), 4);
			memcpy(hardwareInfo.cpuInfo.cpuType + 16 * t + 12, &(hardwareInfo.cpuInfo.reg.dedx), 4);
		}
	};

	__declspec(dllexport) void hardwareInfoScan(struct HardwareInfo &hardwareInfo) 
	{
		//getGPUInfo();
		getCPUInfo(hardwareInfo);
	};

	__declspec(dllexport) char * osInfoScan(char * osVersion) {
		/*
		返回值最后位'\n'
		*/
		memset(osVersion, 0, 128);
		HKEY hKey;
		const char dataSet[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, dataSet, 0, KEY_READ, &hKey))
		{
			char dwValue[128];
			DWORD dwSize = 128; // 注册表中数据的大小，可以设置稍大一些，否则第一次返回1，程序修改该值，第二次成功
			DWORD dwType = REG_SZ; // 注册表的键值的类型
			memset(dwValue, 0, 128);
			int ret;
			if (ret = RegQueryValueEx(hKey, "BuildLabEx", 0, &dwType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
				return 0;
			}
			else
			{
				strcat(osVersion, dwValue);
				strcat(osVersion, "\n");

				memset(dwValue, 0, 128);
				dwSize = 128;
				if (ret = RegQueryValueEx(hKey, "ReleaseId", 0, &dwType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS)
				{
					printf("%d", ret);
					RegCloseKey(hKey);
					return 0;
				}
				else
				{
					strcat(osVersion, dwValue);
					strcat(osVersion, "\n");
					RegCloseKey(hKey);
				}
			}
			return osVersion;
		}
	};
	__declspec(dllexport) void userInfoScan() {};
	__declspec(dllexport) void fileInfoScan() {};
}