struct PSInput
{
	float4 position : SV_Position;
	float4 color : Color;
};

float4 main(PSInput input) : SV_Target
{
#if 0
	float3 light = float3(-20.f, 20.f, -10.f);
	float3 lightdir = normalize(light - input.world_position.xyz);

	float c = max(dot(normalize(input.normal), lightdir), 0.f);
#endif
	return float4(input.color.xyzw);
}
