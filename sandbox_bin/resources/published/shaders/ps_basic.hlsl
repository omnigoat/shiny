struct PSInput
{
	float4 position : SV_Position;
	float4 world_position : Position;
	float3 normal : Normal;
};

float4 main(PSInput input) : SV_Target
{
	float3 light = float3(20.f, 20.f, 10.f);
	float3 lightdir = normalize(light - input.world_position.xyz);

	float c = max(dot(normalize(input.normal), lightdir), 0.f);

	return float4(c, c, c, 1.f);
}
