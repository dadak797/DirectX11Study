#include "ShapesModel.h"


ShapesModel::ShapesModel()
    : m_VertexBuffer(nullptr), m_IndexBuffer(nullptr)
{
    m_GridWorld = XMMatrixIdentity();

    XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
    XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
    m_BoxWorld = XMMatrixMultiply(boxScale, boxOffset);

    XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
    XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
    m_CenterSphereWorld = XMMatrixMultiply(centerSphereScale, centerSphereOffset);

    for (int i = 0; i < 5; ++i)
    {
        m_CylWorld[i * 2 + 0] = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
        m_CylWorld[i * 2 + 1] = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);

        m_SphereWorld[i * 2 + 0] = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
        m_SphereWorld[i * 2 + 1] = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);
    }
}

ShapesModel::~ShapesModel()
{
    ReleaseCOM(m_VertexBuffer);
    ReleaseCOM(m_IndexBuffer);
}

bool ShapesModel::InitializeBuffers(ID3D11Device* device)
{
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData grid;
    GeometryGenerator::MeshData sphere;
    GeometryGenerator::MeshData cylinder;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
    geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
    //geoGen.CreateSphere(0.5f, 20, 20, sphere);
    geoGen.CreateGeosphere(0.5f, 3, sphere);
    geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    m_BoxVertexOffset = 0;
    m_GridVertexOffset = static_cast<int>(box.Vertices.size());
    m_SphereVertexOffset = static_cast<int>(m_GridVertexOffset + grid.Vertices.size());
    m_CylinderVertexOffset = static_cast<int>(m_SphereVertexOffset + sphere.Vertices.size());

    // Cache the index count of each object.
    m_BoxIndexCount = static_cast<int>(box.Indices.size());
    m_GridIndexCount = static_cast<int>(grid.Indices.size());
    m_SphereIndexCount = static_cast<int>(sphere.Indices.size());
    m_CylinderIndexCount = static_cast<int>(cylinder.Indices.size());

    // Cache the starting index for each object in the concatenated index buffer.
    m_BoxIndexOffset = 0;
    m_GridIndexOffset = m_BoxIndexCount;
    m_SphereIndexOffset = m_GridIndexOffset + m_GridIndexCount;
    m_CylinderIndexOffset = m_SphereIndexOffset + m_SphereIndexCount;

    m_VertexCount = static_cast<int>(box.Vertices.size() + grid.Vertices.size()
                  + sphere.Vertices.size() + cylinder.Vertices.size());

    m_IndexCount = m_BoxIndexCount + m_GridIndexCount 
                 + m_SphereIndexCount + m_CylinderIndexCount;

    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.

    std::vector<VertexType> vertices(m_VertexCount);

    XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Position = box.Vertices[i].Position;
        vertices[k].Color = black;
    }

    for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].Position = grid.Vertices[i].Position;
        vertices[k].Color = black;
    }

    for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
    {
        vertices[k].Position = sphere.Vertices[i].Position;
        vertices[k].Color = black;
    }

    for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Position = cylinder.Vertices[i].Position;
        vertices[k].Color = black;
    }

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

    // Pack the indices of all the meshes into one index buffer.
    std::vector<UINT> indices;
    indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
    indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
    indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
    indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

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

    return false;
}

void ShapesModel::RenderBuffers(ID3D11DeviceContext* deviceContext)
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
