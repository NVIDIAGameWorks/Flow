
struct Input
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};

float4 meshPS(Input input) : SV_TARGET
{
	float color = 0.1f;

	color += 0.4f * max(0.f,dot(input.normal, float3(0.57f, 0.57f, 0.57f)));

	color += 0.4f * max(0.f, dot(input.normal, float3(-0.57f, 0.57f, 0.57f)));

	color += 0.4f * max(0.f, dot(input.normal, float3(0.57f, 0.57f, -0.57f)));

	color += 0.4f * max(0.f, dot(input.normal, float3(-0.57f, 0.57f, -0.57f)));

	return float4(color.xxx, 1.0f);
}