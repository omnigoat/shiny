struct GSOutput
{
	float4 pos : SV_POSITION;
	float4 normal : Normal;
};

[maxvertexcount(3)]
void main(triangle float4 input[3] : SV_POSITION, inout TriangleStream<GSOutput> output)
{
	float3 normal = cross(input[1] - input[0], input[2] - input[0]);

	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.pos = input[i];
		element.normal = float4(normal, 0.f);
		output.Append(element);
	}
}
