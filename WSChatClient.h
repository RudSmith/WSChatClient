#pragma once

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable: 4996)

#include <WinSock2.h>
#include <iostream>
#include <thread>
#include <string> 
#include <stdexcept>

namespace WSChat 
{
	
	enum class ErrorCode
	{
		NoError = 0,
		StartupError,
		NetworkError,
		EmptyNickname,
		NotUniqueNickname,
		UndefinedError
	};

	class WSChatClient
	{
	public:
		WSChatClient(const std::string &serverIP = "127.0.0.1", int serverPort = 1111);

		void connectToServer();
		void disconnectFromServer();

		void incomingMsgHandle();
		void outgoingMsgHandle();

	private:
		ErrorCode initializeNetwork();
		void authorize();

		WSAData m_wsaData;
		WORD m_DLLVersion;

		std::string m_serverIP;
		int m_serverPort;
		std::string m_nickname;

		SOCKADDR_IN m_serverAddr;
		SOCKET m_connToServer;

		bool m_isConnected;
	};

} // namespace WSChat