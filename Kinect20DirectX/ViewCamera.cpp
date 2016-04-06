#include "KinectManager.h"
#include "ViewCamera.h"

ViewCamera::ViewCamera() :
    m_pViewConstantBuffer(nullptr)
{
}

ViewCamera::~ViewCamera()
{
    if (m_pViewConstantBuffer != nullptr)
    {
        m_pViewConstantBuffer->Release();
    }
}

void ViewCamera::Initialize(XMMATRIX& viewMatrix, ID3D11Device * pd3dDevice)
{
    if (pd3dDevice == nullptr)
    {
        throw E_INVALIDARG;
    }
    
    m_viewMatrix = viewMatrix;

    // Create the constant buffer for view matrix
    D3D11_BUFFER_DESC viewBufferDesc = { 0 };
    viewBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    viewBufferDesc.ByteWidth = sizeof(ViewConstantBuffer);
    viewBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    viewBufferDesc.CPUAccessFlags = 0;
    THROW_IF_FAILED(pd3dDevice->CreateBuffer(&viewBufferDesc, nullptr, &m_pViewConstantBuffer));
}

void ViewCamera::Update(ID3D11DeviceContext * pImmediateContext)
{
    if (pImmediateContext == nullptr)
    {
        throw E_INVALIDARG;
    }

    auto viewWorldMatrix = this->GetTransformation(m_viewMatrix);

    // setup projection matrix
    ViewConstantBuffer cb1;
    XMStoreFloat4x4(&cb1.view, XMMatrixTranspose(viewWorldMatrix));
    pImmediateContext->UpdateSubresource(m_pViewConstantBuffer, 0, nullptr, &cb1, 0, 0);
    pImmediateContext->GSSetConstantBuffers(1, 1, &m_pViewConstantBuffer);
}
