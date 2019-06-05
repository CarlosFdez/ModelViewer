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

// register slots (uniforms?)
//Texture2D objTexture : TEXTURE: register(t0);
//SamplerState objSamplerState : SAMPLER: register(s0);

float4 main(VertexShaderOutput input) : SV_TARGET
{
	//float3 lightDirection = normalize(float3(2.0, 4.0, 3.0));

	float3 lightDirection = normalize(float3(0., 0.5, -1));
	float lambert = saturate(dot(lightDirection, input.normal));

	float3 lightValue = float3(0.8, 0.78, 0.76);

	// Maintain the current color (alpha 1)
	return float4(lightValue * lambert, 1.0);

	//return float4(1.0, 1.0, 1.0, 1.0);
}