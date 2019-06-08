cbuffer VSBuffer : register(b0)
{
	float4x4 model;
	float4x4 viewProjection;
}

struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	float4 worldPos: POSITION0;
	float3 normal: NORMAL0;
	float3 color: COLOR;
};