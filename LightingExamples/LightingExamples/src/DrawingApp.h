#pragma once

#include "D3DApp.h"
#include "Shader.h"
#include "WaveModel.h"
//#include "MathHelper.h"


class DrawingApp : public D3DApp
{
public:
    DrawingApp(HINSTANCE hInstance);
    ~DrawingApp();

    bool Initialize() override;
    void OnResize() override;
    void UpdateScene(float dt) override;
    void DrawScene() override;

    void OnMouseDown(WPARAM btnState, int x, int y) override;
    void OnMouseUp(WPARAM btnState, int x, int y) override;
    void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
    WaveModel* m_Model;
    Shader* m_Shader;
    
    XMMATRIX m_View;
    XMMATRIX m_Projection;

    float m_Theta, m_Phi, m_Radius;
    POINT m_LastMousePos;

    DirectionalLight m_DirLight;
    PointLight m_PointLight;
    SpotLight m_SpotLight;

    XMFLOAT3 m_EyePosition;
};

