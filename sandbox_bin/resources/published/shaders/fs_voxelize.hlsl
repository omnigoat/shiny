#include "morton.hlsli"

struct triangle_intersection_info_t
{
	float4 aabb;
	float3 e0, e1, e2;
	float3 n;
	float3 dp;
	float3 c;
	float d1, d2;
	float4 ne0xy, ne1xy, ne2xy, ne0yz, ne1yz, ne2yz, ne0zx, ne1zx, ne2zx;
	float de0xy, de1xy, de2xy, de0yz, de1yz, de2yz, de0zx, de1zx, de2zx;
};

struct FSInput
{
	float4 position : SV_Position;

	nointerpolation uint proj : ProjIdx;
	triangle_intersection_info_t tri : tri;

#if 0
	nointerpolation float4 aabb : AABB;
	nointerpolation float3 e0 : e0, e1 : e1, e2 : e2;
	nointerpolation float3 n : Normal;
	nointerpolation float3 dp : DP;
	nointerpolation float3 c : C;
	nointerpolation float d1 : D1, d2 : D2;
	nointerpolation float4 ne0xy : ne0xy, ne1xy : ne1xy, ne2xy : ne2xy, ne0yz : ne0yz, ne1yz : ne1yz, ne2yz : ne2yz, ne0zx : ne0zx, ne1zx : ne1zx, ne2zx : ne2zx;
	nointerpolation float de0xy : de0xy, de1xy : de1xy, de2xy : de2xy, de0yz : de0yz, de1yz : de1yz, de2yz : de2yz, de0zx : de0zx, de1zx : de1zx, de2zx : de2zx;
#endif
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


bool intersect(triangle_intersection_info_t tri, float3 p)
{
	// triangle-plane
	if ((dot(tri.n, p) + tri.d1) * (dot(tri.n, p) + tri.d2) > 0.f)
		return false;

	// xy-plane
	float4 pxy = {p.x, p.y, 0.f, 0.f};
	if ((dot(tri.ne0xy, pxy) + tri.de0xy) < 0.f || (dot(tri.ne1xy, pxy) + tri.de1xy) < 0.f || (dot(tri.ne2xy, pxy) + tri.de2xy) < 0.f)
		return false;

	// yz-plane
	float4 pyz = {p.y, p.z, 0.f, 0.f};
	if ((dot(tri.ne0yz, pyz) + tri.de0yz) < 0.f || (dot(tri.ne1yz, pyz) + tri.de1yz) < 0.f || (dot(tri.ne2yz, pyz) + tri.de2yz) < 0.f)
		return false;

	// zx-plane
	float4 pzx = {p.z, p.x, 0.f, 0.f};
	if ((dot(tri.ne0zx, pzx) + tri.de0zx) < 0.f || (dot(tri.ne1zx, pzx) + tri.de1zx) < 0.f || (dot(tri.ne2zx, pzx) + tri.de2zx) < 0.f)
		return false;

	return true;
}




float4 main(FSInput input) : SV_Target
{
	// transformed-space-position now in [0, dimensions] range
	//  - note: flip y because SV_Position, mult z because depth
	float3 tsp = float3(input.position.x, dimensions.y - input.position.y, input.position.z * dimensions.z);

	// rounded-transform-space-position (for voxel)
	float3 rtsp = floor(tsp);
	// unprojected-position (voxel-coord)
	float3 up = mul(projs[input.proj], float4(rtsp, 1.f)).xyz;

	// position in [0, 1]
	float3 p = tsp / dimensions.xyz;
	// rounded-position
	float3 rp = rtsp / dimensions.xyz;

	// cull against aabb of triangle
	if (p.x < input.tri.aabb.x || p.y < input.tri.aabb.y || p.x > input.tri.aabb.z || p.y > input.tri.aabb.w)
		discard;


	
	float dzdx = ddx(tsp.z);
	float dzdy = ddy(tsp.z);

	float2 dzdxy = float2(dzdx, dzdy);
	
	float2 minzdxy = min(p.z + dzdxy, p.z - dzdxy);
	float2 maxzdxy = max(p.z + 1.f / dimensions.xy + dzdxy, p.z + 1.f / dimensions.xy - dzdxy);

	float minz = floor(min(minzdxy.x, minzdxy.y) * dimensions.xy) / dimensions.xy;
	float maxz = floor(max(maxzdxy.x, maxzdxy.y) * dimensions.xy) / dimensions.xy;

	float zstep = 1.f / dimensions.z;

	for (float f = minz - 10 * zstep; f < maxz + 10 * zstep; f += zstep)
	{
		float3 fp = float3(rp.x, rp.y, f);

		if (intersect(input.tri, fp))
		{
			fp = mul(projs[input.proj], float4(fp * dimensions, 1.f)).xyz;

			uint idx;
			InterlockedAdd(countbuf[0], 1, idx);
			morton_encoding32(fragments[idx], fp.x, fp.y, fp.z);
		}
	}

	discard;
	return float4(1.f, 0.f, 0.f, 0.f);
}