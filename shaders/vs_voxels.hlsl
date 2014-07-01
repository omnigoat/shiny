struct vs_input_t
{
	float4 position : Position;
};

struct ps_input_t
{
	float4 position : SV_Position;
	float4 texcoord : Texcoord;
};

//
// Vertex Shader
//
ps_input_t vs_main(in vs_input_t input)
{
	ps_input_t output;
	output.position = input.position;
	output.texcoord = float4(0.5f - input.position.xy, 0.5f, 0.f);
	return output;
}