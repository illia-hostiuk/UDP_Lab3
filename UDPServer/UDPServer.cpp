#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void receiveFile(
    SOCKET serverSocket,
    const char* outputFile
)
{
    sockaddr_in clientAddr;

    int clientSize = sizeof(clientAddr);

    char buffer[1024];

    ofstream file(outputFile, ios::binary);

    cout << "Receiving file...\n";

    while (true)
    {
        int bytesReceived = recvfrom(
            serverSocket,
            buffer,
            sizeof(buffer),
            0,
            (sockaddr*)&clientAddr,
            &clientSize
        );

        if (bytesReceived <= 0)
            break;

        // END marker
        if (bytesReceived == 3 &&
            buffer[0] == 'E' &&
            buffer[1] == 'N' &&
            buffer[2] == 'D')
        {
            break;
        }

        file.write(buffer, bytesReceived);

        cout << "Received block size: "
            << bytesReceived
            << endl;

        // RESPONSE TO CLIENT
        sendto(
            serverSocket,
            "OK",
            2,
            0,
            (sockaddr*)&clientAddr,
            clientSize
        );
    }

    file.close();

    cout << "File saved as: "
        << outputFile
        << endl;
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

    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";

        WSACleanup();

        return 1;
    }

    cout << "UDP socket created\n";

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(644);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(
        serverSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    ) == SOCKET_ERROR)
    {
        cout << "Bind failed. Error: "
            << WSAGetLastError()
            << endl;

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }

    cout << "Bind successful\n";

    cout << "\n=============================\n";
    cout << "WAITING WHOLE FILE\n";

    receiveFile(
        serverSocket,
        "received_whole.tex"
    );

    cout << "\n=============================\n";
    cout << "WAITING 44 FRAGMENTS\n";

    receiveFile(
        serverSocket,
        "received_fragments.tex"
    );

    closesocket(serverSocket);

    WSACleanup();

    cout << "\nUDP Server closed\n";

    return 0;
}