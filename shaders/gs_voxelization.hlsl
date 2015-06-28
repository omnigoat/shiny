struct VSOutput
{
	float4 position : SV_Position;
	float4 world_position : Position;
};

struct GSOutput
{
	float4 position : SV_Position;
	float4 world_position : Position;
	float3 normal : Normal;
};


#if 0
// for dims {left: -1, right: 1, top: 1.f, bottom: -1f, near: 0.f, far: 1.f}
static matrix ortho = 
{
	float4(1.f, 0.f, 0.f, 0.f),
	float4(0.f, 1.f, 0.f, 0.f),
	float4(0.f, 0.f, 0.5f, 0.f),
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
	float3 normal = cross(input[1].world_position.xyz - input[0].world_position.xyz, input[2].world_position.xyz - input[1].world_position.xyz);

	// 
	float3 pn = abs(normal);
	int proj_idx;
	if (pn.x > pn.y && pn.x > pn.z)
		proj_idx = 0;
	else if (pn.y > pn.x && pn.y > pn.z)
		proj_idx = 1;
	else
		proj_idx = 2;

	// transformed position
	float4 position_clipspace[3];
	for (uint i2 = 0; i2 != 3; ++i2)
		position_clipspace[i2] = mul(projs[proj_idx], input[i2].world_position);


	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.position = input[i].position;
		element.world_position = input[i].world_position;
		element.normal = normal;
		output.Append(element);
	}
}