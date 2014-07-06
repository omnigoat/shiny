struct vs_input_t
{
	float4 position : Position;
};

struct ps_input_t
{
	float4 position : SV_Position;
	float3 texcoord : Texcoord;
};

//
// Vertex Shader
//
ps_input_t main(in vs_input_t input)
{
	ps_input_t output;
	output.position = input.position;
	output.texcoord = float3(input.position.xy, 1.f);
	return output;
}