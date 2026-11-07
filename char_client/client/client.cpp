#include "client.hpp"

namespace mychat {
	Client::Client() {
		m_close = !init();
	}

	Client::~Client() {
		closesocket(m_connectionSocket.load());
		WSACleanup();
	}

	bool Client::init() {
        m_cmdParser.register_command("/help", [](const std::vector<std::string>& args) {
            std::cout << "/exit - exit from program\n/clear - clear console messages\n/connect - connect to other server";
        });

        m_cmdParser.register_command("/exit", [](const std::vector<std::string>& args) {
            std::cout << "Exiting from program...\n";
            exit(0);
        });

        m_cmdParser.register_command("/clear", [](const std::vector<std::string>& args) {
            std::system("cls");
        });

        m_cmdParser.register_command("/connect", [this](const std::vector<std::string>& args) {
            if (args.size() < 1) {
                std::cout << "error can not read the ip. use this syntax: /connect <server-ip>.\n";
                return 0;
            }

            closesocket(m_connectionSocket);

            sockaddr_in client_addr;
            client_addr.sin_family = AF_INET;
            inet_pton(AF_INET, args[0].c_str(), &client_addr.sin_addr);
            client_addr.sin_port = htons(PORT);

            mi_Result = connect(m_connectionSocket.load(), (SOCKADDR*)&client_addr, sizeof(client_addr));
            if (mi_Result == SOCKET_ERROR) {
                std::cerr << "Не удалось подключиться к серверу: " << WSAGetLastError() << "\n";
                closesocket(m_connectionSocket.load());
                WSACleanup();
                m_close = false;
            }
        });


        mi_Result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
        if (mi_Result != 0) {
            std::cerr << "WSAStartup error: " << mi_Result << "\n";
            return false;
        }

        m_connectionSocket.store(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        if (m_connectionSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation error: " << WSAGetLastError() << "\n";
            WSACleanup();
            return false;
        }

        return true;
	}

    void Client::plugin() {

        std::string server_ip;
        std::cout << "Enter IP address (enter for loopback): ";
        std::getline(std::cin, server_ip);
        if (server_ip.empty()) server_ip = "127.0.0.1";

        sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        inet_pton(AF_INET, server_ip.c_str(), &client_addr.sin_addr);
        client_addr.sin_port = htons(PORT);
    

        mi_Result = connect(m_connectionSocket.load(), (SOCKADDR*)&client_addr, sizeof(client_addr));
        if (mi_Result == SOCKET_ERROR) {
            std::cerr << "Не удалось подключиться к серверу: " << WSAGetLastError() << "\n";
            closesocket(m_connectionSocket.load());
            WSACleanup();
            m_close = false;
        }
    }

    void Client::poll() {

        std::thread reciveT(&Client::recive, this);
        reciveT.detach();

        std::string input_message;
        while (!m_close) {
            std::cout << "Your message: ";
            std::getline(std::cin, input_message);
            if (input_message.empty()) continue;

            if (input_message[0] == '/') m_cmdParser.parse(input_message);

            mi_Result = send(m_connectionSocket.load(), input_message.c_str(), (int)input_message.length(), NULL);
            if (mi_Result == SOCKET_ERROR) {
                std::cerr << "Send error: " << WSAGetLastError() << "\n";
            }
        }
    }

    void Client::recive() {
        char recvbuf[512];

        while (!m_close) {
            mi_reciveResult = recv(m_connectionSocket.load(), recvbuf, DEFAULT_BUFLEN, 0);
            if (mi_reciveResult > 0) {
                std::string message(recvbuf, mi_reciveResult);
                std::cout << "\nServer: " << message << "\nYour message: ";
                std::cout.flush();
            }

            else if (mi_reciveResult == 0) {
                std::cout << "\nServer broke connection.\n";
                break;
            }
            else {
                std::cerr << "\nData recive error: " << WSAGetLastError() << "\n";
                break;
            }
        }
    }
}