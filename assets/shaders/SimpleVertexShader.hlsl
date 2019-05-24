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
	vertexShaderOutput.pos = float4(input.pos.x, input.pos.y, 0.5f, 1.0f);
	vertexShaderOutput.color = input.color;
	return vertexShaderOutput;
}