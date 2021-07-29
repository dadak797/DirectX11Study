#pragma comment(lib, "d3dcompiler.lib")

#include "Shader.h"
#include "MathHelper.h"

#include <fstream>
#include <d3dcompiler.h>


Shader::Shader()
    : m_VertexShader(nullptr), m_PixelShader(nullptr)
    , m_InputLayout(nullptr), m_MatrixBuffer(nullptr), m_LightBuffer(nullptr)
{
}

Shader::~Shader()
{
    ReleaseCOM(m_LightBuffer);
    ReleaseCOM(m_MatrixBuffer);
    ReleaseCOM(m_InputLayout);
    ReleaseCOM(m_PixelShader);
    ReleaseCOM(m_VertexShader);
}

void Shader::OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hWnd, LPCWSTR shaderFile)
{
    std::ofstream fout;

    // Get a pointer to the error message text buffer.
    const char* compileErrors = (const char*)(errorMessage->GetBufferPointer());

    // Get the length of the message.
    size_t bufferSize = errorMessage->GetBufferSize();

    // Open a file to write the error message to.
    fout.open("ShaderError.txt");

    // Write out the error message.
    for (size_t i = 0; i < bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    // Close the file.
    fout.close();

    // Release the error message.
    ReleaseCOM(errorMessage);

    // Pop a message up on the screen to notify the user to check the text file for compile errors.
    MessageBox(hWnd, L"Error compiling shader.  Check ShaderError.txt for message.", shaderFile, MB_OK);
}

bool Shader::InitializeShaders(ID3D11Device* device, HWND hWnd, LPCWSTR vertexShaderFile, LPCWSTR pixelShaderFile)
{
    ID3DBlob* vertexShaderBuffer = nullptr;
    ID3DBlob* pixelShaderBuffer = nullptr;
    ID3DBlob* errorMessage = nullptr;

    // Compile the vertex shader code.
    HR(D3DCompileFromFile(vertexShaderFile, NULL, NULL, "LightVertexShader", "vs_5_0", 0, 0, &vertexShaderBuffer, &errorMessage));
    
    // If the shader failed to compile it should have written something to the error message.
    if (errorMessage)
    {
        OutputShaderErrorMessage(errorMessage, hWnd, vertexShaderFile);
        return false;
    }

    // Compile the pixel shader code.
    HR(D3DCompileFromFile(pixelShaderFile, NULL, NULL, "LightPixelShader", "ps_5_0", 0, 0, &pixelShaderBuffer, &errorMessage));

    // If the shader failed to compile it should have writen something to the error message.
    if (errorMessage)
    {
        OutputShaderErrorMessage(errorMessage, hWnd, pixelShaderFile);
        return false;
    }

    // Create the vertex shader from the buffer.
    HR(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_VertexShader));

    // Create the pixel shader from the buffer.
    HR(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_PixelShader));

    constexpr int numElements = 2;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[numElements];

    // Create the vertex input layout description.
    // This setup needs to match the VertexType stucture.
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "NORMAL";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    // Create the vertex input layout.
    HR(device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
       vertexShaderBuffer->GetBufferSize(), &m_InputLayout));

    // Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
    ReleaseCOM(vertexShaderBuffer);
    ReleaseCOM(pixelShaderBuffer);

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    HR(device->CreateBuffer(&matrixBufferDesc, NULL, &m_MatrixBuffer));

    // Setup the description of the light dynamic constant buffer that is in the pixel shader.
    // Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
    D3D11_BUFFER_DESC lightBufferDesc;
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    HR(device->CreateBuffer(&lightBufferDesc, NULL, &m_LightBuffer));

    return true;
}

void Shader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix
    , DirectionalLight dirLight, PointLight pointLight, SpotLight spotLight, XMFLOAT3 eyePosition, Material material)
{
    // Transpose the matrices to prepare them for the shader.
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // Lock the constant buffer so it can be written to.
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR(deviceContext->Map(m_MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    // Get a pointer to the data in the constant buffer.
    MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

    // Copy the matrices into the constant buffer.
    dataPtr->gWorld = worldMatrix;
    dataPtr->gView = viewMatrix;
    dataPtr->gProjection = projectionMatrix;
    dataPtr->gWorldInvTranspose = MathHelper::InverseTranspose(worldMatrix);

    // Unlock the constant buffer.
    deviceContext->Unmap(m_MatrixBuffer, 0);

    // Set the position of the constant buffer in the vertex shader.
    unsigned int bufferNumber = 0;

    // Finanly set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_MatrixBuffer);

    //// Lock the light constant buffer so it can be written to.
    HR(deviceContext->Map(m_LightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    // Get a pointer to the data in the constant buffer.
    LightBufferType* dataPtr2 = (LightBufferType*)mappedResource.pData;

    // Copy the lighting variables into the constant buffer.
    dataPtr2->gDirLight = dirLight;
    dataPtr2->gPointLight = pointLight;
    dataPtr2->gSpotLight = spotLight;
    dataPtr2->gMaterial = material;
    dataPtr2->gEyePosW = eyePosition;
    dataPtr2->pad = 0.0f;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_LightBuffer, 0);

    // Set the position of the light constant buffer in the pixel shader.
    bufferNumber = 0;

    // Finally set the light constant buffer in the pixel shader with the updated values.
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_LightBuffer);
}

void Shader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount, UINT indexOffset, int vertexOffset)
{
    // Set the vertex input layout.
    deviceContext->IASetInputLayout(m_InputLayout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_VertexShader, NULL, 0);
    deviceContext->PSSetShader(m_PixelShader, NULL, 0);

    // Render the triangle.
    deviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}