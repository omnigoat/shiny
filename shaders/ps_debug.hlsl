cbuffer buf_model : register(b1)
{
	matrix world;
	float4 color;
};

float4 main() : SV_Target
{
	return float4(1.f, 0.f, 0.f, 0.2f);
}

