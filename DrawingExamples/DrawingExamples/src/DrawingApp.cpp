#include "DrawingApp.h"
#include "MathHelper.h"


DrawingApp::DrawingApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
    , m_Model(new /*BoxModel()*//*HillsModel()*/ShapesModel()), m_ColorShader(new ColorShader())
    , m_Theta(1.5f * MathHelper::Pi), m_Phi(/*0.25f*/0.1f * MathHelper::Pi), m_Radius(/*5.0f*//*200.0f*/15.0f)
{
    //m_MainWndCaption = L"Box Demo";
    //m_MainWndCaption = L"Hills Demo";
    m_MainWndCaption = L"Shapes Demo";

    m_LastMousePos.x = 0;
    m_LastMousePos.y = 0;

    XMMATRIX identity = XMMatrixIdentity();
    m_MatrixBuffer.world = identity;
    m_MatrixBuffer.view = identity;
    m_MatrixBuffer.projection = identity;
}

DrawingApp::~DrawingApp()
{
    delete m_Model;
    delete m_ColorShader;
}

bool DrawingApp::Initialize()
{
    if (!D3DApp::Initialize())
    {
        return false;
    }
    
    m_Model->InitializeBuffers(m_D3DDevice);
    m_ColorShader->InitializeShaders(m_D3DDevice, m_hMainWnd, L"src/ColorVertexShader.vs", L"src/ColorPixelShader.ps");

    return true;
}

void DrawingApp::OnResize()
{
    D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    m_MatrixBuffer.projection = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, GetAspectRatio(), m_ScreenNear, m_ScreenDepth);
}

void DrawingApp::UpdateScene(float dt)
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

void DrawingApp::DrawScene()
{
    assert(m_D3DDeviceContext);
    assert(m_SwapChain);

    // Begin Scene
    m_D3DDeviceContext->ClearRenderTargetView(m_RenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    m_D3DDeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    /*
    m_MatrixBuffer.world = m_WorldMatrix;
    // m_MatrixBuffer.projection is updated in OnResize()
    // m_MatrixBuffer.view is updated in UpdateScene()

    m_Model->RenderBuffers(m_D3DDeviceContext);

    m_ColorShader->SetShaderParameters(m_D3DDeviceContext, m_MatrixBuffer.world, m_MatrixBuffer.view, m_MatrixBuffer.projection);
    m_ColorShader->RenderShader(m_D3DDeviceContext, m_Model->GetIndexCount());
    */

    m_Model->RenderBuffers(m_D3DDeviceContext);

    // Draw the grid
    m_ColorShader->SetShaderParameters(m_D3DDeviceContext, m_Model->GetGridWorld(), m_MatrixBuffer.view, m_MatrixBuffer.projection);
    m_ColorShader->RenderShader(m_D3DDeviceContext, m_Model->GetGridIndexCount(), m_Model->GetGridIndexOffset(), m_Model->GetGridVertexOffset());

    // Draw the box
    m_ColorShader->SetShaderParameters(m_D3DDeviceContext, m_Model->GetBoxWorld(), m_MatrixBuffer.view, m_MatrixBuffer.projection);
    m_ColorShader->RenderShader(m_D3DDeviceContext, m_Model->GetBoxIndexCount(), m_Model->GetBoxIndexOffset(), m_Model->GetBoxVertexOffset());

    // Draw center sphere
    m_ColorShader->SetShaderParameters(m_D3DDeviceContext, m_Model->GetCenterSphereWorld(), m_MatrixBuffer.view, m_MatrixBuffer.projection);
    m_ColorShader->RenderShader(m_D3DDeviceContext, m_Model->GetSphereIndexCount(), m_Model->GetSphereIndexOffset(), m_Model->GetSphereVertexOffset());

    // Draw the cylinders
    for (int i = 0; i < 10; i++)
    {
        m_ColorShader->SetShaderParameters(m_D3DDeviceContext, m_Model->GetCylWorld()[i], m_MatrixBuffer.view, m_MatrixBuffer.projection);
        m_ColorShader->RenderShader(m_D3DDeviceContext, m_Model->GetCylinderIndexCount(), m_Model->GetCylinderIndexOffset(), m_Model->GetCylinderVertexOffset());
    }

    // Draw the spheres
    for (int i = 0; i < 10; i++)
    {
        m_ColorShader->SetShaderParameters(m_D3DDeviceContext, m_Model->GetSphereWorld()[i], m_MatrixBuffer.view, m_MatrixBuffer.projection);
        m_ColorShader->RenderShader(m_D3DDeviceContext, m_Model->GetSphereIndexCount(), m_Model->GetSphereIndexOffset(), m_Model->GetSphereVertexOffset());
    }    

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

void DrawingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    m_LastMousePos.x = x;
    m_LastMousePos.y = y;

    SetCapture(m_hMainWnd);
}

void DrawingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void DrawingApp::OnMouseMove(WPARAM btnState, int x, int y)
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
        float dx = /*0.005f*//*0.2f*/0.01f * static_cast<float>(x - m_LastMousePos.x);
        float dy = /*0.005f*//*0.2f*/0.01f * static_cast<float>(y - m_LastMousePos.y);

        // Update the camera radius based on input.
        m_Radius -= dx - dy;

        // Restrict the radius.
        m_Radius = MathHelper::Clamp(m_Radius, /*3.0f*//*50.0f*/3.0f, /*15.0f*//*500.0f*/200.0f);
    }

    m_LastMousePos.x = x;
    m_LastMousePos.y = y;
}