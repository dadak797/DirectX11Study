//***************************************************************************************
// HillsDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#pragma once

#include <Windows.h>
#include <DirectXMath.h>
using namespace DirectX;


class Waves
{
public:
	Waves();
	~Waves();

	UINT RowCount() const;
	UINT ColumnCount() const;
	UINT VertexCount() const;
	UINT TriangleCount() const;

	// Returns the solution at the ith grid point.
	const XMFLOAT3& operator[](int i) const { return m_CurrSolution[i]; }

    // Returns the solution normal at the ith grid point.
    const XMFLOAT3& Normal(int i) const { return m_Normals[i]; }

    // Returns the unit tangent vector at the ith grid point in the local x-axis direction.
    const XMFLOAT3& TangentX(int i) const { return m_TangentX[i]; }

	void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
	void Update(float dt);
	void Disturb(UINT i, UINT j, float magnitude);

private:
	UINT m_NumRows;
	UINT m_NumCols;

	UINT m_VertexCount;
	UINT m_TriangleCount;

	// Simulation constants we can precompute.
	float m_K1;
	float m_K2;
	float m_K3;

	float m_TimeStep;
	float m_SpatialStep;

	XMFLOAT3* m_PrevSolution;
	XMFLOAT3* m_CurrSolution;
    XMFLOAT3* m_Normals;
    XMFLOAT3* m_TangentX;
};