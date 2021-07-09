#pragma once

#include "D3DUtil.h"


struct MatrixBufferType
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

class ColorShader
{
public:
    ColorShader();
    ~ColorShader();

    bool InitializeShaders(ID3D11Device* device, HWND hWnd, LPCWSTR vertexShaderFile, LPCWSTR pixelShaderFile);
    void OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hWnd, LPCWSTR shaderFile);
    void SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);
    
protected:
    ID3D11VertexShader* m_VertexShader;
    ID3D11PixelShader* m_PixelShader;
    ID3D11InputLayout* m_InputLayout;
    ID3D11Buffer* m_MatrixBuffer;
};

