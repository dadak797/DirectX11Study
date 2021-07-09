#pragma once

#include "D3DApp.h"
#include "ColorShader.h"
#include "Model.h"


class BoxApp : public D3DApp
{
public:
    BoxApp(HINSTANCE hInstance);
    ~BoxApp();

    bool Initialize() override;
    void OnResize() override;
    void UpdateScene(float dt) override;
    void DrawScene() override;

    void OnMouseDown(WPARAM btnState, int x, int y) override;
    void OnMouseUp(WPARAM btnState, int x, int y) override;
    void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
//    void InitializeBuffers();
//    bool InitializeShaders(LPCWSTR vsFileName, LPCWSTR psFileName);
//    void OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hwnd, LPCWSTR shaderFileName);
//
//private:
//    ID3D11VertexShader* m_VertexShader;
//    ID3D11PixelShader* m_PixelShader;
//    ID3D11InputLayerout* m_InputLayout;
//    ID3D11Buffer* m_MatrixBuffer;
private:
    Model* m_Model;
    ColorShader* m_ColorShader;
    MatrixBufferType m_MatrixBuffer;

    float m_Theta, m_Phi, m_Radius;
    POINT m_LastMousePos;
};

