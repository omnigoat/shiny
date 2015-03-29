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


struct VSOutput
{
	float4 position : SV_Position;
	float4 world_position : Position;
};

VSOutput main(float4 position : Position)
{
	VSOutput output;

	matrix wvp = mul(proj, view);

	output.position = mul(wvp, position);
	output.world_position = position;
	return output;
}