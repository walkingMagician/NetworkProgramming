#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <map>
#include <mutex>


#define DEFAULT_PORT			"27015"
#define DEFAULT_BUFFER_LENGTH	1500

extern int clientID;
extern std::map<SOCKET, int> clientIds;
extern std::mutex mtx_client;

struct ThreadParameters
{
	SOCKET socket;
	int id;
	bool run_flag;
};