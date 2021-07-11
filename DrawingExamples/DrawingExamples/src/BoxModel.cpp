#include "BoxModel.h"


BoxModel::BoxModel()
    : m_VertexBuffer(nullptr), m_IndexBuffer(nullptr)
{

}

BoxModel::~BoxModel()
{
    ReleaseCOM(m_VertexBuffer);
    ReleaseCOM(m_IndexBuffer);
}

bool BoxModel::InitializeBuffers(ID3D11Device* device)
{
    VertexType vertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), Colors::White   },
        { XMFLOAT3(-1.0f, +1.0f, -1.0f), Colors::Black   },
        { XMFLOAT3(+1.0f, +1.0f, -1.0f), Colors::Red     },
        { XMFLOAT3(+1.0f, -1.0f, -1.0f), Colors::Green   },
        { XMFLOAT3(-1.0f, -1.0f, +1.0f), Colors::Blue    },
        { XMFLOAT3(-1.0f, +1.0f, +1.0f), Colors::Yellow  },
        { XMFLOAT3(+1.0f, +1.0f, +1.0f), Colors::Cyan    },
        { XMFLOAT3(+1.0f, -1.0f, +1.0f), Colors::Magenta }
    };

    m_VertexCount = sizeof(vertices) / sizeof(vertices[0]);

    UINT indices[] = {
        // front face
        0, 1, 2,
        0, 2, 3,
        // back face
        4, 6, 5,
        4, 7, 6,
        // left face
        4, 5, 1,
        4, 1, 0,
        // right face
        3, 2, 6,
        3, 6, 7,
        // top face
        1, 5, 6,
        1, 6, 2,
        // bottom face
        4, 0, 3,
        4, 3, 7
    };

    m_IndexCount = sizeof(indices) / sizeof(indices[0]);

    // Set up the description of the static vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_VertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Now create the vertex buffer.
    HR(device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer));

    // Set up the description of the static index buffer.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(UINT) * m_IndexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    HR(device->CreateBuffer(&indexBufferDesc, &indexData, &m_IndexBuffer));

    return true;
}

void BoxModel::RenderBuffers(ID3D11DeviceContext* deviceContext)
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