#include "WSChatClient.h"

namespace WSChat 
{

	WSChatClient::WSChatClient(const std::string& serverIP, int serverPort)
		: m_DLLVersion{ MAKEWORD(2, 1) },
		m_wsaData{},
		m_connToServer{},
		m_nickname{},
		m_serverIP{ serverIP },
		m_serverPort{ serverPort },
		m_serverAddr{},
		m_isConnected { false }
	{ }

	void WSChatClient::connectToServer()
	{
		if (initializeNetwork() == ErrorCode::NoError)
		{
			m_isConnected = true;
			authorize();
			std::thread listenToUserInput(&WSChatClient::outgoingMsgHandle, this);
			listenToUserInput.detach();

			incomingMsgHandle();
		}
	}

	void WSChatClient::incomingMsgHandle()
	{
		size_t msgLen;
		std::string msg;

		// Получаем длину сообщения и сообщение, выводим его
		while (m_isConnected)
		{
			try
			{
				recv(m_connToServer, (char*)&msgLen, sizeof(size_t), NULL);
				msg.resize(msgLen + 1);
				recv(m_connToServer, (char*)msg.data(), msgLen, NULL);
				msg[msgLen] = '\0';
				std::cout << msg << std::endl;
			}
			catch (std::length_error)
			{
				std::cout << "Connection closed." << std::endl;
			}
		}
	}

	void WSChatClient::outgoingMsgHandle()
	{
		size_t msgLen;
		std::string msg;

		while (true) 
		{
			std::getline(std::cin, msg);
			size_t msgLen = msg.size();
			send(m_connToServer, (char*)&msgLen, sizeof(size_t), NULL);
			send(m_connToServer, (char*)msg.data(), msg.size(), NULL);

			if (msg == "/quit") {
				std::cout << "Client: disconnecting from server." << std::endl;
				shutdown(m_connToServer, SD_SEND);
				closesocket(m_connToServer);
				WSACleanup();

				m_isConnected = false;
				break;
			}

			Sleep(10);
		}
	}

	ErrorCode WSChatClient::initializeNetwork()
	{
		// Загружаем библиотеку winsock
		if (WSAStartup(m_DLLVersion, &m_wsaData) != 0)
		{
			std::cout << "LOG: Error loading winsock." << std::endl;
			return ErrorCode::StartupError;
		}

		unsigned long ipAddr = INADDR_NONE;
		ipAddr = inet_addr(m_serverIP.c_str());

		if (ipAddr == INADDR_NONE)
		{
			std::cout << "LOG: Wrong IP format." << std::endl;
			return ErrorCode::StartupError;
		}

		m_serverAddr.sin_addr.s_addr = ipAddr;
		m_serverAddr.sin_port = htons(m_serverPort);
		m_serverAddr.sin_family = AF_INET;

		// Коннектимся к серверу
		m_connToServer = socket(AF_INET, SOCK_STREAM, NULL);
		if (connect(m_connToServer, (SOCKADDR*)&m_serverAddr, sizeof(m_serverAddr)) != 0) {
			std::cout << "LOG: failed to connect to server.\n";
			return ErrorCode::NetworkError;
		}

		return ErrorCode::NoError;
	}

	void WSChatClient::authorize()
	{
		ErrorCode error = ErrorCode::UndefinedError;

		while (error != ErrorCode::NoError) {
			std::cout << "Please, enter your nickname: ";
			std::getline(std::cin, m_nickname);
			int msgLen = m_nickname.size();

			send(m_connToServer, (char*)&msgLen, sizeof(int), NULL);
			send(m_connToServer, (char*)m_nickname.data(), m_nickname.size(), NULL);

			recv(m_connToServer, (char*)&error, sizeof(ErrorCode), NULL);

			switch (error)
			{
			case ErrorCode::EmptyNickname:
				std::cout << "You entered empty nickname. Please reconnect with correct nickname." << std::endl;
				system("pause");
				break;
			case ErrorCode::NotUniqueNickname:
				std::cout << "Nickname you entered is used by another user. Please reconnect with correct nickname." << std::endl;
				system("pause");
				break;
			default:
				std::cout << "Hello, " << m_nickname << "!" << std::endl;
				break;
			}
		}
	}
} // namespace WSChat