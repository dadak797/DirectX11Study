#pragma once

#include "GeometryGenerator.h"


class HillsModel
{
private:
    struct VertexType
    {
        XMFLOAT3 Position;
        XMFLOAT4 Color;
    };

public:
    HillsModel();
    ~HillsModel();

    bool InitializeBuffers(ID3D11Device* device);
    void RenderBuffers(ID3D11DeviceContext* deviceContext);
    int GetIndexCount() const { return m_IndexCount; }

private:
    ID3D11Buffer* m_VertexBuffer;
    ID3D11Buffer* m_IndexBuffer;
    int m_VertexCount, m_IndexCount;

    float GetHeight(float x, float z) const;
};

