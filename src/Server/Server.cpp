#include "Server.h"
#include <thread>
#include <vector>
#include <iostream>
#include <string>


Server::~Server()
{
	// Clean up all the remaining threads

	delete m_listenSocket; 
}

void Server::InitClientStates(int start, int end)
{
	for (int i = start; i < end; i++)
	{
		m_connectionStates[i].fd = -1; // Negative means fd is free / WSAPOLL it to ignore socket
		m_connectionStates[i].events = 0;
		m_connectionStates[i].revents = 0;
	}
}

void Server::HandleClient(Socket clientSocket) 
{
        char receiveBuffer[16384];
		char sendBuffer[16384];

        while (true) {
            int bytesReceived = clientSocket.Receive(receiveBuffer, sizeof(receiveBuffer)); 
            if (bytesReceived <= 0) {
                fprintf(stderr,"Client disconnected\n");
                break;
            }
			// Ensure the receive buffer is null terminated to make strings happy
            receiveBuffer[bytesReceived] = '\0';

            if (std::string(receiveBuffer) == "exit") 
			{
                printf("Client requested exit. Closing connection.\n");
                break;
            }
			// Display client message
            
            printf("Client: %s\n", receiveBuffer);
            
            // Get server response
            printf("Server: ");
            std::cin.getline(sendBuffer, sizeof(sendBuffer));

            clientSocket.SendMessage(sendBuffer);
        }
		
}

void Server::Run(int port)
{
	// Allocate memory for the connectionStates array
	size_t arraySlotSize = sizeof(WSAPOLLFD) * MAXCONNECTIONS;
	m_connectionStates = (WSAPOLLFD *)malloc(arraySlotSize);
	
	InitClientStates(0, MAXCONNECTIONS);
	m_listenSocket->FindListenSocket(std::to_string(port));
	printf("Server listening on port %d\n", port);

	// Add this listener to the states set so the 0th element will always be the local listener
	m_connectionStates[0].fd = m_listenSocket->GetSocket();
	m_connectionStates[0].events = POLLIN;
	m_connectionStates[0].revents = 0;
	m_fdCount++; // Increment array length 

	while (true) 
	{
        Socket clientSocket = m_listenSocket->AcceptConnection();
        if (clientSocket.GetSocket() != INVALID_SOCKET) {
            printf("New client connected!\n");
        }

        // Launch a new thread for each client
		// TODO: I need to remove the threads once the connection is closed otherwise
	}
}

