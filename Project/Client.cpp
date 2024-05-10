#include <iostream>
#include <winsock2.h>
#include <string>
#include "UserManagement.h"

#pragma comment(lib, "ws2_32.lib")  // Link with Winsock library

using namespace std;

class Client {
private:
    int clientID;
    string userName;
    SOCKET connSocket;
    sockaddr_in serverAddr;

public:
    Client(string name, int ID, string serverIP, int serverPort) : userName(name), clientID(ID) {
        // Initialize Winsock
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != NO_ERROR) {
            cerr << "WSAStartup failed with error: " << iResult << endl;
            return;
        }

        // Create a SOCKET for connecting to server
        connSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connSocket == INVALID_SOCKET) {
            cerr << "socket failed with error: " << WSAGetLastError() << endl;
            WSACleanup();
            return;
        }

        // Set up the sockaddr_in structure
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
        serverAddr.sin_port = htons(serverPort);

        // Connect to server.
        if (connect(connSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "connect failed with error: " << WSAGetLastError() << endl;
            closesocket(connSocket);
            WSACleanup();
            return;
        }
    }

    ~Client() {
        // Close the socket
        if (connSocket != INVALID_SOCKET) {
            closesocket(connSocket);
            WSACleanup();
        }
    }

    void sendMessage(string message) {
        int iResult = send(connSocket, message.c_str(), (int)message.length(), 0);
        if (iResult == SOCKET_ERROR) {
            cerr << "send failed with error: " << WSAGetLastError() << endl;
        } else {
            cout << "Client " << clientID << " sent message: " << message << endl;
        }
    }

    void receiveMessage() {
        char recvbuf[512];
        int iResult = recv(connSocket, recvbuf, 512, 0);
        if (iResult > 0) {
            recvbuf[iResult] = '\0';
            cout << "Client " << clientID << " received message: " << recvbuf << endl;
        } else if (iResult == 0) {
            cout << "Connection closed\n";
        } else {
            cerr << "recv failed with error: " << WSAGetLastError() << endl;
        }
    }

    int getClientID() const {
        return clientID;
    }
    string getUserName() const {
        return userName;
    }
};


int main() {
    try {
        std::string serverIP = "127.0.0.1"; // 服务器的IP地址
        int serverPort = 5019;              // 服务器监听的端口

        // 创建Client对象
        Client myClient("User1", 1, serverIP, serverPort);

        // 发送消息到服务器
        myClient.sendMessage("Hello, this is Client 1!");

        // 接收来自服务器的消息
        myClient.receiveMessage();

        // 可以循环发送和接收消息或者实现更复杂的交互逻辑
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}