#include "morton.hlsli"

struct FSInput
{
	noperspective float4 position : SV_Position;
	nointerpolation float3 normal : Normal;
	nointerpolation float4 aabb : AABB;
	nointerpolation uint proj_idx : ProjIdx;
	centroid float3 posagain : Position;
};


cbuffer lulz : register(b0)
{
	float4 bounds;
};

cbuffer lulz2 : register(b1)
{
	float4 dimensions;
};

RWStructuredBuffer<uint> countbuf : register(u1);
RWStructuredBuffer<uint> fragments : register(u2);

static matrix projs[] =
{
	// project down x-axis
	matrix
	(
		float4( 0.f, 0.f, 1.f, 0.f),
		float4( 0.f, 1.f, 0.f, 0.f),
		float4( 1.f, 0.f, 0.f, 0.f),
		float4( 0.f, 0.f, 0.f, 1.f)
	),

	// project down y-axis
	matrix
	(
		float4( 0.f, 1.f, 0.f, 0.f),
		float4( 0.f, 0.f, 1.f, 0.f),
		float4( 1.f, 0.f, 0.f, 0.f),
		float4( 0.f, 0.f, 0.f, 1.f)
	),

	// project down z-axis
	matrix
	(
		float4( 1.f, 0.f, 0.f, 0.f),
		float4( 0.f, 1.f, 0.f, 0.f),
		float4( 0.f, 0.f, 1.f, 0.f),
		float4( 0.f, 0.f, 0.f, 1.f)
	),

	// debugging, wipe triangle out
	matrix
	(
		float4( 0.f, 0.f, 0.f, 0.f),
		float4( 0.f, 0.f, 0.f, 0.f),
		float4( 0.f, 0.f, 0.f, 0.f),
		float4( 0.f, 0.f, 0.f, 1.f)
	),
};

float4 main(FSInput input) : SV_Target
{
	float3 q = input.posagain.xyz;

	if (q.x < input.aabb.x || q.y < input.aabb.y || q.x > input.aabb.z || q.y > input.aabb.w)
		discard;

	// position now in [0, 1] range
	float3 p = float3(input.posagain.xy * 0.5f + 0.5f, input.posagain.z);
	// quantize
	float d = dimensions.x;
	p = floor(p * d);

	// unproject and expand to 128 for now
	float3 p2 = mul(projs[input.proj_idx], float4(p, 1.f)).xyz;

	// write new fragment
	uint idx;
	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z);

	discard;
	return float4(1.f, 0.f, 0.f, 0.f);
}
