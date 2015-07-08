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
};


cbuffer buf_voxelize : register(c2)
{
	float2 dimensions;
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
		float4(-1.f, 0.f, 0.f, 0.f),
		float4( 0.f, 0.f, 0.f, 1.f)
	),

	// project down y-axis
	matrix
	(
		float4( 0.f, 1.f,  0.f, 0.f),
		float4( 0.f, 0.f, -1.f, 0.f),
		float4(-1.f, 0.f,  0.f, 0.f),
		float4( 0.f, 0.f,  0.f, 1.f)
	),

	// project down z-axis
	matrix
	(
		float4( 1.f, 0.f, 0.f, 0.f),
		float4( 0.f, 1.f, 0.f, 0.f),
		float4( 0.f, 0.f, 1.f, 0.f),
		float4( 0.f, 0.f, 0.f, 1.f)
	)
};


[maxvertexcount(3)]
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> output)
{
	float2 hpixel = 0.5f / dimensions;


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

	// edges in homogenous clip-space
	float3 e01 = v1.xyw - v0.xyw;
	float3 e12 = v2.xyw - v1.xyw;
	float3 e20 = v0.xyw - v2.xyw;

	// planes in homogenous clip-space
	float3 p0 = cross(e20, v0.xyw);
	float3 p1 = cross(e01, v1.xyw);
	float3 p2 = cross(e12, v2.xyw);

	// dilate planes
	p0.z -= dot(hpixel, abs(p0.xy));
	p1.z -= dot(hpixel, abs(p1.xy));
	p2.z -= dot(hpixel, abs(p2.xy));

	// calculate aabb
	float4 aabb = p0.xyxy;
	aabb = float4(min(aabb.xy, p1.xy), max(aabb.zw, p1.xy));
	aabb = float4(min(aabb.xy, p2.xy), max(aabb.zw, p2.xy));

	// calcualte depth gradient
	//float2 gradient = 

	// output triangle
	GSOutput g0;
	g0.position.xyw = cross(p0, p1);
	g0.position.z = v0.z;
	g0.normal = normal;
	g0.aabb = aabb;
	output.Append(g0);

	GSOutput g1;
	g1.position.xyw = cross(p1, p2);
	g1.position.z = v1.z;
	g1.normal = normal;
	g1.aabb = aabb;
	output.Append(g1);

	GSOutput g2;
	g2.position.xyw = cross(p2, p0);
	g2.position.z = v2.z;
	g2.normal = normal;
	g2.aabb = aabb;
	output.Append(g2);
}
