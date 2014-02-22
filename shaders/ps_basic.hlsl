struct ps_input
{
	float4 position : SV_Position;
	float4 color : Color;
};

float4 main(ps_input input) : SV_Target
{
	return input.color;
}
