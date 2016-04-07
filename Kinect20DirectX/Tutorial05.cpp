//--------------------------------------------------------------------------------------
// File: Tutorial05.cpp
//
// This application demonstrates animation using matrix transformations
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729722.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once

#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
//#include "NetworkKinectManager.h"
//#include "KinectManager.h"
//#include "CameraSnapshot.h"
#include "ViewCamera.h"
#include "Axis.h"
#include <vector>
#include <memory>
#include <fstream>


using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11Device1*          g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
IDXGISwapChain1*        g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11Texture2D*        g_pDepthStencil = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
ID3D11VertexShader*     g_pVertexShader = nullptr;
ID3D11GeometryShader*   g_pGeometryShader = nullptr;
ID3D11PixelShader*      g_pPixelShader = nullptr;
ID3D11InputLayout*      g_pVertexLayout = nullptr;
ID3D11Buffer*           g_pVertexBuffer = nullptr;
ID3D11Buffer*           g_pIndexBuffer = nullptr;
ID3D11Buffer*           g_pProjectionConstantBuffer = nullptr;
ID3D11Buffer*           g_pViewConstantBuffer = nullptr;
XMMATRIX                g_View;
XMMATRIX                g_Projection;
Axis                    g_Axis;

XMMATRIX firstKinect = {
    0.999229,    0,    -0.0392598,    0,
    -0.00154133,    0.999229,    -0.0392295,    0,
    0.0392295,    0.0392598,    0.998459,    0,
    -0.00615183,    0.028416,    0.161817,    1,
};

XMMATRIX secondKinect = {
    -1.19209e-07,    0,    -1,    0,
    0,    1,    0,    0,
    1,    0,    -1.19209e-07,    0,
    1.85,    -0.025,    -1.0875,    1,
};

//KinectManager           g_Kinect;
#define KINECT_NUMBER   0x02
NetworkKinectManager    g_NetworKinects[KINECT_NUMBER];
const char*             g_KinectAddresses[] = { "127.0.0.1", "10.124.134.109", "127.0.0.1" };
XMMATRIX                g_originTransformationMatrices[] = {
    XMMatrixIdentity() * firstKinect,
    XMMatrixIdentity() * secondKinect /* XMMatrixRotationY(XM_PI / 2)*/,
    XMMatrixIdentity() * XMMatrixRotationY(2 * 2 * XM_PI / 3),
};


float rotationStep = 0.0125f;
float translationStep = 0.0125f;

bool captureFrameCamera1 = false;

std::shared_ptr<ViewCamera> viewCamera(new ViewCamera());
std::vector<std::shared_ptr<CameraSnapshot>> capturedSnapshots;
std::vector<std::shared_ptr<MoveableObject>> moveableObjects;
short activeMoveableObject = 0;





//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1920, 1080 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Direct3D 11 Tutorial 5",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if( FAILED(hr) )
    {
        if( pErrorBlob )
        {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    for (int i = 0; i < KINECT_NUMBER; i++)
    {
        g_NetworKinects[i].InititializeClient(g_KinectAddresses[i]);
    }

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

	D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );

        if ( hr == E_INVALIDARG )
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                                    D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        }

        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface( __uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice) );
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory) );
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface( __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2) );
    if ( dxgiFactory2 )
    {
        // DirectX 11.1 or later
        hr = g_pd3dDevice->QueryInterface( __uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1) );
        if (SUCCEEDED(hr))
        {
            (void) g_pImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1) );
        }

        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd( g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1 );
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface( __uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain) );
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation( g_hWnd, DXGI_MWA_NO_ALT_ENTER );

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &pBackBuffer ) );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, nullptr, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr, *pGSBlob = nullptr, *pPSBlob = nullptr;
    hr = CompileShaderFromFile( L"SampleVertexShader.hlsl", "main", "vs_5_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( nullptr,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    hr = CompileShaderFromFile(L"SampleGeometryShader.hlsl", "main", "gs_5_0", &pGSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    hr = CompileShaderFromFile(L"SamplePixelShader.hlsl", "main", "ps_5_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "PSIZE", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );


	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader );
	pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // create the geometry shader
    hr = g_pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &g_pGeometryShader);
    pGSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Create vertex buffer

    
    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet( 0.0f, 0.0f, 1.3f, 1.0f );
    XMVECTOR At = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 1.0f );
    
    auto view = XMMatrixLookToRH(Eye, XMVectorSet(0, 0, -1, 1), Up);
    
    viewCamera->Initialize(view, g_pd3dDevice);
    
    // camera is always 0 moveable object
    moveableObjects.push_back(viewCamera);

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovRH( XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f );

    // Initialize axises
    g_Axis.Initialize(g_pd3dDevice);

    // Create the constant buffer for projection matrix
    D3D11_BUFFER_DESC projectionBufferDesc = { 0 };
    projectionBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    projectionBufferDesc.ByteWidth = sizeof(ProjectionConstantBuffer);
    projectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    projectionBufferDesc.CPUAccessFlags = 0;
    RETURN_IF_FAILED(g_pd3dDevice->CreateBuffer(&projectionBufferDesc, nullptr, &g_pProjectionConstantBuffer));

    // setup projection matrix for geometry shader
    ProjectionConstantBuffer cb1;
    XMStoreFloat4x4(&cb1.projection, XMMatrixTranspose(g_Projection));
    g_pImmediateContext->UpdateSubresource(g_pProjectionConstantBuffer, 0, nullptr, &cb1, 0, 0);
    g_pImmediateContext->GSSetConstantBuffers(0, 1, &g_pProjectionConstantBuffer);

    // setup shaders
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pProjectionConstantBuffer ) g_pProjectionConstantBuffer->Release();
    if (g_pViewConstantBuffer) g_pViewConstantBuffer->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain1 ) g_pSwapChain1->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext1 ) g_pImmediateContext1->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice1 ) g_pd3dDevice1->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}

MoveableObject* GetActiveObject()
{
    int i = 0;
    for (auto object = moveableObjects.begin(); object != moveableObjects.end(); object++, i++)
    {
        if (i == activeMoveableObject)
        {
           return object->get();
        }
    }
    return viewCamera.get();
}

void DumpTransformationMatrices()
{
    std::string dirPath = "G:\\";
    
    // cleanup file before writing
    std::ofstream out(dirPath, std::ofstream::trunc);
    out.close();

    for (auto snapshot = capturedSnapshots.begin(); snapshot != capturedSnapshots.end(); snapshot++)
    {
        snapshot->get()->DumpTransformationMatrix(dirPath);
    }
}

HRESULT HandleKeyDown(WPARAM wParam)
{
    switch (wParam)
    {
    case '0':
        activeMoveableObject = 0;
        break;
    case '1':
        activeMoveableObject = 1;
        break;
    case '2':
        activeMoveableObject = 2;
        break;
    case '3':
        activeMoveableObject = 3;
        break;

    case 'A':
        GetActiveObject()->RotateAxisZ((float)XM_PI * rotationStep);
        break;
    case 'D':
        GetActiveObject()->RotateAxisZ((float)XM_PI * rotationStep * -1.0f);
        break;
    case 'W':
        GetActiveObject()->RotateAxisX((float)XM_PI * rotationStep);
        break;
    case 'S':
        GetActiveObject()->RotateAxisX((float)XM_PI * rotationStep * -1.0f);
        break;
    case 'Q':
        GetActiveObject()->RotateAxisY((float)XM_PI * rotationStep);
        break;
    case 'E':
        GetActiveObject()->RotateAxisY((float)XM_PI * rotationStep * -1.0f);
        break;

    case VK_LEFT:
        GetActiveObject()->MoveAxisX(translationStep * -1.0f);
        
        break;

    case VK_RIGHT:
        GetActiveObject()->MoveAxisX(translationStep);
        break;

    case VK_UP:
        GetActiveObject()->MoveAxisY(translationStep);
        break;

    case VK_DOWN:
        GetActiveObject()->MoveAxisY(translationStep * -1.0f);

        break;

    case VK_HOME:
        GetActiveObject()->MoveAxisZ(translationStep);
        break;

    case VK_END:
        GetActiveObject()->MoveAxisZ(translationStep * -1.0f);
        break;

    case VK_INSERT:

        // Process the INS key. 

        break;

    case VK_DELETE:

        // Process the DEL key. 

        break;

    case VK_F1:

        captureFrameCamera1 = true;

        break;
    
    case VK_ESCAPE:
        
        DumpTransformationMatrices();
       
        break;
        // Process other non-character keystrokes. 

    default:
        break;
    }

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

    case WM_KEYDOWN:
        HandleKeyDown(wParam);
        break;
    case WM_CHAR:
        HandleKeyDown(wParam);
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
    //
    // Clear the back buffer
    //
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, Colors::MidnightBlue );

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    // Update Camera view
    viewCamera->Update(g_pImmediateContext);

    g_Axis.Draw(g_pImmediateContext, viewCamera->GetViewMatrix(), g_Projection);

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->GSSetShader(g_pGeometryShader, nullptr, 0);

    // draw 
    for (auto snapshot = capturedSnapshots.begin(); snapshot != capturedSnapshots.end(); snapshot++)
    {
        snapshot->get()->Draw(g_pImmediateContext);
    }
    
    // will render dynamic snapshots only if we didn't capture any of them
    if (capturedSnapshots.size() == 0)
    {
        for (int i = 0; i < KINECT_NUMBER; i++)
        {
            std::shared_ptr<CameraSnapshot> dynamiccamerasnapshot(new CameraSnapshot());
            if (dynamiccamerasnapshot->Initialize(g_pd3dDevice, g_NetworKinects[i], g_originTransformationMatrices[i]))
            {
                dynamiccamerasnapshot->Draw(g_pImmediateContext);
                if (captureFrameCamera1)
                {
                    capturedSnapshots.push_back(dynamiccamerasnapshot);
                    moveableObjects.push_back(dynamiccamerasnapshot);
                }
            }
        }

        if (captureFrameCamera1) captureFrameCamera1 = false;
    }

    g_pSwapChain->Present( 0, 0 );
}


