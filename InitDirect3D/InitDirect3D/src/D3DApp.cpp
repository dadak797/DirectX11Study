#include "D3DApp.h"


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
    , m_ClientWidth(800), m_ClientHeight(600), m_bFullScreen(false)
    , m_bEnable4xMsaa(false), m_4xMsaaQuality(0)
    , m_hMainWnd(0)
    , m_bAppPaused(false)
    , m_bMinimized(false), m_bMaximized(false)
    , m_bResizing(false)
    , m_D3DDevice(nullptr)
    , m_D3DImmediateContext(nullptr)
    , m_SwapChain(nullptr)
    , m_DepthStencilBuffer(nullptr)
    , m_RenderTargetView(nullptr)
    , m_DepthStencilView(nullptr)
{
    ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));

    // Get a pointer to the application object so we can forward 
    // Windows messages to the object's window procedure through
    // the global window procedure.
    g_D3DApp = this;
}

D3DApp::~D3DApp()
{
    ReleaseCOM(m_RenderTargetView);
    ReleaseCOM(m_DepthStencilView);
    ReleaseCOM(m_SwapChain);
    ReleaseCOM(m_DepthStencilBuffer);

    // Restore all default settings.
    if (m_D3DImmediateContext)
    {
        m_D3DImmediateContext->ClearState();
    }

    ReleaseCOM(m_D3DImmediateContext);
    ReleaseCOM(m_D3DDevice);
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
    DEVMODE dmScreenSettings;
    int originPosX, originPosY;

    // Setup the windows class with default settings
    wc.cbSize = sizeof(WNDCLASSEX);  // Size in RAM
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Window style
    wc.lpfnWndProc = MainWndProc;  // WndPro
    wc.cbClsExtra = 0;  // These two are or allocating extra bytes
    wc.cbWndExtra = 0;
    wc.hInstance = m_hAppInstance;  // Handle to the instance
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);  // Icon in the top left (IDI_APPLICATION)
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
        originPosX = (GetSystemMetrics(SM_CXSCREEN) - m_ClientWidth) / 2;
        originPosY = (GetSystemMetrics(SM_CYSCREEN) - m_ClientHeight) / 2;
    }

    // Create the window with the screen settings and get the handle to it.
    m_hMainWnd = CreateWindowEx(WS_EX_APPWINDOW, L"D3DAppClass", m_MainWndCaption.c_str(),
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        originPosX, originPosY, m_ClientWidth, m_ClientHeight, NULL, NULL, m_hAppInstance, NULL);

    if (!m_hMainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    // Bring the window up on the screen and set it as main focus.
    ShowWindow(m_hMainWnd, SW_SHOW);
    UpdateWindow(m_hMainWnd);
    //SetForegroundWindow(m_hMainWnd);
    //SetFocus(m_hMainWnd);

    // Hide the mouse cursor.
    //ShowCursor(false);

    return true;
}