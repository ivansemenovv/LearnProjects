
#include "KinectManager.h"

KinectManager::KinectManager()
    : m_ActiveVoxelBuffer(nullptr)
{
    m_BufferSwapLock = CreateMutex(nullptr, false, nullptr);
}

float CalculateVoxelSize(float nearPlane, float fov, int hRes, int vRes, float voxelDistance)
{
    // Calculate how long the near plane would be in world space
    float lengthOfNearPlane = 2 * (nearPlane / tan(fov / 2));

    float aspectRatio = (float)hRes / vRes;

    // Calculate how bix the voxel would be to make up one pixel's space
    float voxelWidthAtNearPlane = lengthOfNearPlane / hRes;

    // What's the factor of the voxelDistance compared to the near plane
    float scaleFactor = (voxelDistance / nearPlane);

    return voxelWidthAtNearPlane * scaleFactor;
 
}
void ThreadProc(KinectManager* manager)
{
    manager->ThreadProc();
}

Voxel tempReadVoxelBuffer[width*height];
void KinectManager::ThreadProc()
{
    while (true)
    {   
        IMultiSourceFrame* pFrame;

        HRESULT hr = m_MultiReader->AcquireLatestFrame(&pFrame);
        if (hr == E_PENDING)
        {
            Sleep(30);
            continue;
        }
        THROW_IF_FAILED(hr);
        
        IDepthFrame* depthframe;
        IDepthFrameReference* frameref = NULL;
        hr = pFrame->get_DepthFrameReference(&frameref);
        THROW_IF_FAILED(hr);
        hr = frameref->AcquireFrame(&depthframe);
        if (frameref) frameref->Release();
        if (FAILED(hr))
        {
            // TODO Log/understand this
            continue;
        }
        if (!depthframe) throw hr;
        
        // Get data from frame
        unsigned int sz;
        unsigned short* buf;
        depthframe->AccessUnderlyingBuffer(&sz, &buf);

        // Write vertex coordinates
        m_Mapper->MapDepthFrameToCameraSpace(width*height, buf, width*height, depth2xyz);

        // Fill in depth2rgb map
        m_Mapper->MapDepthFrameToColorSpace(width*height, buf, width*height, depth2rgb);
        if (depthframe) depthframe->Release();


        IColorFrame* colorframe;
        IColorFrameReference* pColorFrameRef = NULL;
        hr = pFrame->get_ColorFrameReference(&pColorFrameRef);
        THROW_IF_FAILED(hr);
        hr = pColorFrameRef->AcquireFrame(&colorframe);
        if (FAILED(hr))
        {
            // TODO Log/understand this
            continue;
        }
        if (pColorFrameRef) pColorFrameRef->Release();

        if (!colorframe) throw hr;

        // Get data from frame
        colorframe->CopyConvertedFrameDataToArray(colorwidth*colorheight * 4, rgbimage, ColorImageFormat_Rgba);
        if (colorframe) colorframe->Release();

        for (int i = 0; i < width*height; i++) {
            ColorSpacePoint colorPoint = depth2rgb[i];
            CameraSpacePoint cameraPoint = depth2xyz[i];

            if (colorPoint.X < 0 || colorPoint.Y < 0 || colorPoint.X > colorwidth || colorPoint.Y > colorheight) {
                //OutputDebugStringW(L"Color point is out of range");
                continue;
            }
            else if (cameraPoint.X < 0 || cameraPoint.Y < 0 || cameraPoint.X > width || cameraPoint.Y > height) {
                //OutputDebugStringW(L"Camera point is out of range");
                continue;
            }
            //if (cameraPoint.Z >= 2) continue;
            int colorIndex = (int)colorPoint.X + colorwidth*(int)colorPoint.Y;
            tempReadVoxelBuffer[i] =
                Voxel(
                    cameraPoint.X, cameraPoint.Y, -cameraPoint.Z,
                    CalculateVoxelSize(m_nearPlane, 58, height, width, cameraPoint.Z),
                    rgbimage[4 * colorIndex + 0] / (float)0xFF,
                    rgbimage[4 * colorIndex + 1] / (float)0xFF,
                    rgbimage[4 * colorIndex + 2] / (float)0xFF
                    );
        }

        if (pFrame) pFrame->Release();
        // Update pointed buffer
        size_t activeVoxelCount;
        Voxel* activeBuffer = AcquireVoxelBuffer(&activeVoxelCount);
        memcpy_s(m_VoxelBuffer1, ARRAYSIZE(m_VoxelBuffer1) * sizeof(Voxel), tempReadVoxelBuffer, ARRAYSIZE(tempReadVoxelBuffer) * sizeof(Voxel));
        m_ActiveVoxelBuffer = m_VoxelBuffer1;
        ReleaseVoxelBuffer();
        memset(tempReadVoxelBuffer, 0, ARRAYSIZE(tempReadVoxelBuffer) * sizeof(Voxel));

    } // thread loop
}
HRESULT KinectManager::Initialize(float nearPlane)
{
    m_nearPlane = nearPlane;
    RETURN_IF_FAILED(GetDefaultKinectSensor(&m_Sensor));

    //RETURN_IF_FAILED(m_Sensor->get_CoordinateMapper(&m_Mapper));

    RETURN_IF_FAILED(m_Sensor->Open());

    /*RETURN_IF_FAILED(m_Sensor->get_ColorFrameSource(&m_ColorSource));

    RETURN_IF_FAILED(m_Sensor->get_DepthFrameSource(&m_DepthSource));

    RETURN_IF_FAILED(m_Sensor->get_BodyFrameSource(&m_BodySource));

    RETURN_IF_FAILED(m_ColorSource->OpenReader(&m_ColorReader));

    RETURN_IF_FAILED(m_DepthSource->OpenReader(&m_DepthReader));

    RETURN_IF_FAILED(m_BodySource->OpenReader(&m_BodyReader));*/

    RETURN_IF_FAILED(m_Sensor->get_CoordinateMapper(&m_Mapper));

    RETURN_IF_FAILED(m_Sensor->OpenMultiSourceFrameReader(
        FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color,
        &m_MultiReader));

    BOOLEAN available;
    RETURN_IF_FAILED(m_Sensor->get_IsAvailable(&available));

    m_ReadThread.reset(new std::thread([this]() {
    
        ::ThreadProc(this);

    }));

    return S_OK;
}

Voxel* KinectManager::AcquireVoxelBuffer(size_t* voxelCount)
{

    DWORD dwResult = WaitForSingleObject(m_BufferSwapLock, INFINITE);
    if (dwResult != WAIT_OBJECT_0)
    {
        return nullptr;
    }

    if (voxelCount)
    {
        if (m_ActiveVoxelBuffer == nullptr)
        {
            *voxelCount = 0;
        }
        else
        {
            *voxelCount = width*height;
        }
    }
    Voxel* nonVolatileBuffer = (Voxel*)m_ActiveVoxelBuffer;
    return nonVolatileBuffer;
}
void KinectManager::ReleaseVoxelBuffer()
{
    (void)ReleaseMutex(m_BufferSwapLock);
}