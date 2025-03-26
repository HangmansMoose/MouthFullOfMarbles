#include "Server.h"

Server::Server(std::string listenAddr, std::string listenPort)
: m_listenAddr(listenAddr), m_listenPort(listenPort)
{
	// Initialise the list of threads
	for (int i = 0; i < MAXTHREADS; i++)
	{
		m_threads[i].h_thread = NULL;
		m_threads[i].fileDescriptor = 0;
	}

}

bool Server::Run()
{
	if (!m_socket.StartListener(m_listenPort))
	{
		printf("Starting listener failed.Check then error output.\n");
		return false;
	}

	return true; 
}


int Server::Receive()
{

}
	
	

