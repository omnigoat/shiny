cbuffer {
    float4x4 wvp_matrix;
}

float4 main(float4 position)
{
	return wvp_matrix * position;
}