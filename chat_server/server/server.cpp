#include "server.hpp"


namespace mychat {
	Server::Server() {
		m_close = !init();
        m_isConnected.store(false);
	}

	Server::~Server() {
		closesocket(m_clientSocket);
		closesocket(m_listenSocket);
		WSACleanup();
	}

	bool Server::init() {
		m_cmdParser.register_command("/help", [](const std::vector<std::string>& args) {
			std::cout << "/exit - exit from program\n/clear - clear console\n/disconnect - close connection\n";
		});

		m_cmdParser.register_command("/exit", [](const std::vector<std::string>& args) {
			std::cout << "Exiting...\n";
			exit(0);
		});

		m_cmdParser.register_command("/clear", [](const std::vector<std::string>& args) {
			std::system("cls");
		});

		m_cmdParser.register_command("/disconnect", [this](const std::vector<std::string>& args) {
            void sendDisconnectWarning();
			disconnect();
		});

        WSADATA wsaData;
        mi_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (mi_result != 0) {
#           ifdef DEVTEST
            std::cout << "WSAStartup error: " << mi_result << "\n";
#           endif
            return false;
        }

        m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listenSocket == INVALID_SOCKET) {
#           ifdef DEVTEST
            std::cout << "Socket error: " << WSAGetLastError() << "\n";
#           endif
            WSACleanup();
            return false;
        }

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        mi_result = bind(m_listenSocket, (SOCKADDR*)&server_addr, sizeof(server_addr));
        if (mi_result == SOCKET_ERROR) {
#           ifdef DEVTEST
            std::cout << "Bind error: " << WSAGetLastError() << "\n";
#           endif
            closesocket(m_listenSocket);
            WSACleanup();
            return false;
        }

        mi_result = listen(m_listenSocket, SOMAXCONN);
        if (mi_result == SOCKET_ERROR) {
#           ifdef DEVTEST
            std::cout << "Listen error: " << WSAGetLastError() << "\n";
#           endif
            closesocket(m_listenSocket);
            WSACleanup();
            return false;
        }

        std::cout << "Server started waiting for client...\n";
        return true;
	}

	void Server::pollConnection() {

		m_clientSocket = accept(m_listenSocket, NULL, NULL);
		if (m_clientSocket == INVALID_SOCKET) {
#           ifdef DEVTEST
			std::cout << "Error (accept): " << WSAGetLastError() << "\n";
#           endif
            return;
		}

		m_isConnected.store(true);
		
        std::cout << "Client succesfuly connected!\n";

        closesocket(m_listenSocket);

        poll();
	}

    void Server::poll() {
        std::thread reciveT(&Server::recive, this);
        reciveT.detach();

        std::string input_message;
        while (m_isConnected) {
            std::cout << "Your message: ";
            std::getline(std::cin, input_message);
            if (input_message.empty()) continue;

            if (m_cmdParser.parse(input_message)) continue;

            mi_result = send(m_clientSocket, input_message.c_str(), (int)input_message.length(), 0);
            if (mi_result == SOCKET_ERROR) {
#               ifdef DEVTEST
                std::cout << "Send error: " << WSAGetLastError() << "\n";
#               endif
            }
        }
    }

    void Server::recive() {
        char recvbuf[BUFFER_LENGHT];

        while (m_isConnected) {
            mi_reciveResult = recv(m_clientSocket, recvbuf, BUFFER_LENGHT, 0);
            if (mi_reciveResult > 0) {
                std::string message(recvbuf, mi_reciveResult);
                if (message == "[DISCONNECT]") {
                    m_isConnected = false;
                    disconnect();
                    return;
                }
                std::cout << "\nClient: " << message << "\nYour message: ";
                std::cout.flush();
            }
            else if (mi_reciveResult == 0) {
#               ifdef DEVTEST
                std::cout << "\nClient disconnected.\n";
#               endif
                return;
            }
            else {
                std::cout << "\nData recieving error: " << WSAGetLastError() << "\n";
                return;
            }
        }
    }

    void Server::disconnect() {
        m_isConnected = false;
        closesocket(m_clientSocket);
        closesocket(m_listenSocket);
        reopenListen();
    }

    void Server::sendDisconnectWarning() {
        if (m_isConnected) {
            const std::string disconnect_message = "[DISCONNECT]";
            mi_result = send(m_clientSocket, disconnect_message.c_str(), (int)disconnect_message.length(), NULL);
            if (mi_result > 0) {
                std::cout << "\nSending disconnect message to client...\n";
            }
        }
    }

    void Server::reopenListen() {
        m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listenSocket == INVALID_SOCKET) {
#           ifdef DEVTEST
            std::cout << "Socket error: " << WSAGetLastError() << "\n";
#           endif
        }

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        mi_result = bind(m_listenSocket, (SOCKADDR*)&server_addr, sizeof(server_addr));
        if (mi_result == SOCKET_ERROR) {
#           ifdef DEVTEST
            std::cout << "Bind error: " << WSAGetLastError() << "\n";
#           endif
        }

        mi_result = listen(m_listenSocket, SOMAXCONN);
        if (mi_result == SOCKET_ERROR) {
#           ifdef DEVTEST
            std::cout << "Listen error: " << WSAGetLastError() << "\n";
#           endif
        }

        std::cout << "Listen socket reopened. waiting for new connection...\n";

        pollConnection();
    }
} 