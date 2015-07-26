
cbuffer lulz : register(c0)
{
	float4 bounds;
};

// so basic!
float4 main(float4 position : Position) : SV_Position
{
	// position in [-1, 1]
	float3 p = (position.xyz - bounds.xyz) / bounds.w;
	// position in ndc
	//float3 p2 = float3(p.xy, p.z * 0.5f + 0.5f);

	return float4(p, 1.f);
}
