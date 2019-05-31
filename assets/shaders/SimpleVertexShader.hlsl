cbuffer VSBuffer : register(b0)
{
	float4x4 modelViewProj;
}

struct VertexShaderInput
{
	float3 pos: POSITION;
	float3 color: COLOR;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float3 color: COLOR;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput vertexShaderOutput;

	// something something set depth value to 0.5f so its always drawn
	vertexShaderOutput.pos = mul(modelViewProj, float4(input.pos, 1.0f));
	vertexShaderOutput.color = input.color;
	return vertexShaderOutput;
}