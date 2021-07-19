#include "WaveModel.h"

WaveModel::WaveModel()
    : m_GridVertexBuffer(nullptr), m_GridIndexBuffer(nullptr)
    , m_GridWorld(XMMatrixIdentity()), m_WavesWorld(XMMatrixIdentity())
    , m_WavesVertexBuffer(nullptr), m_WavesIndexBuffer(nullptr)
{
}

WaveModel::~WaveModel()
{
    ReleaseCOM(m_GridVertexBuffer);
    ReleaseCOM(m_GridIndexBuffer);
}

bool WaveModel::InitializeBuffers(ID3D11Device* device)
{
    m_Waves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);
    BuildLandGeometryBuffers(device);
    BuildWavesGeometryBuffers(device);

    return true;
}

void WaveModel::RenderGridBuffers(ID3D11DeviceContext* deviceContext)
{
    // Set vertex buffer stride and offset.
    UINT stride = sizeof(VertexType);
    UINT offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_GridVertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_GridIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void WaveModel::RenderWavesBuffers(ID3D11DeviceContext* deviceContext)
{
    // Set vertex buffer stride and offset.
    UINT stride = sizeof(VertexType);
    UINT offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_WavesVertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_WavesIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void WaveModel::BuildLandGeometryBuffers(ID3D11Device* device)
{
    GeometryGenerator::MeshData grid;

    GeometryGenerator geoGen;
    geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

    m_GridVertexCount = static_cast<int>(grid.Vertices.size());
    m_GridIndexCount = static_cast<int>(grid.Indices.size());

    // Extract the vertex elements we are interested and apply the height function to
    // each vertex.  In addition, color the vertices based on their height so we have
    // sandy looking beaches, grassy low hills, and snow mountain peaks.

    std::vector<VertexType> vertices(grid.Vertices.size());
    for (size_t i = 0; i < grid.Vertices.size(); ++i)
    {
        XMFLOAT3 p = grid.Vertices[i].Position;

        p.y = GetHeight(p.x, p.z);

        vertices[i].Position = p;

        // Color the vertex based on its height.
        if (p.y < -10.0f)
        {
            // Sandy beach color.
            vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
        }
        else if (p.y < 5.0f)
        {
            // Light yellow-green.
            vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        }
        else if (p.y < 12.0f)
        {
            // Dark yellow-green.
            vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
        }
        else if (p.y < 20.0f)
        {
            // Dark brown.
            vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
        }
        else
        {
            // White snow.
            vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    // Set up the description of the static vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_GridVertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = &vertices[0];
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    HR(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_GridVertexBuffer));

    // Set up the description of the static index buffer.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = sizeof(UINT) * m_GridIndexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = &grid.Indices[0];
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    HR(device->CreateBuffer(&indexBufferDesc, &indexData, &m_GridIndexBuffer));
}

float WaveModel::GetHeight(float x, float z) const
{
    return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

void WaveModel::BuildWavesGeometryBuffers(ID3D11Device* device)
{
    m_WaveVertexCount = m_Waves.VertexCount();

    // Create the vertex buffer.  Note that we allocate space only, as
    // we will be updating the data every time step of the simulation.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_Waves.VertexCount();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    HR(device->CreateBuffer(&vertexBufferDesc, 0, &m_WavesVertexBuffer));

    // Create the index buffer.  The index buffer is fixed, so we only 
    // need to create and set once.

    std::vector<UINT> indices(3 * m_Waves.TriangleCount()); // 3 indices per face
    m_WavesIndexCount = static_cast<int>(indices.size());

    // Iterate over each quad.
    UINT m = m_Waves.RowCount();
    UINT n = m_Waves.ColumnCount();
    int k = 0;
    for (UINT i = 0; i < m - 1; ++i)
    {
        for (DWORD j = 0; j < n - 1; ++j)
        {
            indices[k] = i * n + j;
            indices[k + 1] = i * n + j + 1;
            indices[k + 2] = (i + 1)*n + j;

            indices[k + 3] = (i + 1)*n + j;
            indices[k + 4] = i * n + j + 1;
            indices[k + 5] = (i + 1)*n + j + 1;

            k += 6; // next quad
        }
    }

    // Set up the description of the static index buffer.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = &indices[0];
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    HR(device->CreateBuffer(&indexBufferDesc, &indexData, &m_WavesIndexBuffer));
}

void WaveModel::WaveVertexBufferUpdate(ID3D11DeviceContext* deviceContext)
{
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(deviceContext->Map(m_WavesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

    WaveModel::VertexType* v = reinterpret_cast<VertexType*>(mappedData.pData);
    for (UINT i = 0; i < m_Waves.VertexCount(); ++i)
    {
        v[i].Position = m_Waves[i];
        v[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    deviceContext->Unmap(m_WavesVertexBuffer, 0);
}
