#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <atomic>

namespace mychat {
	class Client {
	private:
		std::atomic<SOCKET> m_connectionSocket;
		WSADATA m_wsaData;

		static const int DEFAULT_BUFLEN = 512;
		static const int PORT = 27015;

		int mi_Result;
		int mi_reciveResult;

		bool m_close;
	public:
		Client();
		~Client();

		void plugin();
		void poll();

	private:
		bool init();
		void recive();
	};
}