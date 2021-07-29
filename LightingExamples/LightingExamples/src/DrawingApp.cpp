#include "DrawingApp.h"
#include "MathHelper.h"


DrawingApp::DrawingApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
    , m_Model(new WaveModel()), m_Shader(new Shader())
    , m_Theta(1.5f * MathHelper::Pi), m_Phi(0.1f * MathHelper::Pi), m_Radius(80.0f)
    , m_EyePosition(0.0f, 0.0f, 0.0f)
{
    m_MainWndCaption = L"Lighting Wave Demo";

    m_LastMousePos.x = 0;
    m_LastMousePos.y = 0;

    XMMATRIX identity = XMMatrixIdentity();
    m_View = identity;
    m_Projection = identity;

    // Directional light.
    m_DirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_DirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    // Point light--position is changed every frame to animate in UpdateScene function.
    m_PointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_PointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_PointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_PointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    m_PointLight.Range = 25.0f;

    // Spot light--position and direction changed every frame to animate in UpdateScene function.
    m_SpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_SpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
    m_SpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_SpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
    m_SpotLight.Spot = 96.0f;
    m_SpotLight.Range = 10000.0f;
}

DrawingApp::~DrawingApp()
{
    delete m_Model;
    delete m_Shader;
}

bool DrawingApp::Initialize()
{
    if (!D3DApp::Initialize())
    {
        return false;
    }
    
    m_Model->InitializeBuffers(m_D3DDevice);
    m_Shader->InitializeShaders(m_D3DDevice, m_hMainWnd, L"src/LightVertexShader.vs", L"src/LightPixelShader.ps");

    return true;
}

void DrawingApp::OnResize()
{
    D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    m_Projection = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, GetAspectRatio(), m_ScreenNear, m_ScreenDepth);
}

void DrawingApp::UpdateScene(float dt)
{
    // Convert Spherical to Cartesian coordinates.
    float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
    float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
    float y = m_Radius * cosf(m_Phi);

    m_EyePosition = XMFLOAT3(x, y, z);

    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    m_View = XMMatrixLookAtLH(pos, target, up);

    // Start Wave Update with Time
    // Every quarter second, generate a random wave.
    static float t_base = 0.0f;
    if ((m_Timer.TotalTime() - t_base) >= 0.25f)
    {
        t_base += 0.25f;

        DWORD i = 5 + rand() % (m_Model->GetWavesRowCount() - 10);
        DWORD j = 5 + rand() % (m_Model->GetWavesColumnCount() - 10);

        float r = MathHelper::RandF(1.0f, 2.0f);

        m_Model->WaveDisturb(i, j, r);
    }

    m_Model->WaveUpdate(dt);

    // Update the wave vertex buffer with the new solution.
    m_Model->WaveVertexBufferUpdate(m_D3DDeviceContext);

    // End Wave Update

    // Animate the lights.

    // Circle light over the land surface.
    m_PointLight.Position.x = 70.0f*cosf(0.2f * m_Timer.TotalTime());
    m_PointLight.Position.z = 70.0f*sinf(0.2f * m_Timer.TotalTime());
    m_PointLight.Position.y = MathHelper::Max(m_Model->GetHillHeight(m_PointLight.Position.x,
        m_PointLight.Position.z), -3.0f) + 10.0f;


    // The spotlight takes on the camera position and is aimed in the
    // same direction the camera is looking.  In this way, it looks
    // like we are holding a flashlight.
    m_SpotLight.Position = m_EyePosition;
    XMStoreFloat3(&m_SpotLight.Direction, XMVector3Normalize(target - pos));
}

void DrawingApp::DrawScene()
{
    assert(m_D3DDeviceContext);
    assert(m_SwapChain);

    // Begin Scene
    m_D3DDeviceContext->ClearRenderTargetView(m_RenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    m_D3DDeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Draw the grid
    m_Model->RenderGridBuffers(m_D3DDeviceContext);
    m_Shader->SetShaderParameters(m_D3DDeviceContext, m_Model->GetGridWorld(), m_View, m_Projection
        , m_DirLight, m_PointLight, m_SpotLight, m_EyePosition, m_Model->GetGridMaterial());
    m_Shader->RenderShader(m_D3DDeviceContext, m_Model->GetGridIndexCount());

    // Draw the waves
    m_Model->RenderWavesBuffers(m_D3DDeviceContext);
    m_Shader->SetShaderParameters(m_D3DDeviceContext, m_Model->GetWavesWorld(), m_View, m_Projection
        , m_DirLight, m_PointLight, m_SpotLight, m_EyePosition, m_Model->GetWavesMaterial());
    m_Shader->RenderShader(m_D3DDeviceContext, m_Model->GetWavesIndexCount());
    
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
        float dx = 0.2f * static_cast<float>(x - m_LastMousePos.x);
        float dy = 0.2f * static_cast<float>(y - m_LastMousePos.y);

        // Update the camera radius based on input.
        m_Radius -= dx - dy;

        // Restrict the radius.
        m_Radius = MathHelper::Clamp(m_Radius, 50.0f, 500.0f);
    }

    m_LastMousePos.x = x;
    m_LastMousePos.y = y;
}