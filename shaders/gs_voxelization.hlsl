struct VSOutput
{
	float4 position : SV_Position;
	float4 world_position : SV_Position;
};

struct GSOutput
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
	// expand triangel by half-diagonal of pixel
	//  = 0.5 * sqrt((1/d)^2 + (1/d)^2)
	//  = 0.5 * sqrt((1/d^2) + (1/d^2))
	//  = 0.5 * sqrt(2/d^2)
	//  = 0.5 * sqrt(2) / d
	const float2 hpixel = 0.5f * 1.4142135637309f / dimensions.xy;

	// we write a lot of shared state
	GSOutput g;

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
	g.n = cross(v1 - v0, v2 - v1);
	g.dp = 1.f / dimensions.xyz;
	g.c = float3(g.n.x > 0.f ? g.dp.x : 0.f, g.n.y > 0.f ? g.dp.y : 0.f, g.n.z > 0.f ? g.dp.z : 0.f);
	g.d1 = dot(g.n, g.c - v0);
	g.d2 = dot(g.n, g.dp - g.c - v0);

	g.e0 = v1 - v0;
	g.e1 = v2 - v1;
	g.e2 = v0 - v2;

	// xy-plane 
	float xym = (g.n.z < 0.f ? -1.f : 1.f);
	g.ne0xy = float4(-g.e0.y, g.e0.x, 0.f, 0.f) * xym;
	g.ne1xy = float4(-g.e1.y, g.e1.x, 0.f, 0.f) * xym;
	g.ne2xy = float4(-g.e2.y, g.e2.x, 0.f, 0.f) * xym;

	float4 v0xy = float4(v0.x, v0.y, 0.f, 0.f);
	float4 v1xy = float4(v1.x, v1.y, 0.f, 0.f);
	float4 v2xy = float4(v2.x, v2.y, 0.f, 0.f);

	g.de0xy = -dot(g.ne0xy, v0xy) + max(0.f, g.dp.x * g.ne0xy.x) + max(0.f, g.dp.y * g.ne0xy.y);
	g.de1xy = -dot(g.ne1xy, v1xy) + max(0.f, g.dp.x * g.ne1xy.x) + max(0.f, g.dp.y * g.ne1xy.y);
	g.de2xy = -dot(g.ne2xy, v2xy) + max(0.f, g.dp.x * g.ne2xy.x) + max(0.f, g.dp.y * g.ne2xy.y);

	// yz-plane
	float yzm = (g.n.x < 0.f ? -1.f : 1.f);
	g.ne0yz = float4(-g.e0.z, g.e0.y, 0.f, 0.f) * yzm;
	g.ne1yz = float4(-g.e1.z, g.e1.y, 0.f, 0.f) * yzm;
	g.ne2yz = float4(-g.e2.z, g.e2.y, 0.f, 0.f) * yzm;

	float4 v0yz = float4(v0.y, v0.z, 0.f, 0.f);
	float4 v1yz = float4(v1.y, v1.z, 0.f, 0.f);
	float4 v2yz = float4(v2.y, v2.z, 0.f, 0.f);

	g.de0yz = -dot(g.ne0yz, v0yz) + max(0.f, g.dp.y * g.ne0yz.x) + max(0.f, g.dp.z * g.ne0yz.y);
	g.de1yz = -dot(g.ne1yz, v1yz) + max(0.f, g.dp.y * g.ne1yz.x) + max(0.f, g.dp.z * g.ne1yz.y);
	g.de2yz = -dot(g.ne2yz, v2yz) + max(0.f, g.dp.y * g.ne2yz.x) + max(0.f, g.dp.z * g.ne2yz.y);

	// zx-plane
	float zxm = (g.n.y < 0.f ? -1.f : 1.f);
	g.ne0zx = float4(-g.e0.x, g.e0.z, 0.f, 0.f) * zxm;
	g.ne1zx = float4(-g.e1.x, g.e1.z, 0.f, 0.f) * zxm;
	g.ne2zx = float4(-g.e2.x, g.e2.z, 0.f, 0.f) * zxm;

	float4 v0zx = float4(v0.z, v0.x, 0.f, 0.f);
	float4 v1zx = float4(v1.z, v1.x, 0.f, 0.f);
	float4 v2zx = float4(v2.z, v2.x, 0.f, 0.f);

	g.de0zx = -dot(g.ne0zx, v0zx) + max(0.f, g.dp.z * g.ne0zx.x) + max(0.f, g.dp.x * g.ne0zx.y);
	g.de1zx = -dot(g.ne1zx, v1zx) + max(0.f, g.dp.z * g.ne1zx.x) + max(0.f, g.dp.x * g.ne1zx.y);
	g.de2zx = -dot(g.ne2zx, v2zx) + max(0.f, g.dp.z * g.ne2zx.x) + max(0.f, g.dp.x * g.ne2zx.y);

	// expand triangle by half-pixels
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

	// output triangle
	g.position = pv0;
	g.aabb = aabb;
	g.proj = proj_idx;
	output.Append(g);

	g.position = pv1;
	g.aabb = aabb;
	g.proj = proj_idx;
	output.Append(g);

	g.position = pv2;
	g.aabb = aabb;
	g.proj = proj_idx;
	output.Append(g);
}
