struct vs_input_t
{
	float4 position : Position;
};

struct ps_input_t
{
	float4 actual_position : SV_Position;
	float3 pixel_delta : PixelDelta;
};

//
// Vertex Shader
//
ps_input_t main(in vs_input_t input)
{
	ps_input_t output = {
		input.position,
		float3(input.position.xy, 0.5f)
	};

	return output;
}