// http webserver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "http_webserver.h"
int main()
{
    using namespace http;
    TCPServer server = TCPServer();
    server.startListen();
    server.acceptConnection();
}
