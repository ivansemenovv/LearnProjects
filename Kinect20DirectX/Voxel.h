#pragma once
#include <DirectXMath.h>

using namespace DirectX;

// Used to send per-vertex data to the vertex shader.
struct VertexPositionColor
{
    XMFLOAT3 pos;
    XMFLOAT3 color;
    float width;
};

class Voxel
{
public:
    Voxel() {}
    Voxel(
        float x,
        float y,
        float z,
        float width,
        float r,
        float g,
        float b
        )
    {
        m_Position = XMFLOAT3(x, y, z);
        m_Color = XMFLOAT3(r, g, b);
        m_Width = width;
        (
            XMVectorSet(1, 1, 1, 1),
            XMVectorSet(0, 0, 0, 1),
            XMVectorSet(x, y, z, 1)
            );

    }

    VertexPositionColor AsVertexStructure()
    {
        VertexPositionColor v;
        v.color = m_Color;
        v.pos = m_Position;
        v.width = m_Width;
        return v;
    }
    XMFLOAT3 m_Position;
    XMFLOAT3 m_Color;
    float m_Width;
}; 


struct VoxelHeader
{
    size_t count;
    Voxel* voxels;
};