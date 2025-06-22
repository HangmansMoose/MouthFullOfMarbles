// These 4 includes just for the ParseNetworkString function in the iphlpapi.h header
// TODO: come back and re-implement the ip addr parsing yourself.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <cstdio>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <WinDNS.h>
#include <iphlpapi.h>

#include "Server/Server.h"
#include "Client/Client.h"
#include "GUI/GUI.h"

#pragma comment (lib, "iphlpapi.lib")

#define DEFAULT_PORT 8008

// OpenGL device context

void PrintUsageAndExit(int rc = 1)
{
	fflush(stderr);
	printf("Usage:\n");
	printf("example_chat client SERVER_ADDR --port SERVER_PORT\n");
	printf("example_chat server [--port PORT]\n");
	
	fflush(stdout);
	exit(-1);
}

// Convert the passed string to lower in place
void ArgsToLower(int argc, char* argv[])
{
	// HACK: This will only work for ASCII, unicode characters will break it.
	// There are other more involved options of course, but I just wanted to get
	// this up and running and have no intention of using cyrillic or emojis
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		for (char& c : arg)
		{
			if (c >= 'A' && c <= 'Z')
			{
				c += 32;
			}
		}
	}
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
	const char* p_ipv4Addr = nullptr;
	ArgsToLower(argc, argv);
	
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

		if (!strcmp(argv[i], "--ipaddress"))
		{
			++i;
			if (i >= argc)
				PrintUsageAndExit();
			p_ipv4Addr = argv[i];
			continue;
		}

		// Anything else, must be server address to connect to
		if (bClient)
		{
			if (!ParseIPAddrString((WCHAR*)p_ipv4Addr))
			{
				fprintf(stderr, "Invalid server address '%s'", argv[i]);
				continue;
			}
			else
			{
				break;
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
		Client client(p_ipv4Addr, nPort);
		ChatInterfaceRun();
		client.Chat();
	}
	else
	{
		
		Server server(nPort); 
		server.Run();
	}
}
