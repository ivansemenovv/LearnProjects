#include <d3d11_1.h>
#include <directxmath.h>
#include "MoveableObject.h"

using namespace DirectX;

struct ViewConstantBuffer
{
    DirectX::XMFLOAT4X4 view;
};

class ViewCamera : public MoveableObject
{
public:
    ViewCamera();
    ~ViewCamera();
    void Initialize(XMMATRIX& viewMatrix, ID3D11Device* pd3dDevice);

    void Update(ID3D11DeviceContext* pImmediateContext);

private: 
    XMMATRIX m_viewMatrix;
    ID3D11Buffer* m_pViewConstantBuffer;
};