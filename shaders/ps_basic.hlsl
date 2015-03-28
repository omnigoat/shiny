struct ps_input
{
	float4 blah : SV_Position;
	float4 pos : Position;
	float4 color : Color;
	float3 normal : Normal;
};

float4 main(ps_input input) : SV_Target
{
	//float3 light = float3(-2.f, 2.f, -2.f);
	//float3 lightdir = normalize(light - input.pos.xyz);

	//float c = max(dot(normalize(input.normal), lightdir), 0.f);
	//c = max(c, c + 0.2f);
	return float4(0.f, 0.f, 0.f, 1.f);
}
