#include "Server.h"
#include "winbase.h"
#include "winsock2.h"
#include <cstdint>
#include <thread>
#include <vector>
#include <iostream>
#include <string>

Server::Server(int port) : m_listenPort(port) {}

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
       // char receiveBuffer[16384];
	   // char sendBuffer[16384];

       // while (true) {
       //     int bytesReceived = clientSocket.Receive(receiveBuffer, sizeof(receiveBuffer)); 
       //     if (bytesReceived <= 0) {
       //         fprintf(stderr,"Client disconnected\n");
       //         break;
       //     }
	   // 	// Ensure the receive buffer is null terminated to make strings happy
       //     receiveBuffer[bytesReceived] = '\0';

       //     if (std::string(receiveBuffer) == "exit") 
	   // 	{
       //         printf("Client requested exit. Closing connection.\n");
       //         break;
       //     }
	   // 	// Display client message
       //     
       //     printf("Client: %s\n", receiveBuffer);
       //     
       //     // Get server response
       //     printf("Server: ");
       //     std::cin.getline(sendBuffer, sizeof(sendBuffer));

       //     clientSocket.SendMessage(sendBuffer);
       // }
		
}

void Server::Run()
{
	// Allocate memory for the connectionStates array
	size_t arraySlotSize = sizeof(WSAPOLLFD) * MAXCONNECTIONS;
	m_connectionStates = (WSAPOLLFD *)malloc(arraySlotSize);
	
	InitClientStates(0, MAXCONNECTIONS);
	m_listenSocket->FindListenSocket(std::to_string(m_listenPort));
	printf("Server listening on port %d\n", m_listenPort);

	// Add this m_listenSocket->GetSocket() to the states set so the 0th element will always be the local m_listenSocket->GetSocket()
	// event flags: POLLPRI, POLLRDBAND, POLLRDNORM, POLLWRNORM
	// The POLLIN flag is a combination of POLLRDNORM and POLLRDBAND, both of which essentially indicate
	// that data can be read without blocking.
	// The POLLOUT flag is an alias of the POLLWRNORM flag and means data can be written without blocking

	m_connectionStates[0].fd = m_listenSocket->GetSocket();
	m_connectionStates[0].events = POLLIN;
	m_connectionStates[0].revents = 0;
	m_fdCount++; // Increment array length 

	m_running = true;
	
	// Get a handle to stdin
	HANDLE h_stdin = GetStdHandle(STD_INPUT_HANDLE);
	const int consoleInBufLen = 1028;
	INPUT_RECORD consoleInBuffer[consoleInBufLen];
	DWORD totalEvents = 0;

	while (m_running) 
	{
		PollClients();

		ReadConsoleInput(h_stdin, consoleInBuffer, consoleInBufLen, &totalEvents);
		bool controlPressed = false;

		for (uint32_t i = 0; i < totalEvents; i++)
		{
			switch(consoleInBuffer[i].EventType)
			{
				case KEY_EVENT:
					KEY_EVENT_RECORD key = consoleInBuffer[i].Event.KeyEvent;
					if(key.bKeyDown && (key.wVirtualKeyCode == VK_ESCAPE))
					{
						m_running = false;
						break;
					}
					
					//if(key.bKeyDown && (key.wVirtualKeyCode == 'C'))
					//{
					//	if(controlPressed)
					//	{
					//		printf("Control-C detected. Quitting!\n");
					//		m_running = false;
					//	}
					//	break;
					//}

			}
		}
		

	}
	printf("Escape detected, Quitting!\n");

}


void Server::PollClients()
{
	struct sockaddr_storage remoteAddr;
	Socket newClient;

	// First get WSAPoll to poll all the sockets in use from the m_connectionStates array
	int pollCount = WSAPoll(m_connectionStates,m_fdCount,-1);

	if (pollCount == SOCKET_ERROR)
	{
		fprintf(stderr,"Unexpected event occurred: %d\n",m_connectionStates[0].revents);
		DWORD dw_error = WSAGetLastError();
		m_listenSocket->GetErrorMessage(dw_error);
		exit(EXIT_FAILURE);
	}

	/* Run through the existing connections looking for data to read:             */
    for (int i = 0 ; i < m_fdCount ; i++)
    {
      if (m_connectionStates[i].revents & POLLIN || // We got one!!
          m_connectionStates[i].revents & POLLHUP)  // Hang up
      {
	/* If m_listenSocket->GetSocket() is ready to read, handle new connection:                       */
        if (m_connectionStates[i].fd == m_listenSocket->GetSocket())
        {
        newClient = m_listenSocket->AcceptConnection(); 
          if (newClient.GetSocket() == INVALID_SOCKET)
          {
            fprintf(stderr,"Accept failed, check the previous output from AcceptConnection\n");
          }
          else
          {
	        // Why are we just adding the FDs, should add the socket object
		    // Because WSAPoll expect an array of WSAPOLLFD...
		    AddNewClientFD(newClient.GetSocket());

            printf("pollserver: new connection from %s on socket %lld\n",
            newClient.GetAddress(),newClient.GetSocket());
          }
        }
	/* If not the m_listenSocket->GetSocket(), we're just a regular client:                          */
        else
        {
		  char clientBuffer[16384];
		  SOCKET senderFd = m_connectionStates[i].fd;
          bool bytesReceived = m_listenSocket->Receive(senderFd, clientBuffer, 
									                   sizeof(clientBuffer) / sizeof(char));
	/* Got error or connection closed by client: */
		  if(!bytesReceived)
		  {
		    printf("Either there was an error with the socket or the remote host has closed the connection\n");
		    printf("Removing fd from list and closing socket\n");
		    closesocket(m_connectionStates[i].fd); // Bye!
            RemoveClientFD(m_connectionStates[i].fd);
			continue;
		  }
	/* We got some good data from a client:*/
          else
          {
	/* Send to everyone!                                                          */
            for (int j = 0 ; j < m_fdCount ; j++)
            {
	          SOCKET destFd = m_connectionStates[j].fd;
	/* Except the m_listenSocket->GetSocket() and sender:                                            */
              if (destFd != m_listenSocket->GetSocket() && destFd != senderFd)
              {
			    if (!m_listenSocket->Send(destFd, clientBuffer))
			    {
			      printf("Error sending to socket with fd %lld, closing socket\n", destFd);

				  closesocket(m_connectionStates[i].fd); // Bye!
				  RemoveClientFD(m_connectionStates[i].fd);
			    }
			  }
		    }
	      }
	    } 
	  } 
    } 
}



// Add new connection to m_connectionStates array
bool Server::AddNewClientFD(SOCKET newClient)
{
	if(m_fdCount == MAXCONNECTIONS)
	{
		printf("Max concurrent connections reached\n");
		return false;
	}

	// Find a free slot
	for(int i = 0; i < m_fdSize; i++)
	{
		if (m_connectionStates[i].fd == -1)
		{
			printf("Found free slot\n");
			m_connectionStates[i].fd = newClient;
			m_connectionStates[i].events = POLLIN;
			m_connectionStates[i].revents = 0;
			m_fdCount++;
			return true;
		}
	}

	printf("Couldn't find a free slot. Perhaps the fdCount is not correct or dropped connections haven't\
			been freed correctly\n");
	return false;
}

// Remove client from m_connectionStates array 
bool Server::RemoveClientFD(SOCKET fdForRemoval)
{
	for(int i = 0; i < m_fdSize; i++)
	{
		if(m_connectionStates[i].fd == fdForRemoval)
		{
			m_connectionStates[i].fd	  = -1;
			m_connectionStates[i].events  =	 0;
			m_connectionStates[i].revents =  0;
			m_fdCount--;
			return true;
		}
	}

	printf("Couldn't find requested fd to remove.\n");
	return false;
}
