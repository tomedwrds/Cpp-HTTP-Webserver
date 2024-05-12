#include "http_webserver.h";
#include <Winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <string>
#include <map>
#include <sstream>
#include <fstream>

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
		while (1) {
			int m_new_socket = accept(m_socket, NULL, NULL);
			if (m_new_socket < 0) {
				exitWithError("Socket failed to accept incoming connection");
			}
			const int BUFFER_SIZE{ 30720 };
			char buffer[BUFFER_SIZE]{ 0 };
			int bytesRecieved = recv(m_new_socket, buffer, BUFFER_SIZE, 0);
			if (bytesRecieved == 0) {
				exitWithError("Failed to receive any bytes from client socket connection");
			}
			else if (bytesRecieved < 0) {
				exitWithError("Failed to recieved any bytes from client due to error.");
			}

			std::string bufferString{ buffer };
			std::string requestFile{ parseResponse(bufferString) };
			//std::map<std::string, std::string> requestData;


			//int nextLineBegins = bufferString.find("\r\n");

			/*bufferString.erase(0, nextLineBegins + 2);

			while (bufferString != "\r\n") {
				int colon = bufferString.find(":");
				int nextLineBegins = bufferString.find("\r\n");
				requestData.insert(std::make_pair(bufferString.substr(0, colon), bufferString.substr(colon + 2, nextLineBegins - (colon + 2))));
				bufferString.erase(0, nextLineBegins + 2);

			}*/

			std::cout << requestFile;

			//respond
			int bytesSent;
			long totalBytesSent{ 0 };
			std::ifstream inFile;
			inFile.open(requestFile);

			std::stringstream strStream;
			strStream << inFile.rdbuf();
			std::string message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + strStream.str();
			while (totalBytesSent < message.size()) {
				bytesSent = send(m_new_socket, message.c_str(), message.size(), 0);
				totalBytesSent += bytesSent;
			}
			closesocket(m_new_socket);

		}
		
		
	}

	std::string TCPServer::parseResponse(const std::string &buffer) {
		//go foward from dash to get path
		int startIndex{5};
		int i{ 0 };
		while (buffer.at(startIndex + i) != ' ') {
			i++;
		}
		//return the file path
		return buffer.substr(startIndex, i);
	}


	void TCPServer::closeServer() {
		closesocket(m_socket);
		WSACleanup();
		exit(0);
	}
}