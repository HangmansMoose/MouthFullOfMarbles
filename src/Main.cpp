#include <cstdio>
#include <string>
#include <cctype>
// These 4 includes just for the ParseNetworkString function in the iphlpapi.h header
// TODO: come back and re-implement the ip addr parsing yourself.
#include <winsock2.h>
#include <ws2ipdef.h>
#include <WinDNS.h>
#include <iphlpapi.h>

#include "Server/Server.h"
#include "Client/Client.h"

#define DEFAULT_PORT 8008 // BOOB HA!


void PrintUsageAndExit(int rc = 1)
{
	fflush(stderr);
	printf("Usage:\n");
	printf("example_chat client SERVER_ADDR --port SERVER_PORT\n");
	printf("example_chat server [--port PORT]\n");
	
	fflush(stdout);
	exit(-1);
}

bool ParseIPAddrString(WCHAR* userInputIPv4)
{
	return ParseNetworkString(userInputIPv4, NET_STRING_IPV4_ADDRESS, 0, 0, 0);
}


int main(int argc, char* argv[])
{
	bool bServer = false;
	bool bClient = false;
	uint16_t nPort = DEFAULT_PORT;
	WCHAR* pIpv4Addr = nullptr;

	for (int i = 1; i < argc; ++i)
	{
		if (!bClient && !bServer)
		{
			// strcmp returns the number of different characters so if it
			// returns 0 (or false) then it means the strings are the same
			if (!strcmp(argv[i], "client"))
			{
				bClient = true;
				continue;
			}
			if (!strcmp(argv[i], "server"))
			{
				bServer = true;
				continue;
			}
		}
		if (!strcmp(argv[i], "--port"))
		{
			++i;
			if (i >= argc)
				PrintUsageAndExit();
			nPort = atoi(argv[i]);
			if (nPort <= 0 || nPort > 65535)
				fprintf(stderr,"Invalid port %d", nPort);
			continue;
		}

		// Anything else, must be server address to connect to
		if (bClient)
		{
			if (!ParseIPAddrString((WCHAR*)argv[i]))
			{
				fprintf(stderr, "Invalid server address '%s'", argv[i]);
				continue;
			}
			else 
			{
				pIpv4Addr = (WCHAR*)argv[i];
			}
		}

		PrintUsageAndExit();
	}

	// If they are both true some form of ridiculousness has happened
	if (bClient == bServer)
	{	
		PrintUsageAndExit();
	}
	// Create client and server sockets
	if (bClient)
	{
		Client* client = new Client();
		client->Run(pIpv4Addr, nPort);
	}
	else
	{
		Server* server = new Server();
		server->Run((uint16_t)nPort);
	}
}
