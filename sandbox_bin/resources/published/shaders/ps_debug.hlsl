cbuffer buf_model : register(b1)
{
	matrix world;
	float4 color;
};

float4 main() : SV_Target
{
	return color;
}

