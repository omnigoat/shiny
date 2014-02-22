cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix proj;
	float time;
}

struct ps_input
{
	float4 position : SV_Position;
	float4 color : Color;
};

ps_input main(float4 position : Position, float4 color : Color)
{
	ps_input output;
	output.position = mul(mul(proj, view), position);
	output.color = color;
	return output;
}