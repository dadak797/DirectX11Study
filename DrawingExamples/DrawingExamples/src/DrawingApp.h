#pragma once

#include "D3DApp.h"
#include "ColorShader.h"
//#include "BoxModel.h"
#include "HillsModel.h"


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
    //BoxModel* m_Model;
    HillsModel* m_Model;
    ColorShader* m_ColorShader;
    MatrixBufferType m_MatrixBuffer;

    float m_Theta, m_Phi, m_Radius;
    POINT m_LastMousePos;
};

