struct VSOutput
{
	float4 position : SV_Position;
	float4 world_position : SV_Position;
};

struct GSOutput
{
	float4 position : SV_Position;
	nointerpolation float3 normal : Normal;
	nointerpolation float4 aabb : AABB;
	nointerpolation uint proj : ProjIdx;
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
	float3 normal = cross(input[1].world_position.xyz - input[0].world_position.xyz, input[2].world_position.xyz - input[1].world_position.xyz);

	// pick projection axis
	float3 pn = abs(normal);
	int proj_idx;
	if (pn.x > pn.y && pn.x > pn.z)
		proj_idx = 0;
	else if (pn.y > pn.x && pn.y > pn.z)
		proj_idx = 1;
	else
		proj_idx = 2;

	// projected verts
	float4 v0 = mul(projs[proj_idx], input[0].world_position);
	float4 v1 = mul(projs[proj_idx], input[1].world_position);
	float4 v2 = mul(projs[proj_idx], input[2].world_position);

	// convert Z to NDC (won't render otherwise!)
	v0.z = v0.z * 0.5f + 0.5f;
	v1.z = v1.z * 0.5f + 0.5f;
	v2.z = v2.z * 0.5f + 0.5f;

	// calculate aabb into [0.f, dimensions] range
	float4 aabb = v0.xyxy;
	aabb = float4(min(aabb.xy, v1.xy), max(aabb.zw, v1.xy));
	aabb = float4(min(aabb.xy, v2.xy), max(aabb.zw, v2.xy));
	aabb = aabb * 0.5f + 0.5f;
	aabb.xy -= 0.5f / dimensions.xy;
	aabb.zw += 0.5f / dimensions.xy;
	aabb *= dimensions.xyxy;

	// equation of lines
	float2 e0 = v1.xy - v0.xy;
	float2 e1 = v2.xy - v1.xy;
	float2 e2 = v0.xy - v2.xy;

	// normals of triangle
	float2 n0 = normalize(float2(e0.y, -e0.x));
	float2 n1 = normalize(float2(e1.y, -e1.x));
	float2 n2 = normalize(float2(e2.y, -e2.x));
	n0 *= -sign(dot(n0, v2.xy - v0.xy));
	n1 *= -sign(dot(n1, v0.xy - v1.xy));
	n2 *= -sign(dot(n2, v1.xy - v2.xy));

	// expand triangel by half-diagonal of pixel
	//  = sqrt((1/d)^2 + (1/d)^2)
	//  = sqrt((1/d^2) + (1/d^2))
	//  = sqrt(2/d^2)
	//  = sqrt(2) / d
	float2 hpixel =  (1.4142135637309f / 2.f) / dimensions.xy;

	v0.xy += hpixel * (e2 / dot(e2, n0.xy) + e0 / dot(e0, n2.xy));
	v1.xy += hpixel * (e0 / dot(e0, n1.xy) + e1 / dot(e1, n0.xy));
	v2.xy += hpixel * (e1 / dot(e1, n2.xy) + e2 / dot(e2, n1.xy));

	// calcualte depth gradient
	//float2 gradient = 
	//v0.xy = v0.xy * 2.f - 1.f;
	//v1.xy = v1.xy * 2.f - 1.f;
	//v2.xy = v2.xy * 2.f - 1.f;


	// output triangle
	GSOutput g0;
	g0.position = v0;
	g0.normal = normal;
	g0.aabb = aabb;
	g0.proj = proj_idx;
	output.Append(g0);

	GSOutput g1;
	g1.position = v1;
	g1.normal = normal;
	g1.aabb = aabb;
	g1.proj = proj_idx;
	output.Append(g1);

	GSOutput g2;
	g2.position = v2;
	g2.normal = normal;
	g2.aabb = aabb;
	g2.proj = proj_idx;
	output.Append(g2);
}
