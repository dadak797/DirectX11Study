#include "SkullModel.h"
#include <fstream>


SkullModel::SkullModel()
    : m_VertexBuffer(nullptr), m_IndexBuffer(nullptr)
    , m_SkullWorld(XMMatrixTranslation(0.0f, -2.0f, 0.0f))
{
}

SkullModel::~SkullModel()
{
    ReleaseCOM(m_VertexBuffer);
    ReleaseCOM(m_IndexBuffer);
}

bool SkullModel::InitializeBuffers(ID3D11Device* device)
{
    std::ifstream fin("src/skull.txt");

    if (!fin.is_open())
    {
        MessageBox(0, L"src/skull.txt not found.", 0, 0);
        return false;
    }

    UINT tcount = 0;
    std::string ignore;

    fin >> ignore >> m_VertexCount;
    fin >> ignore >> tcount;
    fin >> ignore >> ignore >> ignore >> ignore;

    float nx, ny, nz;
    XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

    std::vector<VertexType> vertices(m_VertexCount);
    for (int i = 0; i < m_VertexCount; ++i)
    {
        fin >> vertices[i].Position.x >> vertices[i].Position.y >> vertices[i].Position.z;

        vertices[i].Color = black;

        // Normal not used in this demo.
        fin >> nx >> ny >> nz;
    }

    fin >> ignore;
    fin >> ignore;
    fin >> ignore;

    m_IndexCount = static_cast<int>(3 * tcount);
    std::vector<UINT> indices(m_IndexCount);
    for (UINT i = 0; i < tcount; ++i)
    {
        fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
    }

    fin.close();

    // Set up the description of the static vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_VertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = &vertices[0];
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    HR(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer));

    // Set up the description of the static index buffer.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = sizeof(UINT) * m_IndexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = &indices[0];
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    HR(device->CreateBuffer(&indexBufferDesc, &indexData, &m_IndexBuffer));

    return true;
}

void SkullModel::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    // Set vertex buffer stride and offset.
    UINT stride = sizeof(VertexType);
    UINT offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}