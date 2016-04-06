#pragma once
#include "CameraSnapshot.h"
#include <fstream>
//#include <string>
//#include <iostream>


CameraSnapshot::CameraSnapshot() :
    m_pVertexBuffer(nullptr),
    m_pModelConstantBuffer(nullptr),
    m_vertexCount(0)
{
}

CameraSnapshot::~CameraSnapshot()
{
    if (m_pVertexBuffer != nullptr)
    {
        m_pVertexBuffer->Release();
    }

    if (m_pModelConstantBuffer != nullptr)
    {
        m_pModelConstantBuffer->Release();
    }
}

bool CameraSnapshot::Initialize(ID3D11Device*  pd3dDevice, NetworkKinectManager & kinectManager, XMMATRIX& originTransformationMatrix)
{
    if (pd3dDevice == nullptr)
    {
        throw E_INVALIDARG;
    } 

    m_originTransformationMatrix = originTransformationMatrix;

    m_serverAddress = kinectManager.GetKinectAddress();

    Voxel* voxels = kinectManager.AcquireVoxelBuffer(&m_vertexCount);
    if (m_vertexCount == 0)
    {
        if (voxels != nullptr)
        {
            free(voxels);
        }
        return false;
    }
    
    // copy voxels to temp array
    std::unique_ptr<VertexPositionColor[]> spVertexArray(new VertexPositionColor[m_vertexCount]);
    VertexPositionColor* pVertexArray = spVertexArray.get();
    for (int i = 0; i < m_vertexCount; i++)
    {
        pVertexArray[i] = voxels[i].AsVertexStructure();
    }

    free(voxels);

    // Bind vertex data
    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = pVertexArray;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
    vertexBufferDesc.ByteWidth = m_vertexCount * sizeof(VertexPositionColor);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.StructureByteStride = sizeof(VertexPositionColor);

    // create buffer
    THROW_IF_FAILED(pd3dDevice->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_pVertexBuffer));


    // Create the constant buffer
    D3D11_BUFFER_DESC bd = { 0 };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ModelConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    THROW_IF_FAILED(pd3dDevice->CreateBuffer(&bd, nullptr, &m_pModelConstantBuffer));

    return true;
}

void CameraSnapshot::Draw(ID3D11DeviceContext * pImmediateContext)
{
    if (pImmediateContext == nullptr)
    {
        throw E_INVALIDARG;
    }

    auto worldMatrix = this->GetTransformation(m_originTransformationMatrix);

    // set new world/model matrix
    ModelConstantBuffer cb1;
    XMStoreFloat4x4(&cb1.model, XMMatrixTranspose(worldMatrix));
    pImmediateContext->UpdateSubresource(m_pModelConstantBuffer, 0, nullptr, &cb1, 0, 0);
    pImmediateContext->GSSetConstantBuffers(2, 1, &m_pModelConstantBuffer);

    // set vertecies
    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    pImmediateContext->Draw(m_vertexCount, 0);
}


void CameraSnapshot::DumpTransformationMatrix(std::string& filePath)
{
    std::ofstream out(filePath, std::ofstream::ate);

    out << "IP Address of kinect: "<< m_serverAddress << std::endl;

    XMFLOAT4X4 fMatrix;
    XMStoreFloat4x4(&fMatrix, this->GetTransformation(m_originTransformationMatrix));
    
    out << "Transformation Matrix " << std::endl;
    out << fMatrix._11 << "\t" << fMatrix._12 << "\t" << fMatrix._13 << "\t" << fMatrix._14 << std::endl;
    out << fMatrix._21 << "\t" << fMatrix._22 << "\t" << fMatrix._23 << "\t" << fMatrix._24 << std::endl;
    out << fMatrix._31 << "\t" << fMatrix._32 << "\t" << fMatrix._33 << "\t" << fMatrix._34 << std::endl;
    out << fMatrix._41 << "\t" << fMatrix._42 << "\t" << fMatrix._43 << "\t" << fMatrix._44 << std::endl;
    out << "-------------------------------------------- " << std::endl;
    out.close();
}