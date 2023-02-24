#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int MAX_CLIENTS = 10; // 最大客户端数
const int BUFFER_SIZE = 1024; // 缓冲区大小
const int PORT = 8888; // 端口号

vector<SOCKET> clients; // 客户端socket列表

void clientHandler(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            cout << "Client " << clientSocket << " disconnected." << endl;
            closesocket(clientSocket);
            return;
        }
        cout << "Received message: " << buffer << endl;
        // 将消息转发给其他客户端
        for (SOCKET& client : clients) {
            if (client != clientSocket) {
                send(client, buffer, bytesReceived, 0);
            }
        }
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    // 创建socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Failed to create socket." << endl;
        return 1;
    }

    // 绑定socket到端口
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Failed to bind socket to port." << endl;
        return 1;
    }

    // 监听连接
    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        cerr << "Failed to listen for connections." << endl;
        return 1;
    }
    cout << "Server listening on port " << PORT << endl;

    // 接受客户端连接并创建线程处理每个客户端
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    while (true) {
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Failed to accept client connection." << endl;
            continue;
        }
        cout << "Client " << clientSocket << " connected." << endl;
        clients.push_back(clientSocket);
        thread t(clientHandler, clientSocket);
        t.detach();
    }

    // 关闭socket和WSA
    for (SOCKET& client : clients) {
        closesocket(client);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
