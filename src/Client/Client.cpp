#include "Client.h"
#include <iostream>

Client::Client(const char* serverIp, int port)
{
	m_socket.CreateSocket();
    m_socket.ConnectToListener(serverIp, port);
    printf("Connected to server!\n");
}

void Client::EnterChat()
{
	char sendBuffer[MAXMSGBUFSIZE];
	char receiveBuffer[MAXMSGBUFSIZE];

	while(true)
	{
		std::cout << "Client: ";
        std::cin.getline(sendBuffer, sizeof(sendBuffer));

	// Client Exit
        if (std::string(sendBuffer) == "exit") 
		{
            printf("Exit requested. Closing connection.\n");
			m_socket.SendMessage("Client closed connection.\n");
            break;
        }
        m_socket.SendMessage(sendBuffer);

        int bytesReceived = m_socket.Receive(receiveBuffer, sizeof(receiveBuffer));
        if (bytesReceived <= 0) {
			std::cerr << "Server disconnected\n";
            break;
        }
        
		std::cout << "Server: " << receiveBuffer << "\n";
	}
}
