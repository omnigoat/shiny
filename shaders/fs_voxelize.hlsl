#include "morton.hlsli"

struct FSInput
{
	float4 position : SV_Position;
	nointerpolation float3 normal : Normal;
	nointerpolation float4 aabb : AABB;
	nointerpolation uint proj_idx : ProjIdx;
};


cbuffer lulz : register(c0)
{
	float4 bounds;
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
	uint idx;
	InterlockedAdd(countbuf[0], 1, idx);

	float3 p2 = (input.position - 0.5f) * 0.5f;
	//float3 p2 = float3(input.position.xy, (input.position.z - 0.5f) * 2.f);
	//float3 p2 = (input.position - 0.5f) * 2.f;
	float3 p = mul(projs[input.proj_idx], p2);
	p = p * 0.5f + 0.5f;
	//float4 p = input.position;
	p = p * bounds.w * 0.5f;

	morton_encoding32(fragments[idx], p.x, p.y, p.z);
	//fragments[idx] = idx;

	//discard;
	return float4(1.f, 0.f, 0.f, 0.f);
}
