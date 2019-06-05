cbuffer VSBuffer : register(b0)
{
	float4x4 model;
	float4x4 viewProjection;
}

struct VertexShaderInput
{
	float3 pos: POSITION;
	float3 normal: NORMAL;
	float3 color: COLOR;
};

struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	float4 worldPos: POSITION0;
	float3 normal: NORMAL0;
	float3 color: COLOR;
};

VertexShaderOutput main(VertexShaderInput input)
{

	// something something set depth value to 0.5f so its always drawn
	VertexShaderOutput vertexShaderOutput;
	vertexShaderOutput.worldPos = mul(model, float4(input.pos, 1.0f));
	vertexShaderOutput.position = mul(viewProjection, vertexShaderOutput.worldPos);
	vertexShaderOutput.normal = input.normal;
	vertexShaderOutput.color = input.color;
	return vertexShaderOutput;
}