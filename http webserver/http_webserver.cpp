#include "http_webserver.h";
#include <Winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <string>
#include <map>
#include <sstream>
#pragma comment(lib, "Ws2_32.lib")

namespace http {
	void exitWithError(const std::string &errorMessage) {
		std::cout << "Error: " << errorMessage << " Code: " << WSAGetLastError() << '\n';
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
		int m_new_socket = accept(m_socket, NULL, NULL);
		if (m_new_socket < 0) {
			exitWithError("Socket failed to accept incoming connection");
		}
		const int BUFFER_SIZE{ 30720 };
		char buffer[BUFFER_SIZE]{ 0 };
		int bytesRecieved = recv(m_new_socket, buffer, BUFFER_SIZE, 0);
		if (bytesRecieved == 0) {
			std::cout << "Failed to receive any bytes from client socket connection";
		}
		else if(bytesRecieved < 0) {
			exitWithError("Failed to recieve data");
		}
		std::string bufferString{ buffer };
		std::map<std::string, std::string> requestData;

		int nextLineBegins = bufferString.find("\r\n");
		std::cout << bufferString;

		bufferString.erase(0, nextLineBegins + 2);

		while (bufferString != "\r\n") {
			int colon = bufferString.find(":");
			int nextLineBegins = bufferString.find("\r\n");
			requestData.insert(std::make_pair(bufferString.substr(0, colon), bufferString.substr(colon + 2, nextLineBegins - (colon + 2))));
			bufferString.erase(0, nextLineBegins+2);

		}
		for (auto& kv : requestData) {
			std::cout << "KEY: `" << kv.first << "`, VALUE: `" << kv.second << '`' << std::endl;
		}

		
	}

	void TCPServer::closeServer() {
		closesocket(m_socket);
		WSACleanup();
		exit(0);
	}
}