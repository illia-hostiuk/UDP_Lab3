#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector<char> readFile(const char* fileName)
{
    ifstream file(fileName, ios::binary);

    return vector<char>(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );
}

void sendEnd(
    SOCKET sock,
    sockaddr_in& serverAddr
)
{
    sendto(
        sock,
        "END",
        3,
        0,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    );
}

void receiveResponse(SOCKET clientSocket)
{
    char response[32];

    int responseSize = recvfrom(
        clientSocket,
        response,
        sizeof(response) - 1,
        0,
        nullptr,
        nullptr
    );

    if (responseSize > 0)
    {
        response[responseSize] = '\0';

        cout << "Server response: "
            << response
            << endl;
    }
}

int main()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";

        return 1;
    }

    cout << "WinSock initialized\n";

    vector<char> fileData = readFile("test.tex");

    cout << "File loaded successfully\n";

    SOCKET clientSocket = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";

        WSACleanup();

        return 1;
    }

    cout << "UDP socket created\n";

    // ENABLE BROADCAST

    BOOL broadcastEnable = TRUE;

    setsockopt(
        clientSocket,
        SOL_SOCKET,
        SO_BROADCAST,
        (char*)&broadcastEnable,
        sizeof(broadcastEnable)
    );

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(644);

    // BROADCAST ADDRESS

    serverAddr.sin_addr.s_addr = INADDR_BROADCAST;

    // WHOLE FILE

    cout << "\n=============================\n";
    cout << "Sending whole file...\n";

    sendto(
        clientSocket,
        fileData.data(),
        fileData.size(),
        0,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    );

    receiveResponse(clientSocket);

    sendEnd(clientSocket, serverAddr);

    cout << "Whole file sent successfully\n";

    // 44 FRAGMENTS

    cout << "\n=============================\n";
    cout << "Sending file as 44 fragments...\n";

    int totalSize = fileData.size();

    int fragmentSize = totalSize / 44;

    int offset = 0;

    for (int i = 0; i < 44; i++)
    {
        int currentSize;

        if (i == 43)
            currentSize = totalSize - offset;
        else
            currentSize = fragmentSize;

        sendto(
            clientSocket,
            &fileData[offset],
            currentSize,
            0,
            (sockaddr*)&serverAddr,
            sizeof(serverAddr)
        );

        cout << "Fragment "
            << i + 1
            << " sent. Size: "
            << currentSize
            << endl;

        receiveResponse(clientSocket);

        offset += currentSize;
    }

    sendEnd(clientSocket, serverAddr);

    cout << "\n44 fragments sent successfully\n";

    closesocket(clientSocket);

    WSACleanup();

    cout << "\nUDP Client closed\n";

    return 0;
}