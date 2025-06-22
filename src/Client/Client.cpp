#include "Client.h"
#include <iostream>

Client::Client(const char* serverIp, int port)
{
	m_socket.CreateSocket();
    m_socket.ConnectToRemoteHost(serverIp, port);
    printf("Connected to server!\n");
}

void Client::Chat()
{
	char* sendBuffer[MAXMSGBUFSIZE] = { nullptr };
	char* receiveBuffer[MAXMSGBUFSIZE] = { nullptr };

	while(true)
	{
		std::cout << "Client: ";
        std::cin.getline(*sendBuffer, sizeof(sendBuffer));

	// Client Exit
        if (std::string(*sendBuffer) == "exit") 
		{
            printf("Exit requested. Closing connection.\n");
			//m_socket.Send("Client closed connection.\n");
            break;
        }
        m_socket.Send(m_socket.GetSocket(), *sendBuffer);

		// Need to overload Receive for the client version that doesn't need to
		// know the fd of the sender
        int bytesReceived = m_socket.Receive(m_socket.GetSocket(),*receiveBuffer, sizeof(receiveBuffer));
        if (bytesReceived <= 0) {
			std::cerr << "Server disconnected\n";
            break;
        }
        
		std::cout << "Server: " << *receiveBuffer << "\n";
	}
}
