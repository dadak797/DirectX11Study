#pragma once

#include "GeometryGenerator.h"


class SkullModel
{
private:
    struct VertexType
    {
        XMFLOAT3 Position;
        XMFLOAT4 Color;
    };

public:
    SkullModel();
    ~SkullModel();

    bool InitializeBuffers(ID3D11Device* device);
    void RenderBuffers(ID3D11DeviceContext* deviceContext);
    int GetIndexCount() const { return m_IndexCount; }

    const XMMATRIX& GetSkullWorld() const { return m_SkullWorld; }

private:
    ID3D11Buffer* m_VertexBuffer;
    ID3D11Buffer* m_IndexBuffer;
    int m_VertexCount, m_IndexCount;

    XMMATRIX m_SkullWorld;
};

