#pragma once

#include "GeometryGenerator.h"


class ShapesModel
{
private:
    struct VertexType
    {
        XMFLOAT3 Position;
        XMFLOAT4 Color;
    };

public:
    ShapesModel();
    ~ShapesModel();

    bool InitializeBuffers(ID3D11Device* device);
    void RenderBuffers(ID3D11DeviceContext* deviceContext);
    int GetIndexCount() const { return m_IndexCount; }

    const XMMATRIX& GetGridWorld() const { return m_GridWorld; }
    const XMMATRIX& GetBoxWorld() const { return m_BoxWorld; }
    const XMMATRIX& GetCenterSphereWorld() const { return m_CenterSphereWorld; }
    const XMMATRIX* GetCylWorld() const { return m_CylWorld; }
    const XMMATRIX* GetSphereWorld() const { return m_SphereWorld; }

    int GetBoxVertexOffset() const { return m_BoxVertexOffset; }
    int GetGridVertexOffset() const { return m_GridVertexOffset; }
    int GetSphereVertexOffset() const { return m_SphereVertexOffset; }
    int GetCylinderVertexOffset() const { return m_CylinderVertexOffset; }

    UINT GetBoxIndexOffset() const { return m_BoxIndexOffset; }
    UINT GetGridIndexOffset() const { return m_GridIndexOffset; }
    UINT GetSphereIndexOffset() const { return m_SphereIndexOffset; }
    UINT GetCylinderIndexOffset() const { return m_CylinderIndexOffset; }

    UINT GetBoxIndexCount() const { return m_BoxIndexCount; }
    UINT GetGridIndexCount() const { return m_GridIndexCount; }
    UINT GetSphereIndexCount() const { return m_SphereIndexCount; }
    UINT GetCylinderIndexCount() const { return m_CylinderIndexCount; }
private:
    ID3D11Buffer* m_VertexBuffer;
    ID3D11Buffer* m_IndexBuffer;
    int m_VertexCount, m_IndexCount;

    XMMATRIX m_SphereWorld[10];
    XMMATRIX m_CylWorld[10];
    XMMATRIX m_BoxWorld;
    XMMATRIX m_GridWorld;
    XMMATRIX m_CenterSphereWorld;

    int m_BoxVertexOffset;
    int m_GridVertexOffset;
    int m_SphereVertexOffset;
    int m_CylinderVertexOffset;

    UINT m_BoxIndexOffset;
    UINT m_GridIndexOffset;
    UINT m_SphereIndexOffset;
    UINT m_CylinderIndexOffset;

    UINT m_BoxIndexCount;
    UINT m_GridIndexCount;
    UINT m_SphereIndexCount;
    UINT m_CylinderIndexCount;
};

