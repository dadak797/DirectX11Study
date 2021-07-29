#pragma once

#include "D3DUtil.h"
#include "LightHelper.h"


//struct MatrixBufferType
//{
//    XMMATRIX world;
//    XMMATRIX view;
//    XMMATRIX projection;
//};

struct LightBufferType
{
    DirectionalLight gDirLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
    Material gMaterial;
    XMFLOAT3 gEyePosW;
    float pad;
};

struct MatrixBufferType
{
    XMMATRIX gWorld;
    XMMATRIX gView;
    XMMATRIX gProjection;
    XMMATRIX gWorldInvTranspose;
};

class Shader
{
public:
    Shader();
    ~Shader();

    bool InitializeShaders(ID3D11Device* device, HWND hWnd, LPCWSTR vertexShaderFile, LPCWSTR pixelShaderFile);
    void OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hWnd, LPCWSTR shaderFile);
    void SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix
        , DirectionalLight dirLight, PointLight pointLight, SpotLight spotLight, XMFLOAT3 eyePosition, Material material);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount, UINT indexOffset = 0, int vertexOffset = 0);

protected:
    ID3D11VertexShader* m_VertexShader;
    ID3D11PixelShader* m_PixelShader;
    ID3D11InputLayout* m_InputLayout;
    ID3D11Buffer* m_MatrixBuffer;
    ID3D11Buffer* m_LightBuffer;
};

