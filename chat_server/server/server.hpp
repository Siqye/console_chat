#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include "../cmd_parser/commandparser.hpp"

namespace mychat { // да
	class Server {
	private:
		CommandParser m_cmdParser;

		SOCKET m_clientSocket;
		SOCKET m_listenSocket;
		WSADATA m_wsaData;

		bool m_close;
		bool m_isConnected;

		int mi_result;
		int mi_reciveResult;

		static const int BUFFER_LENGHT = 512;
		static const int PORT = 27015;
	public:
		Server();
		~Server();

		void pollConnection();
		void poll();

	private:
		void sendDisconnectWarning();
		void disconnect();
		bool init();
		void recive();
	};
}