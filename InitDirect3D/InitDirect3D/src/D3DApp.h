#pragma once

#include "D3DUtil.h"
#include "GameTimer.h"
#include <string>


class D3DApp
{
public:
    D3DApp(HINSTANCE hInstance);
    virtual ~D3DApp();

    HINSTANCE GetAppInstance() const { return m_hAppInstance; }
    HWND      GetMainWnd() const { return m_hMainWnd; }
    float     GetAspectRatio() const { return static_cast<float>(m_ClientWidth) / m_ClientHeight; }

    int Run();

    // Framework methods.  Derived client class overrides these methods to 
    // implement specific application requirements.
    virtual bool Initialize();
    virtual void OnResize();
    virtual void UpdateScene(float dt) = 0;
    virtual void DrawScene() = 0;
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Convenience overrides for handling mouse input.
    virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
    virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
    virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

protected:
    bool InitMainWindow();
    bool InitDirect3D();

    void CalculateFrameStats();

protected:
    HINSTANCE m_hAppInstance;
    HWND      m_hMainWnd;
    bool      m_bAppPaused;
    bool      m_bMinimized;
    bool      m_bMaximized;
    bool      m_bResizing;
    UINT      m_4xMsaaQuality;

    GameTimer m_Timer;

    ID3D11Device* m_D3DDevice;
    ID3D11DeviceContext* m_D3DDeviceContext;
    IDXGISwapChain* m_SwapChain;
    ID3D11Texture2D* m_DepthStencilBuffer;
    ID3D11DepthStencilState* m_DepthStencilState;
    ID3D11RenderTargetView* m_RenderTargetView;
    ID3D11DepthStencilView* m_DepthStencilView;
    D3D11_VIEWPORT m_ScreenViewport;
    ID3D11RasterizerState* m_RasterState;

    // Derived class should set these in derived constructor to customize starting values.
    std::wstring m_MainWndCaption;
    D3D_DRIVER_TYPE m_D3DDriverType;
    int m_ClientWidth, m_ClientHeight;
    int m_InitWidth, m_InitHeight;
    bool m_bEnable4xMsaa;

    bool m_bFullScreen;
    bool m_VSyncEnabled;
    float m_ScreenDepth, m_ScreenNear;
    int m_VideoCardMemory;
    char m_VideoCardDescription[128];

    XMMATRIX m_ProjectionMatrix, m_WorldMatrix, m_OrthoMatrix;
};

