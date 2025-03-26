#pragma once
#include <stdint.h>
#include <map>

#include "Network/Sockets.h" 

#define BUFLEN 16384
#define MAXTHREADS 50

class Server {
public:
	// Singleton structure to ensure only one ever exists
	static Server* const server;
	bool Run();

private:

	int Receive();
	struct Client_t
	{
		std::string m_sClientAlias;
	};
	struct ConnInfo_t
	{
		HANDLE h_thread;
		SOCKET fileDescriptor;

	};

	Server(std::string listenAddr, std::string listenPort);
	char m_recBuffer[BUFLEN];
	int m_nRecBufLength = BUFLEN;
	std::map< ConnInfo_t, Client_t > m_clientMap;
	Socket m_socket;
	SOCKET m_listenFd;
	SOCKET m_newFd;
	std::string m_listenAddr;
	std::string m_listenPort;
	DWORD dw_threadStatus;
	struct ConnInfo_t m_threads[MAXTHREADS];




};		
