#pragma comment(lib, "dxgi.lib")

#include "D3DApp.h"
#include <dxgi.h>
#include <windowsx.h>


namespace
{
    // This is just used to forward Windows messages from a global window
    // procedure to our member function window procedure because we cannot
    // assign a member function to WNDCLASS::lpfnWndProc.
    D3DApp* g_D3DApp = 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return g_D3DApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
    : m_hAppInstance(hInstance)
    , m_MainWndCaption(L"DirectX11 Application")
    , m_D3DDriverType(D3D_DRIVER_TYPE_HARDWARE)
    , m_InitWidth(800), m_InitHeight(600), m_ClientWidth(m_InitWidth), m_ClientHeight(m_InitHeight), m_bFullScreen(false)
    , m_bEnable4xMsaa(false), m_4xMsaaQuality(0)
    , m_hMainWnd(0)
    , m_bAppPaused(false)
    , m_bMinimized(false), m_bMaximized(false)
    , m_bResizing(false)
    , m_D3DDevice(nullptr)
    , m_D3DDeviceContext(nullptr)
    , m_SwapChain(nullptr)
    , m_DepthStencilBuffer(nullptr)
    , m_RenderTargetView(nullptr)
    , m_DepthStencilView(nullptr)
    , m_RasterState(nullptr)
    , m_VSyncEnabled(true)
    , m_ScreenDepth(1000.0f), m_ScreenNear(1.0f)
{
    ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));

    // Get a pointer to the application object so we can forward 
    // Windows messages to the object's window procedure through
    // the global window procedure.
    g_D3DApp = this;
}

D3DApp::~D3DApp()
{
    if (m_SwapChain)
    {
        m_SwapChain->SetFullscreenState(false, NULL);
    }
    ReleaseCOM(m_RasterState);
    ReleaseCOM(m_DepthStencilView);
    ReleaseCOM(m_DepthStencilState);
    ReleaseCOM(m_DepthStencilBuffer);
    ReleaseCOM(m_RenderTargetView);
    
    // Restore all default settings.
    if (m_D3DDeviceContext)
    {
        m_D3DDeviceContext->ClearState();
    }
    ReleaseCOM(m_D3DDeviceContext);
    ReleaseCOM(m_D3DDevice);
    ReleaseCOM(m_SwapChain);
}

int D3DApp::Run()
{
    // Initialize the message structure.
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    m_Timer.Reset();

    while (msg.message != WM_QUIT)
    {
        // If there are Window messages then process them.
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Otherwise, do animation/game stuff.
        else
        {
            m_Timer.Tick();

            if (!m_bAppPaused)
            {
                CalculateFrameStats();
                UpdateScene(m_Timer.DeltaTime());
                DrawScene();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return static_cast<int>(msg.wParam);
}

bool D3DApp::Initialize()
{
    if (!InitMainWindow())
    {
        return false;
    }

    if (!InitDirect3D())
    {
        return false;
    }

    return true;
}

bool D3DApp::InitMainWindow()
{
    WNDCLASSEX wc;

    // Setup the windows class with default settings
    wc.cbSize = sizeof(WNDCLASSEX);  // Size in RAM
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Window style
    wc.lpfnWndProc = MainWndProc;  // WndProc
    wc.cbClsExtra = 0;  // These two are or allocating extra bytes
    wc.cbWndExtra = 0;
    wc.hInstance = m_hAppInstance;  // Handle to the instance
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);  // Icon in the top left (Other Icon: IDI_APPLICATION)
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"D3DAppClass";

    // Register the window class.
    if (!RegisterClassEx(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    DEVMODE dmScreenSettings;
    int originPosX, originPosY;

    if (m_bFullScreen)
    {
        // If full screen set the screen to maximum size of the users desktop and 32bit.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = static_cast<unsigned long>(GetSystemMetrics(SM_CXSCREEN));
        dmScreenSettings.dmPelsHeight = static_cast<unsigned long>(GetSystemMetrics(SM_CYSCREEN));
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Change the display settings to full screen.
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        // Set the position of the window to the top left corner.
        originPosX = 0;
        originPosY = 0;
    }
    else
    {
        // Place the window in the middle of the screen.
        originPosX = (GetSystemMetrics(SM_CXSCREEN) - m_InitWidth) / 2;
        originPosY = (GetSystemMetrics(SM_CYSCREEN) - m_InitHeight) / 2;
    }

    // Create the window with the screen settings and get the handle to it.
    m_hMainWnd = CreateWindowEx(WS_EX_APPWINDOW, L"D3DAppClass", m_MainWndCaption.c_str(),
        /*WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX*/WS_OVERLAPPEDWINDOW,
        originPosX, originPosY, m_InitWidth, m_InitHeight, NULL, NULL, m_hAppInstance, NULL);

    if (!m_hMainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    // Bring the window up on the screen and set it as main focus.
    ShowWindow(m_hMainWnd, SW_SHOW);
    //UpdateWindow(m_hMainWnd);
    SetForegroundWindow(m_hMainWnd);
    SetFocus(m_hMainWnd);

    // Hide the mouse cursor.
    //ShowCursor(false);

    return true;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        // WM_ACTIVATE is sent when the window is activated or deactivated.  
        // We pause the game when the window is deactivated and unpause it 
        // when it becomes active.  
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_bAppPaused = true;
            m_Timer.Stop();
        }
        else
        {
            m_bAppPaused = false;
            m_Timer.Start();
        }
        return 0;

        // WM_SIZE is sent when the user resizes the window.  
    case WM_SIZE:
        // Save the new client area dimensions.
        m_ClientWidth = LOWORD(lParam);
        m_ClientHeight = HIWORD(lParam);
        if (m_D3DDevice)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                m_bAppPaused = true;
                m_bMinimized = true;
                m_bMaximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_bAppPaused = false;
                m_bMinimized = false;
                m_bMaximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {
                // Restoring from minimized state?
                if (m_bMinimized)
                {
                    m_bAppPaused = false;
                    m_bMinimized = false;
                    OnResize();
                }
                // Restoring from maximized state?
                else if (m_bMaximized)
                {
                    m_bAppPaused = false;
                    m_bMaximized = false;
                    OnResize();
                }
                else if (m_bResizing)
                {
                    // If user is dragging the resize bars, we do not resize 
                    // the buffers here because as the user continuously 
                    // drags the resize bars, a stream of WM_SIZE messages are
                    // sent to the window, and it would be pointless (and slow)
                    // to resize for each WM_SIZE message received from dragging
                    // the resize bars.  So instead, we reset after the user is 
                    // done resizing the window and releases the resize bars, which 
                    // sends a WM_EXITSIZEMOVE message.

                    //OnResize();  // For continuous view updating
                }
                else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                {
                    OnResize();
                }
            }
        }
        return 0;

        // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
    case WM_ENTERSIZEMOVE:
        m_bAppPaused = true;
        m_bResizing = true;
        m_Timer.Stop();
        return 0;

        // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
        // Here we reset everything based on the new window dimensions.
    case WM_EXITSIZEMOVE:
        m_bAppPaused = false;
        m_bResizing = false;
        m_Timer.Start();
        OnResize();
        return 0;

        // WM_DESTROY is sent when the window is being destroyed.
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        // The WM_MENUCHAR message is sent when a menu is active and the user presses 
        // a key that does not correspond to any mnemonic or accelerator key. 
    case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

        // Catch this message so to prevent the window from becoming too small.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_MOUSEMOVE:
        OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::InitDirect3D()
{
    // Create a directX graphic interface factory
    IDXGIFactory* factory;
    HR(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));

    // Use the factory to create an adapter for the primary graphics interface (video card).
    IDXGIAdapter* adapter;
    HR(factory->EnumAdapters(0, &adapter));

    // Enumerate the primary adapter output (monitor).
    IDXGIOutput* adapterOutput;
    HR(adapter->EnumOutputs(0, &adapterOutput));

    // Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
    unsigned int numModes;
    HR(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL));

    // Create a list to hold all the possible display modes for this monitor/video card combination.
    DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];

    // Now fill the display mode list structures.
    HR(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList));

    // Now go through all the display modes and find the one that matches the screen width and height.
    // When a match is found store the numerator and denominator of the refresh rate for that monitor.
    unsigned int numerator, denominator;

    for (unsigned int i = 0; i < numModes; i++)
    {
        if (displayModeList[i].Width == static_cast<unsigned int>(m_InitWidth))
        {
            if (displayModeList[i].Height == static_cast<unsigned int>(m_InitHeight))
            {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
            }
        }
    }

    // Get the adapter (video card) description.
    DXGI_ADAPTER_DESC adapterDesc;
    HR(adapter->GetDesc(&adapterDesc));

    // Store the dedicated video card memory in megabytes.
    m_VideoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // Convert the name of the video card to a character array and store it.
    size_t stringLength;
    int error = wcstombs_s(&stringLength, m_VideoCardDescription, 128, adapterDesc.Description, 128);
    if (error != 0)
    {
        return false;
    }

    // Release the display mode list.
    delete[] displayModeList;
    displayModeList = nullptr;

    // Release the adapter output.
    ReleaseCOM(adapterOutput);

    // Release the adapter.
    ReleaseCOM(adapter);

    // Release the factory.
    ReleaseCOM(factory);

    // Initialize the swap chain description.
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // Set to a single back buffer.
    swapChainDesc.BufferCount = 1;

    // Set the width and height of the back buffer.
    swapChainDesc.BufferDesc.Width = m_InitWidth;
    swapChainDesc.BufferDesc.Height = m_InitHeight;

    // Set regular 32-bit surface for the back buffer.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Set the refresh rate of the back buffer.
    if (m_VSyncEnabled)
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // Set the usage of the back buffer.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Set the handle for the window to render to.
    swapChainDesc.OutputWindow = m_hMainWnd;

    // Use 4X MSAA? 
    if (m_bEnable4xMsaa)
    {
        swapChainDesc.SampleDesc.Count = 4;
        swapChainDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    }
    // No MSAA
    else
    {
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
    }

    // Set to full screen or windowed mode.
    if (m_bFullScreen)
    {
        swapChainDesc.Windowed = false;
    }
    else
    {
        swapChainDesc.Windowed = true;
    }

    // Set the scan line ordering and scaling to unspecified.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Discard the back buffer contents after presenting.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Don't set the advanced flags.
    swapChainDesc.Flags = 0;

    // Set the feature level to DirectX 11.
    // Create the swap chain, Direct3D device, and Direct3D device context.
    D3D_FEATURE_LEVEL selectedFeatureLevel;

    // One feature is selected considering the performance of the hardware.
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
                                          D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

    HR(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, ARRAYSIZE(featureLevels),
          D3D11_SDK_VERSION, &swapChainDesc, &m_SwapChain, &m_D3DDevice, &selectedFeatureLevel, &m_D3DDeviceContext));

    // The remaining steps that need to be carried out for d3d creation
    // also need to be executed every time the window is resized.  So
    // just call the OnResize method here to avoid code duplication.
    OnResize();

    // Setup the projection matrix.
    float fieldOfView = static_cast<float>(XM_PI) / 4.0f;
    float screenAspect = static_cast<float>(m_ClientWidth) / static_cast<float>(m_ClientHeight);

    // Create the projection matrix for 3D rendering.
    m_ProjectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, m_ScreenNear, m_ScreenDepth);

    // Initialize the world matrix to the identity matrix.
    m_WorldMatrix = XMMatrixIdentity();

    // Create an orthographic projection matrix for 2D rendering.
    m_OrthoMatrix = XMMatrixOrthographicLH(static_cast<float>(m_ClientWidth), static_cast<float>(m_ClientHeight), m_ScreenNear, m_ScreenDepth);

    return true;
}

void D3DApp::OnResize()
{
    assert(m_D3DDevice);
    assert(m_D3DDeviceContext);
    assert(m_SwapChain);
    
    if (!m_D3DDevice || !m_D3DDeviceContext || !m_SwapChain) return;

    // Release the old views, as they hold references to the buffers we
    // will be destroying.  Also release the old depth/stencil buffer.
    ReleaseCOM(m_RasterState);
    ReleaseCOM(m_DepthStencilView);
    ReleaseCOM(m_DepthStencilState);
    ReleaseCOM(m_DepthStencilBuffer);
    ReleaseCOM(m_RenderTargetView);

    // Resize the swap chain and recreate the render target view.
    HR(m_SwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    // Get the pointer to the back buffer.
    ID3D11Texture2D* backBufferPtr;
    HR(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr));

    // Create the render target view with the back buffer pointer.
    HR(m_D3DDevice->CreateRenderTargetView(backBufferPtr, NULL, &m_RenderTargetView));

    // Release pointer to the back buffer as we no longer need it.
    ReleaseCOM(backBufferPtr);

    // Initialize the description of the depth buffer.
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // Set up the description of the depth buffer.
    depthBufferDesc.Width = m_ClientWidth;
    depthBufferDesc.Height = m_ClientHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description.
    HR(m_D3DDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_DepthStencilBuffer));

    // Initialize the description of the stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create the depth stencil state.
    HR(m_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState));

    // Set the depth stencil state.
    m_D3DDeviceContext->OMSetDepthStencilState(m_DepthStencilState, 1);

    // Initialize the depth stencil view.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    HR(m_D3DDevice->CreateDepthStencilView(m_DepthStencilBuffer, &depthStencilViewDesc, &m_DepthStencilView));

    // Bind the render target view and depth stencil buffer to the output render pipeline.
    m_D3DDeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

    // Setup the raster description which will determine how and what polygons will be drawn.
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state from the description we just filled out.
    HR(m_D3DDevice->CreateRasterizerState(&rasterDesc, &m_RasterState));

    // Now set the rasterizer state.
    m_D3DDeviceContext->RSSetState(m_RasterState);

    // Setup the viewport for rendering.
    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<float>(m_ClientWidth);
    viewport.Height = static_cast<float>(m_ClientHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // Create the viewport.
    m_D3DDeviceContext->RSSetViewports(1, &viewport);
}

void D3DApp::CalculateFrameStats()
{
    // Code computes the average frames per second, and also the 
    // average time it takes to render one frame.  These stats 
    // are appended to the window caption bar.

    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << m_MainWndCaption << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(m_hMainWnd, outs.str().c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}