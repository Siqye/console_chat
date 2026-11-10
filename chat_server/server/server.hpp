#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <mutex>
#include <atomic>
#include "../cmd_parser/commandparser.hpp"

namespace mychat {
	class Server {
	private:
		CommandParser m_cmdParser;

		SOCKET m_clientSocket;
		SOCKET m_listenSocket;
		WSADATA m_wsaData;

		std::mutex m_mutex;

		bool m_close;
		std::atomic<bool> m_isConnected;

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
		void reopenListen();
		void sendDisconnectWarning();
		void disconnect();
		bool init();
		void recive();
	};
}