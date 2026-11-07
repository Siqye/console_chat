#include "server.hpp"

namespace mychat {
	Server::Server() {
		m_close = !init();
	}

	Server::~Server() {
		closesocket(m_clientSocket);
		closesocket(m_listenSocket);
		WSACleanup();
	}

	bool Server::init() {
        m_cmdParser.register_command("/help", [](const std::vector<std::string>& args) {
            std::cout << "/exit - exit from program\n";
        });

        m_cmdParser.register_command("/exit", [](const std::vector<std::string>& args) {
            std::cout << "Exiting from program...\n";
            exit(0);
        });

        m_cmdParser.register_command("/clear", [](const std::vector<std::string>& args) {
            std::system("cls");
        });

        m_cmdParser.register_command("/disconnect", [this](const std::vector<std::string>& args) {
            this->closeConnection();
            this->pollConnection();
        });

        WSADATA wsaData;
        mi_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (mi_result != 0) {
            std::cerr << "WSAStartup error: " << mi_result << "\n";
            return false;
        }

        m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listenSocket == INVALID_SOCKET) {
            std::cerr << "Socket error: " << WSAGetLastError() << "\n";
            WSACleanup();
            return false;
        }

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        mi_result = bind(m_listenSocket, (SOCKADDR*)&server_addr, sizeof(server_addr));
        if (mi_result == SOCKET_ERROR) {
            std::cerr << "Bind error: " << WSAGetLastError() << "\n";
            closesocket(m_listenSocket);
            WSACleanup();
            return false;
        }

        mi_result = listen(m_listenSocket, SOMAXCONN);
        if (mi_result == SOCKET_ERROR) {
            std::cerr << "Listen error: " << WSAGetLastError() << "\n";
            closesocket(m_listenSocket);
            WSACleanup();
            return false;
        }

        std::cout << "Server started waiting for client...\n";
        return true;
	}

	void Server::pollConnection() {
		m_clientSocket = accept(m_listenSocket, NULL, NULL);
		if (m_listenSocket == INVALID_SOCKET) {
			std::cerr << "Error (accept): " << WSAGetLastError() << "\n";
			closesocket(m_listenSocket);
			WSACleanup();
		}

		std::cout << "Client succesfuly connected!\n";
		closesocket(m_listenSocket);
	}

    void Server::poll() {
        std::thread reciveT(&Server::recive, this);
        reciveT.detach();

        std::string input_message;
        while (!m_close) {
            std::cout << "Your message: ";
            std::getline(std::cin, input_message);
            if (input_message.empty()) continue;

            if (m_cmdParser.parse(input_message)) continue;

            mi_result = send(m_clientSocket, input_message.c_str(), (int)input_message.length(), 0);
            if (mi_result == SOCKET_ERROR) {
                std::cerr << "Send error: " << WSAGetLastError() << "\n";
                break;
            }
        }
    }

    void Server::recive() {
        char recvbuf[BUFFER_LENGHT];

        while (!m_close) {
            mi_reciveResult = recv(m_clientSocket, recvbuf, BUFFER_LENGHT, 0);
            if (mi_reciveResult > 0) {
                std::string message(recvbuf, mi_reciveResult);
                std::cout << "\nClient: " << message << "\nYour message: ";
                std::cout.flush();
            }
            else if (mi_reciveResult == 0) {
                std::cout << "\nClient disconnected.\n";
            }
            else {
                std::cout << "\nData recieving error: " << WSAGetLastError() << "\n";
            }
        }
    }

    void Server::closeConnection() {
        closesocket(m_clientSocket);
        pollConnection();
    }
} 