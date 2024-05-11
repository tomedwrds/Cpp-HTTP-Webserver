#include "http_webserver.h";
#include <Winsock2.h>
#include <iostream>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

namespace http {
	void exitWithError(const std::string &errorMessage) {
		std::cout << errorMessage;
	}

	TCPServer::TCPServer()
	{
		startServer();
	}
	TCPServer::~TCPServer() {
		closeServer();
	}
	int TCPServer::startServer() {

		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			exitWithError("WSAStartup failed");
			return 1;
		}

		struct addrinfo* result, hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0;
		int status = getaddrinfo("0.0.0.0", "8080", &hints, &result);
		
		if (status) {
			std::cout << status;
			exitWithError("getaddrinfo failed");
			return 1;
		};


		m_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (m_socket < 0) {
			exitWithError("Cannot create socket");
			return 1;
		}

		if (bind(m_socket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
			exitWithError("Cannot bind socket");
			return 1;
		}
		freeaddrinfo(result);

		return 0;
	}

	void TCPServer::startListen() {
		SOCKET ClientSocket;
		if (listen(m_socket, 20) < 0) {
			exitWithError("Socket listen failed");
		}
		
	}

	void TCPServer::acceptConnection() {
		if (accept(m_socket, NULL, NULL) < 0) {
			exitWithError("Socket failed to accept incoming connection");
		}
	}

	void TCPServer::closeServer() {
		closesocket(m_socket);
		WSACleanup();
		exit(0);
	}
}