#include "NetworkKinectManager.h"

#include <d3d11_1.h>
#include <directxmath.h>
#include "MoveableObject.h"


using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
// Constant buffer used to send MVP matrices to the vertex shader.
struct ProjectionConstantBuffer
{
    DirectX::XMFLOAT4X4 projection;
};

struct ModelConstantBuffer
{
    DirectX::XMFLOAT4X4 model;
};


class  CameraSnapshot : public MoveableObject
{
public:
    CameraSnapshot();
    bool Initialize(ID3D11Device*  pd3dDevice, NetworkKinectManager& kinectManager, XMMATRIX& originTransformationMatrix);
    void Draw(ID3D11DeviceContext* pImmediateContext);
    void DumpTransformationMatrix(std::string& dirPath);
    ~CameraSnapshot();

private:
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pModelConstantBuffer;
    size_t m_vertexCount;
    XMMATRIX m_originTransformationMatrix;

    std::string m_serverAddress;
};
