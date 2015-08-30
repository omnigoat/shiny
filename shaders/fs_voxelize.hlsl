#include "morton.hlsli"

struct FSInput
{
	float4 position : SV_Position;

	nointerpolation uint proj : ProjIdx;
	nointerpolation float4 aabb : AABB;
	nointerpolation float3 e0 : e0, e1 : e1, e2 : e2;
	nointerpolation float3 n : Normal;
	nointerpolation float3 dp : DP;
	nointerpolation float3 c : C;
	nointerpolation float d1 : D1, d2 : D2;
	nointerpolation float4 ne0xy : ne0xy, ne1xy : ne1xy, ne2xy : ne2xy, ne0yz : ne0yz, ne1yz : ne1yz, ne2yz : ne2yz, ne0zx : ne0zx, ne1zx : ne1zx, ne2zx : ne2zx;
	nointerpolation float de0xy : de0xy, de1xy : de1xy, de2xy : de2xy, de0yz : de0yz, de1yz : de1yz, de2yz : de2yz, de0zx : de0zx, de1zx : de1zx, de2zx : de2zx;
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
	float3 pr2 = float3(input.position.x, dimensions.y - input.position.y, input.position.z * dimensions.z);
	float3 pr = floor(pr2);

	// position in [0, 1]
	float3 p = pr / dimensions.xyz;

	// cull against aabb of triangle
	if (p.x < input.aabb.x || p.y < input.aabb.y || p.x > input.aabb.z || p.y > input.aabb.w)
		discard;


#if 1
	// triangle-plane
	if ((dot(input.n, p) + input.d1) * (dot(input.n, p) + input.d2) > 0.f)
		return false;

	// xy-plane
	float4 pxy = {p.x, p.y, 0.f, 0.f};
	if ((dot(input.ne0xy, pxy) + input.de0xy) < 0.f || (dot(input.ne1xy, pxy) + input.de1xy) < 0.f || (dot(input.ne2xy, pxy) + input.de2xy) < 0.f)
		return false;

	// yz-plane
	float4 pyz = {p.y, p.z, 0.f, 0.f};
	if ((dot(input.ne0yz, pyz) + input.de0yz) < 0.f || (dot(input.ne1yz, pyz) + input.de1yz) < 0.f || (dot(input.ne2yz, pyz) + input.de2yz) < 0.f)
		return false;

	// zx-plane
	float4 pzx = {p.z, p.x, 0.f, 0.f};
	if ((dot(input.ne0zx, pzx) + input.de0zx) < 0.f || (dot(input.ne1zx, pzx) + input.de1zx) < 0.f || (dot(input.ne2zx, pzx) + input.de2zx) < 0.f)
		return false;
#endif


	// unproject
	float3 p2 = mul(projs[input.proj], float4(pr, 1.f)).xyz;

	// write new fragment(s)
	//  - currently brute-forcing all 5 possible voxels
	uint idx;
	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z);


	float dzdx = ddx(p2.z);
	float dzdy = ddy(p2.y);

	float2 dzdxy = float2(dzdx, dzdy);

	float2 minzdxy = min(p2.z + dzdxy * 0.5f, p2.z - dzdxy * 0.5f);
	float2 maxzdxy = max(p2.z + dzdxy * 0.5f, p2.z - dzdxy * 0.5f);

	float minz = min(minzdxy.x, minzdxy.y);
	float maxz = max(maxzdxy.x, maxzdxy.y);

	//float minzf = floor(minz * dimensions.x);
	//float maxzf = floor(maxz * dimensions.x);

#if 1
	for (float f = minz - 1.f; f <= maxz + 1; f += 1)
	{
		InterlockedAdd(countbuf[0], 1, idx);
		morton_encoding32(fragments[idx], p2.x, p2.y, f);
	}

#else
	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z + 1);

	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z + 2);

	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z - 2);

	InterlockedAdd(countbuf[0], 1, idx);
	morton_encoding32(fragments[idx], p2.x, p2.y, p2.z - 1);
#endif

	//InterlockedAdd(countbuf[0], 1, idx);
	//morton_encoding32(fragments[idx], p2.x, p2.y, p2.z + 3);

	//InterlockedAdd(countbuf[0], 1, idx);
	//morton_encoding32(fragments[idx], p2.x, p2.y, p2.z - 3);

	discard;
	return float4(1.f, 0.f, 0.f, 0.f);
}
