
cbuffer lulz : register(c0)
{
	float4 bounds;
};

// so basic!
noperspective float4 main(float4 position : Position) : SV_Position
{
	// position in [-1, 1]
	float3 p = (position.xyz - bounds.xyz) / bounds.w;

	return float4(p, 1.f);
}
