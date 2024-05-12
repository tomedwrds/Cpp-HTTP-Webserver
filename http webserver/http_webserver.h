#pragma once
#include <string> 
namespace http {
	class TCPServer {
	public:
		TCPServer();
		~TCPServer();
		void startListen();
		void acceptConnection();
	
	private:
		int m_socket{};
		int m_new_socket{};
		long m_incomingMessage{};
		unsigned int m_socketAddress_len{};
		std::string m_serverMessage{};

		int startServer();
		void closeServer();
		std::string parseResponse(const std::string &);
		void sendResponse(const std::string &);
		void sendFile(const std::string&);

		
	};
}