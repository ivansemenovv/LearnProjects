#include <d3d11_1.h>
#include <directxmath.h>
#include "MoveableObject.h"
#include "KinectManager.h"

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
    bool Initialize(ID3D11Device*  pd3dDevice, KinectManager& kinectManager);
    void Draw(ID3D11DeviceContext* pImmediateContext);

    ~CameraSnapshot();

private:
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11Buffer* m_pModelConstantBuffer;
    UINT m_vertexCount;
};