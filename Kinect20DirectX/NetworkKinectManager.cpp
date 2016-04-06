#pragma once
#include "NetworkKinectManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ws2tcpip.h>

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>

NetworkKinectManager::NetworkKinectManager(): m_connectSocket(INVALID_SOCKET)
{
}

NetworkKinectManager::~NetworkKinectManager()
{
    if (m_connectSocket != INVALID_SOCKET)
    {
        closesocket(m_connectSocket);
        WSACleanup();
    }
}

void NetworkKinectManager::InititializeClient(PCSTR szIPAddress)
{
    if (szIPAddress == nullptr)
    {
        throw E_INVALIDARG;
    }

    m_serverAddress = std::string(szIPAddress);

    WSADATA wsaData;
    struct addrinfo *result = NULL,
        *ptr = NULL,
        hints;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        char message[255] = { 0 };
        sprintf_s(message,"WSAStartup failed with error: %d\n", iResult);
        throw message;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(m_serverAddress.c_str(), DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        char message[255] = { 0 };
        sprintf_s(message, "getaddrinfo failed with error: %d\n", iResult);
        throw message;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        m_connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (m_connectSocket == INVALID_SOCKET) {
            WSACleanup();
            char message[255] = { 0 };
            sprintf_s(message, "socket failed with error: %ld\n", WSAGetLastError());
            throw message;
        }

        // Connect to server.
        iResult = connect(m_connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(m_connectSocket);
            m_connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
}

void NetworkKinectManager::InititializeServer()
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        char message[255] = { 0 };
        sprintf_s(message, "WSAStartup failed with error: %d\n", iResult);
        throw message;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;


    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        char message[255] = { 0 };
        sprintf_s(message, "getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        throw message;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        char message[255] = { 0 };
        sprintf_s(message, "socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        throw message;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        char message[255] = { 0 };
        sprintf_s(message, "bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        throw message;

    }
    freeaddrinfo(result);

    printf("listening");
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        char message[255] = { 0 };
        sprintf_s(message, "listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        throw message;
    }

    // Accept a client socket
    m_connectSocket = accept(ListenSocket, NULL, NULL);
    if (m_connectSocket == INVALID_SOCKET) {
        char message[255] = { 0 };
        sprintf_s(message, "accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        throw message;
    }

    // No longer need server socket
    closesocket(ListenSocket);
}

void NetworkKinectManager::ProcessRequests(KinectManager* kinectManager)
{
    UINT totalMessageSize = 0;
    size_t voxelCount = 0;
    Voxel* voxels = nullptr;
    while(true) {
        BYTE command = ReadCommand();
        switch (command)
        {
            case PROTOCOL_COMMAND_GET_VOXELS:
                printf("command 'Get Voxel' is received\n");
                voxels = nullptr;
                while (voxels == nullptr)
                {
                    voxels = kinectManager->AcquireVoxelBuffer(&voxelCount);
                    printf("%d of voxels will be sent\n", voxelCount);
                    if (voxels != nullptr)
                    {
                        // send total size of buffer with voxels ahead
                        totalMessageSize = voxelCount * sizeof(Voxel);
                        SendMessage((PBYTE)&totalMessageSize, sizeof(UINT));

                        // send buffer with voxels
                        SendMessage((PBYTE)voxels, voxelCount * sizeof(Voxel));
                        kinectManager->ReleaseVoxelBuffer();
                        break;
                    }
                    kinectManager->ReleaseVoxelBuffer();
                    
                }

                break;

        default:
            break;
        }
        

    } while (true);
}


Voxel * NetworkKinectManager::AcquireVoxelBuffer(size_t * voxelCount)
{
    UNREFERENCED_PARAMETER(voxelCount);
    *voxelCount = 0;

    if (m_connectSocket == INVALID_SOCKET) {
        char message[255] = { 0 };
        sprintf_s(message, "client is not connected");
        throw message;
    }

    BYTE requestVoxelCommand = PROTOCOL_COMMAND_GET_VOXELS;

    // Send command to request voxels
    SendMessage((PBYTE)&requestVoxelCommand, sizeof(BYTE));

    UINT totalSize = 0;
    ReadMessage((PBYTE)&totalSize, sizeof(UINT), 0);
    if (totalSize == 0)
    {
        return nullptr;
    }

    PBYTE pBuffer = (PBYTE)malloc(totalSize);
    ReadMessage(pBuffer, totalSize, 0);

    *voxelCount = totalSize / sizeof(Voxel);
    return (Voxel*)pBuffer;
}


void NetworkKinectManager::ReadMessage(PBYTE pBuff, UINT cbBytesNeeded, INT flags)
{
    int iResult = 0;
    UINT iCurOffset = 0;

    do
    {
        iResult = recv(m_connectSocket, (char*)(pBuff + iCurOffset), cbBytesNeeded - iCurOffset, flags);
        if (iResult == 0)
        {
            char message[255] = { 0 };
            sprintf_s(message, "Connection closed reading message size...stopping\n");
            throw message;
            
        }
        else if (iResult < 0)
        {
            char message[255] = { 0 };
            sprintf_s(message, "recv failed with error: %d\n", WSAGetLastError());
            throw message;
        }

        iCurOffset += iResult;

    } while (iCurOffset < cbBytesNeeded);
}

BYTE NetworkKinectManager::ReadCommand()
{
    BYTE command;
    ReadMessage((PBYTE)&command, sizeof(BYTE), 0);
    return command;
}

void NetworkKinectManager::SendCommand(BYTE command)
{
    BYTE commandCode = command;
    SendMessage((PBYTE)&commandCode, sizeof(BYTE));
}

void NetworkKinectManager::SendMessage(PBYTE pBuff, UINT cbBytes)
{
    int iSendResult = send(m_connectSocket, (char*)pBuff, cbBytes, 0);
    if (iSendResult == SOCKET_ERROR) {
        char message[255] = { 0 };
        sprintf_s(message, "send failed with error: %d\n", WSAGetLastError());
        throw message;
    }
}

