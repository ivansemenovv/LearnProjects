#include <string>
#include "Voxel.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#include "KinectManager.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PROTOCOL_COMMAND_GET_VOXELS 0x01

class NetworkKinectManager sealed
{
public: 
    NetworkKinectManager();
    NetworkKinectManager::~NetworkKinectManager();

    // client functions
    void InititializeClient(PCSTR szIPAddress);
    Voxel* AcquireVoxelBuffer(size_t* voxelCount);
    
    // server funxtions
    void InititializeServer();
    void ProcessRequests(KinectManager* kinectManager);
    std::string& GetKinectAddress()
    {
        return m_serverAddress;
    }

private:
    void ReadMessage(PBYTE pBuff, UINT cbBytesNeeded, INT flags);
    void SendMessage(PBYTE pBuff, UINT cbBytes);
    void SendCommand(BYTE command);
    BYTE ReadCommand();

    Voxel* m_ActiveVoxelBuffer;
    std::string m_serverAddress;
    SOCKET m_connectSocket;
};