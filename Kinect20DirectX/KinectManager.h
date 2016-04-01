#pragma once
#include <windows.h>
#include <Kinect.h>
#include <DirectXMath.h>
#include <vector>
#include <thread>

#define RETURN_IF_FAILED(hr) { auto __hr = (hr); if (FAILED(__hr)) { return __hr; } }
#define THROW_IF_FAILED(hr) { auto __hr = (hr); if (FAILED(__hr)) { throw __hr; } }


const int width = 512;
const int height = 424;
const int colorwidth = 1920;
const int colorheight = 1080;

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


float CalculateVoxelSize(float nearPlane, float fov, int hRes, int vRes, float voxelDistance);

class KinectManager sealed
{
public:
    KinectManager();
    // Need nearplane to calculate voxel size
    HRESULT Initialize(float nearPlane);

    Voxel* AcquireVoxelBuffer(size_t* voxelCount);
    void ReleaseVoxelBuffer();
    
private:
    float m_nearPlane;
    IKinectSensor *m_Sensor;
    IColorFrameSource *m_ColorSource;
    IDepthFrameSource *m_DepthSource;
    IBodyFrameSource *m_BodySource;
    IColorFrameReader *m_ColorReader;
    IDepthFrameReader *m_DepthReader;
    IBodyFrameReader *m_BodyReader;
    ICoordinateMapper *m_Mapper;
    IMultiSourceFrameReader* m_MultiReader;

    unsigned char rgbimage[colorwidth*colorheight * 4];    // Stores RGB color image
    ColorSpacePoint depth2rgb[width*height];             // Maps depth pixels to rgb pixels
    CameraSpacePoint depth2xyz[width*height];			 // Maps depth pixels to 3d coordinates

    Voxel m_VoxelBuffer1[width*height];
    Voxel m_VoxelBuffer2[width*height];
    volatile Voxel* m_ActiveVoxelBuffer;
    HANDLE m_BufferSwapLock;

    std::unique_ptr<std::thread> m_ReadThread;
    friend void ThreadProc(KinectManager* manager);
    void ThreadProc();
};

