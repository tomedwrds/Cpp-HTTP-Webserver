#include "http_webserver.h";
#include <Winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <thread>

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
			closeServer();
			return 1;
		}

		struct addrinfo* result, hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0;
		int status = getaddrinfo("0.0.0.0", "80", &hints, &result);
		
		if (status) {
			std::cout << status;
			exitWithError("getaddrinfo failed");
			return 1;
		};


		m_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (m_socket < 0) {
			exitWithError("Failed to create socket");
			return 1;
		}

		if (bind(m_socket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
			exitWithError("Failed to bind to socket");
			return 1;
		}
		freeaddrinfo(result);

		return 0;
	}

	void TCPServer::startListen() {
		SOCKET ClientSocket;
		if (listen(m_socket, 100) < 0) {
			exitWithError("Socket listen failed");
		}
		std::cout << "Starting listen on 0.0.0.0/80\n";
		
	}

	void TCPServer::acceptConnection() {
		while (1) {

			//Accept incoming connection
			m_new_socket = accept(m_socket, NULL, NULL);
			if (m_new_socket < 0) {
				exitWithError("Socket failed to accept incoming connection");
			}
			std::cout << "Connection made\n";

			//Receive data on incoming conection
			const int BUFFER_SIZE{ 1024 };
			char buffer[BUFFER_SIZE]{ 0 };

			int bytesRecieved{ recv(m_new_socket, buffer, BUFFER_SIZE, 0) };
			std::cout << bytesRecieved;
			if (bytesRecieved < 0) {
				exitWithError("Failed to recieved any bytes from client due to error.");
			}
			else if(bytesRecieved == 0) {
				std::cout << WSAGetLastError();
			}
			std::cout << buffer;
			//Parse response
			std::string bufferString{ buffer };
			std::string requestFile{ parseResponse(bufferString) };
			
			std::cout << "Request made for " << requestFile << '\n';

			//check if request is malformed
			try {
				sendFile(requestFile);

			} catch(...) {
				sendResponse("HTTP/1.1 400 Bad Request");

			}
			shutdown(m_new_socket, SD_SEND);
			closesocket(m_new_socket);

		}
		
		
	}

	std::string TCPServer::parseResponse(const std::string &buffer) {
		
		//go foward from dash to get path
		int startIndex{ 5 };
		int i{ 0 };
		while (buffer.at(startIndex + i) != ' ') {
			i++;
		}
		//return the file path
		return buffer.substr(startIndex, i);
		
	}

	void TCPServer::sendFile(const std::string& filePath) {

		//Fetch file
		std::ifstream inFile;
		inFile.open(filePath);
		std::string message{};
		std::stringstream strStream;

		if (inFile.fail()) {
			sendResponse("HTTP/1.1 404 Not Found\r\n\r\n");
		}
		else {
			strStream << inFile.rdbuf();
			sendResponse("HTTP/1.1 200 OK\r\nContent - Type: text / html\r\n\r\n" + strStream.str());
		}

	}

	void TCPServer::sendResponse(const std::string &message) {
		int bytesSent;
		long totalBytesSent{ 0 };

		while (totalBytesSent < message.size()) {
			bytesSent = send(m_new_socket, message.c_str(), message.size(), 0);
			if (bytesSent < 0) {
				exitWithError("Failed to send response");
			}
			else {
				totalBytesSent += bytesSent;
			}
		}
		std::cout << totalBytesSent << " bytes sent\n";

	}



	void TCPServer::closeServer() {
		closesocket(m_socket);
		WSACleanup();
		exit(0);
	}
}