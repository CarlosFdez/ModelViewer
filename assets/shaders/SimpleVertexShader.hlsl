#include "base.hlsl"

struct VertexShaderInput
{
	float3 pos: POSITION;
	float3 normal: NORMAL;
	float3 color: COLOR;
};

VertexShaderOutput main(VertexShaderInput input)
{
	// Create vertex shader outputs that 
	VertexShaderOutput vertexShaderOutput;
	vertexShaderOutput.worldPos = mul(model, float4(input.pos, 1.0f));
	vertexShaderOutput.position = mul(viewProjection, vertexShaderOutput.worldPos);
	vertexShaderOutput.normal = mul(model, float4(input.normal, 0.0f));
	vertexShaderOutput.color = input.color;
	return vertexShaderOutput;
}