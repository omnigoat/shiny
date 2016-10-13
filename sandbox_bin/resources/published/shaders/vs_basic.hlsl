cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix inverse_view;
	matrix proj;
	matrix inverse_vp;
	float time;
}; 

struct VSOutput
{
	float4 position : SV_Position;
	float4 pos : Position;
};

VSOutput main(float4 position : Position)
{
	VSOutput output;

	matrix wvp = mul(proj, view);

	output.position = mul(wvp, float4(position.xyz / 5.f, 1.f));
	output.pos = position;
	return output;
}