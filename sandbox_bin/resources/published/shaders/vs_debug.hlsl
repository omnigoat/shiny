cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix proj;
	float time;
};

cbuffer buf_model : register(b1)
{
	matrix world;
	float4 color;
};


float4 main(float4 position : Position, float4 color : Color) : SV_Position
{
	matrix wvp = mul(proj, mul(view, world));

	return mul(wvp, position);
}