cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix proj;
	float time;
};

cbuffer buf_model : register(b1)
{
	matrix world;
};


struct ps_input
{
	float4 position : SV_Position;
	float4 pos : Position;
	float4 color : Color;
	float3 normal : Normal;
};

ps_input main(float4 position : Position, float4 color : Color)
{
	ps_input output;

	matrix wvp = mul(proj, mul(view, world));

	output.position = mul(wvp, position);
	output.pos = output.position;
	output.color = color;
	output.normal = mul(world, position);
	return output;
}