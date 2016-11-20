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

[maxvertexcount(3)]
void main(triangle VSOutput input[3], inout TriangleStream<GSOutput> output)
{
	float3 normal = cross(input[1].world_position.xyz - input[0].world_position.xyz, input[2].world_position.xyz - input[1].world_position.xyz);

	for (uint i = 0; i < 3; ++i)
	{
		GSOutput element;
		element.position = input[i].position;
		element.world_position = input[i].world_position;
		element.normal = normal;
		output.Append(element);
	}
}