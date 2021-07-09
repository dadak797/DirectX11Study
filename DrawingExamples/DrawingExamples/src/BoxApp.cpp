#include "BoxApp.h"
#include "MathHelper.h"


BoxApp::BoxApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
    , m_Model(new Model()), m_ColorShader(new ColorShader())
    , m_Theta(1.5f * MathHelper::Pi), m_Phi(0.25f * MathHelper::Pi), m_Radius(5.0f)
{
    m_MainWndCaption = L"Box Demo";

    m_LastMousePos.x = 0;
    m_LastMousePos.y = 0;

    XMMATRIX identity = XMMatrixIdentity();
    m_MatrixBuffer.world = identity;
    m_MatrixBuffer.view = identity;
    m_MatrixBuffer.projection = identity;
}

BoxApp::~BoxApp()
{
    delete m_ColorShader;
}

bool BoxApp::Initialize()
{
    if (!D3DApp::Initialize())
    {
        return false;
    }
    
    m_Model->InitializeBuffers(m_D3DDevice);
    m_ColorShader->InitializeShaders(m_D3DDevice, m_hMainWnd, L"src/ColorVertexShader.vs", L"src/ColorPixelShader.ps");

    return true;
}

void BoxApp::OnResize()
{
    D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    m_MatrixBuffer.projection = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, GetAspectRatio(), m_ScreenNear, m_ScreenDepth);
}

void BoxApp::UpdateScene(float dt)
{
    // Convert Spherical to Cartesian coordinates.
    float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
    float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
    float y = m_Radius * cosf(m_Phi);

    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    m_MatrixBuffer.view = XMMatrixLookAtLH(pos, target, up);
}

void BoxApp::DrawScene()
{
    assert(m_D3DDeviceContext);
    assert(m_SwapChain);

    // Begin Scene
    m_D3DDeviceContext->ClearRenderTargetView(m_RenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    m_D3DDeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_MatrixBuffer.world = m_WorldMatrix;
    //m_MatrixBuffer.projection is updated in OnResize()
    // m_MatrixBuffer.view is updated in UpdateScene()

    m_Model->RenderBuffers(m_D3DDeviceContext);

    m_ColorShader->SetShaderParameters(m_D3DDeviceContext, m_MatrixBuffer.world, m_MatrixBuffer.view, m_MatrixBuffer.projection);
    m_ColorShader->RenderShader(m_D3DDeviceContext, m_Model->GetIndexCount());

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

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    m_LastMousePos.x = x;
    m_LastMousePos.y = y;

    SetCapture(m_hMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

        // Update angles based on input to orbit camera around box.
        m_Theta -= dx;
        m_Phi -= dy;

        // Restrict the angle mPhi.
        m_Phi = MathHelper::Clamp(m_Phi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        float dx = 0.005f * static_cast<float>(x - m_LastMousePos.x);
        float dy = 0.005f * static_cast<float>(y - m_LastMousePos.y);

        // Update the camera radius based on input.
        m_Radius -= dx - dy;

        // Restrict the radius.
        m_Radius = MathHelper::Clamp(m_Radius, 3.0f, 15.0f);
    }

    m_LastMousePos.x = x;
    m_LastMousePos.y = y;
}