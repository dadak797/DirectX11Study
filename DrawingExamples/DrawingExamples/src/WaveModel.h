#pragma once

#include "GeometryGenerator.h"
#include "Waves.h"


class WaveModel
{
private:
    struct VertexType
    {
        XMFLOAT3 Position;
        XMFLOAT4 Color;
    };

public:
    WaveModel();
    ~WaveModel();

    bool InitializeBuffers(ID3D11Device* device);
    void RenderGridBuffers(ID3D11DeviceContext* deviceContext);
    void RenderWavesBuffers(ID3D11DeviceContext* deviceContext);
    int GetGridIndexCount() const { return m_GridIndexCount; }
    int GetWavesIndexCount() const { return m_WavesIndexCount; }

    const XMMATRIX& GetGridWorld() const { return m_GridWorld; }
    const XMMATRIX& GetWavesWorld() const { return m_WavesWorld; }

    void WaveDisturb(UINT i, UINT j, float mag) { m_Waves.Disturb(i, j, mag); }
    void WaveUpdate(float dt) { m_Waves.Update(dt); }
    void WaveVertexBufferUpdate(ID3D11DeviceContext* deviceContext);

private:
    ID3D11Buffer* m_GridVertexBuffer;
    ID3D11Buffer* m_GridIndexBuffer;
    ID3D11Buffer* m_WavesVertexBuffer;
    ID3D11Buffer* m_WavesIndexBuffer;
    int m_GridVertexCount, m_GridIndexCount;
    int m_WaveVertexCount, m_WavesIndexCount;

    Waves m_Waves;

    XMMATRIX m_GridWorld;
    XMMATRIX m_WavesWorld;

    void BuildLandGeometryBuffers(ID3D11Device* device);
    void BuildWavesGeometryBuffers(ID3D11Device* device);
    float GetHeight(float x, float z) const;
};

