cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix proj;
	float time;
}

float4 main(float4 position : Position) : SV_Position
{
	return mul(mul(proj, view), position);
}