#pragma once

#include "D3DUtil.h"


class BoxModel
{
private:
    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

public:
    BoxModel();
    ~BoxModel();

    bool InitializeBuffers(ID3D11Device* device);
    void RenderBuffers(ID3D11DeviceContext* deviceContext);
    int GetIndexCount() const { return m_IndexCount; }

private:
    ID3D11Buffer* m_VertexBuffer;
    ID3D11Buffer* m_IndexBuffer;
    int m_VertexCount, m_IndexCount;
};

