cbuffer MatrixBuffer
{
	matrix gWorld;
	matrix gView;
	matrix gProjection;
	matrix gWorldInvTranspose;
};

struct VertexIn
{
	float4 PosL    : POSITION;
	float3 NormalL : NORMAL;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
};

VertexOut LightVertexShader(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(vin.PosL, gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
	vout.PosH = mul(vin.PosL, gWorld);
	vout.PosH = mul(vout.PosH, gView);
	vout.PosH = mul(vout.PosH, gProjection);
	
	return vout;
}