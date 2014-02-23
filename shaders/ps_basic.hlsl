struct ps_input
{
	float4 blah : SV_Position;
	float4 pos : Position;
	float4 color : Color;
	float3 normal : Normal;
};

float4 main(ps_input input) : SV_Target
{
	float3 light = float3(-2.f, 2.f, -2.f);
	float3 lightdir = normalize(light - input.pos.xyz);

	float c = dot(normalize(input.normal), lightdir);

	return float4(input.color.rgb * c, input.color.a);
}
