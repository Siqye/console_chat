#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <atomic>
#include "../cmd_parser/commandparser.hpp"

namespace mychat {
	class Client {
	private:
		SOCKET m_connectionSocket;
		WSADATA m_wsaData;
		CommandParser m_cmdParser;

		static const int DEFAULT_BUFLEN = 512;
		static const int PORT = 27015;

		int mi_Result;
		int mi_reciveResult;
		// status
		bool m_close; 
		bool m_isConnected;
	public:
		Client();
		~Client();

		void plugin();
		void poll();

	private:
		void disconnect();

		void sendDisconnectWarning();

		bool init(); 
		void recive();
	};
}