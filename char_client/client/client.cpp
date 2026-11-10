#include "client.hpp"

namespace mychat {
	Client::Client() {
		m_close = !init();
        m_isConnected = false;
	}

	Client::~Client() {
		closesocket(m_connectionSocket);
		WSACleanup();
	}

	bool Client::init() {
        m_cmdParser.register_command("/help", [](const std::vector<std::string>& args) {
            std::cout << "/exit - exit from program\n/clear - clear console\n/disconnect - close conneciton\n";
        });

        m_cmdParser.register_command("/exit", [](const std::vector<std::string>& args) {
            std::cout << "Exiting from program...\n";
            exit(0);
        });

        m_cmdParser.register_command("/clear", [](const std::vector<std::string>& args) {
            std::system("cls");
        });

        m_cmdParser.register_command("/disconnect", [this](const std::vector<std::string>& args) {
            disconnect();
        });

        mi_Result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
        if (mi_Result != 0) {
            std::cerr << "WSAStartup error: " << mi_Result << "\n";
            return false;
        }

        m_connectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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


        mi_Result = connect(m_connectionSocket, (SOCKADDR*)&client_addr, sizeof(client_addr));
        if (mi_Result == SOCKET_ERROR) {
            std::cerr << "Connection error: " << WSAGetLastError() << "\n";
            closesocket(m_connectionSocket);
            m_isConnected = false;
            return;
        }

        std::cout << "Succefuly connected to the server " << server_ip << "\n";
        m_isConnected = true;

        poll();
    }

    void Client::poll() {

        std::thread reciveT(&Client::recive, this);
        reciveT.detach();

        std::string input_message;
        while (!m_close) {

            if (!m_isConnected) {
                std::cout << "\n[Server close conneciton]\n";
                if (input_message.empty()) continue;

                if (input_message == "/exit") {
                    m_close = true;
                    break;
                }

                reconnect();
                continue;
            }

            std::cout << "Your message: ";
            std::getline(std::cin, input_message);
            if (input_message.empty()) continue;

            if (input_message[0] == '/') {
                m_cmdParser.parse(input_message);
                continue;
            }

            mi_Result = send(m_connectionSocket, input_message.c_str(), (int)input_message.length(), NULL);
            if (mi_Result == SOCKET_ERROR) {
                std::cerr << "Send error: " << WSAGetLastError() << "\n";
            }
        }
    }

    void Client::recive() {
        char recvbuf[512];

        while (m_isConnected) {
            mi_reciveResult = recv(m_connectionSocket, recvbuf, DEFAULT_BUFLEN, 0);
            if (mi_reciveResult > 0) {
                std::string message(recvbuf, mi_reciveResult);
                if (message == "[DISCONNECT]") {
                    disconnect();
                    return;
                }
                std::cout << "\nServer: " << message << "\nYour message: ";
                std::cout.flush();
            }
            else if (mi_reciveResult == 0) {
                std::cout << "\nServer close connection.\n";
                disconnect();
                return;
            }
            else {
                std::cerr << "\nData recieving error: " << WSAGetLastError() << "\n";
                disconnect();
                return;
            }
        }
    }

    void Client::disconnect() {
        m_isConnected = false;
        sendDisconnectWarning();
        closesocket(m_connectionSocket);
        std::cout << "\nServer close connection.\n";
        reconnect();
        plugin();
    }

    void Client::sendDisconnectWarning() {
        if (m_isConnected) {
            const std::string disconnect_message = "[DISCONNECT]";
            mi_Result = send(m_connectionSocket, disconnect_message.c_str(), (int)disconnect_message.length(), NULL);
            if (mi_Result > 0) {
                std::cout << "\nSending disconnect message to server...\n";
            }
        }
    }

    void Client::reconnect() {
        closesocket(m_connectionSocket);

        m_connectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_connectionSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation error: " << WSAGetLastError() << "\n";
            m_isConnected = false;
            return;
        }

        plugin();
    }
}