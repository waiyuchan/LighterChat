#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int BUFFER_SIZE = 1024; // 缓冲区大小
const int PORT = 8888; // 端口号
const char* SERVER_ADDRESS = "127.0.0.1"; // 服务器地址

void receiveHandler(SOCKET serverSocket) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        bytesReceived = recv(serverSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            cout << "Disconnected from server." << endl;
            closesocket(serverSocket);
            exit(0);
        }
        cout << "Received message: " << buffer << endl;
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

    // 连接到服务器
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr);
    if (connect(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Failed to connect to server." << endl;
        return 1;
    }
    cout << "Connected to server." << endl;

    // 接收消息的线程
    thread receiveThread(receiveHandler, serverSocket);
    receiveThread.detach();

    // 发送消息
    char buffer[BUFFER_SIZE];
    int bytesSent;
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        cin.getline(buffer, BUFFER_SIZE);
        bytesSent = send(serverSocket, buffer, strlen(buffer), 0);
        if (bytesSent == SOCKET_ERROR) {
            cerr << "Failed to send message." << endl;
            break;
        }
    }

    // 关闭socket和WSA
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
