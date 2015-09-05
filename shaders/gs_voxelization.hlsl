struct VSOutput
{
	float4 position : SV_Position;
	float4 world_position : SV_Position;
};

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

struct GSOutput
{
	float4 position : SV_Position;
	
	nointerpolation uint proj : ProjIdx;
	triangle_intersection_info_t tri : tri;
};


cbuffer buf_voxelize : register(c0)
{
	float4 dimensions;
};



#if 0
// for dims {left: -1, right: 1, top: 1.f, bottom: -1f, near: 0.f, far: 1.f}
static matrix ortho = 
{
	float4(1.f, 0.f, 0.f, 0.f),
	float4(0.f, 1.f, 0.f, 0.f),
	float4(0.f, 0.f, 1.f, 0.f),
	float4(0.f, 0.f, 0.f, 1.f)
};
#endif

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
		float4( 0.f, 0.f, 1.f, 0.f),
		float4( 1.f, 0.f, 0.f, 0.f),
		float4( 0.f, 1.f, 0.f, 0.f),
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


[maxvertexcount(3)]
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> output)
{
	// we write a lot of shared state
	GSOutput g;
	triangle_intersection_info_t tri;

	float3 wsn = cross(input[1].world_position.xyz - input[0].world_position.xyz, input[2].world_position.xyz - input[1].world_position.xyz);

	// pick projection axis
	float3 pn = abs(wsn);
	int proj_idx;
	if (pn.x > pn.y && pn.x > pn.z)
		proj_idx = 0;
	else if (pn.y > pn.x && pn.y > pn.z)
		proj_idx = 1;
	else
		proj_idx = 2;

	// projected verts [-1.f, 1.f]
	float4 pv0 = mul(projs[proj_idx], input[0].world_position);
	float4 pv1 = mul(projs[proj_idx], input[1].world_position);
	float4 pv2 = mul(projs[proj_idx], input[2].world_position);

	// verts for triangle intersection in [0.f, 1.f] range
	float3 v0 = pv0.xyz * 0.5f + 0.5f;
	float3 v1 = pv1.xyz * 0.5f + 0.5f;
	float3 v2 = pv2.xyz * 0.5f + 0.5f;

	// aabb [0.f, 1.f]
	float4 aabb = v0.xyxy;
	aabb = float4(min(aabb.xy, v1.xy), max(aabb.zw, v1.xy));
	aabb = float4(min(aabb.xy, v2.xy), max(aabb.zw, v2.xy));
	aabb.xy -= 0.5f / dimensions.xy;
	aabb.zw += 0.5f / dimensions.xy;

	// triangle intersection values
	//  n: normal
	//  dp: delta-p, where p is size of voxel
	//  c: critical-point
	//  d1 & d2: just like, some values that we'll need
	tri.n = cross(v1 - v0, v2 - v1);
	tri.dp = 1.f / dimensions.xyz;
	tri.c = float3(tri.n.x > 0.f ? tri.dp.x : 0.f, tri.n.y > 0.f ? tri.dp.y : 0.f, tri.n.z > 0.f ? tri.dp.z : 0.f);
	tri.d1 = dot(tri.n, tri.c - v0);
	tri.d2 = dot(tri.n, tri.dp - tri.c - v0);

	tri.e0 = v1 - v0;
	tri.e1 = v2 - v1;
	tri.e2 = v0 - v2;

	// xy-plane 
	float xym = (tri.n.z < 0.f ? -1.f : 1.f);
	tri.ne0xy = float4(-tri.e0.y, tri.e0.x, 0.f, 0.f) * xym;
	tri.ne1xy = float4(-tri.e1.y, tri.e1.x, 0.f, 0.f) * xym;
	tri.ne2xy = float4(-tri.e2.y, tri.e2.x, 0.f, 0.f) * xym;

	float4 v0xy = float4(v0.x, v0.y, 0.f, 0.f);
	float4 v1xy = float4(v1.x, v1.y, 0.f, 0.f);
	float4 v2xy = float4(v2.x, v2.y, 0.f, 0.f);

	tri.de0xy = -dot(tri.ne0xy, v0xy) + max(0.f, tri.dp.x * tri.ne0xy.x) + max(0.f, tri.dp.y * tri.ne0xy.y);
	tri.de1xy = -dot(tri.ne1xy, v1xy) + max(0.f, tri.dp.x * tri.ne1xy.x) + max(0.f, tri.dp.y * tri.ne1xy.y);
	tri.de2xy = -dot(tri.ne2xy, v2xy) + max(0.f, tri.dp.x * tri.ne2xy.x) + max(0.f, tri.dp.y * tri.ne2xy.y);

	// yz-plane
	float yzm = (tri.n.x < 0.f ? -1.f : 1.f);
	tri.ne0yz = float4(-tri.e0.z, tri.e0.y, 0.f, 0.f) * yzm;
	tri.ne1yz = float4(-tri.e1.z, tri.e1.y, 0.f, 0.f) * yzm;
	tri.ne2yz = float4(-tri.e2.z, tri.e2.y, 0.f, 0.f) * yzm;

	float4 v0yz = float4(v0.y, v0.z, 0.f, 0.f);
	float4 v1yz = float4(v1.y, v1.z, 0.f, 0.f);
	float4 v2yz = float4(v2.y, v2.z, 0.f, 0.f);

	tri.de0yz = -dot(tri.ne0yz, v0yz) + max(0.f, tri.dp.y * tri.ne0yz.x) + max(0.f, tri.dp.z * tri.ne0yz.y);
	tri.de1yz = -dot(tri.ne1yz, v1yz) + max(0.f, tri.dp.y * tri.ne1yz.x) + max(0.f, tri.dp.z * tri.ne1yz.y);
	tri.de2yz = -dot(tri.ne2yz, v2yz) + max(0.f, tri.dp.y * tri.ne2yz.x) + max(0.f, tri.dp.z * tri.ne2yz.y);

	// zx-plane
	float zxm = (tri.n.y < 0.f ? -1.f : 1.f);
	tri.ne0zx = float4(-tri.e0.x, tri.e0.z, 0.f, 0.f) * zxm;
	tri.ne1zx = float4(-tri.e1.x, tri.e1.z, 0.f, 0.f) * zxm;
	tri.ne2zx = float4(-tri.e2.x, tri.e2.z, 0.f, 0.f) * zxm;

	float4 v0zx = float4(v0.z, v0.x, 0.f, 0.f);
	float4 v1zx = float4(v1.z, v1.x, 0.f, 0.f);
	float4 v2zx = float4(v2.z, v2.x, 0.f, 0.f);

	tri.de0zx = -dot(tri.ne0zx, v0zx) + max(0.f, tri.dp.z * tri.ne0zx.x) + max(0.f, tri.dp.x * tri.ne0zx.y);
	tri.de1zx = -dot(tri.ne1zx, v1zx) + max(0.f, tri.dp.z * tri.ne1zx.x) + max(0.f, tri.dp.x * tri.ne1zx.y);
	tri.de2zx = -dot(tri.ne2zx, v2zx) + max(0.f, tri.dp.z * tri.ne2zx.x) + max(0.f, tri.dp.x * tri.ne2zx.y);

	// expand triangle by half-pixels
	//  = v * 0.5 * sqrt((1/d)^2 + (1/d)^2)
	//  = v * 0.5 * sqrt((1/d^2) + (1/d^2))
	//  = v * 0.5 * sqrt(2/d^2)
	//  = v * 0.5 * sqrt(2) / d
	//    where v = viewport width = 2  (-1 to 1)
	//  = sqrt(2) / d
	const float2 hpixel = 1.4142135637309f / dimensions.xy;

	float2 e0ss = pv1.xy - pv0.xy;
	float2 e1ss = pv2.xy - pv1.xy;
	float2 e2ss = pv0.xy - pv2.xy;

	float2 n0ss = normalize(float2(e0ss.y, -e0ss.x));
	float2 n1ss = normalize(float2(e1ss.y, -e1ss.x));
	float2 n2ss = normalize(float2(e2ss.y, -e2ss.x));

	n0ss *= -sign(dot(n0ss, pv2.xy - pv0.xy));
	n1ss *= -sign(dot(n1ss, pv0.xy - pv1.xy));
	n2ss *= -sign(dot(n2ss, pv1.xy - pv2.xy));

	pv0.xy += hpixel * (e2ss / dot(e2ss, n0ss.xy) + e0ss / dot(e0ss, n2ss.xy));
	pv1.xy += hpixel * (e0ss / dot(e0ss, n1ss.xy) + e1ss / dot(e1ss, n0ss.xy));
	pv2.xy += hpixel * (e1ss / dot(e1ss, n2ss.xy) + e2ss / dot(e2ss, n1ss.xy));

	// convert Z to NDC (won't render otherwise!)
	pv0.z = pv0.z * 0.5f + 0.5f;
	pv1.z = pv1.z * 0.5f + 0.5f;
	pv2.z = pv2.z * 0.5f + 0.5f;

	tri.aabb = aabb;
	g.tri = tri;

	// output triangle
	g.position = pv0;
	g.proj = proj_idx;
	output.Append(g);

	g.position = pv1;
	g.proj = proj_idx;
	output.Append(g);

	g.position = pv2;
	g.proj = proj_idx;
	output.Append(g);
}
