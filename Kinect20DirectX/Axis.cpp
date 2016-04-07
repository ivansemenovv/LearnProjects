#include "Axis.h"

HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

Axis::Axis() :  m_pVertexShader(nullptr), 
                m_pModelConstantBuffer(nullptr),
                m_pVertexBuffer(nullptr),
                m_pIndexBuffer(nullptr),
                m_pVertexLayout(nullptr)
{

}

Axis::~Axis()
{
    if (m_pVertexShader != nullptr)
    {
        m_pVertexShader->Release();
    }

    if (m_pModelConstantBuffer != nullptr)
    {
        m_pModelConstantBuffer->Release();
    }

    if (m_pVertexBuffer != nullptr)
    {
        m_pVertexBuffer->Release();
    }

    if (m_pIndexBuffer != nullptr)
    {
        m_pIndexBuffer->Release();
    }
    if (m_pVertexLayout != nullptr)
    {
        m_pVertexLayout->Release();
    }
}

void Axis::Initialize(ID3D11Device * pd3dDevice)
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr, *pGSBlob = nullptr, *pPSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(L"SampleVertexShader.hlsl", "AxisVertexShader", "vs_5_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        throw hr;
    }

    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        throw hr;
    }

    // Create vertex buffer
    VertexPositionColor vertices[] =
    {
        { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), 1.0 },
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 1.0f), 1.0 },
        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 1.0f), 1.0 },
        { XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0 },
    };
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VertexPositionColor) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
    if (FAILED(hr))
        throw hr;

    // Create index buffer
    WORD indices[] =
    {
        0,1,
        0,2,
        0,3
    };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
    if (FAILED(hr))
        throw hr;

    // Create the constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    THROW_IF_FAILED(pd3dDevice->CreateBuffer(&bd, nullptr, &m_pModelConstantBuffer));

    
}

void Axis::Draw(ID3D11DeviceContext * pImmediateContext, XMMATRIX& view, XMMATRIX& projection)
{
    if (pImmediateContext == nullptr)
    {
        throw E_INVALIDARG;
    }
    // disable gepmetry shader
    pImmediateContext->GSSetShader(nullptr, nullptr, 0);

    // Set primitive topology
    pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // set vertecies
    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    // Set index buffer
    pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    m_ConstantBuffer.mWorld = XMMatrixTranspose(XMMatrixIdentity());
    m_ConstantBuffer.mView = XMMatrixTranspose(view);
    m_ConstantBuffer.mProjection = XMMatrixTranspose(projection);
    pImmediateContext->UpdateSubresource(m_pModelConstantBuffer, 0, nullptr, &m_ConstantBuffer, 0, 0);
    
    // set vertex shader
    pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
    pImmediateContext->VSSetConstantBuffers(0, 1, &m_pModelConstantBuffer);


    pImmediateContext->DrawIndexed(6, 0, 0);
}
