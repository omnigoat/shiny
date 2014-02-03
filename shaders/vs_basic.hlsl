cbuffer vert_in
{
	float4x4 wvp_matrix;
}

float4 main(float4 position : POSITION) : SV_POSITION
{
	return mul(wvp_matrix, position);
}