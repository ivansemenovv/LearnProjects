#pragma once
#include "CameraSnapshot.h"
#include <d3d11_1.h>
#include <directxmath.h>

struct ConstantBuffer
{
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
};


class Axis
{
public:
    Axis();
    ~Axis();
    void Initialize(ID3D11Device*  pd3dDevice);
    void Draw(ID3D11DeviceContext* pImmediateContext, XMMATRIX& view, XMMATRIX& projection);

private:
    ID3D11VertexShader*     m_pVertexShader;
    ID3D11Buffer*           m_pModelConstantBuffer;
    ID3D11Buffer*           m_pVertexBuffer;
    ID3D11Buffer*           m_pIndexBuffer;
    ID3D11InputLayout*      m_pVertexLayout;
    ConstantBuffer          m_ConstantBuffer;
};
