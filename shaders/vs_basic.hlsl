cbuffer buf_scene : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float time;
}

float4 main(float4 position : Position) : SV_Position
{
	return float4(position.xy * fmod(time * 0.001f, 1.f), position.zw); //;
}
