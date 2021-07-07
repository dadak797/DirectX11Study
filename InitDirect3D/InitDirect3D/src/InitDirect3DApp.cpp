#include "InitDirect3DApp.h"


InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
{

}

InitDirect3DApp::~InitDirect3DApp()
{
}

bool InitDirect3DApp::Initialize()
{
    if (!D3DApp::Initialize())
    {
        return false;
    }

    return true;
}

void InitDirect3DApp::OnResize()
{
    D3DApp::OnResize();

    //DrawScene();  // For continuous view updating
}

void InitDirect3DApp::UpdateScene(float dt)
{

}

void InitDirect3DApp::DrawScene()
{
    assert(m_D3DDeviceContext);
    assert(m_SwapChain);

    // Begin Scene
    m_D3DDeviceContext->ClearRenderTargetView(m_RenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    m_D3DDeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // End Scene
    if (m_VSyncEnabled)
    {
        HR(m_SwapChain->Present(1, 0));
    }
    else
    {
        HR(m_SwapChain->Present(0, 0));
    }
}