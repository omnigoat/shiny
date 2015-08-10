#include "morton.hlsli"

struct FSInput
{
	centroid float4 position : SV_Position;
	nointerpolation float3 normal : Normal;
	nointerpolation float4 aabb : AABB;
	nointerpolation uint proj_idx : ProjIdx;
};


cbuffer cb1 : register(b0)
{
	float4 bounds;
};

cbuffer cb2 : register(b1)
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


bool inter()
{
	
	return true;
}




float4 main(FSInput input) : SV_Target
{
	// position now in [0, dimensions] range
	//  - note: flip y because SV_Position, mult z because depth
	float3 p = floor(float3(input.position.x, dimensions.y - input.position.y, input.position.z * dimensions.z));

	// cull against aabb of triangle
	if (p.x < input.aabb.x || p.y < input.aabb.y || p.x > input.aabb.z || p.y > input.aabb.w)
		discard;

	// gradient
	//float2 dz = float2(ddx(input.posagain.z), ddy(input.posagain.z));

	// unproject
	float3 p2 = mul(projs[input.proj_idx], float4(p, 1.f)).xyz;

	// write new fragment(s)
	//  - currently brute-forcing all 5 possible voxels
	uint idx;
	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z);

	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z + 1);

	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z + 2);

	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z - 2);

	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z - 1);

	//InterlockedAdd(countbuf[0], 1, idx);
	//morton_encoding32(fragments[idx], p2.x, p2.y, p2.z + 3);

	//InterlockedAdd(countbuf[0], 1, idx);
	//morton_encoding32(fragments[idx], p2.x, p2.y, p2.z - 3);

	discard;
	return float4(1.f, 0.f, 0.f, 0.f);
}
