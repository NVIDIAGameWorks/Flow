
cbuffer params : register(b0)
{
	float4x4 projection;
};

struct Input
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct Output
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};

Output meshVS( Input input, uint instance : SV_InstanceID )
{
	Output output;
	output.position = mul(float4(input.position,1.f),projection);
	output.normal = input.normal;

	//output.position.z *= -1.f;
	//output.position.z += 0.5f;

	//output.position.y -= 1.f * float(instance) + 0.1f;

	return output;
}