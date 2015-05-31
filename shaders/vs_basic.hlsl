cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix proj;
	float time;
};

struct VSOutput
{
	float4 position : SV_Position;
	float4 color : Color;
};

VSOutput main(float4 position : Position, float4 color : color)
{
	VSOutput output;

	matrix wvp = mul(proj, view);

	output.position = mul(wvp, position);
	output.color = color;
	return output;
}